/* WireProtocolReader.c
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

#include <errno.h>
#include <limits.h>

#include <Debug.h>
#include <Endian.h>
#include <Math.h>
#include <Memory.h>
#include <WireProtocolReader.h>


/*
 *--------------------------------------------------------------------------
 *
 * WireProtocolReader_Init --
 *
 *       Initialize a new WireProtocolReader that will read from the
 *       underlying socket of @sock. Read data will be buffered by @reader
 *       until a supplimental call to WireProtocolReader_Read().
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       @reader is initialized.
 *
 *--------------------------------------------------------------------------
 */

void
WireProtocolReader_Init (WireProtocolReader *reader, /* IN */
                         Socket *sock)               /* IN */
{
   ASSERT (reader);
   ASSERT (sock);

   Memory_Zero (reader, sizeof *reader);

   reader->sock = sock;
   reader->buflen = 0;
   reader->bufalloc = 512;
   reader->buf = Memory_Malloc (reader->bufalloc);
}


/*
 *--------------------------------------------------------------------------
 *
 * WireProtocolReader_Destroy --
 *
 *       Clean up after a WireProtocolReader and release any resources.
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
WireProtocolReader_Destroy (WireProtocolReader *reader) /* IN */
{
   ASSERT (reader);

   Memory_Free (reader->buf);
}


static void
WireProtocolReader_GrowBuffer (WireProtocolReader *reader, /* IN */
                               uint32_t minsize)           /* IN */
{
   size_t size;

   ASSERT (reader);
   ASSERT (minsize < INT_MAX);

   if (minsize > reader->bufalloc) {
      size = UInt32_NextPowerOf2 (minsize);
      reader->buf = Memory_SafeRealloc (reader->buf, size);
      reader->bufalloc = size;
   }
}


static bool
WireProtocolReader_TryFill (WireProtocolReader *reader, /* IN */
                            size_t minsize)             /* IN */
{
   ssize_t ret;

   ASSERT (reader);

   if (minsize <= reader->buflen) {
      return true;
   }

   WireProtocolReader_GrowBuffer (reader, minsize);

   for (;;) {
      ret = Socket_Recv (reader->sock,
                         reader->buf + reader->buflen,
                         reader->bufalloc - reader->buflen,
                         0,
                         reader->timeout);

      if (ret <= 0) {
         return false;
      }

      reader->buflen += ret;

      if (minsize <= reader->buflen) {
         return true;
      }
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * WireProtocolReader_Read --
 *
 *       Reads the next message from the underlying socket.
 *
 *       Any side-effects from previous calls to this function are
 *       invalidated as part of this call. So buffer the contents of
 *       @message if you need the data to persist among multiple calls.
 *
 * Parameters:
 *       @reader  : a reader to read from.
 *       @message : a location to a message that will point to private
 *                  buffered data owned by the reader instance.
 *
 * Returns:
 *       true if successful; otherwise false.
 *
 * Side effects:
 *       @message is initialized if true is returned.
 *
 *--------------------------------------------------------------------------
 */

bool
WireProtocolReader_Read (WireProtocolReader *reader,   /* IN */
                         WireProtocolMessage *message) /* OUT */
{
   ASSERT (reader);
   ASSERT (message);

   if (reader->buflen > reader->msglen) {
      memmove (reader->buf,
               reader->buf + reader->msglen,
               reader->buflen - reader->msglen);
   }

   reader->buflen -= reader->msglen;

   if (WireProtocolReader_TryFill (reader, sizeof reader->msglen)) {
      if (reader->buflen >= sizeof reader->msglen) {
         memcpy (&reader->msglen, reader->buf, sizeof reader->msglen);
         reader->msglen = UINT32_FROM_LE (reader->msglen);
         if (WireProtocolReader_TryFill (reader, reader->msglen)) {
            WireProtocolMessage_FromLe (message);
            return WireProtocolMessage_Scatter (message,
                                                reader->buf,
                                                reader->msglen);
         }
      }
   }

   return false;
}
