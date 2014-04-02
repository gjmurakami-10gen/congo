AC_CHECK_MEMBER([struct statfs.f_fstypename],
                [AC_DEFINE(HAVE_STRUCT_STATFS_F_FSTYPENAME, 1, [If we have struct statfs f_fstypename])],
                [],
                [#include <sys/statfs.h>])

AC_CHECK_MEMBER([struct statfs.f_type],
                [AC_DEFINE(HAVE_STRUCT_STATFS_F_TYPE, 1, [If we have struct statfs f_type])],
                [],
                [#include <sys/statfs.h>])

AS_IF([test "$ac_cv_member_struct_statfs_f_fstypename" != yes && test "$ac_cv_member_struct_statfs_f_type" != yes],
      [AC_MSG_ERROR([

m4_text_box([Congo requires <sys/statfs.h> with either statfs.f_fstypename or statfs.f_type.])
])])
