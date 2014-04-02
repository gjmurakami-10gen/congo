/* Sched.h
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


#ifndef SCHED_H
#define SCHED_H


#include <Macros.h>


BEGIN_DECLS


#ifndef TASK_USE_LTHREAD
# include <pthread.h>
# include <unistd.h>
extern pthread_mutex_t gSchedLock;
extern pthread_cond_t  gSchedCond;
# define Sched_Create() \
   do { \
      pthread_mutex_init (&gSchedLock, NULL); \
      pthread_cond_init (&gSchedCond, NULL); \
      pthread_mutex_lock (&gSchedLock); \
   } while (0)
# define Sched_Run() \
   do { \
      pthread_cond_wait (&gSchedCond, &gSchedLock); \
      pthread_mutex_unlock (&gSchedLock); \
   } while (0)
# define Sched_Quit() \
   do { \
      pthread_mutex_lock (&gSchedLock); \
      pthread_cond_broadcast (&gSchedCond); \
      pthread_mutex_unlock (&gSchedLock); \
   } while (0)
#else
# include <lthread.h>
# define Sched_Create()
# define Sched_Run      lthread_run
# define Sched_Quit()
#endif


END_DECLS


#endif /* SCHED_H */
