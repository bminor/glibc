/* Diagnostics probes for the x86 CPU family.  Generic version.
   Copyright (C) 2024 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <sysdep.h>

ENTRY (_dl_x86_probe_cmpxchg16b)
	xorl %eax, %eax
	movq %rax, -8(%rsp)
	movq %rax, -16(%rsp)
	xorl %edx, %edx
	cmpxchg16b -8(%rsp)
	ret
END (_dl_x86_probe_cmpxchg16b)

ENTRY (_dl_x86_probe_sahf)
	xorl %eax, %eax
	sahf
	ret
END (_dl_x86_probe_sahf)

ENTRY (_dl_x86_probe_popcnt)
	xorl %eax, %eax
	popcnt %eax, %eax
	ret
END (_dl_x86_probe_popcnt)

ENTRY (_dl_x86_probe_sse3)
	pxor %xmm0, %xmm0
	addsubpd %xmm0,  %xmm0
	ret
END (_dl_x86_probe_sse3)

ENTRY (_dl_x86_probe_sse4_1)
	pxor %xmm0, %xmm0
	blendpd $1, %xmm0, %xmm0
	ret
END (_dl_x86_probe_sse4_1)

ENTRY (_dl_x86_probe_sse4_2)
	pxor %xmm0, %xmm0
	pcmpestri $0, %xmm0, %xmm0
	ret
END (_dl_x86_probe_sse4_2)

ENTRY (_dl_x86_probe_ssse3)
	pxor %xmm0, %xmm0
	phaddd %xmm0, %xmm0
	ret
END (_dl_x86_probe_ssse3)

ENTRY (_dl_x86_probe_avx)
	vzeroall
	ret
END (_dl_x86_probe_avx)

ENTRY (_dl_x86_probe_avx_xmm)
	pxor %xmm0, %xmm0
	pxor %xmm1, %xmm1
	vpxor %xmm0, %xmm1, %xmm2
	ret
END (_dl_x86_probe_avx_xmm)

ENTRY (_dl_x86_probe_avx2)
	vpxor %ymm0, %ymm0, %ymm0
	vpermd %ymm0, %ymm0, %ymm0
	ret
END (_dl_x86_probe_avx2)

ENTRY (_dl_x86_probe_bmi1)
	xorl  %eax,  %eax
	andnl %eax, %eax, %eax
	ret
END (_dl_x86_probe_bmi1)

ENTRY (_dl_x86_probe_bmi1_tzcnt)
	xorl %eax, %eax
	/* Executes as bsfl if unsupported.  */
	tzcntl %eax, %eax
	cmp $32, %eax
	jne 1f
	ret
1:
	ud2
END (_dl_x86_probe_bmi1_tzcnt)

ENTRY (_dl_x86_probe_bmi2)
	xorl  %eax,  %eax
	bzhil %eax, %eax, %eax
	ret
END (_dl_x86_probe_bmi2)

ENTRY (_dl_x86_probe_f16c)
	pxor %xmm0, %xmm0
	vcvtph2ps %xmm0, %xmm0
	ret
END (_dl_x86_probe_f16c)

ENTRY (_dl_x86_probe_fma)
	pxor %xmm0, %xmm0
	vfmadd132pd %xmm0, %xmm0, %xmm0
	ret
END (_dl_x86_probe_fma)

ENTRY (_dl_x86_probe_fma4)
	pxor %xmm0, %xmm0
	vfmaddpd %xmm0, %xmm0, %xmm0, %xmm0
	ret
END (_dl_x86_probe_fma4)

ENTRY (_dl_x86_probe_lzcnt)
	xorl %eax, %eax
	/* Executes as bsrl if unsupported.  */
	lzcntl %eax, %eax
	cmp $32, %eax
	jne 1f
	ret
1:
	ud2
END (_dl_x86_probe_lzcnt)

ENTRY (_dl_x86_probe_movbe)
	movbeq (%rsp), %rax
	ret
1:
	ud2
END (_dl_x86_probe_movbe)

ENTRY (_dl_x86_probe_osxsave)
	xorl %ecx, %ecx
	xgetbv
	ret
END (_dl_x86_probe_osxsave)

ENTRY (_dl_x86_probe_avx512f)
	xorl %eax, %eax
	kmovw %eax, %k0
	ret
END (_dl_x86_probe_avx512f)

ENTRY (_dl_x86_probe_avx512bw)
	vpxorq %zmm0, %zmm0, %zmm0
	vdbpsadbw $0, %zmm0, %zmm0, %zmm0
	ret
END (_dl_x86_probe_avx512bw)

