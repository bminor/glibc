dnl We define the macro GLIBC_PROVIDES to do an AC_PROVIDE for each macro
dnl which appears in configure.in before the sysdep configure scripts are run.
dnl Each sysdep configure.in does GLIBC_PROVIDES first, to avoid any
dnl AC_REQUIREs or AC_BEFOREs duplicating their code.
dnl
define([GLIBC_PROVIDES], [dnl
AC_PROVIDE([AC_PROG_INSTALL])dnl
AC_PROVIDE([AC_PROG_RANLIB])dnl
AC_PROVIDE([AC_PROG_CC])dnl
AC_PROVIDE([AC_PROG_CPP])dnl
# This file is generated from configure.in by Autoconf.  DO NOT EDIT!
])dnl
