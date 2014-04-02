EXTRA_DIST =
noinst_LTLIBRARIES =
bin_PROGRAMS =

if WITH_LIBBSON
SUBDIRS = src/libbson
endif

SHARED_CFLAGS = \
	$(CONGO_DEBUG_CFLAGS) \
	$(MAINTAINER_CFLAGS) \
	$(BSON_CFLAGS) \
	-D_GNU_SOURCE \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/Clock \
	-I$(top_srcdir)/src/Commands \
	-I$(top_srcdir)/src/Containers \
	-I$(top_srcdir)/src/Core \
	-I$(top_srcdir)/src/Counters \
	-I$(top_srcdir)/src/File \
	-I$(top_srcdir)/src/Log \
	-I$(top_srcdir)/src/Memory \
	-I$(top_srcdir)/src/Net \
	-I$(top_srcdir)/src/Options \
	-I$(top_srcdir)/src/Random \
	-I$(top_srcdir)/src/Sched \
	-I$(top_srcdir)/src/Storage \
	-I$(top_srcdir)/src/String \
	-I$(top_srcdir)/src/Test \
	-I$(top_srcdir)/src/Threads \
	-I$(top_srcdir)/src/Tunable

SHARED_LIBS = \
	$(BSON_LIBS) \
	$(PTHREAD_LIB)

if HAVE_LTHREAD
SHARED_CFLAGS += -DTASK_USE_LTHREAD
SHARED_LIBS += -llthread
endif

SHARED_LDFLAGS = $(PTHREAD_LDFLAGS)

ACLOCAL_AMFLAGS = -I build/autotools/m4 ${ACLOCAL_FLAGS}
DISTCHECK_CONFIGURE_FLAGS = --enable-silent-rules --enable-maintainer-flags

include src/Makefile.am
include tests/Makefile.am
include tools/Makefile.am
