GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/x86.

if test $enable_cet != no; then
  # Check if CET can be enabled.
  AC_CACHE_CHECK(whether CET can be enabled,
		 libc_cv_x86_cet_available, [dnl
cat > conftest.c <<EOF
#if !defined __CET__ || __CET__ != 3
# error CET isn't available.
#endif
EOF
		 if AC_TRY_COMMAND(${CC-cc} -c $CFLAGS -fcf-protection -include cet.h conftest.c 1>&AS_MESSAGE_LOG_FD); then
		   libc_cv_x86_cet_available=yes
		 else
		   libc_cv_x86_cet_available=no
		 fi
		 rm -rf conftest*])
  if test $libc_cv_x86_cet_available != yes; then
    AC_MSG_ERROR([$CC doesn't support CET])
  fi
fi
if test $enable_cet != no; then
  # Check if assembler supports CET.
  AC_CACHE_CHECK(whether $AS supports CET,
		 libc_cv_x86_cet_as, [dnl
cat > conftest.s <<EOF
	incsspd %ecx
EOF
		 if AC_TRY_COMMAND(${CC-cc} -c $CFLAGS conftest.s -o conftest.o 1>&AS_MESSAGE_LOG_FD); then
		   libc_cv_x86_cet_as=yes
		 else
		   libc_cv_x86_cet_as=no
		 fi
		 rm -rf conftest*])
  if test $libc_cv_x86_cet_as = no; then
    AC_MSG_ERROR([$AS doesn't support CET])
  fi
fi
if test $enable_cet = yes; then
  AC_DEFINE(DEFAULT_DL_X86_CET_CONTROL, cet_elf_property)
elif test $enable_cet = permissive; then
  AC_DEFINE(DEFAULT_DL_X86_CET_CONTROL, cet_permissive)
fi
LIBC_CONFIG_VAR([enable-cet], [$enable_cet])
