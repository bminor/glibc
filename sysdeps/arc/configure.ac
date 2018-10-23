GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/arc.

AC_DEFINE(PI_STATIC_AND_HIDDEN)
libc_cv_have_sdata_section=no

# For ARC, historically ; was used for comments and not newline
# Later # also got added to comment list, but ; couldn't be switched to
# canonical newline as there's lots of code out there which will break
libc_cv_asm_line_sep='`'
AC_DEFINE_UNQUOTED(ASM_LINE_SEP, $libc_cv_asm_line_sep)
