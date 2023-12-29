/* Linux/x86-64 CET initializers function.
   Copyright (C) 2023 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <cpu-features-offsets.h>
#include_next <dl-cet.h>

#define X86_STRINGIFY_1(x)	#x
#define X86_STRINGIFY(x)	X86_STRINGIFY_1 (x)

/* Enable shadow stack before calling _dl_init if it is enabled in
   GL(dl_x86_feature_1).  Call _dl_setup_x86_features to setup shadow
   stack.  */
#define RTLD_START_ENABLE_X86_FEATURES \
"\
	# Check if shadow stack is enabled in GL(dl_x86_feature_1).\n\
	movl _rtld_local+" X86_STRINGIFY (RTLD_GLOBAL_DL_X86_FEATURE_1_OFFSET) "(%rip), %edx\n\
	testl $" X86_STRINGIFY (X86_FEATURE_1_SHSTK) ", %edx\n\
	jz 1f\n\
	# Enable shadow stack if enabled in GL(dl_x86_feature_1).\n\
	movl $" X86_STRINGIFY (ARCH_SHSTK_SHSTK) ", %esi\n\
	movl $" X86_STRINGIFY (ARCH_SHSTK_ENABLE) ", %edi\n\
	movl $" X86_STRINGIFY (__NR_arch_prctl) ", %eax\n\
	syscall\n\
1:\n\
	# Pass GL(dl_x86_feature_1) to _dl_cet_setup_features.\n\
	movl %edx, %edi\n\
	# Align stack for the _dl_cet_setup_features call.\n\
	andq $-16, %rsp\n\
	call _dl_cet_setup_features\n\
	# Restore %rax and %rsp from %r12 and %r13.\n\
	movq %r12, %rax\n\
	movq %r13, %rsp\n\
"
