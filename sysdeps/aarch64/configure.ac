GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/aarch64.

# Static PIE is supported.
AC_DEFINE(SUPPORT_STATIC_PIE)

# We check to see if the compiler and flags are
# selecting the big endian ABI and if they are then
# we set libc_cv_aarch64_be to yes which causes
# HAVE_AARCH64_BE to be defined in config.h and
# in include/libc-symbols.h and thus available to
# shlib-versions to select the appropriate name for
# the dynamic linker via %ifdef.
AC_CACHE_CHECK([for big endian],
  [libc_cv_aarch64_be],
  [AC_EGREP_CPP(yes,[#ifdef __AARCH64EB__
                      yes
                     #endif
  ], libc_cv_aarch64_be=yes, libc_cv_aarch64_be=no)])
if test $libc_cv_aarch64_be = yes; then
  AC_DEFINE(HAVE_AARCH64_BE)
  LIBC_CONFIG_VAR([default-abi], [lp64_be])
else
  LIBC_CONFIG_VAR([default-abi], [lp64])
fi

if test x"$build_mathvec" = xnotset; then
  build_mathvec=yes
fi

if test $build_mathvec = no; then
  AC_MSG_WARN([mathvec is disabled, this results in incomplete ABI.])
fi

libc_cv_support_sframe=yes
