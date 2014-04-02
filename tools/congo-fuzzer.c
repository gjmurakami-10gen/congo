/* congo-fuzzer.c
 *
 * Copyright (C) 2014 Christian Hergert <christian.hergert@mongodb.com>
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
#include <stdlib.h>

#include <Endian.h>
#include <HashTable.h>
#include <Log.h>
#include <OptionContext.h>
#include <OptionEntry.h>
#include <Random.h>
#include <Sched.h>
#include <Signals.h>
#include <Socket.h>
#include <SocketManager.h>
#include <Task.h>
#include <WireProtocol.h>
#include <WireProtocolReader.h>
#include <WireProtocolWriter.h>


typedef void (*Fuzzer) (WireProtocolMessage *message);


static char      *gBindIp = "0.0.0.0";
static int        gBindPort = 27000;
static char      *gHost = "localhost";
static int        gPort = 27017;
static int        gNextFuzzer;
static HashTable *gProxies;


/*
 * TODO: We leak proxies.
 */


static OptionEntry entries[] = {
   { "bind_ip", 0, 0, OPTION_ARG_STRING, &gBindIp,
     "The ip address to bind to [0.0.0.0]" },
   { "bind_port", 0, 0, OPTION_ARG_INT, &gBindPort,
     "The port to bind to [27000]" },
   { "host", 0, 0, OPTION_ARG_STRING, &gHost,
     "The hostname to forward traffic to in client mode [localhost]" },
   { "port", 0, 0, OPTION_ARG_INT, &gPort,
     "The port to connect in client mode [27017]" },
};


static Connection *
Fuzzer_GetServerConnection (Connection *client) /* IN */
{
   Connection *server;

   if (!(server = HashTable_Lookup (gProxies, client))) {
      server = Memory_SafeMalloc0 (sizeof *server);
      if (!Connection_InitFromHost (server, gHost, gPort)) {
         Memory_Free (server);
         return NULL;
      }
      server->no_header_mutate = true;
      HashTable_Insert (gProxies, client, server);
   }

   return server;
}


static void
Fuzzer_FuzzReplyFlags (WireProtocolMessage *message) /* IN */
{
   WireProtocolReply *reply = &message->reply;

   reply->flags |= Random_Int32 ();
}


static void
Fuzzer_FuzzReplyBsonLength (WireProtocolMessage *message) /* IN */
{
   WireProtocolReply *reply = &message->reply;
   const int32_t len = UINT32_TO_LE (-1234);
   bson_reader_t *reader;
   const bson_t *doc;
   uint8_t *data;

   reader = bson_reader_new_from_data (reply->documents, reply->documents_len);

   while ((doc = bson_reader_read (reader, NULL))) {
      data = (uint8_t *)bson_get_data (doc);
      memcpy (data, &len, 4);
   }

   bson_reader_destroy (reader);
}



static Fuzzer gFuzzers [] = {
   Fuzzer_FuzzReplyBsonLength,
   Fuzzer_FuzzReplyFlags,
};


static Fuzzer
Fuzzer_GetNext (void)
{
   int fuzzer;
   fuzzer = (gNextFuzzer++) % N_ELEMENTS (gFuzzers);
   return gFuzzers [fuzzer];
}


static bool
Fuzzer_ShouldFuzz (Connection *connection,         /* IN */
                   WireProtocolMessage *message)   /* IN */
{
   const char *key;
   bson_iter_t iter;
   uint32_t len;
   bson_t b;

#define IS_KEY(k) (0 == strcasecmp ((k), key))

   if (message->header.opcode == WIRE_PROTOCOL_QUERY) {
      memcpy (&len, message->query.query, 4);
      len = UINT32_FROM_LE (len);
      if (bson_init_static (&b, message->query.query, len) &&
          bson_iter_init (&iter, &b) &&
          bson_iter_next (&iter)) {
         key = bson_iter_key (&iter);
         if (IS_KEY ("whatsmyuri") ||
             IS_KEY ("ismaster") ||
             IS_KEY ("getlasterror") ||
             IS_KEY ("getLog") ||
             IS_KEY ("replSetGetStatus")) {
            return false;
         }
      }

      return true;
   }

#undef IS_KEY

   return false;
}


