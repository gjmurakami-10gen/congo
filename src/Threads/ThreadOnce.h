/* ThreadOnce.h
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


#ifndef THREAD_ONCE_H
#define THREAD_ONCE_H


#include <Macros.h>
#include <Platform.h>


BEGIN_DECLS


#if defined(PLATFORM_POSIX)
# include <pthread.h>
# define ThreadOnce       pthread_once_t
# define ThreadOnce_Once  pthread_once
# define THREAD_ONCE_INIT PTHREAD_ONCE_INIT
#elif defined(PLATFORM_WIN32)
typedef int ThreadOnce;
# define THREAD_ONCE_INIT 0
void ThreadOnce_Once (ThreadOnce *once, void (*func) (void));
#else
# error "Unknown threading platform detected."
#endif


END_DECLS


#endif /* THREAD_ONCE_H */
