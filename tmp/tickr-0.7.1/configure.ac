#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([tickr], [0.7.1], [manutm007@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset select socket strerror strncasecmp strtoul])

PKG_CHECK_MODULES(GTK2, gtk+-2.0, ,)
PKG_CHECK_MODULES(XML2, libxml-2.0, ,)
PKG_CHECK_MODULES(GNUTLS, gnutls, ,)
PKG_CHECK_MODULES(FRIBIDI, fribidi, ,)

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/tickr/Makefile
                 src/libetm-0.5.0/Makefile])
AC_OUTPUT
