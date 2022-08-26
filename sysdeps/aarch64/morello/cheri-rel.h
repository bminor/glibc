#include <stdint.h>
#include <ldsodefs.h>
#include <cheri_perms.h>

static inline uintptr_t
morello_relative (uintptr_t base,
		  const ElfW(Rela) *reloc,
		  void *reloc_addr)
{
  uint64_t *__attribute__((may_alias)) u64_reloc_addr = reloc_addr;

  /* Fragment identified by r_offset has the following information:
     | 64-bit: address | 56-bits: length | 8-bits: permissions | */
  unsigned long loc = u64_reloc_addr[0];
  unsigned long len = u64_reloc_addr[1] & ((1UL << 56) - 1);
  unsigned long perm = u64_reloc_addr[1] >> 56;
  uintptr_t value;

  value = base + loc;

  value = __builtin_cheri_bounds_set_exact (value, len);

  value = value + reloc->r_addend;

  if (perm == 1)
    value = __builtin_cheri_perms_and (value, CAP_PERM_MASK_R);
  if (perm == 2)
    value = __builtin_cheri_perms_and (value, CAP_PERM_MASK_RW);
  if (perm == 4)
    value = __builtin_cheri_perms_and (value, CAP_PERM_MASK_RX);

  /* Seal executable capabilities with MORELLO_RB.  */
  if (perm == 4)
    value = __builtin_cheri_seal_entry (value);
  return value;
}
