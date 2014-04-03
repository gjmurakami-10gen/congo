/* SocketManager.h
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


#ifndef SOCKET_MANAGER_H
#define SOCKET_MANAGER_H


#include <sys/uio.h>

#include <List.h>
#include <Connection.h>
#include <WireProtocol.h>


BEGIN_DECLS


typedef struct _SocketManager         SocketManager;
typedef struct _SocketManagerHandlers SocketManagerHandlers;


struct _SocketManagerHandlers
{
   bool (*Accept)        (SocketManager *manager,
                          Connection *connection,
                          void *handler_data);

   bool (*HandleMessage) (SocketManager *manager,
                          Connection *connection,
                          WireProtocolMessage *message,
                          void *handler_data);

   /*
    * Default handler will dispatch to the following typed
    * message handlers.
    */

   bool (*HandleQuery)   (SocketManager *manager,
                          Connection *connection,
                          WireProtocolQuery *query,
                          void *handler_data);
};


struct _SocketManager
{
   SocketManagerHandlers  handlers;
   void                  *handlers_data;
   List                  *listeners;
   bool                   running;
};


void SocketManager_Init        (SocketManager *socket_manager);
void SocketManager_AddListener (SocketManager *socket_manager,
                                const char *bind_ip,
                                uint16_t port);
void SocketManager_SetHandlers (SocketManager *socket_manager,
                                const SocketManagerHandlers *handlers,
                                void *handlers_data);
void SocketManager_Start       (SocketManager *socket_manager);
void SocketManager_Stop        (SocketManager *socket_manager);
void SocketManager_Destroy     (SocketManager *socket_manager);


END_DECLS


#endif /* SOCKET_MANAGER_H */