static bool
Fuzzer_HandleMessage (SocketManager *socket_manager,  /* IN */
                      Connection *connection,         /* IN */
                      WireProtocolMessage *message,   /* IN */
                      void *handler_data)             /* IN */
{
   WireProtocolMessage reply;
   Connection *server;
   int64_t cursor_id;
   bool ret;
   bool is_exhaust = false;
   bool is_query;
   bool should_fuzz;

   WireProtocolMessage_Printf (message);
   printf ("\n");

   server = Fuzzer_GetServerConnection (connection);

   /*
    * Ensure the connection will not mutate request_id.
    */
   if (!connection->no_header_mutate) {
      connection->no_header_mutate = true;
   }

   /*
    * Check some things ahead of time before we mutate them.
    */
   is_query = (message->header.opcode == WIRE_PROTOCOL_QUERY);
   is_exhaust = ((message->header.opcode == WIRE_PROTOCOL_QUERY) &&
                 (message->query.flags & WIRE_PROTOCOL_QUERY_EXHAUST));
   should_fuzz = Fuzzer_ShouldFuzz (connection, message);

   /*
    * Ensure we send the iovec straight across for OP_INSERT.
    */
   if (message->header.opcode == WIRE_PROTOCOL_INSERT) {
      message->insert.documents = &message->insert.documents_recv;
   }

   /*
    * Send the request to the server with no mutations.
    */
   ret = Connection_Send (server, message);
   if (!ret) {
      return false;
   }

   /*
    * If this wasn't a query, nothing more to do.
    */
   if (!is_query) {
      return true;
   }

   /*
    * Loop through handling replies as long as we need to. We need to
    * continue for a while potentially in the case of exhaust.
    */
   do {
      /*
       * Try to receive the reply from the server.
       */
      ret = Connection_Recv (server, &reply);
      if (!ret || (reply.header.opcode != WIRE_PROTOCOL_REPLY)) {
         return false;
      }

      WireProtocolMessage_Printf (&reply);
      printf ("\n");

      cursor_id = reply.reply.cursor_id;

      /*
       * Fuzz the message if necessary.
       */
      connection->writer.fuzzer = should_fuzz ? Fuzzer_GetNext () : NULL;

      /*
       * Send the reply to the client.
       */
      ret = Connection_Send (connection, &reply);
      if (!ret) {
         return false;
      }
   } while (is_exhaust && cursor_id);

   return true;
}


int
main (int argc,     /* IN */
      char *argv[]) /* IN */
{
   SocketManagerHandlers handlers = {
      Fuzzer_HandleMessage,
   };
   SocketManager socket_manager;
   OptionContext context;
   Error error;

   OptionContext_Init (&context,
                       "congo-fuzzer",
                       "A transparent network fuzzer.");
   OptionContext_AddEntries (&context, entries, N_ELEMENTS (entries));
   if (!OptionContext_Parse (&context, argc, argv, &error)) {
      fprintf (stderr, "%s\n", error.message);
   }

#ifndef _WIN32
   Signals_Init ();
#endif
   Random_Init ();

   gProxies = HashTable_Create (1024, Pointer_Hash, Pointer_Equal, NULL, NULL);

   SocketManager_Init (&socket_manager);
   SocketManager_SetHandlers (&socket_manager, &handlers, NULL);
   SocketManager_AddListener (&socket_manager, gBindIp, gBindPort);
   SocketManager_Start (&socket_manager);

   Sched_Run ();

   SocketManager_Stop (&socket_manager);
   SocketManager_Destroy (&socket_manager);

   return EXIT_SUCCESS;
}
