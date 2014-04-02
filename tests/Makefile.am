noinst_PROGRAMS = test-congo
TEST_PROGS = test-congo


test_congo_CFLAGS = $(SHARED_CFLAGS)
test_congo_SOURCES = \
	tests/CoreTests.c \
	tests/MutatorTests.c \
	tests/test-congo.c

test_congo_LDADD = libCongo.la


test: $(TEST_PROGS)
	@ for TEST_PROG in $(TEST_PROGS) ; do \
		./$$TEST_PROG ; \
	done


debug: test-congo
	$(LIBTOOL) --mode=execute gdb --args test-congo -f -p
