/* ThreadOnce.c
 *
 * Copyright (C) 2014 Christian Hergert <christian@hergert.me>
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


#include <Atomic.h>
#include <Thread.h>
#include <ThreadOnce.h>


#if defined(PLATFORM_WIN32)
/*
 *--------------------------------------------------------------------------
 *
 * ThreadOnce_Once --
 *
 *       Ensure a function is only called once.
 *
 *       XXX:
 *
 *       Note that this is succeptable to a thread cancellation problem
 *       that could cause the cancelled thread never to mark the
 *       ThreadOnce as initialized.
 *
 *       We will ignore that for now!
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       @once is modified.
 *
 *--------------------------------------------------------------------------
 */

void
ThreadOnce_Once (ThreadOnce *once,
                 void (*func) (void))
{
   MemoryBarrier ();

   if (*once == 0) {
      if (AtomicInt_CompareAndSwap (once, 0, 1)) {
         func ();
         AtomicInt_Set (once, 2);
         MemoryBarrier ();
         return;
      }
   }

   while (*once != 2) {
      Thread_Yield ();
      MemoryBarrier ();
   }
}
#endif
