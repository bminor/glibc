#define aio_write64 __renamed_aio_write64

#include <rt/aio_write.c>

#undef aio_write64

weak_alias (aio_write, aio_write64)
