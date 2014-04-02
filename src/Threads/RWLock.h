/* RWLock.h
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


#ifndef RW_LOCK_H
#define RW_LOCK_H


#if defined(PLATFORM_POSIX)
# include <pthread.h>
#endif

#include <Macros.h>
#include <Platform.h>
#include <Types.h>


BEGIN_DECLS


#if defined(PLATFORM_POSIX)
# define RWLock                pthread_rwlock_t
# define RWLockAttr            pthread_rwlock_attr_t
# define RWLock_Init           pthread_rwlock_init
# define RWLock_ReadLock       pthread_rwlock_rdlock
# define RWLock_TryReadLock    pthread_rwlock_tryrdlock
# define RWLock_WriteLock      pthread_rwlock_wrlock
# define RWLock_TryWriteLock   pthread_rwlock_trywrlock
# define RWLock_Unlock         pthread_rwlock_unlock
#endif


END_DECLS


#endif /* RW_LOCK_H */
