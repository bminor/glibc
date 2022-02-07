GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/x86_64.

dnl Check if -mprefer-vector-width=128 works.
AC_CACHE_CHECK(-mprefer-vector-width=128, libc_cv_cc_mprefer_vector_width, [dnl
LIBC_TRY_CC_OPTION([-mprefer-vector-width=128],
		   [libc_cv_cc_mprefer_vector_width=yes],
		   [libc_cv_cc_mprefer_vector_width=no])
])
LIBC_CONFIG_VAR([config-cflags-mprefer-vector-width],
		[$libc_cv_cc_mprefer_vector_width])

if test x"$build_mathvec" = xnotset; then
  build_mathvec=yes
fi

test -n "$critic_missing" && AC_MSG_ERROR([
*** $critic_missing])
