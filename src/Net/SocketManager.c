/* SocketManager.c
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
#include <errno.h>

#include <Counter.h>
#include <Log.h>
#include <Memory.h>
#include <QueryCommand.h>
#include <Socket.h>
#include <SocketManager.h>
#include <Task.h>
#include <WireProtocol.h>
#include <WireProtocolReader.h>


#undef LOG_DOMAIN
#define LOG_DOMAIN "Sockets"

#define DEFAULT_BACKLOG 128


typedef struct
{
   SocketManager *socket_manager;
   Socket socket;
   Task task;
} ListenTask;


typedef struct
{
   SocketManager *socket_manager;
   Socket socket;
   Task task;
} RecvTask;


static bool
SocketManager_HandleMessage (SocketManager *socket_manager,
                             Connection *connection,
                             WireProtocolMessage *message,
                             void *handlers_data)
{
   bool ret = false;

   ASSERT (socket_manager);
   ASSERT (connection);
   ASSERT (message);

   switch (message->header.opcode) {
   case WIRE_PROTOCOL_QUERY:
      if (socket_manager->handlers.HandleQuery) {
         ret = socket_manager->handlers.HandleQuery (
            socket_manager, connection, &message->query, handlers_data);
      }
   default:
      break;
   }

   return ret;
}


void
SocketManager_Init (SocketManager *socket_manager)
{
   ASSERT (socket_manager);

   Memory_Zero (socket_manager, sizeof *socket_manager);

   socket_manager->handlers.HandleMessage = SocketManager_HandleMessage;
}


static bool
SocketManager_Accept (SocketManager *manager,
                      Connection *connection,
                      void *handlers_data)
{
   return true;
}


void
SocketManager_SetHandlers (SocketManager *socket_manager,
                           const SocketManagerHandlers *handlers,
                           void *handlers_data)
{
   ASSERT (handlers);

   memcpy (&socket_manager->handlers, handlers, sizeof *handlers);
   socket_manager->handlers_data = handlers_data;

   if (!socket_manager->handlers.Accept) {
      socket_manager->handlers.Accept = SocketManager_Accept;
   }

   if (!socket_manager->handlers.HandleMessage) {
      socket_manager->handlers.HandleMessage = SocketManager_HandleMessage;
   }
}


static void
SocketManager_RecvLoop (void *data) /* IN */
{
   WireProtocolMessage msg;
   Connection connection;
   RecvTask *task = data;
   Socket *client = data;
   bool ret = true;

   ASSERT (client);

   Connection_Init (&connection, &task->socket);

   if (!task->socket_manager->handlers.Accept
         (task->socket_manager,
          &connection,
          task->socket_manager->handlers_data)) {
      goto fail;
   }

   while (ret && Connection_Recv (&connection, &msg)) {
      ret = task->socket_manager->handlers.HandleMessage (
         task->socket_manager, &connection, &msg,
         task->socket_manager->handlers_data);
   }

fail:
   LOG_MESSAGE ("[%s]: Closing connection.", task->socket.name);

   Connection_Destroy (&connection);
   Socket_Close (&task->socket);
   Memory_Free (task);
}


static void
SocketManager_AcceptLoop (void *data) /* IN */
{
   ListenTask *task = data;
   RecvTask *recv_task;
   Socket csd;

   if (0 != Socket_Listen (&task->socket, DEFAULT_BACKLOG)) {
      goto fail;
   }

   for (;;) {
      if (!Socket_Accept (&task->socket, &csd)) {
         LOG_WARNING ("Failed to accept connection: %s: sd=%d",
                      strerror (errno), task->socket.sd);
         continue;
      }

      LOG_MESSAGE ("[%s]: Connection established.", csd.name);

      recv_task = Memory_SafeMalloc0 (sizeof *recv_task);
      recv_task->socket_manager = task->socket_manager;
      memcpy (&recv_task->socket, &csd, sizeof csd);

      Task_Create (&recv_task->task, SocketManager_RecvLoop, recv_task);
   }

fail:
   Socket_Close (&task->socket);
   Memory_Free (task);
}


static void
SocketManager_StartListenTask (SocketManager *socket_manager, /* IN */
                               ListenTask *task)              /* IN */
{
   ASSERT (socket_manager);
   ASSERT (task);

   task->socket_manager = socket_manager;
   Task_Create (&task->task, SocketManager_AcceptLoop, task);
}


