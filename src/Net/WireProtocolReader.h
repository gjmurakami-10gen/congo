/* WireProtocolReader.h
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


#ifndef WIRE_PROTOCOL_READER_H
#define WIRE_PROTOCOL_READER_H


#include <Macros.h>
#include <Socket.h>
#include <Types.h>
#include <WireProtocol.h>


BEGIN_DECLS


typedef struct
{
   Socket  *sock;
   uint8_t *buf;
   size_t   bufalloc;
   size_t   buflen;
   uint32_t msglen;
   uint64_t timeout;
} WireProtocolReader;


void WireProtocolReader_Init    (WireProtocolReader *reader,
                                 Socket *sock);
void WireProtocolReader_Destroy (WireProtocolReader *reader);
bool WireProtocolReader_Read    (WireProtocolReader *reader,
                                 WireProtocolMessage *message);


END_DECLS


#endif /* WIRE_PROTOCOL_READER_H */
