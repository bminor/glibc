GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/unix/sysv/linux/aarch64.

if test $aarch64_config_abi = ilp32; then
  arch_minimum_kernel=10.0.0
  LIBC_SLIBDIR_RTLDDIR([libilp32], [lib])
else
  arch_minimum_kernel=3.7.0
  LIBC_SLIBDIR_RTLDDIR([lib64], [lib])
fi

ldd_rewrite_script=$dir/ldd-rewrite.sed
