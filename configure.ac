#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
dnl Copyright (C) 2004 Stefan Siegl, Germany

AC_PREREQ(2.59)
AC_INIT("wsdebug", 0.1, wsdebug-devel@lists.berlios.de)
AC_REVISION($Revision: 1.1 $)
AC_CONFIG_SRCDIR([debug.c])
AM_CONFIG_HEADER(config.h)

# Initialize automake
AM_INIT_AUTOMAKE(wsdebug, 0.1)

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
dnl AC_PROG_RANLIB

# Checks for libraries.

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
