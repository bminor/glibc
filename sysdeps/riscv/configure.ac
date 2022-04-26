GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/riscv/elf.

# Check if static linker supports R_RISCV_ALIGN
AC_CACHE_CHECK([for R_RISCV_ALIGN linker relaxation support], [libc_cv_riscv_r_align],[dnl
  cat > conftest.S <<EOF
.align 3
foo:
  li a0,42
  ret
EOF
  libc_cv_riscv_r_align=no
  if AC_TRY_COMMAND([${CC-cc} $CFLAGS $CPPFLAGS $LDFLAGS -nostdlib -nostartfiles $no_ssp -shared -fPIC -o contests.o conftest.S])
  then
    libc_cv_riscv_r_align=yes
  fi
  rm -rf conftest.*])
LIBC_CONFIG_VAR([riscv-r-align], [$libc_cv_riscv_r_align])
