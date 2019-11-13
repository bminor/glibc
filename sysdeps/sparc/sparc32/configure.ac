GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/sparc/sparc32

# Test if compiler targets at least sparcv8.
AC_CACHE_CHECK([for at least sparcv8 support],
  [libc_cv_sparcv8],
  [AC_EGREP_CPP(yes,[#if defined (__sparc_v8__) || defined (__sparc_v9__)
                      yes
                     #endif
  ], libc_cv_sparcv8=yes, libc_cv_sparcv8=no)])
if test $libc_cv_sparcv8 = no; then
  AC_MSG_ERROR([no support for pre-v8 sparc])
fi
