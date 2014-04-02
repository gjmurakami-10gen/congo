/* Mutable.c
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


#include <string.h>

#include <Core/Debug.h>
#include <Storage/Mutable.h>


#define PAGE_SIZE 4096


/*
 *--------------------------------------------------------------------------
 *
 * Mutable_IO --
 *
 *       Perform Input or Output on a set of mutable buffers.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       The contents of @buf are overwritten if @mutate is false.
 *       The contents of @mutable's buffers are overwritten if
 *       @mutate is true.
 *
 *--------------------------------------------------------------------------
 */

void
Mutable_IO (Mutable *mutable, /* IN */
            bool mutate,      /* IN */
            void *buf,        /* IN */
            size_t count,     /* IN */
            off_t offset)     /* IN */
{
   void *src;
   void *dst;
   size_t cur;
   size_t len;

   ASSERT (mutable);
   ASSERT (buf);
   ASSERT (count);

   cur = offset / PAGE_SIZE;
   offset = offset % PAGE_SIZE;

   for (; count; cur++) {
      len = MIN (count, (PAGE_SIZE - offset));

      if (mutate) {
         src = buf;
         dst = ((char *)mutable->iov [cur].iov_base) + offset;
      } else {
         src = ((char *)mutable->iov [cur].iov_base) + offset;
         dst = buf;
      }

      memcpy (dst, src, len);

      offset = 0;
      count -= len;

      dst = ((uint8_t *)dst) + len;
      src = ((uint8_t *)src) + len;
   }
}


void
Mutable_Init (Mutable *mutable,  /* IN */
              struct iovec *iov, /* IN */
              size_t iovcnt)     /* IN */
{
   ASSERT (mutable);
   ASSERT (iov);
   ASSERT (iovcnt);

   mutable->iov = iov;
   mutable->iovcnt = iovcnt;
}


void
Mutable_Destroy (Mutable *mutable) /* IN */
{
   ASSERT (mutable);

   /* Nothing Currently */
}
