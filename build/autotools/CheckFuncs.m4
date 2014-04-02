need_librt=0

# Check if -lrt is required for clock_gettime().
AC_SEARCH_LIBS([clock_gettime], [rt],
               [AC_DEFINE(HAVE_CLOCK_GETTIME, 1, [Have clock_gettime])])
AS_IF([test "$ac_cv_search_clock_gettime" = "-lrt"],
      [need_librt=1])

# Check for shm functions.
AC_SEARCH_LIBS([shm_open], [rt],
               [AC_DEFINE(HAVE_SHM_OPEN, 1, [Have shm_open])])
AS_IF([test "$ac_cv_search_shm_open" = "-lrt"],
      [need_librt=1])

# Link against librt if shm_open or clock_gettime require it.
AS_IF([test "x$need_librt" = xyes],
      [LDFLAGS="$LDFLAGS -lrt"])

# Check for sched_getcpu
AC_CHECK_FUNCS([sched_getcpu])

# Check for various functions we can take advantage of
AC_HAVE_FUNCS([fallocate fdatasync ftruncate mmap munmap posix_fadvise \
               posix_fallocate pthread_setname_np pthread_yield \
               pthread_yield_np shm_open shm_unlink])
