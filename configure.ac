#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
dnl Copyright (C) 2004 Stefan Siegl, Germany

AC_PREREQ(2.59)
AC_INIT(wsdebug, 0.1, wsdebug-devel@lists.berlios.de)
AC_REVISION($Revision: 1.4 $)
AC_CONFIG_SRCDIR([debug.c])
AM_CONFIG_HEADER(config.h)

# Initialize automake
AM_INIT_AUTOMAKE(wsdebug, 0.1)

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
dnl AC_PROG_RANLIB

# Check for readline library
m4_include([readline.m4])
VL_LIB_READLINE

# Check for availibility of GNU MP library
AC_ARG_ENABLE(gnump, AC_HELP_STRING([--disable-gnump], [don't use GNU MP library]),
		     [case "${enableval}" in
		     yes) use_gnump=true;;
		     no) use_gnump=false;;
		     *) AC_MSG_ERROR(bad value ${enableval} for --disable-gnump);;
		     esac], [use_gnump=true])
if test x$use_gnump = xtrue; then
   AC_CHECK_LIB(gmp, __gmpz_init)
fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h malloc.h memory.h stdlib.h string.h termio.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
dnl AC_FUNC_MALLOC
dnl AC_FUNC_REALLOC
AC_FUNC_SETVBUF_REVERSED
AC_CHECK_FUNCS([memmove memset strtoul])

AC_OUTPUT([Makefile])
