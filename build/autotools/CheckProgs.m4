AC_PATH_PROG(MV, mv)
if test -z "$MV"; then
    AC_MSG_ERROR([You need 'mv' to compile libbson])
fi

AC_PATH_PROG(GREP, grep)
if test -z "$GREP"; then
    AC_MSG_ERROR([You need 'grep' to compile libbson])
fi

AC_PROG_INSTALL
