/* Cond.h
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


#ifndef COND_H
#define COND_H


#include <Macros.h>
#include <Platform.h>
#include <Types.h>


BEGIN_DECLS


#if defined(PLATFORM_POSIX)
# include <pthread.h>
# define Cond           pthread_cond_t
# define Cond_Init      pthread_cond_init
# define Cond_Signal    pthread_cond_signal
# define Cond_Broadcast pthread_cond_broadcast
# define Cond_Destroy   pthread_cond_destroy
# define Cond_Wait      pthread_cond_wait
#elif defined(PLATFORM_WIN32)
# define Cond              HANDLE
# define Cond_Init(p,a)    (*(p) = CreateEvent(NULL, 0, 0, NULL))
# define Cond_Destroy(p)   CloseHandle(*(p))
# define Cond_Signal(p)    SetEvent(*(p))
# define Cond_Broadcast(p) PulseEvent(*(p))
# define Cond_Wait(pc,pm)  \
   do { \
      SignalObjectAndWait(*(pm), *(pc), INFINITE, 0); \
      WaitForSingleObject(*(pm), INFINITE); \
   } while (0)
#else
# error "You're platform is not supported."
#endif


END_DECLS


#endif /* COND_H */
