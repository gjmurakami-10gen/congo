/* TestSuite.c
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


#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <Atomic.h>
#include <CString.h>
#include <Debug.h>
#include <Log.h>
#include <Memory.h>
#include <Mutex.h>
#include <Platform.h>
#include <TestSuite.h>
#include <Thread.h>
#include <TimeSpec.h>

#if !defined(PLATFORM_WIN32)
# include <sys/wait.h>
#endif


void
TestSuite_Init (TestSuite *suite,
                const char *name,
                int argc,
                char **argv)
{
   int i;

   ASSERT (suite);
   ASSERT (name);

   Memory_Zero (suite, sizeof *suite);

   suite->name = CString_Dup (name);
   suite->flags = 0;
   suite->prgname = CString_Dup (argv [0]);

   for (i = 0; i < argc; i++) {
      if (0 == strcmp ("-v", argv [i])) {
         suite->flags |= TEST_VERBOSE;
      } else if (0 == strcmp ("-f", argv [i])) {
         suite->flags |= TEST_NOFORK;
      } else if (0 == strcmp ("-p", argv [i])) {
         suite->flags |= TEST_NOTHREADS;
      } else if ((0 == strcmp ("-h", argv [i])) ||
                 (0 == strcmp ("--help", argv [i]))) {
         suite->flags |= TEST_HELPONLY;
      } else if ((0 == strcmp ("-l", argv [i]))) {
         if (argc - 1 == i) {
            fprintf (stderr, "-l requires an argument.\n");
            exit (EXIT_FAILURE);
         }
         suite->testname = CString_Dup (argv [++i]);
      }
   }
}


static bool
TestSuite_CheckDummy (void)
{
   return true;
}


void
TestSuite_Add (TestSuite  *suite,
               const char *name,
               TestFunc    func)
{
   TestSuite_AddFull (suite, name, func, TestSuite_CheckDummy);
}


void
TestSuite_AddFull (TestSuite  *suite,
                   const char *name,
                   TestFunc    func,
                   bool (*check) (void))
{
   Test *test;
   Test *iter;

   ASSERT (suite);
   ASSERT (name);
   ASSERT (func);

   test = Memory_SafeMalloc0 (sizeof *test);
   test->name = strdup (name);
   test->func = func;
   test->check = check;
   test->next = NULL;

   if (!suite->tests) {
      suite->tests = test;
      return;
   }

   for (iter = suite->tests; iter->next; iter = iter->next) { }

   iter->next = test;
}


#if !defined(PLATFORM_WIN32)
static int
TestSuite_RunFuncInChild (TestSuite *suite,
                          TestFunc   func)
{
   pid_t child;
   int exit_code = -1;
   int fd;

   ASSERT (suite);
   ASSERT (func);

   if (-1 == (child = fork())) {
      return -1;
   }

   if (!child) {
      fd = open ("/dev/null", O_WRONLY);
      dup2 (fd, STDOUT_FILENO);
      close (fd);
      func ();
      exit (0);
   }

   if (-1 == waitpid (child, &exit_code, 0)) {
      perror ("waitpid()");
   }

   return exit_code;
}
#endif


static void
TestSuite_RunTest (TestSuite *suite,
                   Test      *test,
                   Mutex     *mutex,
                   int       *count)
{
   uint64_t ts1;
   uint64_t ts2;
   struct timespec ts3;
   char name[64];
   char buf[256];
   int status;

   ASSERT (suite);
   ASSERT (test);

   snprintf (name, sizeof name, "%s%s", suite->name, test->name);
   name [sizeof name - 1] = '\0';

   if (!test->check || test->check ()) {
      if ((suite->flags & TEST_VERBOSE)) {
         Log_Cork ();
      }

      ts1 = TimeSpec_GetMonotonic ();

#if !defined(PLATFORM_POSIX)
      test->func ();
      status = 0;
#else
      if ((suite->flags & TEST_NOFORK)) {
         test->func ();
         status = 0;
      } else {
         status = TestSuite_RunFuncInChild (suite, test->func);
      }
#endif

      ts2 = TimeSpec_GetMonotonic ();

      ts3.tv_sec = (ts2 - ts1) / 1000000UL;
      ts3.tv_nsec = ((ts2 - ts1) % 1000000UL) * 1000UL;

      Mutex_Lock (mutex);
      snprintf (buf, sizeof buf,
                "    { \"status\": \"%s\", "
                      "\"name\": \"%s\", "
                      "\"elapsed\": %u.%09u }%s\n",
               (status == 0) ? "PASS" : "FAIL",
               name,
               (unsigned)ts3.tv_sec,
               (unsigned)ts3.tv_nsec,
               ((*count) == 1) ? "" : ",");
      buf [sizeof buf - 1] = 0;
      fprintf (stdout, "%s", buf);
      Mutex_Unlock (mutex);
   } else {
      Mutex_Lock (mutex);
      snprintf (buf, sizeof buf, "    { \"status\": \"SKIP\", \"name\": \"%s\" },\n", test->name);
      buf [sizeof buf - 1] = '\0';
      fprintf (stdout, "%s", buf);
      Mutex_Unlock (mutex);
   }

   if ((suite->flags & TEST_VERBOSE)) {
      Log_Uncork ();
   }
}


static void
TestSuite_PrintHelp (TestSuite *suite, /* IN */
                     FILE *stream)     /* IN */
{
   Test *iter;

   fprintf (stream,
"usage: %s [OPTIONS]\n"
"\n"
"Options:\n"
"    -h, --help   Show this help menu.\n"
"    -f           Do not fork() before running tests.\n"
"    -l NAME      Run test by name.\n"
"    -p           Do not run tests in parallel.\n"
"    -v           Be verbose with logs.\n"
"\n"
"Tests:\n",
            suite->prgname);

   for (iter = suite->tests; iter; iter = iter->next) {
      fprintf (stream, "    %s%s\n", suite->name, iter->name);
   }

   fprintf (stream, "\n");
}


