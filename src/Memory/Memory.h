/* Memory.h
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


#ifndef MEMORY_H
#define MEMORY_H


#include <stdlib.h>
#include <string.h>

#include <Macros.h>
#include <Types.h>


BEGIN_DECLS


#if defined(__GNUC__)
# if !defined(_WIN32)
#  define Memory_Barrier __sync_synchronize
# else
#  define Memory_Barrier MemoryBarrier
# endif
#endif


#define Memory_New(Type)      Memory_SafeMalloc(sizeof(Type))
#define Memory_New0(Type)     Memory_SafeMalloc0(sizeof(Type))
#define Memory_Zero(Mem,Size) memset((Mem), 0, (Size))


void  Memory_Free        (void *mem);
void *Memory_Malloc      (size_t size);
void *Memory_Malloc0     (size_t size);
void *Memory_Malloc0N    (size_t elemsize,
                          size_t count);
void *Memory_Memalign    (size_t size,
                          size_t alignment);
void *Memory_SafeMalloc  (size_t size);
void *Memory_SafeMallocN (size_t elemsize,
                          size_t count);
void *Memory_SafeMalloc0 (size_t size);
void *Memory_SafeRealloc (void *mem,
                          size_t size);


END_DECLS


#endif /* DB_MEMORY_H */
