GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/s390/s390-64.

# Minimal checking for static PIE support in ld.
# Compare to ld testcase/bugzilla:
# <binutils-source>/ld/testsuite/ld-elf/pr22263-1.rd
AC_CACHE_CHECK([for s390-specific static PIE requirements], \
[libc_cv_s390x_staticpie_req], [dnl
  cat > conftest1.c <<EOF
__thread int * foo;

void
bar (void)
{
  *foo = 1;
}
EOF
  cat > conftest2.c <<EOF
extern __thread int *foo;
extern void bar (void);
static int x;

int
main ()
{
  foo = &x;
  return 0;
}
EOF
  libc_cv_s390x_staticpie_req=no
  if AC_TRY_COMMAND([${CC-cc} $CFLAGS $CPPFLAGS $LDFLAGS -fPIE -c conftest1.c -o conftest1.o]) \
     && AC_TRY_COMMAND([${CC-cc} $CFLAGS $CPPFLAGS $LDFLAGS -fPIE -c conftest2.c -o conftest2.o]) \
     && AC_TRY_COMMAND([${CC-cc} $CFLAGS $CPPFLAGS $LDFLAGS -pie -o conftest conftest1.o conftest2.o]) \
     && AC_TRY_COMMAND([! LC_ALL=C $READELF -Wr conftest | grep R_390_TLS_TPOFF])
  then
    libc_cv_s390x_staticpie_req=yes
  fi
  rm -rf conftest.*])
if test $libc_cv_s390x_staticpie_req = yes; then
   # Static PIE is supported only on 64bit.
   # Ensure you also have those patches for:
   # - binutils (ld)
   #   - "[PR ld/22263] s390: Avoid dynamic TLS relocs in PIE"
   #     https://sourceware.org/git/?p=binutils-gdb.git;a=commit;h=26b1426577b5dcb32d149c64cca3e603b81948a9
   #     (Tested by configure check above)
   #     Otherwise there will be a R_390_TLS_TPOFF relocation, which fails to
   #     be processed in _dl_relocate_static_pie() as static TLS map is not setup.
   #   - "s390: Add DT_JMPREL pointing to .rela.[i]plt with static-pie"
   #     https://sourceware.org/git/?p=binutils-gdb.git;a=commit;h=d942d8db12adf4c9e5c7d9ed6496a779ece7149e
   #     (We can't test it in configure as we are not able to link a static PIE
   #     executable if the system glibc lacks static PIE support)
   #     Otherwise there won't be DT_JMPREL, DT_PLTRELA, DT_PLTRELASZ entries
   #     and the IFUNC symbols are not processed, which leads to crashes.
   #
   # - kernel (the mentioned links to the commits belong to 5.19 merge window):
   #   - "s390/mmap: increase stack/mmap gap to 128MB"
   #     https://git.kernel.org/pub/scm/linux/kernel/git/s390/linux.git/commit/?h=features&id=f2f47d0ef72c30622e62471903ea19446ea79ee2
   #   - "s390/vdso: move vdso mapping to its own function"
   #     https://git.kernel.org/pub/scm/linux/kernel/git/s390/linux.git/commit/?h=features&id=57761da4dc5cd60bed2c81ba0edb7495c3c740b8
   #   - "s390/vdso: map vdso above stack"
   #     https://git.kernel.org/pub/scm/linux/kernel/git/s390/linux.git/commit/?h=features&id=9e37a2e8546f9e48ea76c839116fa5174d14e033
   #   - "s390/vdso: add vdso randomization"
   #     https://git.kernel.org/pub/scm/linux/kernel/git/s390/linux.git/commit/?h=features&id=41cd81abafdc4e58a93fcb677712a76885e3ca25
   #   (We can't test the kernel of the target system)
   #   Otherwise if /proc/sys/kernel/randomize_va_space is turned off (0),
   #   static PIE executables like ldconfig will crash.  While startup sbrk is
   #   used to enlarge the HEAP.  Unfortunately the underlying brk syscall fails
   #   as there is not enough space after the HEAP.  Then the address of the TLS
   #   image is invalid and the following memcpy in __libc_setup_tls() leads
   #   to a segfault.
   #   If /proc/sys/kernel/randomize_va_space is activated (default: 2), there
   #   is enough space after HEAP.
   #
   # - glibc
   #   - "Linux: Define MMAP_CALL_INTERNAL"
   #     https://sourceware.org/git/?p=glibc.git;a=commit;h=c1b68685d438373efe64e5f076f4215723004dfb
   #   - "i386: Remove OPTIMIZE_FOR_GCC_5 from Linux libc-do-syscall.S"
   #     https://sourceware.org/git/?p=glibc.git;a=commit;h=6e5c7a1e262961adb52443ab91bd2c9b72316402
   #   - "i386: Honor I386_USE_SYSENTER for 6-argument Linux system calls"
   #     https://sourceware.org/git/?p=glibc.git;a=commit;h=60f0f2130d30cfd008ca39743027f1e200592dff
   #   - "ia64: Always define IA64_USE_NEW_STUB as a flag macro"
   #     https://sourceware.org/git/?p=glibc.git;a=commit;h=18bd9c3d3b1b6a9182698c85354578d1d58e9d64
   #   - "Linux: Implement a useful version of _startup_fatal"
   #     https://sourceware.org/git/?p=glibc.git;a=commit;h=a2a6bce7d7e52c1c34369a7da62c501cc350bc31
   #   - "Linux: Introduce __brk_call for invoking the brk system call"
   #     https://sourceware.org/git/?p=glibc.git;a=commit;h=b57ab258c1140bc45464b4b9908713e3e0ee35aa
   #   - "csu: Implement and use _dl_early_allocate during static startup"
   #     https://sourceware.org/git/?p=glibc.git;a=commit;h=f787e138aa0bf677bf74fa2a08595c446292f3d7
   #   The mentioned patch series by Florian Weimer avoids the mentioned failing
   #   sbrk syscall by falling back to mmap.
   AC_DEFINE(SUPPORT_STATIC_PIE)
fi
