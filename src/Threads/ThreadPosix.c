/* ThreadPosix.c
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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include <Debug.h>
#include <Memory.h>
#include <Thread.h>


typedef struct
{
   char *name;
   ThreadFunc func;
   void *data;
} ThreadInfo;


/*
 *--------------------------------------------------------------------------
 *
 * Thread_Worker --
 *
 *       Worker function to call the users thread function.
 *
 *       This function will mask all signals before proceeding. This
 *       should simplify the signal handling process as threads will
 *       not need to deal with signals at all.
 *
 *       However, that means a thread should be configured to handle
 *       signals in your process. Possibly the main thread.
 *
 * Returns:
 *       User defined value that can be retrieved with Thread_Join().
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static void *
Thread_Worker (void *data) /* IN */
{
   ThreadInfo *info = data;
   sigset_t set;
   void *ret;

   ASSERT (info);
   ASSERT (info->func);

   sigfillset (&set);
   pthread_sigmask (SIG_BLOCK, &set, NULL);

#if defined(HAVE_PTHREAD_SETNAME_NP)
   if (info->name) {
#if defined(PLATFORM_APPLE)
      pthread_setname_np (info->name);
#elif defined(PLATFORM_LINUX)
      pthread_setname_np (pthread_self (), info->name);
#endif
   }
#endif

   ret = info->func (info->data);

   free (info->name);
   free (info);

   return ret;
}


/*
 *--------------------------------------------------------------------------
 *
 * Thread_Init --
 *
 *       Create and initialize a new pthread.
 *
 *       If the system supports it, @name will be used to name the thread.
 *
 *       @func will be called with @data inside of the new thread.
 *
 * Returns:
 *       true if successful; otherwise false and @error is set.
 *
 * Side effects:
 *       @thread is initialized.
 *
 *--------------------------------------------------------------------------
 */

bool
Thread_Init (Thread *thread,     /* OUT */
             const char *name,   /* IN */
             ThreadFunc func,    /* IN */
             void *data)         /* IN */
{
   ThreadInfo *info;

   ASSERT (thread);
   ASSERT (func);

   info = Memory_SafeMalloc0 (sizeof *info);
   info->name = name ? strdup (name) : NULL;
   info->func = func;
   info->data = data;

   return (0 == pthread_create (thread, NULL, Thread_Worker, info));
}


void
Thread_Yield (void)
{
#if defined(HAVE_PTHREAD_YIELD)
   pthread_yield ();
#elif defined(HAVE_PTHREAD_YIELD_NP)
   pthread_yield_np ();
#endif
}


Thread
Thread_Self (void)
{
   return pthread_self ();
}


bool
Thread_Cancel (Thread thread)
{
   return (0 == pthread_cancel (thread));
}


void *
Thread_Join (Thread thread)
{
   void *ret = NULL;

   if (0 == pthread_join (thread, &ret)) {
      return ret;
   }

   return NULL;
}
