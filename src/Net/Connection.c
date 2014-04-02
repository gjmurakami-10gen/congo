/* Connection.c
 *
 * Copyright (C) 2014 MongoDB, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <bson.h>

#include <Connection.h>
#include <Debug.h>
#include <Endian.h>
#include <Log.h>
#include <Memory.h>
#include <Random.h>
#include <Resolver.h>
#include <Socket.h>


bool
Connection_InitFromHost (Connection *connection, /* OUT */
                         const char *host,       /* IN */
                         uint16_t port)          /* IN */
{
   struct addrinfo hints = { 0 };
   struct addrinfo *results = NULL, *rp;
   char portstr [16];
   bool success = false;
   int ret;

   ASSERT (connection);
   ASSERT (host);
   ASSERT (port);

   Memory_Zero (connection, sizeof *connection);

   connection->socket = &connection->inline_socket;

   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = 0;
   hints.ai_protocol = 0;

   snprintf (portstr, sizeof portstr, "%hu", port);
   portstr [sizeof portstr - 1] = '\0';

   ret = Resolver_GetAddrInfo (host, portstr, &hints, &results);
   if (ret != 0) {
      return false;
   }

   for (rp = results; rp; rp = rp->ai_next) {
      if (!Socket_Init (connection->socket,
                        rp->ai_family,
                        rp->ai_socktype,
                        rp->ai_protocol)) {
         continue;
      }

      ret = Socket_Connect (connection->socket,
                            rp->ai_addr,
                            (socklen_t)rp->ai_addrlen,
                            0);
      if (ret != 0) {
         continue;
      }

      WireProtocolReader_Init (&connection->reader, connection->socket);
      WireProtocolWriter_Init (&connection->writer, connection->socket);
      success = true;

      break;
   }

   freeaddrinfo (results);

   return success;
}


void
Connection_Init (Connection *connection, /* OUT */
                 Socket *socket)         /* IN */
{
   ASSERT (connection);

   connection->socket = socket;
   WireProtocolReader_Init (&connection->reader, socket);
   WireProtocolWriter_Init (&connection->writer, socket);
   connection->last_request_id = Random_Int32 ();
}


void
Connection_Destroy (Connection *connection) /* IN */
{
   ASSERT (connection);

   Socket_Close (connection->socket);
   connection->socket = NULL;
}


void
Connection_SetTimeout (Connection *connection, /* IN */
                       uint64_t timeout)       /* IN */
{
   ASSERT (connection);

   connection->reader.timeout = timeout;
   connection->writer.timeout = timeout;
}


bool
Connection_Recv (Connection *connection,       /* IN */
                 WireProtocolMessage *message) /* OUT */
{
   bool ret;

   ASSERT (connection);
   ASSERT (message);

   ret = WireProtocolReader_Read (&connection->reader, message);
   connection->bytes_recv += message->header.msg_len;
   connection->msg_recv++;
   return ret;
}


void
Connection_SetRequestId (Connection *connection,       /* IN */
                         WireProtocolMessage *message) /* IN */
{
   ASSERT (connection);
   ASSERT (message);

   message->header.request_id = ++connection->last_request_id;
}


bool
Connection_Send (Connection *connection,       /* IN */
                 WireProtocolMessage *message) /* IN */
{
   bool ret;

   ASSERT (connection);
   ASSERT (message);

   if (!connection->no_header_mutate) {
      Connection_SetRequestId (connection, message);
   }

   ret = WireProtocolWriter_Write (&connection->writer, message);
   connection->bytes_sent += UINT32_FROM_LE (message->header.msg_len);
   connection->msg_sent++;
   return ret;
}


bool
Connection_Command (Connection *connection,       /* IN */
                    const char *collection,       /* IN */
                    const bson_t *command,        /* IN */
                    WireProtocolMessage *reply)   /* OUT */
{
   WireProtocolMessage request;
   int32_t request_id;

   ASSERT (connection);
   ASSERT (collection);
   ASSERT (command);
   ASSERT (reply);

   request.query.msg_len = 0;
   request.query.request_id = 0;
   request.query.response_to = 0;
   request.query.opcode = WIRE_PROTOCOL_QUERY;
   request.query.flags = 0;
   request.query.collection = collection;
   request.query.skip = 0;
   request.query.n_return = 1;
   request.query.query = bson_get_data (command);
   request.query.fields = NULL;

   if (Connection_Send (connection, &request)) {
      if (Connection_Recv (connection, reply)) {
         request_id = UINT32_FROM_LE (request.header.request_id);
         return ((reply->header.opcode == WIRE_PROTOCOL_REPLY) &&
                 (reply->header.response_to == request_id));
      }
   }

   return false;
}


