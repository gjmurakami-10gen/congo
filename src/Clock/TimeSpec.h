/* Clock.h
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


#ifndef CLOCK_H
#define CLOCK_H


#include <Macros.h>
#include <Types.h>


BEGIN_DECLS


#define MSEC_PER_SEC    1000UL
#define USEC_PER_SEC    1000000UL
#define NANOSEC_PER_SEC 1000000000UL


typedef struct
{
   int64_t tv_sec;
   int64_t tv_usec;
} TimeSpec;


uint64_t TimeSpec_GetMonotonic    (void);
void     TimeSpec_InitMonotonic   (TimeSpec *ts);
void     TimeSpec_InitRealtime    (TimeSpec *ts);
void     TimeSpec_AddMilliSeconds (TimeSpec *ts,
                                   int64_t msec);
int      TimeSpec_Compare         (const TimeSpec *ts1,
                                   const TimeSpec *ts2);
void     TimeSpec_Subtract        (TimeSpec *ret,
                                   const TimeSpec *left,
                                   const TimeSpec *right);


END_DECLS


#endif /* CLOCK_H */
