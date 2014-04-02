/* Atomic.h
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


#ifndef ATOMIC_H
#define ATOMIC_H


#include <Macros.h>
#include <Types.h>
#include <Memory.h>


BEGIN_DECLS


#if defined(__GNUC__)
# define AtomicInt_Add(p, v)               (__sync_add_and_fetch(p, v))
# define AtomicInt_Increment(p)            (__sync_add_and_fetch(p, 1))
# define AtomicInt_Decrement(p)            (__sync_sub_and_fetch(p, 1))
# define AtomicInt_DecrementAndTest(p)     (__sync_sub_and_fetch(p, 1) == 0)
# define AtomicInt_Sub(p, v)               (__sync_sub_and_fetch(p, v))
# define AtomicInt_SubAndTest(p, v)        (__sync_sub_and_fetch(p, v) == 0)
# define AtomicInt_CompareAndSwap(p, oldval, newval) \
                                           (__sync_val_compare_and_swap(p, oldval, newval))
# define AtomicInt64_Add(p, v)             (__sync_add_and_fetch_8(p, v))
# define AtomicInt64_Increment(p)          (__sync_add_and_fetch_8(p, 1))
# define AtomicInt64_Decrement(p)          (__sync_sub_and_fetch_8(p, 1))
# define AtomicInt64_DecrementAndTest(p)   (__sync_sub_and_fetch_8(p, 1) == 0)
# define AtomicInt64_Sub(p, v)             (__sync_sub_and_fetch_8(p, v))
# define AtomicInt64_SubAndTest(p, v)      (__sync_sub_and_fetch_8(p, v) == 0)
# define AtomicInt64_CompareAndSwap        AtomicInt_CompareAndSwap
#elif defined(_MSC_VER)
# define AtomicInt_Add(p, v)               (InterlockedAdd(p, v))
# define AtomicInt_Increment(p)            (InterlockedIncrement(p))
# define AtomicInt_Decrement(p)            (InterlockedDecrement(p))
# define AtomicInt_DecrementAndTest(p)     (InterlockedDecrement(p) == 0)
# define AtomicInt_Sub(p, v)               (InterlockedAdd(p, -(v)))
# define AtomicInt_SubAndTest(p, v)        (InterlockedAdd(p, -(v)) == 0)
# define AtomicInt_CompareAndSwap(p,o,n)   (InterlockedCompareExchange(p,n,o))
# define AtomicInt64_Add(p, v)             (InterlockedAdd64(p, v))
# define AtomicInt64_Increment(p)          (InterlockedIncrement64(p))
# define AtomicInt64_Decrement(p)          (InterlockedDecrement64(p))
# define AtomicInt64_DecrementAndTest(p)   (InterlockedDecrement64(p) == 0)
# define AtomicInt64_Sub(p, v)             (InterlockedAdd64(p, -(v)))
# define AtomicInt64_SubAndTest(p, v)      (InterlockedAdd64(p, -(v)) == 0)
# define AtomicInt64_CompareAndSwap(p,o,n) (InterlockedCompareExchange64(p,n,o))
#else
# error "Unknown compiler, teach me how to do atomics!"
#endif


static __inline__ int32_t
AtomicInt_Get (const volatile int32_t *atomic) /* IN */
{
   Memory_Barrier ();
   return *atomic;
}


static __inline__ void
AtomicInt_Set (volatile int32_t *atomic, /* IN */
               int32_t new_value)        /* IN */
{
   *atomic = new_value;
   Memory_Barrier ();
}


static __inline__ int64_t
AtomicInt64_Get (const volatile int64_t *atomic) /* IN */
{
   Memory_Barrier ();
   return *atomic;
}


static __inline__ void
AtomicInt64_Set (volatile int64_t *atomic, /* IN */
                 int64_t new_value)        /* IN */
{
   *atomic = new_value;
   Memory_Barrier ();
}


static __inline__ void *
AtomicPtr_Get (const volatile void *atomic) /* IN */
{
   Memory_Barrier ();
   return *(void **)atomic;
}


static __inline__ void
AtomicPtr_Set (void **atomic, /* OUT */
               void *new_ptr) /* IN */
{
   *atomic = new_ptr;
   Memory_Barrier ();
}


END_DECLS


#endif /* ATOMIC_H */