void
SocketManager_Start (SocketManager *socket_manager)
{
   List *iter;

   ASSERT (socket_manager);
   ASSERT (!socket_manager->running);

   socket_manager->running = true;

   for (iter = socket_manager->listeners; iter; iter = iter->next) {
      SocketManager_StartListenTask (socket_manager, iter->data);
   }
}


void
SocketManager_Stop (SocketManager *socket_manager)
{
   ASSERT (socket_manager);
   ASSERT (socket_manager->running);

#if 0
   List *iter;
   for (iter = socket_manager->listeners; iter; iter = iter->next) {
      Socket_Close ((Socket *)iter->data);
   }
#endif

   socket_manager->running = false;
}


void
SocketManager_Destroy (SocketManager *socket_manager)
{
   ASSERT (socket_manager);

   if (socket_manager->running) {
      SocketManager_Stop (socket_manager);
   }

#if 0
   while (socket_manager->listeners) {
      Memory_Free (socket_manager->listeners->data);
      socket_manager->listeners = List_Remove (socket_manager->listeners,
                                               socket_manager->listeners);
   }
#endif
}


void
SocketManager_AddListener (SocketManager *socket_manager, /* IN */
                           const char *bind_ip,           /* IN */
                           uint16_t port)                 /* IN */
{
   struct sockaddr_in6 addr6;
   struct sockaddr_in addr4;
   struct in6_addr ip6addr;
   struct in_addr ip4addr;
   ListenTask *task;
   int ret = false;
   int family;
   int opt = 1;

   ASSERT (socket_manager);
   ASSERT (!socket_manager->running);
   ASSERT (bind_ip);
   ASSERT (port);

   task = Memory_SafeMalloc0 (sizeof *task);
   task->socket_manager = socket_manager;

   if (*bind_ip == '[') {
      family = AF_INET6;
      if (1 != inet_pton (AF_INET6, bind_ip, &ip6addr)) {
         LOG_WARNING ("Failed to parse IPv6 address \"%s\": %s",
                      bind_ip, strerror (errno));
         return;
      }
   } else if (strstr (bind_ip, "/")) {
      family = AF_UNIX;
   } else {
      family = AF_INET;
      if (1 != inet_pton (AF_INET, bind_ip, &ip4addr)) {
         LOG_WARNING ("Failed to parse IPv4 address \"%s\": %s",
                      bind_ip, strerror (errno));
         return;
      }
   }

   if (!Socket_Init (&task->socket, family, SOCK_STREAM, 0)) {
      LOG_WARNING ("Failed to initialize socket: %s",
                   strerror (errno));
      Memory_Free (task);
      return;
   }

   switch (family) {
   case AF_INET:
      addr4.sin_family = AF_INET;
      addr4.sin_port = htons (port);
      addr4.sin_addr = ip4addr;
      ret = Socket_Bind (&task->socket,
                         (const struct sockaddr *)&addr4,
                         sizeof addr4);
      break;
   case AF_INET6:
      LOG_WARNING ("IPv6 not yet supported.");
      addr6.sin6_family = AF_INET6;
      addr6.sin6_port = htons (port);
      addr6.sin6_addr = ip6addr;
      ret = Socket_Bind (&task->socket,
                         (const struct sockaddr *)&addr6,
                         sizeof addr6);
      break;
   case AF_UNIX:
      LOG_WARNING ("UNIX Sockets not yet supported.");
      break;
   default:
      ASSERT (false);
   }

   if (ret != 0) {
      LOG_WARNING ("Failed to bind() socket: %s", strerror (errno));
      Socket_Close (&task->socket);
      Memory_Free (task);
      return;
   }

   ret = Socket_SetSockOpt (&task->socket, SOL_SOCKET, SO_REUSEADDR, &opt,
                            sizeof opt);
   if (ret == -1) {
      LOG_WARNING ("Failed to set SO_REUSEADDR.");
   }

   socket_manager->listeners = List_Append (socket_manager->listeners, task);

   if (socket_manager->running) {
      SocketManager_StartListenTask (socket_manager, task);
   }

   return;
}
