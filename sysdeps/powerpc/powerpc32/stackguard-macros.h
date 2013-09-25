#include <stdint.h>

#define STACK_CHK_GUARD \
  ({												\
     uintptr_t x;										\
     asm ("lwz %0,%1(2)"									\
	  : "=r" (x)										\
	  : "i" (offsetof (tcbhead_t, stack_guard) - TLS_TCB_OFFSET - sizeof (tcbhead_t))	\
         );											\
     x;												\
   })

#define POINTER_CHK_GUARD \
  ({												\
     uintptr_t x;										\
     asm ("lwz %0,%1(2)"									\
	  : "=r" (x)										\
	  : "i" (offsetof (tcbhead_t, pointer_guard) - TLS_TCB_OFFSET - sizeof (tcbhead_t))	\
         );											\
     x;												\
   })
