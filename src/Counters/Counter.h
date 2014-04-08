/* Counter.h
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


#ifndef COUNTER_H
#define COUNTER_H


#include <Atomic.h>
#include <Macros.h>
#include <Platform.h>
#include <Types.h>


/*
 * This is a counter implementation that sacrifices correctness for a bit
 * more performance. We avoid using atomic instructions by trying to limit
 * access per-cacheline to a given CPU. Therefore, a counter may be split
 * up between multiple cache-lines (1/8 of a cacheline per counter per CPU).
 *
 * This means that updates could potentially race if a thread is migrated to
 * another CPU between the time Platform_GetCurrentCpu() is called and the add
 * instruction is executed. The call gets inlined, and on GNU/Linux-x86_64
 * this comes out to about 7 instructions (rdtscp, a couple of mov's, and an
 * addq).
 *
 * To read the value, however, we must volatile read each cacheline for the
 * counter and add the results together.
 *
 * -- Christian (Jan 15, 2014)
 */

BEGIN_DECLS


#ifdef HAVE_PLATFORM_GETCURRENTCPU
# define COUNTER_ADD(c,v) \
   c.values [Platform_GetCurrentCpu()].value += value
#else
# warning "Platform_GetCurrentCpu() is not supported on your platform. " \
          "Counters will use atomics which has performance implications."
# define COUNTER_ADD(c,v) AtomicInt64_Add(&c.values[0].value, v)
#endif


#define COUNTER(Identifier, Category, Name, Description) \
   static Counter __##Identifier; \
   \
   static void \
   Identifier##_Register (void) __attribute__((constructor)); \
   \
   static void \
   Identifier##_Register (void) \
   { \
      __##Identifier.category = Category; \
      __##Identifier.name = Name; \
      __##Identifier.description = Description; \
      Counter_Register (&__##Identifier); \
   } \
   \
   static __inline__ void \
   Identifier##_Add (int64_t value) \
   { \
      COUNTER_ADD(__##Identifier, value); \
   } \
   \
   static __inline__ void \
   Identifier##_Increment (void) \
   { \
      Identifier##_Add (1); \
   } \
   \
   static __inline__ void \
   Identifier##_Decrement (void) \
   { \
      Identifier##_Add (-1); \
   } \
   \
   static __inline__ int64_t \
   Identifier##_Get (void) \
   { \
      return Counter_Get (&__##Identifier); \
   }


typedef struct _Counter      Counter;
typedef struct _CounterValue CounterValue;


struct _CounterValue
{
   volatile int64_t value;
   int64_t          padding [7];
};


struct _Counter
{
   CounterValue *values;
   const char   *category;
   const char   *name;
   const char   *description;
};


typedef void (*CounterForeachFunc) (Counter *counter,
                                    void *user_data);


/*
 * TODO: I'd like to see this cleaned up to have a Counters structure that
 *       contains the top-level information and can be initialized local
 *       or remote. That becomes the handle for iteration, etc.
 */


void    Counters_Init       (void);
void    Counters_InitRemote (pid_t pid);
void    Counters_Foreach    (CounterForeachFunc func,
                             void *user_data);
void    Counter_Register    (Counter *counter);
int64_t Counter_Get         (const Counter *counter);
void    Counter_Reset       (Counter *counter);


END_DECLS

#endif /* COUNTER_H */
