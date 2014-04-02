/* Resolver.h
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


#ifndef RESOLVER_H
#define RESOLVER_H


#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <Macros.h>
#include <Types.h>


BEGIN_DECLS


int Resolver_GetAddrInfo (const char *host,
                          const char *service,
                          const struct addrinfo *hints,
                          struct addrinfo **results);



END_DECLS


#endif /* _RESOLVER_H */
