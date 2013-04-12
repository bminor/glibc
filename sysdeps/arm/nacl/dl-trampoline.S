/* XXX temporary stubs */

	.text
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, #function
	.p2align 4
_dl_runtime_resolve:
	sfi_trap
	.size _dl_runtime_resolve, .-_dl_runtime_resolve

#ifndef PROF
	.globl _dl_runtime_profile
	.type _dl_runtime_profile, #function
	.p2align 4
_dl_runtime_profile:
	sfi_trap
	.size _dl_runtime_profile, .-_dl_runtime_profile
#endif
	.previous
