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
if test $libc_cv_aarch64_be = yes; then
  AC_DEFINE(HAVE_AARCH64_BE)
  LIBC_CONFIG_VAR([default-abi], [lp64_be])
else
  LIBC_CONFIG_VAR([default-abi], [lp64])
fi

AC_CACHE_CHECK([for pcs attribute support],
               libc_cv_gcc_pcs_attribute, [dnl
cat > conftest.c <<EOF
__attribute__((aarch64_vector_pcs)) extern void foo (void);
EOF
libc_cv_gcc_pcs_attribute=no
if ${CC-cc} -c -Wall -Werror conftest.c -o conftest.o 1>&AS_MESSAGE_LOG_FD \
   2>&AS_MESSAGE_LOG_FD ; then
  libc_cv_gcc_pcs_attribute=yes
fi
rm -f conftest*])

# Enable libmvec by default.
if test x"$build_mathvec" = xnotset; then
  build_mathvec=yes
fi

# Only test libmvec if the compiler supports aarch64_vector_pcs.
if test x"$build_mathvec" = xyes; then
  if test $libc_cv_gcc_pcs_attribute = yes; then
    LIBC_CONFIG_VAR([test-mathvec], [yes])
  fi
fi
