AC_PREREQ(2.60)

m4_include([build/autotools/Versions.m4])

AC_INIT([congo],[congo_version],[https://github.com/chergert/congo])
AC_CONFIG_MACRO_DIR([build/autotools/m4])
AC_CONFIG_AUX_DIR([build/autotools])
AC_SUBST(ACLOCAL_AMFLAGS, "-I build/autotools")
AC_CONFIG_HEADERS([config.h])

CONGO_MAJOR_VERSION=congo_major_version
CONGO_MINOR_VERSION=congo_minor_version
CONGO_MICRO_VERSION=congo_micro_version
CONGO_API_VERSION=1.0
CONGO_VERSION=congo_version
AC_SUBST(CONGO_MAJOR_VERSION)
AC_SUBST(CONGO_MINOR_VERSION)
AC_SUBST(CONGO_MICRO_VERSION)
AC_SUBST(CONGO_API_VERSION)
AC_SUBST(CONGO_VERSION)

m4_include([build/autotools/CheckCompiler.m4])
m4_include([build/autotools/CheckProgs.m4])
m4_include([build/autotools/CheckHost.m4])
m4_include([build/autotools/CheckTarget.m4])
m4_include([build/autotools/CheckHeaders.m4])
m4_include([build/autotools/CheckFuncs.m4])
m4_include([build/autotools/CheckStructs.m4])

m4_include([build/autotools/SetupLibtool.m4])
LT_INIT
CONGO_LT_CURRENT=lt_current
CONGO_LT_REVISION=lt_revision
CONGO_LT_AGE=lt_age
CONGO_LT_VERSION="$CONGO_LT_CURRENT:$CONGO_LT_REVISION:$CONGO_LT_AGE"
CONGO_LT_LDFLAGS="-version-info $CONGO_LT_VERSION"

m4_include([build/autotools/ReadCommandLineArguments.m4])
m4_include([build/autotools/FindDependencies.m4])
m4_include([build/autotools/AutoHarden.m4])
m4_include([build/autotools/MaintainerFlags.m4])
m4_include([build/autotools/Optimizations.m4])

# We would put AM_INIT_AUTOMAKE into SetupAutomake.m4, but seems to cause
# autoconf errors.
AM_INIT_AUTOMAKE([foreign subdir-objects tar-ustar])
AM_MAINTAINER_MODE([enable])
m4_include([build/autotools/SetupAutomake.m4])

AC_CONFIG_FILES([
    Makefile
])

m4_include([build/autotools/Libbson.m4])

m4_include([build/autotools/PrintBuildConfiguration.m4])
