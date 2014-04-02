/* QueryCommand.h
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


#ifndef QUERY_COMMAND_H
#define QUERY_COMMAND_H


#include <bson.h>

#include <Command.h>
#include <Macros.h>
#include <Socket.h>
#include <WireProtocol.h>
#include <WireProtocolReader.h>
#include <Types.h>


BEGIN_DECLS


typedef struct
{
   Command parent;
   Socket *client;
   WireProtocolReader *reader;
   void *writer;
   WireProtocolQuery *query;
   bson_t q;
   bson_t f;
} QueryCommand;


bool QueryCommand_Init (QueryCommand *command,
                        Socket *client,
                        WireProtocolReader *reader,
                        void *writer,
                        WireProtocolQuery *query);


END_DECLS


#endif /* QUERY_COMMAND_H */
