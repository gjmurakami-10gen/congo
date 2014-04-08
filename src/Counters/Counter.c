/* Counter.c
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
#include <stdio.h>
#include <string.h>
#if !defined(_WIN32)
# include <sys/mman.h>
# include <sys/shm.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <Counter.h>
#include <Debug.h>
#include <Memory.h>
#include <Platform.h>
#include <ThreadOnce.h>
#include <Types.h>


#define COUNTERS_MAGIC     11552277
#define COUNTERS_MAX_PAGES 100


typedef struct
{
   Counter  **counters;
   unsigned   len;
   unsigned   initialized : 1;
   unsigned   did_malloc : 1;
   uint8_t   *mem;
   size_t     memsize;
} Counters;


#pragma pack(push, 1)
typedef struct
{
   uint32_t magic;
   uint32_t size;
   uint32_t len;
   char     padding [116];
} CountersHeader;

typedef struct
{
   uint32_t offset;
   char     category [28];
   char     name [32];
   char     description [64];
} CounterInfo;
#pragma pack(pop)


STATIC_ASSERT (sizeof (CounterInfo) == 128);
STATIC_ASSERT (sizeof (CounterValue) == 64);


static Counters   gCounters;
static pid_t      gCountersPid;
static ThreadOnce gCountersOnce = THREAD_ONCE_INIT;


/*
 *--------------------------------------------------------------------------
 *
 * Counters_Destroy --
 *
 *       Cleanup after runtime counters.
 *
 *       This is normally called atexit() time. It will remove the shared
 *       memory segment if it has been allocated.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static void
Counters_Destroy (void)
{
#if defined(PLATFORM_POSIX)
   char name [32];
   int pid;

   pid = getpid ();

   snprintf (name, sizeof name, "/Counters-%u", pid);
   name [sizeof name - 1] = '\0';
   shm_unlink (name);
#endif

   if (gCounters.did_malloc) {
      Memory_Free (gCounters.mem);
#if defined(PLATFORM_POSIX)
   } else {
      munmap (gCounters.mem, gCounters.memsize);
      gCounters.mem = NULL;
      gCounters.memsize = 0;
#endif
   }

   Memory_Free (gCounters.counters);
   gCounters.counters = NULL;
   gCounters.len = 0;
}


/*
 *--------------------------------------------------------------------------
 *
 * Counters_AllocBuffer --
 *
 *       Allocates a buffer that can store all of the information about
 *       counters as well as the numeric values for counters.
 *
 *       If shm failed to allocate, malloc will be used.
 *
 * Returns:
 *       A mmap() or malloc() based buffer.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static void *
Counters_AllocBuffer (size_t size) /* IN */
{
   void *mem;
#if defined(PLATFORM_POSIX)
   char name [32];
   int pid;
   int fd;

   if (getenv ("COUNTERS_DISABLE_SHM")) {
      goto use_malloc;
   }

   pid = getpid ();
   snprintf (name, sizeof name, "/Counters-%u", pid);
   name [sizeof name - 1] = '\0';

   if (-1 == (fd = shm_open (name, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP))) {
      goto use_malloc;
   }

   /*
    * ftruncate() will cause reads to be zero. Therefore, we don't need to
    * do write() of zeroes to initialize the shared memory area.
    */
   if (-1 == ftruncate (fd, size)) {
      goto failure;
   }

   /*
    * Memory map the shared memory segement so that we can store our counters
    * within it. We need to layout the counters into the segment so that other
    * processes can traverse and read the values by loading the shared page.
    */
   mem = mmap (NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
   if (mem == MAP_FAILED) {
      goto failure;
   }

   close (fd);
   memset (mem, 0, size);
   atexit (Counters_Destroy);

   return mem;

failure:
   shm_unlink (name);
   close (fd);

use_malloc:
#else
#endif
   gCounters.did_malloc = 1;
   mem = Memory_SafeMalloc0 (size);
   return mem;
}


/*
 *--------------------------------------------------------------------------
 *
 * Counters_LayoutInAlloc --
 *
 *       Lays the counters out within the counter allocation.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static void
Counters_LayoutInAlloc (Counters *counters) /* IN */
{
   CountersHeader *hdr;
   CounterInfo info;
   size_t off = sizeof (CountersHeader);
   size_t ctr_off;
   int cpucount;
   int i;
   int j;

   ASSERT (counters);

   cpucount = Platform_GetCpuCount ();
   ctr_off = off + (sizeof (CounterInfo) * counters->len);
   memset (counters->mem + ctr_off, 0, counters->memsize - ctr_off);

   for (i = 0, j = 0; i < counters->len; i++, j++) {
      strncpy (info.category,
               counters->counters [i]->category,
               sizeof (info.category));
      strncpy (info.name,
               counters->counters [i]->name,
               sizeof (info.name));
      strncpy (info.description,
               counters->counters [i]->description,
               sizeof (info.description));

      info.category [sizeof info.category - 1] = '\0';
      info.name [sizeof info.name - 1] = '\0';
      info.description [sizeof info.description - 1] = '\0';
      info.offset = ctr_off + (j * 8);

      /*
       * The following requires an alignment of a pointer (so 8 on 64-bit).
       * Since we do everything cache-line aligned, this should always be
       * the case.
       */
      ASSERT ((((size_t)(counters->mem + ctr_off)) % 8) == 0);
      counters->counters [i]->values =
         (CounterValue *)(void *)(counters->mem + info.offset);

      if (j == 8) {
         ctr_off += cpucount * sizeof (CounterValue);
         j = 0;
      }

      memcpy (counters->mem + off, &info, sizeof info);
      off += sizeof info;
   }

   hdr = (CountersHeader *)counters->mem;
   hdr->magic = COUNTERS_MAGIC;
   hdr->size = counters->memsize;

   Memory_Barrier ();

   hdr->len = counters->len;

   Memory_Barrier ();
}


/*
 *--------------------------------------------------------------------------
 *
 * Counters_DoInitRemote --
 *
 *       Performs initialization of counters accessing a remote process.
 *       This uses the pid_t to try to connect to the shared memory
 *       segment of the remote process.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static void
Counters_DoInitRemote (void)
{
#if defined(PLATFORM_POSIX)
   uint32_t magic = 0;
   uint32_t len = 0;
   void *mem;
   char name [32];
   int fd;

   ASSERT (gCountersPid);

   snprintf (name, sizeof name, "/Counters-%u", (int)gCountersPid);
   name [sizeof name - 1] = '\0';

   if (-1 == (fd = shm_open (name, O_RDONLY, 0))) {
      perror ("Failed to load shared memory segment");
      return;
   }

   if ((4 != pread (fd, &magic, 4, 0)) || (magic != COUNTERS_MAGIC)) {
      perror ("Shared memory segment contains invalid magic");
      close (fd);
      return;
   }

   if (4 != pread (fd, &len, 4, 4)) {
      perror ("Shared memory segment contains invalid length");
      close (fd);
      return;
   }

   if ((len < Platform_GetPageSize ()) ||
       (len > (Platform_GetPageSize () * COUNTERS_MAX_PAGES))) {
      fprintf (stderr, "Shared memory segment is too large!\n");
      close (fd);
      return;
   }

   if (MAP_FAILED == (mem = mmap (NULL, len, PROT_READ, MAP_SHARED, fd, 0))) {
      perror ("Failed to mmap() shared memory segment.");
      close (fd);
      return;
   }

   gCounters.mem = mem;
   gCounters.memsize = len;

   atexit (Counters_Destroy);
   close (fd);
#else
   LOG_WARNING ("Remote counters are not supported on Windows.");
   atexit (Counters_Destroy);
#endif
}


/*
 *--------------------------------------------------------------------------
 *
 * Counters_DoInit --
 *
 *       Initializes counter for the local process.
 *
 *       Currently, accessing counters locally and a remote process in the
 *       same process space is not supported. (Patches Accepted).
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static void
Counters_DoInit (void)
{
   size_t size = 0;
   int pagesize;
   int ncounters;
   int ncpu;
   int ngroups;

   ASSERT (!gCountersPid);

   ncpu = Platform_GetCpuCount ();
   ncounters = gCounters.len;
   ngroups = (gCounters.len / 8) + 1;
   pagesize = Platform_GetPageSize ();

   size = (sizeof (CountersHeader) +
           (ncounters * sizeof (CounterInfo)) +
           (ncpu * ngroups * sizeof (CounterValue)));
   size = ((size / pagesize) + 1) * pagesize;

   gCounters.mem = Counters_AllocBuffer (size);
   gCounters.memsize = size;

   Counters_LayoutInAlloc (&gCounters);

   gCounters.initialized = 1;
}


/*
 *--------------------------------------------------------------------------
 *
 * Counters_Init --
 *
 *       Initialize the counter infrastructure. This should be called after
 *       all of the counters have been registered (which is normally done
 *       automatically with __attribute__((constructor)).
 *
 *       This MUST be called before calling MyCounter_Increment() or any
 *       other counter mutation function.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
Counters_Init (void) /* IN */
{
   ThreadOnce_Once (&gCountersOnce, Counters_DoInit);
}


/*
 *--------------------------------------------------------------------------
 *
 * Counters_InitRemote --
 *
 *       Loads the remote shared memory instance for a given process
 *       containing counters.
 *
 *       This can currently only be called once, and cannot be mixed
 *       with Counters_Init() meaning you can either have local counters
 *       or remote counters in your process.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
Counters_InitRemote (pid_t pid) /* IN */
{
   gCountersPid = pid;
   ThreadOnce_Once (&gCountersOnce, Counters_DoInitRemote);
}


/*
 *--------------------------------------------------------------------------
 *
 * Counters_Foreach --
 *
 *       Iterate all of the counters that have been registered.
 *
 *       @func will be called for every counter.
 *       @user_data will be provided to @func.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
Counters_Foreach (CounterForeachFunc  func, /* IN */
                  void *user_data)          /* IN */
{
   CountersHeader *hdr;
   CounterInfo *info;
   Counter ctr;
   int i;

   ASSERT (func);

   if (!gCountersPid) {
      for (i = 0; i < gCounters.len; i++) {
         func (gCounters.counters [i], user_data);
      }
   } else {
      hdr = (CountersHeader *)gCounters.mem;

      /*
       * TODO: We should probably be careful here not to jump past the
       *       potential end of the map.
       */

      for (i = 0; i < hdr->len; i++) {
         info = (CounterInfo *)(gCounters.mem +
                                sizeof (CountersHeader) +
                                (sizeof (CounterInfo) * i));
         ctr.category = info->category;
         ctr.name = info->name;
         ctr.description = info->description;
         ASSERT ((((size_t)(gCounters.mem + info->offset)) % 8) == 0);
         ctr.values = (CounterValue *)(void *)(gCounters.mem + info->offset);
         func (&ctr, user_data);
      }
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * Counter_Register --
 *
 *       Register a counter in the system. This may only be called
 *       BEFORE Counters_Init() has been called. It is normally done for
 *       you via the COUNTER() macro using __attribute__((constructor)).
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
Counter_Register (Counter *counter) /* IN */
{
   size_t size;

   ASSERT (counter);
   ASSERT (!gCounters.initialized);

   size = (gCounters.len + 1) * sizeof (void *);
   gCounters.counters = Memory_SafeRealloc (gCounters.counters, size);
   gCounters.counters [gCounters.len++] = counter;
}


/*
 *--------------------------------------------------------------------------
 *
 * Counter_Get --
 *
 *       Fetch the value of a counter. This is done by performing a
 *       volatile read among all of the cachelines for the counter.
 *
 * Returns:
 *       A 64-bit integer containing the current counter value.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

int64_t
Counter_Get (const Counter *counter) /* IN */
{
   int64_t value = 0;
   int ncpu;
   int i;

   ASSERT (counter);

   ncpu = Platform_GetCpuCount ();

   for (i = 0; i < ncpu; i++) {
      value += counter->values [i].value;
   }

   return value;
}


/*
 *--------------------------------------------------------------------------
 *
 * Counter_Reset --
 *
 *       Reset a counter back to zero.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
Counter_Reset (Counter *counter) /* IN */
{
   int ncpu;
   int i;

   ASSERT (counter);

   ncpu = Platform_GetCpuCount ();

   for (i = 0; i < ncpu; i++) {
      AtomicInt64_Set (&counter->values [i].value, 0);
   }
}
