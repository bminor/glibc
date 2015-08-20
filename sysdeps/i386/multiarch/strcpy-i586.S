#define strcpy __strcpy_i586
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name)
#include <sysdeps/i386/i586/strcpy.S>
