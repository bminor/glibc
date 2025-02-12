sinclude(./aclocal.m4)dnl Autoconf lossage
GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/unix/sysv/linux/powerpc/.

LIBC_LINKER_FEATURE([--no-tls-get-addr-optimize], [-Wl,--no-tls-get-addr-optimize],
                    [libc_cv_tls_get_addr_optimize=yes], [libc_cv_tls_get_addr_optimize=no])
LIBC_CONFIG_VAR([have-tls-get-addr-optimize], [$libc_cv_tls_get_addr_optimize])

ldd_rewrite_script=sysdeps/unix/sysv/linux/powerpc/ldd-rewrite.sed
