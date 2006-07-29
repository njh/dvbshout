
dnl Require autoconf version >= 2.57
AC_PREREQ(2.57)


dnl ############# Initialization

AC_INIT([dvbshout], [0.3], [njh@ecs.soton.ac.uk])

AC_CONFIG_SRCDIR(src/dvbshout.c)
AC_CANONICAL_SYSTEM

dnl Version 1.7 of automake is recommended
AM_INIT_AUTOMAKE( 1.7 )
AM_CONFIG_HEADER(src/config.h)


dnl ############# Compiler and tools Checks

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_C_INLINE


dnl ############## Library Checks

# Check for libshout
PKG_CHECK_MODULES(libshout, shout >= 2.2)
CFLAGS="$CFLAGS $libshout_CFLAGS"
LIBS="$LIBS $libshout_LIBS"

# Check for ortp
PKG_CHECK_MODULES(oRTP, ortp >= 0.9.1)
CFLAGS="$CFLAGS $oRTP_CFLAGS"
LIBS="$LIBS $oRTP_LIBS"


dnl ############## Header Checks

AC_HEADER_STDC
AC_CHECK_HEADER([linux/dvb/frontend.h])
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])


dnl ############## Final Output

CFLAGS="$CFLAGS -g -Wall -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE"

AC_OUTPUT([Makefile src/Makefile])