bool
Connection_Ping (Connection *connection,     /* IN */
                 WireProtocolMessage *reply) /* OUT */
{
   bson_t cmd;
   bool ret = false;

   ASSERT (connection);
   ASSERT (reply);

   bson_init (&cmd);
   BSON_APPEND_INT32 (&cmd, "ping", 1);
   ret = Connection_Command (connection, "admin.$cmd", &cmd, reply);
   bson_destroy (&cmd);

   return ret;
}


bool
Connection_IsMaster (Connection *connection,     /* IN */
                     WireProtocolMessage *reply) /* OUT */
{
   bson_t cmd;
   bool ret = false;

   ASSERT (connection);
   ASSERT (reply);

   bson_init (&cmd);
   BSON_APPEND_INT32 (&cmd, "isMaster", 1);
   ret = Connection_Command (connection, "admin.$cmd", &cmd, reply);
   bson_destroy (&cmd);

   return ret;
}

char *
Connection_ServerVersionString (Connection *connection) /* IN */
{
   WireProtocolMessage reply;
   bson_reader_t *reader;
   const bson_t *doc;
   bson_iter_t iter;
   bson_t cmd;
   char *ret = NULL;

   ASSERT (connection);

   bson_init (&cmd);
   BSON_APPEND_INT32 (&cmd, "buildInfo", 1);

   if (Connection_Command (connection, "admin.$cmd", &cmd, &reply)) {
      reader = bson_reader_new_from_data (reply.reply.documents,
                                          reply.reply.documents_len);
      if ((doc = bson_reader_read (reader, NULL))) {
         if (bson_iter_init_find (&iter, doc, "version") &&
             BSON_ITER_HOLDS_UTF8 (&iter)) {
            ret = bson_iter_dup_utf8 (&iter, NULL);
         }
         bson_reader_destroy (reader);
      }
   }

   bson_destroy (&cmd);

   return ret;
}


bool
Connection_ServerVersion (Connection *connection, /* IN */
                          int32_t *major,         /* OUT */
                          int32_t *minor,         /* OUT */
                          int32_t *micro,         /* OUT */
                          int32_t *release)       /* OUT */
{
   WireProtocolMessage reply;
   bson_reader_t *reader;
   const bson_t *doc;
   bson_iter_t ar;
   bson_iter_t iter;
   bson_t cmd;
   bool ret = false;

   ASSERT (connection);
   ASSERT (major);
   ASSERT (minor);
   ASSERT (micro);
   ASSERT (release);

   *major = 0;
   *minor = 0;
   *micro = 0;
   *release = 0;

   bson_init (&cmd);
   BSON_APPEND_INT32 (&cmd, "buildInfo", 1);

   if (Connection_Command (connection, "admin.$cmd", &cmd, &reply)) {
      reader = bson_reader_new_from_data (reply.reply.documents,
                                          reply.reply.documents_len);
      if ((doc = bson_reader_read (reader, NULL))) {
         if (bson_iter_init_find (&iter, doc, "versionArray") &&
             bson_iter_recurse (&iter, &ar)) {
            if (bson_iter_next (&ar) && BSON_ITER_HOLDS_INT32 (&ar)) {
               *major = bson_iter_int32 (&ar);
            }
            if (bson_iter_next (&ar) && BSON_ITER_HOLDS_INT32 (&ar)) {
               *minor = bson_iter_int32 (&ar);
            }
            if (bson_iter_next (&ar) && BSON_ITER_HOLDS_INT32 (&ar)) {
               *micro = bson_iter_int32 (&ar);
            }
            if (bson_iter_next (&ar) && BSON_ITER_HOLDS_INT32 (&ar)) {
               *release = bson_iter_int32 (&ar);
            }
            ret = true;
         }
         bson_reader_destroy (reader);
      }
   }

   bson_destroy (&cmd);

   return ret;
}


