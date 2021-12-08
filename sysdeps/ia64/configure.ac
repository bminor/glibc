GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/ia64.

dnl It is always possible to access static and hidden symbols in an
dnl position independent way.
AC_DEFINE(PI_STATIC_AND_HIDDEN)

# PIE builds fail on binutils 2.37 and earlier, see:
# https://sourceware.org/bugzilla/show_bug.cgi?id=28672
AC_DEFINE(PIE_UNSUPPORTED)
# work around problem with autoconf and empty lines at the end of files
