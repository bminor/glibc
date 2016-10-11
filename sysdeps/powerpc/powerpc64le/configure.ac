GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/powerpc/powerpc64le.

dnl Require __float128 support on powerpc64le (available in GCC 6.2).
AC_CACHE_CHECK([if $CC is sufficient to build libc for powerpc64le],
	       libc_cv_compiler_powerpc64le_compiler_ok, [
AC_PREPROC_IFELSE([AC_LANG_SOURCE([[
#if !defined __GNUC__ || __GNUC__ < 6 || (__GNUC__ == 6 && __GNUC_MINOR__ < 2)
#error insufficient compiler
#endif
]])],
		  [libc_cv_compiler_powerpc64le_compiler_ok=yes],
		  [libc_cv_compiler_powerpc64le_compiler_ok=no])])
AS_IF([test "$libc_cv_compiler_powerpc64le_compiler_ok" != "yes"],
      [critic_missing="$critic_missing GCC >= 6.2 is required on powerpc64le."])

test -n "$critic_missing" && AC_MSG_ERROR([*** $critic_missing])