void
Connection_Query (Connection *connection,   /* IN */
                  const char *collection,   /* IN */
                  const bson_t *query,      /* IN */
                  const bson_t *fields,     /* IN */
                  ConnectionCursor *cursor) /* OUT */
{
   ASSERT (connection);
   ASSERT (query);
   ASSERT (cursor);

   Memory_Zero (cursor, sizeof *cursor);

   cursor->connection = connection;
   cursor->collection = collection;

   cursor->request.query.msg_len = 0;
   cursor->request.query.request_id = 0;
   cursor->request.query.response_to = 0;
   cursor->request.query.opcode = WIRE_PROTOCOL_QUERY;
   cursor->request.query.flags = 0;
   cursor->request.query.collection = collection;
   cursor->request.query.skip = 0;
   cursor->request.query.n_return = 100;
   cursor->request.query.query = bson_get_data (query);
   cursor->request.query.fields = fields ? bson_get_data (fields) : NULL;
}


bool
Connection_Update (Connection *connection,        /* IN */
                   const char *collection,        /* IN */
                   WireProtocolUpdateFlags flags, /* IN */
                   const bson_t *selector,        /* IN */
                   const bson_t *update)          /* IN */
{
   WireProtocolMessage request;

   ASSERT (connection);
   ASSERT (collection);
   ASSERT (selector);
   ASSERT (update);

   request.update.msg_len = 0;
   request.update.request_id = 0;
   request.update.response_to = 0;
   request.update.opcode = WIRE_PROTOCOL_UPDATE;
   request.update.zero = 0;
   request.update.collection = collection;
   request.update.flags = flags;
   request.update.selector = bson_get_data (selector);
   request.update.update = bson_get_data (update);

   return Connection_Send (connection, &request);
}


bool
Connection_Delete (Connection *connection,        /* IN */
                   const char *collection,        /* IN */
                   WireProtocolDeleteFlags flags, /* IN */
                   const bson_t *selector)        /* IN */
{
   WireProtocolMessage request;

   ASSERT (connection);
   ASSERT (collection);
   ASSERT (selector);

   request.delete.msg_len = 0;
   request.delete.request_id = 0;
   request.delete.response_to = 0;
   request.delete.opcode = WIRE_PROTOCOL_DELETE;
   request.delete.zero = 0;
   request.delete.collection = collection;
   request.delete.flags = flags;
   request.delete.selector = bson_get_data (selector);

   return Connection_Send (connection, &request);
}


bool
Connection_Insert (Connection *connection,
                   const char *collection,
                   WireProtocolInsertFlags flags,
                   const bson_t *document)
{
   WireProtocolMessage request;
   struct iovec iov;

   ASSERT (connection);
   ASSERT (collection);
   ASSERT (document);

   iov.iov_base = (void *)bson_get_data (document);
   iov.iov_len = document->len;

   request.insert.msg_len = 0;
   request.insert.request_id = 0;
   request.insert.response_to = 0;
   request.insert.opcode = WIRE_PROTOCOL_INSERT;
   request.insert.flags = flags;
   request.insert.collection = collection;
   request.insert.documents = &iov;
   request.insert.n_documents = 1;

   return Connection_Send (connection, &request);
}