ENTRY (_dl_x86_probe_avx512bw_ymm)
	vpxorq %ymm0, %ymm0, %ymm0
	vdbpsadbw $0, %ymm0, %ymm0, %ymm0
	ret
END (_dl_x86_probe_avx512bw)

ENTRY (_dl_x86_probe_avx512cd)
	vpxorq %zmm0, %zmm0, %zmm0
	vplzcntd %zmm0, %zmm0
	ret
END (_dl_x86_probe_avx512cd)

ENTRY (_dl_x86_probe_avx512cd_ymm0)
	vpxorq %ymm0, %ymm0, %ymm0
	vplzcntd %ymm0, %ymm0
	ret
END (_dl_x86_probe_avx512cd_ymm0)

ENTRY (_dl_x86_probe_avx512dq)
	vpxorq %zmm0, %zmm0, %zmm0
	vpmullq %zmm0, %zmm0, %zmm0
	ret
END (_dl_x86_probe_avx512dq)

ENTRY (_dl_x86_probe_avx512dq_ymm0)
	vpxorq %ymm0, %ymm0, %ymm0
	vpmulld %ymm0, %ymm0, %ymm0
	ret
END (_dl_x86_probe_avx512dq_ymm0)

ENTRY (_dl_x86_probe_avx512vl)
	xorl %eax, %eax
	vpbroadcastq %rax, %xmm1
	ret
END (_dl_x86_probe_avx512vl)

ENTRY (_dl_x86_probe_adx)
	xorl %eax, %eax
	adcxl %eax, %eax
	ret
END (_dl_x86_probe_adx)

ENTRY (_dl_x86_probe_aes)
	pxor %xmm0, %xmm0
	aesenc %xmm0, %xmm0
	ret
END (_dl_x86_probe_aes)

ENTRY (_dl_x86_probe_aes_avx)
	pxor %xmm0, %xmm0
	vaesenc %xmm0, %xmm0, %xmm0
	ret
END (_dl_x86_probe_aes_avx)

ENTRY (_dl_x86_probe_vaes)
	vpxor %ymm0, %ymm0, %ymm0
	vaesenc %ymm0, %ymm0, %ymm0
	ret
END (_dl_x86_probe_vaes)

ENTRY (_dl_x86_probe_sha)
	pxor %xmm0, %xmm0
	sha1rnds4 $0, %xmm0, %xmm0
	ret
END (_dl_x86_probe_sha)

ENTRY (_dl_x86_probe_avx512_vbmi)
	vpxorq %zmm0, %zmm0, %zmm0
	vpermb %zmm0, %zmm0, %zmm0
	ret
END (_dl_x86_probe_avx512_vbmi)

ENTRY (_dl_x86_probe_avx512_vbmi_xmm)
	pxor %xmm0, %xmm0
	vpermb %xmm0, %xmm0, %xmm0
	ret
END (_dl_x86_probe_avx512_vbmi_xmm)

ENTRY (_dl_x86_probe_avx512_vbmi2)
	vpxorq %zmm0, %zmm0, %zmm0
	vpshrdd $1, %zmm0, %zmm0, %zmm0
	ret
END (_dl_x86_probe_avx512_vbmi2)

ENTRY (_dl_x86_probe_avx512_vbmi2_xmm)
	pxor %xmm0, %xmm0
	vpshrdd $1, %xmm0, %xmm0, %xmm0
	ret
END (_dl_x86_probe_avx512_vbmi2_xmm)

ENTRY (_dl_x86_probe_avx_vnni)
	pxor %xmm0, %xmm0
	/* Default is to use EVEX encoding.  */
	/* {vex} vpdpbusd %xmm0, %xmm0, %xmm0 */
	.byte 0xc4, 0xe2, 0x79, 0x50, 0xc0
	ret
END (_dl_x86_probe_avx_vnni)

ENTRY (_dl_x86_probe_avx512_vnni)
	vpxorq %zmm0, %zmm0, %zmm0
	vpdpbusd %zmm0, %zmm0, %zmm0
	ret
END (_dl_x86_probe_avx512_vnni)

ENTRY (_dl_x86_probe_avx512_ifma)
	vpxorq %zmm0, %zmm0, %zmm0
	vpmadd52luq %zmm0, %zmm0, %zmm0
	ret
END (_dl_x86_probe_avx512_ifma)

ENTRY (_dl_x86_probe_apx_f)
	.byte 0x62, 0xf4, 0x7c, 0x18, 0x01, 0xff /* add %edi, %edi, %eax */
	ret
END (_dl_x86_probe_apx_f)