static void
TestSuite_PrintJsonHeader (TestSuite *suite) /* IN */
{
   struct utsname u;
   uint64_t pagesize;
   uint64_t npages = 0;

   ASSERT (suite);

   if (0 != uname (&u)) {
      perror ("uname()");
      return;
   }

	pagesize = Platform_GetPageSize ();
#if defined(PLATFORM_LINUX)
   npages = sysconf (_SC_PHYS_PAGES);
#endif

   fprintf (stdout,
            "{\n"
            "  \"host\": {\n"
            "    \"sysname\": \"%s\",\n"
            "    \"release\": \"%s\",\n"
            "    \"machine\": \"%s\",\n"
            "    \"memory\": {\n"
            "      \"pagesize\": %"PRIu64",\n"
            "      \"npages\": %"PRIu64"\n"
            "    }\n"
            "  },\n"
            "  \"tests\": [\n",
            u.sysname,
            u.release,
            u.machine,
            pagesize,
            npages);

   fflush (stdout);
}


static void
TestSuite_PrintJsonFooter (void) /* IN */
{
   fprintf (stdout, "  ]\n}\n");
}


typedef struct
{
   TestSuite *suite;
   Test *test;
   Mutex *mutex;
   int *count;
} ParallelInfo;


static void *
TestSuite_ParallelWorker (void *data) /* IN */
{
   ParallelInfo *info = data;

   ASSERT (info);

   TestSuite_RunTest (info->suite, info->test, info->mutex, info->count);

   if (AtomicInt_DecrementAndTest (info->count)) {
      TestSuite_PrintJsonFooter ();
      exit (0);
   }

   return NULL;
}


static void
TestSuite_RunParallel (TestSuite *suite) /* IN */
{
   ParallelInfo *info;
   Thread *threads;
   Mutex mutex;
   Test *test;
   int count = 0;
   int i;

   ASSERT (suite);

   assert (0 == Mutex_Init (&mutex, NULL));

   for (test = suite->tests; test; test = test->next) {
      count++;
   }

   threads = Memory_SafeMalloc0 (sizeof *threads * count);

   Memory_Barrier ();

   for (test = suite->tests, i = 0; test; test = test->next, i++) {
      info = Memory_SafeMalloc0 (sizeof *info);
      info->suite = suite;
      info->test = test;
      info->count = &count;
      info->mutex = &mutex;
      Thread_Init (&threads [i], test->name, TestSuite_ParallelWorker, info);
   }

   sleep (30);

   fprintf (stderr, "Timed out, aborting!\n");

   abort ();
}


static void
TestSuite_RunSerial (TestSuite *suite) /* IN */
{
   Test *test;
   Mutex mutex;
   int count = 0;

   Mutex_Init (&mutex, NULL);

   for (test = suite->tests; test; test = test->next) {
      count++;
   }

   for (test = suite->tests; test; test = test->next) {
      TestSuite_RunTest (suite, test, &mutex, &count);
      count--;
   }

   TestSuite_PrintJsonFooter ();

   Mutex_Destroy (&mutex);
}


static void
TestSuite_RunNamed (TestSuite *suite,     /* IN */
                    const char *testname) /* IN */
{
   Mutex mutex;
   char name[128];
   Test *test;
   int count = 1;

   ASSERT (suite);
   ASSERT (testname);

   Mutex_Init (&mutex, NULL);

   for (test = suite->tests; test; test = test->next) {
      snprintf (name, sizeof name, "%s%s",
                suite->name, test->name);
      name [sizeof name - 1] = '\0';

      if (0 == strcmp (name, testname)) {
         TestSuite_RunTest (suite, test, &mutex, &count);
      }
   }

   TestSuite_PrintJsonFooter ();

   Mutex_Destroy (&mutex);
}


int
TestSuite_Run (TestSuite *suite) /* IN */
{
   if ((suite->flags & TEST_HELPONLY)) {
      TestSuite_PrintHelp (suite, stderr);
      return 0;
   }

   TestSuite_PrintJsonHeader (suite);

   if (suite->testname) {
      TestSuite_RunNamed (suite, suite->testname);
   } else if ((suite->flags & TEST_NOTHREADS)) {
      TestSuite_RunSerial (suite);
   } else {
      TestSuite_RunParallel (suite);
   }

   return 0;
}


void
TestSuite_Destroy (TestSuite *suite)
{
   Test *test;
   Test *tmp;

   for (test = suite->tests; test; test = tmp) {
      tmp = test->next;
      CString_Free (test->name);
      Memory_Free (test);
   }

   CString_Free (suite->name);
   CString_Free (suite->prgname);
   CString_Free (suite->testname);
}