bool
Connection_GetLastError (Connection *connection,
                         const char *collection,
                         const bson_t *gle,
                         Error *error)
{
   WireProtocolMessage request;
   WireProtocolMessage reply;
   bson_iter_t iter;
   bson_t cmd;
   bson_t doc;
   bool ret;

   ASSERT (connection);
   ASSERT (collection);
   ASSERT (gle);

   bson_init (&cmd);
   BSON_APPEND_INT32 (&cmd, "getLastError", 1);

   if (bson_iter_init (&iter, gle)) {
      while (bson_iter_next (&iter)) {
         bson_append_iter (&cmd, bson_iter_key (&iter), -1, &iter);
      }
   }

   request.query.msg_len = 0;
   request.query.request_id = 0;
   request.query.response_to = 0;
   request.query.opcode = WIRE_PROTOCOL_QUERY;
   request.query.flags = 0;
   request.query.collection = collection;
   request.query.skip = 0;
   request.query.n_return = 1;
   request.query.query = bson_get_data (&cmd);
   request.query.fields = NULL;

   ret = Connection_Send (connection, &request);

   bson_destroy (&cmd);

   if (!ret) {
      Error_Init (error,
                  CONNECTION_ERROR,
                  CONNECTION_ERROR_SEND_FAILURE,
                  "Failed to send request.");
      return false;
   }

   ret = Connection_Recv (connection, &reply);

   if (!ret) {
      Error_Init (error,
                  CONNECTION_ERROR,
                  CONNECTION_ERROR_RECV_FAILURE,
                  "Failed to recv request.");
      return false;
   }

   if ((reply.reply.flags & WIRE_PROTOCOL_REPLY_QUERY_FAILURE)) {
      Error_Init (error,
                  CONNECTION_ERROR,
                  CONNECTION_ERROR_QUERY_FAILURE,
                  "The query failed.");
      return false;
   }

   if (!bson_init_static (&doc,
                          reply.reply.documents,
                          reply.reply.documents_len)) {
      Error_Init (error,
                  CONNECTION_ERROR,
                  CONNECTION_ERROR_RECV_FAILURE,
                  "Invalid reply message.");
      return false;
   }

   ret = (bson_iter_init_find (&iter, &doc, "ok") &&
          bson_iter_as_bool (&iter));

   if (!ret) {
      Error_Init (error,
                  CONNECTION_ERROR,
                  CONNECTION_ERROR_QUERY_FAILURE,
                  "The getlasterror returned failure.");
   }

   bson_destroy (&doc);

   return ret;
}


void
ConnectionCursor_Destroy (ConnectionCursor *cursor) /* IN */
{
   ASSERT (cursor);

   if (cursor->reader) {
      bson_reader_destroy (cursor->reader);
      cursor->reader = NULL;
   }
}


bool
ConnectionCursor_HasError (ConnectionCursor *cursor) /* IN */
{
   return cursor->has_error;
}


bool
ConnectionCursor_MoveNext (ConnectionCursor *cursor, /* IN */
                           const bson_t **doc)       /* OUT */
{
   ASSERT (cursor);
   ASSERT (doc);

   *doc = NULL;

again:
   if (cursor->is_done) {
      return false;
   }

   if (!cursor->has_sent) {
      if (!Connection_Send (cursor->connection, &cursor->request)) {
         cursor->has_error = true;
         return false;
      }
      if (!Connection_Recv (cursor->connection, &cursor->reply)) {
         cursor->has_error = true;
         return false;
      }
      if (cursor->reply.header.opcode != WIRE_PROTOCOL_REPLY) {
         cursor->has_error = true;
         return false;
      }
      cursor->has_sent = true;
      cursor->is_eof = false;
   }

   cursor->cursor_id = cursor->reply.reply.cursor_id;

   if (!cursor->reader) {
      cursor->reader =
         bson_reader_new_from_data (cursor->reply.reply.documents,
                                    cursor->reply.reply.documents_len);
      ASSERT (cursor->reader);
      cursor->is_eof = false;
   }

   if ((*doc = bson_reader_read (cursor->reader, &cursor->is_eof))) {
      return true;
   }

   if (cursor->is_eof) {
      bson_reader_destroy (cursor->reader);
      cursor->reader = NULL;
      if (!cursor->reply.reply.documents_len) {
         cursor->is_done = true;
         return false;
      }
   }

   cursor->request.getmore.msg_len = 0;
   cursor->request.getmore.request_id = 0;
   cursor->request.getmore.response_to = 0;
   cursor->request.getmore.opcode = WIRE_PROTOCOL_GETMORE;
   cursor->request.getmore.zero = 0;
   cursor->request.getmore.collection = cursor->collection;
   cursor->request.getmore.n_return = 100;
   cursor->request.getmore.cursor_id = cursor->cursor_id;

   cursor->is_eof = false;
   cursor->has_sent = false;

   goto again;
}
