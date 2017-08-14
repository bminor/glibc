GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/aarch64.

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

if test $aarch64_config_abi = ilp32; then
  AC_DEFINE(HAVE_AARCH64_ILP32)
fi

if test $libc_cv_aarch64_be = yes; then
  AC_DEFINE(HAVE_AARCH64_BE)
  libc_aarch64_be=_be
else
  libc_aarch64_be=
fi

LIBC_CONFIG_VAR([default-abi], [${aarch64_config_abi}${libc_aarch64_be}])
