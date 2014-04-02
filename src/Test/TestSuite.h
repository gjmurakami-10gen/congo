/* TestSuite.h
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


#ifndef TEST_SUITE_H
#define TEST_SUITE_H


#include <stdio.h>


#include <Macros.h>
#include <Types.h>


BEGIN_DECLS


typedef void (*TestFunc) (void);
typedef struct _Test      Test;
typedef struct _TestSuite TestSuite;


#define TEST_VERBOSE   (1 << 0)
#define TEST_NOFORK    (1 << 1)
#define TEST_HELPONLY  (1 << 2)
#define TEST_NOTHREADS (1 << 3)


#define ASSERT_TRUE(val) assert((val) == true)
#define ASSERT_FALSE(val) assert((val) == false)
#define ASSERT_TRUE_WITH_ERROR(val, err) \
   do { \
      if (!(val)) { \
         fprintf (stderr, "Domain=%d Code=%d Message=%s\n", \
                  (err)->domain, (err)->code, (err)->message); \
      } \
   } while (0)


struct _Test
{
   Test     *next;
   char     *name;
   TestFunc  func;
   int       exit_code;
   bool    (*check) (void);
};


struct _TestSuite
{
   char *prgname;
   char *name;
   char *testname;
   Test *tests;
   int   flags;
};


void TestSuite_Init    (TestSuite *suite,
                        const char *name,
                        int argc,
                        char **argv);
void TestSuite_Add     (TestSuite *suite,
                        const char *name,
                        TestFunc func);
void TestSuite_AddFull (TestSuite *suite,
                        const char *name,
                        TestFunc func,
                        bool (*check) (void));
int  TestSuite_Run     (TestSuite *suite);
void TestSuite_Destroy (TestSuite *suite);


END_DECLS


#endif /* TEST_SUITE_H */
