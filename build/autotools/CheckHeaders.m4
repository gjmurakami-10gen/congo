AC_HEADER_STDBOOL

AC_CHECK_HEADERS([sys/statfs.h])

AC_CHECK_HEADERS([lthread.h])
AM_CONDITIONAL(HAVE_LTHREAD, [test "$ac_cv_header_lthread_h" = yes])
