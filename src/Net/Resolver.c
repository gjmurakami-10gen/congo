/* Resolver.c
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

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <Debug.h>
#include <Resolver.h>
#include <Task.h>


/*
 *--------------------------------------------------------------------------
 *
 * Resolver_GetAddrInfo --
 *
 *       getaddrinfo wrapper for use by coroutines.
 *
 * TODO:
 *       This should use c-ares instead of a blocking call on the worker
 *       thread poo.
 *
 * Returns:
 *       0 on success, otherwise -1 and errno is set.
 *
 * Side effects:
 *       @results is set with an array of results. which should be freed
 *       with freeaddrinfo().
 *
 *--------------------------------------------------------------------------
 */

int
Resolver_GetAddrInfo (const char *host,             /* IN */
                      const char *service,          /* IN */
                      const struct addrinfo *hints, /* IN */
                      struct addrinfo **results)    /* OUT */
{
   int ret;

   ASSERT (host);
   ASSERT (service);
   ASSERT (results);

   //Task_BeginBlockingCall ();
   ret = getaddrinfo (host, service, hints, results);
   //Task_EndBlockingCall ();

   return ret;
}
