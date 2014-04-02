/* Platform.h
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


#ifndef PLATFORM_H
#define PLATFORM_H


#if defined(__linux__)
# include <sched.h>
#elif defined(__sun)
# include <sys/processor.h>
#endif
#include <sys/types.h>
#include <unistd.h>


#include <Macros.h>
#include <Types.h>


BEGIN_DECLS


#if defined(_MSC_VER) || defined(_WIN32)
# define PLATFORM_WIN32 1
#elif defined(__linux__)
# define PLATFORM_LINUX 1
#elif defined(__APPLE__)
# define PLATFORM_APPLE 1
#elif defined(__FreeBSD__)
# define PLATFORM_FREEBSD 1
#elif defined(__NetBSD__)
# define PLATFORM_NETBSD 1
#elif defined(__OpenBSD__)
# define PLATFORM_OPENBSD 1
#elif defined(__DragonFly__)
# define PLATFORM_DRAGONFLY 1
#elif defined(__sun)
# define PLATFORM_SUN 1
#endif


#if defined(PLATFORM_LINUX) || \
    defined(PLATFORM_APPLE) || \
    defined(PLATFORM_SUN) || \
    defined(PLATFORM_FREEBSD) || \
    defined(PLATFORM_OPENBSD) || \
    defined(PLATFORM_NETBSD) || \
    defined(PLATFORM_DRAGONFLY)
# define PLATFORM_POSIX 1
#endif


#if defined(PLATFORM_POSIX) && defined(PLATFORM_WIN32)
# error "Invalid platform detected, both posix and win32."
#endif

#if !defined(PLATFORM_POSIX) && !defined(PLATFORM_WIN32)
# error "You're platform is not currently supported."
#endif


#define HAVE_PLATFORM_GETCURRENTCPU 1
#if defined(ENABLE_RDTSCP)
static __inline__ unsigned
Platform_GetCurrentCpu (void)
{
   uint32_t rax, rdx, aux;
   __asm__ volatile ("rdtscp\n" : "=a" (rax), "=d" (rdx), "=c" (aux) : : );
   return aux;
}
#elif defined(__linux__)
static __inline__ volatile unsigned
Platform_GetCurrentCpu (void)
{
   return sched_getcpu ();
}
#elif defined(__sun)
static __inline__ volatile unsigned
Platform_GetCurrentCpu (void)
{
   return getcpuid ();
}
#elif defined(_MSC_VER) || defined(_WIN32)
static __inline__ volatile unsigned
Platform_GetCurrentCpu (void)
{
   return GetCurrentProcessorNumber ();
}
#else
#undef HAVE_PLATFORM_GETCURRENTCPU
#endif


unsigned    Platform_GetCpuCount (void) GNUC_CONST;
size_t      Platform_GetPageSize (void) GNUC_CONST;
const char *Platform_GetTempDir  (void) GNUC_CONST;


END_DECLS


#endif /* PLATFORM_H */
