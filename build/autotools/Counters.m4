# Enable fast counters with rdtscp
AS_IF([test "$enable_rdtscp" = "yes"],
      [AC_DEFINE(ENABLE_RDTSCP, [1], [Use rdtscp instruction for faster counters.])])
