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

# Only consider BTI supported if -mbranch-protection=bti is
# on by default in the compiler and the linker produces
# binaries with GNU property notes in PT_GNU_PROPERTY segment.
AC_CACHE_CHECK([for BTI support], [libc_cv_aarch64_bti], [dnl
  cat > conftest.c <<EOF
void foo (void) { }
EOF
  libc_cv_aarch64_bti=no
  if AC_TRY_COMMAND([${CC-cc} $CFLAGS $CPPFLAGS $LDFLAGS -nostdlib -nostartfiles $no_ssp -shared -fPIC -o conftest.so conftest.c]) \
     && AC_TRY_COMMAND([$READELF -lW conftest.so | grep -q GNU_PROPERTY]) \
     && AC_TRY_COMMAND([$READELF -nW conftest.so | grep -q "NT_GNU_PROPERTY_TYPE_0.*AArch64 feature:.* BTI"])
  then
    libc_cv_aarch64_bti=yes
  fi
  rm -rf conftest.*])
if test $libc_cv_aarch64_bti = yes; then
  AC_DEFINE(HAVE_AARCH64_BTI)
fi
