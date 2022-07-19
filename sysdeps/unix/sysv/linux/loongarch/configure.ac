sinclude(./aclocal.m4)dnl Autoconf lossage
GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/unix/sysv/linux/loongarch.

arch_minimum_kernel=5.19.0

libc_cv_loongarch_int_abi=no
AC_EGREP_CPP(4 8 8, [__SIZEOF_INT__ __SIZEOF_LONG__ __SIZEOF_POINTER__
  ], libc_cv_loongarch_int_abi=lp64)
if test $libc_cv_loongarch_int_abi = no; then
  AC_MSG_ERROR([Unable to determine integer ABI])
fi

LIBC_CONFIG_VAR([default-abi], [$libc_cv_loongarch_int_abi])

case $libc_cv_loongarch_int_abi in
lp64)
  LIBC_SLIBDIR_RTLDDIR([lib64], [lib64])
  ;;
esac

ldd_rewrite_script=sysdeps/unix/sysv/linux/loongarch/ldd-rewrite.sed
