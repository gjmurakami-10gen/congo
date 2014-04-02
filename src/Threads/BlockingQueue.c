/* BlockingQueue.c
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


#include <BlockingQueue.h>
#include <Debug.h>
#include <Memory.h>


/*
 *--------------------------------------------------------------------------
 *
 * BlockingQueue_Init --
 *
 *       Initialize a blocking queue.
 *
 *       This uses a Mutex and Condition for synchronization. Items are
 *       stored in an array while tracking the head and tail of the queue.
 *
 *       This means that you must initialize with the size you want for
 *       that array. It will not be dynamically grown.
 *
 *       If you are at your size limit, you block just like readers do.
 *
 *       @maxitems *MUST* be a power of two.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       @queue is initialized.
 *
 *--------------------------------------------------------------------------
 */

void
BlockingQueue_Init (BlockingQueue *queue, /* IN */
                    int maxitems)         /* IN */
{
   ASSERT (queue);
   ASSERT (maxitems > 0);
   ASSERT ((maxitems & (maxitems - 1)) == 0);

   Memory_Zero (queue, sizeof *queue);

   Mutex_Init (&queue->mutex, NULL);
   Cond_Init (&queue->rdcond, NULL);
   Cond_Init (&queue->wrcond, NULL);

   Array_InitSized (&queue->items, sizeof (void *), false, maxitems);
   queue->items.len = maxitems;

   queue->mask = maxitems - 1;
   queue->head = 0;
   queue->count = 0;
}


static __inline__ bool
BlockingQueue_IsFullLocked (const BlockingQueue *queue) /* IN */
{
   return (queue->count == queue->items.len);
}


static __inline__ bool
BlockingQueue_IsEmptyLocked (const BlockingQueue *queue) /* IN */
{
   return (queue->count == 0);
}


void
BlockingQueue_Push (BlockingQueue *queue, /* IN */
                    void *data)           /* IN */
{
   int idx;

   ASSERT (queue);

   Mutex_Lock (&queue->mutex);

   while (BlockingQueue_IsFullLocked (queue)) {
      Cond_Wait (&queue->wrcond, &queue->mutex);
   }

   idx = (queue->head + queue->count) & queue->mask;
   Array_Index (&queue->items, void*, idx) = data;
   queue->count++;

   Cond_Signal (&queue->rdcond);

   Mutex_Unlock (&queue->mutex);
}


void *
BlockingQueue_Pop (BlockingQueue *queue) /* IN */
{
   void *ret;

   ASSERT (queue);

   Mutex_Lock (&queue->mutex);

   while (BlockingQueue_IsEmptyLocked (queue)) {
      Cond_Wait (&queue->rdcond, &queue->mutex);
   }

   ret = Array_Index (&queue->items, void*, queue->head);
   queue->head = (queue->head + 1) & queue->mask;
   queue->count--;

   Cond_Signal (&queue->wrcond);

   Mutex_Unlock (&queue->mutex);

   return ret;
}


void
BlockingQueue_Destroy (BlockingQueue *queue) /* IN */
{
   ASSERT (queue);

   Mutex_Destroy (&queue->mutex);
   Cond_Destroy (&queue->rdcond);
   Cond_Destroy (&queue->wrcond);
   Array_Destroy (&queue->items);

   Memory_Zero (queue, sizeof *queue);
}
