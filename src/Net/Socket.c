/* Socket.c
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


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <Debug.h>
#include <Memory.h>
#include <Socket.h>
#include <Task.h>
#include <TimeSpec.h>


/*
 *--------------------------------------------------------------------------
 *
 * Socket_Init --
 *
 *       Opens a new socket using the domain, type and protocol supplied.
 *       @sd will be initialized.
 *
 * Returns:
 *       false on failure, otherwise true.
 *
 * Side effects:
 *       @sd is initialized if true is returned.
 *
 *--------------------------------------------------------------------------
 */

bool
Socket_Init (Socket *sd,   /* IN */
             int domain,   /* IN */
             int type,     /* IN */
             int protocol) /* IN */
{
   ASSERT (sd);

   Memory_Zero (sd, sizeof *sd);

   sd->domain = domain;
   sd->type = type;
   sd->protocol = protocol;
   sd->sd = Task_Socket (domain, type, protocol);

#if defined(_WIN32)
   return (sd->sd != INVALID_SOCKET);
#else
   return (sd->sd != -1);
#endif
}


/*
 *--------------------------------------------------------------------------
 *
 * Socket_Close --
 *
 *       Closes the socket descriptor @sd. The socket should no longer
 *       be used after calling this.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
Socket_Close (Socket *sd) /* IN */
{
   ASSERT (sd);

#if defined(_WIN32)
   if (sd->sd != INVALID_SOCKET) {
      closesocket (sd->sd);
      sd->sd = INVALID_SOCKET;
   }
#else
   if (sd->sd != -1) {
      Task_Close (sd->sd);
      sd->sd = -1;
   }
#endif
}


/*
 *--------------------------------------------------------------------------
 *
 * Socket_Connect --
 *
 *       Connects the socket @sd to the host specified by @addr.
 *
 *       This is performed in a non-blocking fashion but will suspend
 *       the current coroutine if the request cannot be servied
 *       immediately.
 *
 * Returns:
 *       -1 on failure, 0 on success.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

int
Socket_Connect (Socket *sd,            /* IN */
                struct sockaddr *addr, /* IN */
                socklen_t addrlen,     /* IN */
                uint64_t timeout)      /* IN */
{
   ASSERT (sd);
   ASSERT (addr);
   ASSERT (addrlen);

   return Task_Connect (sd->sd, addr, addrlen, timeout);
}


/*
 *--------------------------------------------------------------------------
 *
 * Socket_Bind --
 *
 *       bind()'s the socket @sd to the address specified.
 *       This should not require suspending the coroutine.
 *
 * Returns:
 *       0 on success, otherwise -1 and errno is set.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

int
Socket_Bind (Socket *sd,                  /* IN */
             const struct sockaddr *addr, /* IN */
             socklen_t addrlen)           /* IN */
{
   ASSERT (sd);
   ASSERT (addr);
   ASSERT (addrlen);

   return bind (sd->sd, addr, addrlen);
}


/*
 *--------------------------------------------------------------------------
 *
 * Socket_Accept --
 *
 *       Accept a new client socket using @sd.
 *
 *       This calls accept() in a non-blocking fashion. However, the
 *       coroutine will be suspended until the operation may complete.
 *
 * Returns:
 *       true on success and client is initialized.
 *       false on failure.
 *
 * Side effects:
 *       @client is initialized if successful.
 *
 *--------------------------------------------------------------------------
 */

bool
Socket_Accept (Socket *sd,     /* IN */
               Socket *client) /* IN */
{
   char name [INET_ADDRSTRLEN + 32];

   ASSERT (sd);
   ASSERT (client);

   Memory_Zero (client, sizeof *client);

   client->domain = sd->domain;
   client->type = sd->type;
   client->protocol = sd->protocol;
   client->addrlen = sizeof client->addr;
   client->sd = Task_Accept (sd->sd, &client->addr, &client->addrlen);

   inet_ntop (AF_INET, &((struct sockaddr_in *)&client->addr)->sin_addr,
              name, sizeof name);

   snprintf (client->name, sizeof client->name, "%s:%hu", name,
             ((struct sockaddr_in *)&client->addr)->sin_port);
   client->name [sizeof client->name - 1] = '\0';

#if defined(_WIN32)
   return (client->sd != INVALID_SOCKET);
#else
   return (client->sd != -1);
#endif
}


/*
 *--------------------------------------------------------------------------
 *
 * Socket_Recv --
 *
 *       Receive @len bytes into @buf using the socket @sd. If @timeout
 *       milliseconds elapse before any bytes are received, then -1 is
 *       returned.
 *
 *       It is not an error for this function to return a number less
 *       than @len bytes.
 *
 *       This function calls recv() in a non-blocking fashion, but the
 *       current coroutine will be suspended if there is nothing yet
 *       to recv().
 *
 * Returns:
 *       -1 on failure, otherwise a non-negative integer.
 *
 * Side effects:
 *       @buf may be modified.
 *
 *--------------------------------------------------------------------------
 */

ssize_t
Socket_Recv (Socket *sd,       /* IN */
             void *buf,        /* IN */
             size_t len,       /* IN */
             int flags,        /* IN */
             uint64_t timeout) /* IN */
{
   ASSERT (sd);
   ASSERT (buf);
   ASSERT (len);

   return Task_Recv (sd->sd, buf, len, flags, timeout);
}


/*
 *--------------------------------------------------------------------------
 *
 * Socket_Send --
 *
 *       Send @len bytes of @buf over the socket @sd.
 *
 *       This function calls send() in a non-blocking fashion and will
 *       suspend the coroutine until the operation has completed or
 *       a failure occured.
 *
 * Returns:
 *       -1 on failure, otherwise a non-negative number of the number of
 *       bytes that were delivered.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

ssize_t
Socket_Send (Socket *sd,       /* IN */
             const void *buf,  /* IN */
             size_t len,       /* IN */
             int flags,        /* IN */
             uint64_t timeout) /* IN */
{
   ASSERT (sd);
   ASSERT (buf);
   ASSERT (len);

   return Task_Send (sd->sd, buf, len, flags);
}


ssize_t
Socket_SendMsg (Socket *sd,               /* IN */
                const struct msghdr *msg, /* IN */
                int flags)                /* IN */
{
   ASSERT (sd);
   ASSERT (msg);

   return Task_SendMsg (sd->sd, msg, flags);
}


int
Socket_SetSockOpt (Socket *sd,         /* IN */
                   int level,          /* IN */
                   int optname,        /* IN */
                   const void *optval, /* IN */
                   socklen_t optlen)   /* IN */
{
   ASSERT (sd);

   return setsockopt (sd->sd, level, optname, optval, optlen);
}


int
Socket_Listen (Socket *sd,  /* IN */
               int backlog) /* IN */
{
   ASSERT (sd);

   return listen (sd->sd, backlog);
}
