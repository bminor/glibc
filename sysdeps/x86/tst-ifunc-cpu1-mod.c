#include <init-arch.h>

static void
one (void)
{
}

static void
two (void)
{
}

void * foo_ifunc (void) __asm__ ("foo") attribute_hidden;
__asm__(".type foo, %gnu_indirect_function");

void *
foo_ifunc (void)
{
  const struct cpu_features * cpu = __get_cpu_features ();
  return cpu->max_cpuid > 1 ? two : one;
}

extern void foo (void) attribute_hidden;
void (*foo_ptr) (void) = foo;
