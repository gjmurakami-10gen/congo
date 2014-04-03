/* congo-proxy.c
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


static char      *gBindIp = "0.0.0.0";
static int        gBindPort = 27000;
static char      *gHost = "localhost";
static int        gPort = 27017;
static HashTable *gProxies;


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
Proxy_GetServerConnection (Connection *client) /* IN */
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


static bool
Proxy_HandleMessage (SocketManager *socket_manager,  /* IN */
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

   WireProtocolMessage_Printf (message);
   printf ("\n");

   server = Proxy_GetServerConnection (connection);

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
       * Send the reply to the client.
       */
      ret = Connection_Send (connection, &reply);
      if (!ret) {
         return false;
      }
   } while (is_exhaust && cursor_id);

   return true;
}


static void
Proxy_ConnectionClosed (SocketManager *socket_manager,
                        Connection *connection,
                        void *handler_data)
{
   Connection *server;

   if ((server = HashTable_Lookup (gProxies, connection))) {
      Connection_Destroy (server);
      HashTable_Remove (gProxies, connection);
   }
}


int
main (int argc,     /* IN */
      char *argv[]) /* IN */
{
   SocketManagerHandlers handlers = {
      .Closed = Proxy_ConnectionClosed,
      .HandleMessage = Proxy_HandleMessage,
   };
   SocketManager socket_manager;
   OptionContext context;
   Error error;

   OptionContext_Init (&context, "congo-proxy", "A logging mongod proxy.");
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
