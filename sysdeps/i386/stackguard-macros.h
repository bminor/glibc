#include <stdint.h>

#define STACK_CHK_GUARD \
  (*(uintptr_t __seg_gs *) offsetof (tcbhead_t, stack_guard))

#define POINTER_CHK_GUARD \
  (*(uintptr_t __seg_gs *) offsetof (tcbhead_t, pointer_guard))
