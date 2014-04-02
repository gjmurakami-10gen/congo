/* Macros.h
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


#ifndef MACROS_H
#define MACROS_H


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stddef.h>


#ifdef __cplusplus
# define BEGIN_DECLS extern "C" {
# define END_DECLS   }
#else
# define BEGIN_DECLS
# define END_DECLS
#endif


#ifdef __GNUC__
# define GNUC_CONST __attribute__((const))
# ifdef __clang__
#  define GNUC_PRINTF(a,b) __attribute__((format (printf, a, b)))
# elif __GNUC_MINOR__ > 4
#  define GNUC_PRINTF(a,b) __attribute__((format (gnu_printf, a, b)))
# else
#  define GNUC_PRINTF(a,b)
# endif
# define GNUC_NULL_TERMINATED __attribute__((sentinel))
# define GNUC_ALIGNED(n) __attribute__((aligned(n)))
#else
# define GNUC_CONST
# define GNUC_PRINTF(a,b)
# define GNUC_NULL_TERMINATED
# define GNUC_ALIGNED(n)
#endif


#ifndef N_ELEMENTS
# define N_ELEMENTS(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif


#ifndef MIN
# ifdef __cplusplus
#  include <algorithm>
#  define MIN(a, b) std::min(a, b)
# else
#  define MIN(a, b) ({ \
                        __typeof__ (a)_a = (a); \
                        __typeof__ (b)_b = (b); \
                        _a < _b ? _a : _b;  \
                     })
# endif
#endif


#ifndef MAX
# ifdef __cplusplus
#  include <algorithm>
#  define MAX(a, b) std::max(a, b)
# else
#  define MAX(a, b) ({ \
                        __typeof__ (a)_a = (a); \
                        __typeof__ (b)_b = (b); \
                        _a > _b ? _a : _b; \
                     })
# endif
#endif



#if defined(__GNUC__)
# define ALIGNOF __alignof__
#elif defined(HAVE_TYPEOF)
# define ALIGNOF(t) \
   ((sizeof (t) > 1)? offsetof(struct { char c; typeof(t) x; }, x) : 1)
#else
# define ALIGNOF(t) \
   ((sizeof (t) > 1)? offsetof(struct { char c; t x; }, x) : 1)
#endif


#if defined(_MSC_VER) && !defined(__inline__)
# define __inline__ __inline
#endif


#if defined(__GNUC__)
#  define LIKELY(x)    __builtin_expect (!!(x), 1)
#  define UNLIKELY(x)  __builtin_expect (!!(x), 0)
#else
#  define LIKELY(v)   (v)
#  define UNLIKELY(v) (v)
#endif


#endif /* MACROS_H */
