/* ThreadPool.c
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


#include <stdio.h>

#include <Atomic.h>
#include <Debug.h>
#include <Macros.h>
#include <Thread.h>
#include <ThreadPool.h>


static void *
ThreadPool_Worker (void *poolptr) /* IN */
{
   ThreadPool *pool = poolptr;
   ThreadPoolFunc func;
   void *func_data;
   void *data;

   ASSERT (pool);

   func = pool->worker;
   func_data = pool->worker_data;

   for (;;) {
      data = BlockingQueue_Pop (&pool->queue);
      func (data, func_data);
   }

   return NULL;
}


void
ThreadPool_Init (ThreadPool *threadpool, /* IN */
                 int nthreads,           /* IN */
                 int qdepth,             /* IN */
                 ThreadPoolFunc worker,  /* IN */
                 void *user_data)        /* IN */
{
   Thread thread;
   char name [32];
   int i;

   ASSERT (threadpool);

   BlockingQueue_Init (&threadpool->queue, qdepth);

   threadpool->worker = worker;
   threadpool->worker_data = user_data;

   Memory_Barrier ();

   Array_Init (&threadpool->threads, sizeof thread, false);

   for (i = 0; i < nthreads; i++) {
      snprintf (name, sizeof name, "ThreadPool-%u", i);
      name [sizeof name - 1] = '\0';
      Thread_Init (&thread, name, ThreadPool_Worker, threadpool);
      Array_Append (&threadpool->threads, thread);
   }
}


void
ThreadPool_Destroy (ThreadPool *threadpool) /* IN */
{
   /*
    * TODO:
    */
}


void
ThreadPool_Push (ThreadPool *threadpool, /* IN */
                 void *data)             /* IN */
{
   ASSERT (threadpool);

   BlockingQueue_Push (&threadpool->queue, data);
}
