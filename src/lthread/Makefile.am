libCongo_la_SOURCES += \
	src/lthread/lthread.c \
	src/lthread/lthread_compute.c \
	src/lthread/lthread.h \
	src/lthread/lthread_int.h \
	src/lthread/lthread_io.c \
	src/lthread/lthread_poller.h \
	src/lthread/lthread_sched.c \
	src/lthread/lthread_socket.c \
	src/lthread/queue.h \
	src/lthread/tree.h

if OS_LINUX
libCongo_la_SOURCES += src/lthread/lthread_epoll.c
endif

if OS_FREEBSD
libCongo_la_SOURCES += src/lthread/lthread_kqueue.c
endif

if OS_DARWIN
libCongo_la_SOURCES += src/lthread/lthread_kqueue.c
endif
