GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/loongarch/elf.

dnl It is always possible to access static and hidden symbols in an
dnl position independent way.
AC_DEFINE(HIDDEN_VAR_NEEDS_DYNAMIC_RELOC)

dnl Test if the toolchain is new enough for static PIE.
dnl We need a GAS supporting explicit reloc (older GAS produces stack-based
dnl reloc and triggers an internal error in the linker).  And, we need GCC to
dnl pass the correct linker flags for static PIE.  GCC >= 13 and GAS >= 2.40
dnl satisfy the requirement, but a distro may backport static PIE support into
dnl earlier GCC or Binutils releases as well.
AC_CACHE_CHECK([if the toolchain is sufficient to build static PIE on LoongArch],
libc_cv_static_pie_on_loongarch, [
  cat > conftest1.S <<\EOF
.global _start
.type _start, @function
_start:
  li.w $a7, 93
  /* This ensures the assembler supports explicit reloc.  */
  pcalau12i $a0, %pc_hi20(x)
  ld.w $a0, $a0, %pc_lo12(x)
  syscall 0

.data
x:
  .word 0
  /* This should produce an R_LARCH_RELATIVE in the static PIE.  */
  .dword _start
EOF
  cat > conftest2.S <<\EOF
.global f
.type f, @function
f:
  /* The linker should be able to handle this and produce a PLT entry.  */
  la.pcrel $t0, $t0, external_func
  jirl $zero, $t0, 0
EOF

  libc_cv_static_pie_on_loongarch=no
  if AC_TRY_COMMAND([${CC-cc} $CFLAGS $CPPFLAGS $LDFLAGS -static-pie -nostdlib -fPIE -o conftest1 conftest1.S]) \
     && AC_TRY_COMMAND([LC_ALL=C $READELF -Wr conftest1 | grep -q R_LARCH_RELATIVE]) \
     && ! AC_TRY_COMMAND([LC_ALL=C $READELF -Wl conftest1 | grep -q INTERP]) \
     && AC_TRY_COMMAND([${CC-cc} $CFLAGS $CPPFLAGS $LDFLAGS -shared -fPIC -o conftest2.so conftest2.S]) \
     && AC_TRY_COMMAND([LC_ALL=C $READELF -Wr conftest2.so | grep -q 'R_LARCH_JUMP_SLOT.*external_func'])
  then
    libc_cv_static_pie_on_loongarch=yes
  fi
  rm -rf conftest* ])

if test "$libc_cv_static_pie_on_loongarch" = yes; then
  AC_DEFINE(SUPPORT_STATIC_PIE)
fi

  # Check if gcc supports option -mcmodel=medium.
AC_CACHE_CHECK(whether $CC supports option -mcmodel=medium,
	       libc_cv_loongarch_cmodel_medium, [
  if AC_TRY_COMMAND(${CC-cc} -c $CFLAGS -mcmodel=medium /dev/null 1>&AS_MESSAGE_LOG_FD); then
    libc_cv_loongarch_cmodel_medium=yes
  else
    libc_cv_loongarch_cmodel_medium=no
  fi])
LIBC_CONFIG_VAR([have-cmodel-medium], [$libc_cv_loongarch_cmodel_medium])

# Check if asm support vector instructions.
AC_CACHE_CHECK(for vector support in assembler, libc_cv_loongarch_vec_asm, [dnl
cat > conftest.s <<\EOF
        vld  $vr0, $sp, 0
EOF
if AC_TRY_COMMAND(${CC-cc} -c $CFLAGS conftest.s -o conftest 1>&AS_MESSAGE_LOG_FD); then
  libc_cv_loongarch_vec_asm=yes
else
  libc_cv_loongarch_vec_asm=no
fi
rm -f conftest*])
if test $libc_cv_loongarch_vec_asm = yes; then
  AC_DEFINE(HAVE_LOONGARCH_VEC_ASM)
fi
