#define aio_write64 __renamed_aio_write64
#include <kaio_misc.h>
#undef aio_write64
#include <aio_write.c>
