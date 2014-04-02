/* Connection.h
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


#ifndef CONNECTION_H
#define CONNECTION_H


#include <bson.h>

#include <Error.h>
#include <Macros.h>
#include <Socket.h>
#include <Types.h>
#include <WireProtocol.h>
#include <WireProtocolReader.h>
#include <WireProtocolWriter.h>


BEGIN_DECLS


typedef struct
{
   Socket             *socket;
   WireProtocolReader  reader;
   WireProtocolWriter  writer;
   int32_t             last_request_id;
   bool                no_header_mutate;
   Socket              inline_socket;
   uint64_t            timeout;
   int64_t             bytes_sent;
   int64_t             bytes_recv;
   int64_t             msg_sent;
   int64_t             msg_recv;
} Connection;


typedef struct
{
   Connection          *connection;
   const char          *collection;
   WireProtocolMessage  request;
   WireProtocolMessage  reply;
   bool                 has_error;
   bool                 has_sent;
   bool                 is_eof;
   bool                 is_done;
   bson_reader_t       *reader;
   uint64_t             cursor_id;
} ConnectionCursor;


#define CONNECTION_ERROR               4000
#define CONNECTION_ERROR_SEND_FAILURE  1
#define CONNECTION_ERROR_RECV_FAILURE  2
#define CONNECTION_ERROR_QUERY_FAILURE 3


void  Connection_Init                (Connection *connection,
                                      Socket *socket);
bool  Connection_InitFromHost        (Connection *connection,
                                      const char *host,
                                      uint16_t port);
void  Connection_Destroy             (Connection *connection);
void  Connection_SetTimeout          (Connection *connection,
                                      uint64_t timeout);
bool  Connection_Recv                (Connection *connection,
                                      WireProtocolMessage *message);
bool  Connection_Send                (Connection *connection,
                                      WireProtocolMessage *message);
void  Connection_Query               (Connection *connection,
                                      const char *collection,
                                      const bson_t *query,
                                      const bson_t *fields,
                                      ConnectionCursor *cursor);
bool  Connection_Insert              (Connection *connection,
                                      const char *collection,
                                      WireProtocolInsertFlags flags,
                                      const bson_t *document);
bool  Connection_Update              (Connection *connection,
                                      const char *collection,
                                      WireProtocolUpdateFlags flags,
                                      const bson_t *selector,
                                      const bson_t *update);
bool  Connection_Delete              (Connection *connection,
                                      const char *collection,
                                      WireProtocolDeleteFlags flags,
                                      const bson_t *selector);
bool  Connection_Ping                (Connection *connection,
                                      WireProtocolMessage *reply);
bool  Connection_IsMaster            (Connection *connection,
                                      WireProtocolMessage *reply);
bool  Connection_ServerVersion       (Connection *connection,
                                      int32_t *major,
                                      int32_t *minor,
                                      int32_t *micro,
                                      int32_t *release);
char *Connection_ServerVersionString (Connection *connection);
bool  Connection_GetLastError        (Connection *connection,
                                      const char *collection,
                                      const bson_t *gle,
                                      Error *error);


bool  ConnectionCursor_MoveNext      (ConnectionCursor *cursor,
                                      const bson_t **doc);
bool  ConnectionCursor_HasError      (ConnectionCursor *cursor);
void  ConnectionCursor_Destroy       (ConnectionCursor *cursor);


END_DECLS


#endif /* CONNECTION_H */
