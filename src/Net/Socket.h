/* Socket.h
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


#ifndef SOCKET_H
#define SOCKET_H


#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <Macros.h>
#include <Types.h>


BEGIN_DECLS


typedef struct _Socket Socket;


struct _Socket
{
   int domain;
   int type;
   int protocol;
#if defined(_WIN32)
   SOCKET sd;
#else
   int sd;
#endif
   struct sockaddr addr;
   socklen_t addrlen;
   char name [INET_ADDRSTRLEN];
};


bool    Socket_Init       (Socket *socket,
                           int domain,
                           int type,
                           int protocol);
int     Socket_Bind       (Socket *sd,
                           const struct sockaddr *addr,
                           socklen_t addrlen);
bool    Socket_Accept     (Socket *sd,
                           Socket *client);
int     Socket_Connect    (Socket *sd,
                           struct sockaddr *addr,
                           socklen_t addrlen,
                           uint64_t timeout);
void    Socket_Close      (Socket *sd);
ssize_t Socket_Recv       (Socket *sd,
                           void *buf,
                           size_t len,
                           int flags,
                           uint64_t timeout_msec);
ssize_t Socket_Send       (Socket *sd,
                           const void *buf,
                           size_t len,
                           int flags,
                           uint64_t timeout);
ssize_t Socket_SendMsg    (Socket *sd,
                           const struct msghdr *msg,
                           int flags);
int     Socket_Listen     (Socket *sd,
                           int backlog);
int     Socket_SetSockOpt (Socket *sd,
                           int level,
                           int optname,
                           const void *optval,
                           socklen_t optlen);


END_DECLS


#endif /* _SOCKET_H */
