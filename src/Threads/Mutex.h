/* Mutex.h
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


#ifndef MUTEX_H
#define MUTEX_H


#include <Macros.h>
#include <Platform.h>
#include <Types.h>


BEGIN_DECLS


#if defined(PLATFORM_POSIX)
# include <pthread.h>
# define Mutex           pthread_mutex_t
# define Mutex_Init      pthread_mutex_init
# define Mutex_Lock      pthread_mutex_lock
# define Mutex_Unlock    pthread_mutex_unlock
# define Mutex_Destroy   pthread_mutex_destroy
#elif defined(PLATFORM_WIN32)
# define Mutex           HANDLE
# define Mutex_Init(m,v) CreateMutex(NULL, FALSE, NULL)
# define Mutex_Lock      WaitForSingleObject
# define Mutex_Unlcok    ReleaseMutex
# define Mutex_Destroy   CloseHandle
#else
# error "You're platform is not supported."
#endif


END_DECLS


#endif /* MUTEX_H */
