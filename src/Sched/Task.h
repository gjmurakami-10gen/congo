/* Task.h
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


#ifndef TASK_H
#define TASK_H


#include <Macros.h>


BEGIN_DECLS


#ifndef TASK_USE_LTHREAD
# include <pthread.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <unistd.h>
typedef pthread_t Task;
# define Task_Detach              pthread_detach
# define Task_Create(t,f,d)       pthread_create(t,NULL,(void *(*)(void*))f,d)
# define Task_Current             pthread_self
# define Task_Sleep(s)            usleep((s) * 1000UL)
# define Task_Socket              socket
# define Task_Accept              accept
# define Task_Close               close
# define Task_Connect(f,a,l,t)    connect(f,a,l)
# define Task_Read(f,b,s,fl)      read(f,b,s)
# define Task_Recv(f,b,s,fl,t)    recv(f,b,s,fl)
# define Task_Send                send
# define Task_SendMsg             sendmsg
# define Task_Write               write
# define Task_BeginBlockingCall()
# define Task_EndBlockingCall()
#else
#include <lthread.h>
typedef struct lthread * Task;
# define Task_Detach             lthread_detach
# define Task_Create             lthread_create
# define Task_Current            lthread_current
# define Task_Sleep              lthread_sleep
# define Task_Socket             lthread_socket
# define Task_Accept             lthread_accept
# define Task_Close              lthread_close
# define Task_Connect            lthread_connect
# define Task_Read               lthread_read
# define Task_Recv               lthread_recv
# define Task_Send               lthread_send
# define Task_SendMsg            lthread_sendmsg
# define Task_Write              lthread_write
# define Task_BeginBlockingCall  lthread_compute_begin
# define Task_EndBlockingCall    lthread_compute_end
#endif


END_DECLS


#endif /* TASK_H */
