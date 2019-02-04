GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/powerpc/powerpc64le.

dnl Require at least POWER8 on powerpc64le
OLD_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS $libc_cv_cc_submachine"
AC_CACHE_CHECK([if the target machine is at least POWER8],
	       libc_cv_target_power8_ok, [
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
#ifndef _ARCH_PWR8
#error invalid target architecture
#endif
]])],
	       [libc_cv_target_power8_ok=yes],
	       [libc_cv_target_power8_ok=no])])
AS_IF([test "$libc_cv_target_power8_ok" != "yes"],
      [critic_missing="$critic_missing POWER8 or newer is required on powerpc64le."])
CFLAGS="$OLD_CFLAGS"

test -n "$critic_missing" && AC_MSG_ERROR([*** $critic_missing])
