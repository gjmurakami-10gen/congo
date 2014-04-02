/* TimeSpec.c
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

#if defined(__APPLE__)
# include <mach/clock.h>
# include <mach/mach.h>
# include <mach/mach_time.h>
#endif

#if !defined(_WIN32)
# include <sys/time.h>
# include <time.h>
#endif

#include <TimeSpec.h>
#include <Debug.h>
#include <Platform.h>
#include <Types.h>


#define WIN32_EPOCH_OFFSET_USEC 11644473600000000ULL


/*
 *--------------------------------------------------------------------------
 *
 * TimeSpec_InitRealtime --
 *
 *       Initialize the TimeSpec @ts using the systems realtime clock.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       ts is initialized with the realtime clocks current time.
 *
 *--------------------------------------------------------------------------
 */

void
TimeSpec_InitRealtime (TimeSpec *ts) /* OUT */
{
#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_REALTIME)
   struct timespec _ts;
   ASSERT (ts);
   clock_gettime (CLOCK_REALTIME, &_ts);
   ts->tv_sec = _ts.tv_sec;
   ts->tv_usec = _ts.tv_nsec / 1000UL;
#else
   struct timeval t = { 0 };
   ASSERT (ts);
   gettimeofday (&t, NULL);
   ts->tv_sec = t.tv_sec;
   ts->tv_usec = t.tv_usec;
#endif
}


void
TimeSpec_InitMonotonic (TimeSpec *ts) /* OUT */
{
#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_MONOTONIC)
   struct timespec _ts;
   clock_gettime (CLOCK_MONOTONIC, &_ts);
   ts->tv_sec = _ts.tv_sec;
   ts->tv_usec = _ts.tv_nsec / 1000UL;
#elif defined(__APPLE__)
   static mach_timebase_info_data_t info = { 0 };
   static double ratio = 0.0;

   if (UNLIKELY (!info.denom)) {
      mach_timebase_info (&info);
      ratio = info.numer / info.denom;
   }

   return (mach_absolute_time() * ratio);
#elif defined(PLATFORM_WIN32)
   uint64_t msec = (GetTickCount64 () / 10UL) - WIN32_EPOCH_OFFSET_USEC;
   ts->tv_sec = msec / USEC_PER_SEC;
   ts->tv_usec = msec % USEC_PER_SEC;
#else
# warning "Monotonic clock is not yet supported on your platform."
   TimeSpec_InitRealtime (ts);
#endif
}


uint64_t
TimeSpec_GetMonotonic (void)
{
   TimeSpec ts;

   TimeSpec_InitMonotonic (&ts);
   return (ts.tv_sec * USEC_PER_SEC) + (ts.tv_usec);
}


/*
 *--------------------------------------------------------------------------
 *
 * TimeSpec_Subtract --
 *
 *       Subtract @right from @left and store the result in @ret.
 *
 *       This might be useful if you want to track the time spent between
 *       two clock samples.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       @ret is set.
 *
 *--------------------------------------------------------------------------
 */

void
TimeSpec_Subtract (TimeSpec *ret,         /* OUT */
                   const TimeSpec *left,  /* IN */
                   const TimeSpec *right) /* IN */
{
   TimeSpec r;

   ASSERT (left);
   ASSERT (right);

   r.tv_sec = (left->tv_sec - right->tv_sec);

   if ((r.tv_usec = (left->tv_usec - right->tv_usec)) < 0) {
      r.tv_usec += USEC_PER_SEC;
      r.tv_sec -= 1;
   }

   *ret = r;
}


/*
 *--------------------------------------------------------------------------
 *
 * TimeSpec_Compare --
 *
 *       Compares the difference between ts1 and ts2. The result is a
 *       qsort() style value.
 *
 * Returns:
 *       -1 if ts1 is less than ts2.
 *        0 if ts1 is equal to ts2.
 *        1 if ts1 is greater than ts2.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

int
TimeSpec_Compare (const TimeSpec *ts1, /* IN */
                  const TimeSpec *ts2) /* IN */
{
   ASSERT (ts1);
   ASSERT (ts2);

   if ((ts1->tv_sec == ts2->tv_sec) &&
       (ts1->tv_usec == ts2->tv_usec)) {
      return 0;
   } else if ((ts1->tv_sec > ts2->tv_sec) ||
              ((ts1->tv_sec == ts2->tv_sec) &&
               (ts1->tv_usec > ts2->tv_usec))) {
      return 1;
   } else {
      return -1;
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * TimeSpec_AddMilliSeconds --
 *
 *       Adds msec milliseconds to the TimeSpec.
 *       @msec may be negative to perform a subtaction.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       ts is modified.
 *
 *--------------------------------------------------------------------------
 */

void
TimeSpec_AddMilliSeconds (TimeSpec *ts, /* INOUT */
                          int64_t msec) /* IN */
{
   ASSERT (ts);

   ts->tv_sec += (msec / MSEC_PER_SEC);
   ts->tv_usec += (msec % MSEC_PER_SEC);

   if (ts->tv_usec > USEC_PER_SEC) {
      ts->tv_sec++;
      ts->tv_usec -= USEC_PER_SEC;
   } else if (ts->tv_usec < 0) {
      ts->tv_sec--;
      ts->tv_usec += USEC_PER_SEC;
   }
}
