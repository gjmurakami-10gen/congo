/* WireProtocolWriter.h
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


#ifndef WIRE_PROTOCOL_WRITER_H
#define WIRE_PROTOCOL_WRITER_H

#include <Macros.h>
#include <Socket.h>
#include <Types.h>
#include <WireProtocol.h>


BEGIN_DECLS


typedef struct _WireProtocolWriter WireProtocolWriter;


struct _WireProtocolWriter
{
   Socket *sock;
   uint64_t timeout;
   void (*fuzzer) (WireProtocolMessage *message);
};


void WireProtocolWriter_Init    (WireProtocolWriter *writer,
                                 Socket *sock);
bool WireProtocolWriter_Write   (WireProtocolWriter *writer,
                                 WireProtocolMessage *message);
void WireProtocolWriter_Destroy (WireProtocolWriter *writer);


END_DECLS


#endif /* WIRE_PROTOCOL_WRITER_H */
