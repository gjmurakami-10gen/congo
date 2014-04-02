/* Thread.h
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


#ifndef THREAD_H
#define THREAD_H


#include <Macros.h>
#include <Platform.h>


BEGIN_DECLS


#if defined(PLATFORM_POSIX)
typedef pthread_t Thread;
#elif defined(PLATFORM_WIN32)
typedef HANDLE Thread;
#else
# warning "Threading on your platform is not supported."
#endif


typedef void *(*ThreadFunc) (void *arg);


bool    Thread_Init   (Thread *thread,
                       const char *name,
                       ThreadFunc func,
                       void *data);
Thread  Thread_Self   (void);
void    Thread_Yield  (void);
bool    Thread_Cancel (Thread thread);
void   *Thread_Join   (Thread thread);


END_DECLS


#endif /* THREAD_H */
