#include <stdint.h>

#define STACK_CHK_GUARD \
  (*(uintptr_t __seg_fs *) offsetof (tcbhead_t, stack_guard))

#define POINTER_CHK_GUARD \
  (*(uintptr_t __seg_fs *) offsetof (tcbhead_t, pointer_guard))
