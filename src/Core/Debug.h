/* Debug.h
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


#ifndef DEBUG_H
#define DEBUG_H


#include <Macros.h>


BEGIN_DECLS


#ifndef DISABLE_ASSERT
# include <assert.h>
# define ASSERT(_a) assert(_a)
#else
# define ASSERT(_a)
#endif


#ifndef __COUNTER__
# define __COUNTER__ __LINE__
#endif


#define STATIC_ASSERT(s) STATIC_ASSERT_ (s, __LINE__)
#define STATIC_ASSERT_JOIN(a, b) STATIC_ASSERT_JOIN2 (a, b)
#define STATIC_ASSERT_JOIN2(a, b) a##b
#define STATIC_ASSERT_(s, l) \
   typedef char STATIC_ASSERT_JOIN (static_assert_test_, __COUNTER__)[(s) ? 1 : -1]


END_DECLS


#endif /* DEBUG_H */
