/* ThreadPool.h
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


#ifndef THREAD_POOL_H
#define THREAD_POOL_H


#include <Array.h>
#include <BlockingQueue.h>
#include <Macros.h>


BEGIN_DECLS


typedef struct _ThreadPool ThreadPool;


typedef void (*ThreadPoolFunc) (void *data,
                                void *user_data);


struct _ThreadPool
{
   ThreadPoolFunc  worker;
   void           *worker_data;
   Array           threads;
   BlockingQueue   queue;
};


void ThreadPool_Init    (ThreadPool *threadpool,
                         int nthreads,
                         int qdepth,
                         ThreadPoolFunc worker,
                         void *user_data);
void ThreadPool_Push    (ThreadPool *threadpool,
                         void *data);
void ThreadPool_Destroy (ThreadPool *threadpool);


END_DECLS


#endif /* THREAD_POOL_H */
