/* Platform.c
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


#if defined(__linux__)
# include <sys/sysinfo.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <Platform.h>

#if defined(PLATFORM_POSIX)
# include <sys/types.h>
# include <sys/sysctl.h>
# include <sys/param.h>
#endif


/*
 *--------------------------------------------------------------------------
 *
 * Platform_GetCpuCount --
 *
 *       Returns the number of CPUs on the system. A hyperthreaded core
 *       will result in 2, not 1 as that is what we get back from the
 *       system.
 *
 * Returns:
 *       A non-zero positive integer.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

unsigned
Platform_GetCpuCount (void)
{
#if defined(__linux__)
   return get_nprocs ();
#elif defined(__FreeBSD__) || \
      defined(__NetBSD__) || \
      defined(__DragonFly__) || \
      defined(__OpenBSD__)
   int mib[2];
   int maxproc;
   size_t len;

   mib[0] = CTL_HW;
   mib[1] = HW_NCPU;
   len = sizeof (maxproc);

   if (-1 == sysctl (mib, 2, &maxproc, &len, NULL, 0)) {
      return 1;
   }

   return len;
#elif defined(__APPLE__) || defined(__sun)
   int ncpu;

   ncpu = sysconf (_SC_NPROCESSORS_ONLN);
   return (ncpu > 0) ? ncpu : 1;
#elif defined(_MSC_VER) || defined(_WIN32)
   SYSTEM_INFO si;
   GetSystemInfo (&si);
   return si.dwNumberOfProcessors;
#else
# warning "Platform_GetCpuCount() not supported, defaulting to 1."
   return 1;
#endif
}


/*
 *--------------------------------------------------------------------------
 *
 * Platform_GetPageSize --
 *
 *       Returns the effective page size for the process. This is typically
 *       4KB or 8KB, however may be larger if HUGEPAGES are used.
 *
 * Returns:
 *       The process page size in bytes.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

size_t
Platform_GetPageSize (void)
{
#if defined(PLATFORM_POSIX)
   return sysconf (_SC_PAGESIZE);
#elif defined(PLATFORM_WIN32)
   SYSTEM_INFO si;
   GetSystemInfo (&si);
   return si.dwPageSize;
#else
# error "Please implement Platform_GetPageSize() for your platform."
#endif
}


/*
 *--------------------------------------------------------------------------
 *
 * Platform_GetTempDir --
 *
 *       Returns the temporary directory to be used.
 *
 *       On windows this is the "TEMP" environment variable. On Posix
 *       systems this is "TMPDIR" or "/tmp" if "TMPDIR" is not specified.
 *
 * Returns:
 *       A string that should not be modified or freed.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

const char *
Platform_GetTempDir (void)
{
   static char *tempdir;

   if (!tempdir) {
#ifdef PLATFORM_POSIX
      char *dir = getenv ("TMPDIR");
      if (!dir) {
         dir = P_tmpdir;
         if (!dir) {
            dir = "/tmp";
         }
      }
      tempdir = dir;
#elif defined(PLATFORM_WIN32)
      tempdir = getenv ("TEMP");
#else
# warning "Unknown platform, using /tmp for temporary directory."
      tempdir = "/tmp";
#endif
   }

   return tempdir;
}
