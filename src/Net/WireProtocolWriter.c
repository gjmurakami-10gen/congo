/* WireProtocolWriter.c
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

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <Debug.h>
#include <Memory.h>
#include <WireProtocolWriter.h>


void
WireProtocolWriter_Init (WireProtocolWriter *writer, /* OUT */
                         Socket *sock)               /* IN */
{
   ASSERT (writer);
   ASSERT (sock);

   writer->sock = sock;
}


bool
WireProtocolWriter_Write (WireProtocolWriter *writer,   /* IN */
                          WireProtocolMessage *message) /* IN */
{
   struct msghdr msg;
   Array iovecs;
   ssize_t ret;
   size_t expected = 0;
   int i;

   ASSERT (writer);
   ASSERT (message);

   Array_Init (&iovecs, sizeof(struct iovec), false);
   WireProtocolMessage_Gather (message, &iovecs);
   WireProtocolMessage_ToLe (message);

   Memory_Zero (&msg, sizeof msg);
   msg.msg_iov = (void *)iovecs.data;
   msg.msg_iovlen = iovecs.len;

   for (i = 0; i < iovecs.len; i++) {
      expected += msg.msg_iov [i].iov_len;
   }

   ret = Socket_SendMsg (writer->sock, &msg, 0);

   Array_Destroy (&iovecs);

   return (ret == expected);
}


void
WireProtocolWriter_Destroy (WireProtocolWriter *writer) /* IN */
{
   ASSERT (writer);

   writer->sock = NULL;
}
