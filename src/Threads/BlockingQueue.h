/* BlockingQueue.h
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


#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H


#include <Array.h>
#include <Macros.h>
#include <Cond.h>
#include <Mutex.h>


BEGIN_DECLS


typedef struct _BlockingQueue BlockingQueue;


struct _BlockingQueue
{
   Mutex  mutex;
   Cond   rdcond;
   Cond   wrcond;
   Array  items;
   int    head;
   int    count;
   int    mask;
} GNUC_ALIGNED (64);


void  BlockingQueue_Init    (BlockingQueue *queue,
                             int maxitems);
void  BlockingQueue_Destroy (BlockingQueue *queue);
void  BlockingQueue_Push    (BlockingQueue *queue,
                             void *data);
void *BlockingQueue_Pop     (BlockingQueue *queue);


END_DECLS


#endif /* BLOCKING_QUEUE_H */
