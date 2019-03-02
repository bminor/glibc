#define WCSCPY __wcscpy
#include <sysdeps/powerpc/power6/wcscpy.c>
libc_hidden_def (__wcscpy)
weak_alias (__wcscpy, wcscpy)
