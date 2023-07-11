/* PLT trampolines.
   Copyright (C) 2022-2023 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

/* Assembler veneer called from the PLT header code for lazy loading.
   The PLT header passes its own args in t0-t2.  */
#ifdef USE_LASX
# define FRAME_SIZE (-((-9 * SZREG - 8 * SZFREG - 8 * SZXREG) & ALMASK))
#elif defined USE_LSX
# define FRAME_SIZE (-((-9 * SZREG - 8 * SZFREG - 8 * SZVREG) & ALMASK))
#elif !defined __loongarch_soft_float
# define FRAME_SIZE (-((-9 * SZREG - 8 * SZFREG) & ALMASK))
#else
# define FRAME_SIZE (-((-9 * SZREG) & ALMASK))
#endif

ENTRY (_dl_runtime_resolve)

	/* Save arguments to stack. */
	ADDI	sp, sp, -FRAME_SIZE

	REG_S	ra, sp, 0*SZREG
	REG_S	a0, sp, 1*SZREG
	REG_S	a1, sp, 2*SZREG
	REG_S	a2, sp, 3*SZREG
	REG_S	a3, sp, 4*SZREG
	REG_S	a4, sp, 5*SZREG
	REG_S	a5, sp, 6*SZREG
	REG_S	a6, sp, 7*SZREG
	REG_S	a7, sp, 8*SZREG

#ifdef USE_LASX
	xvst	xr0, sp, 9*SZREG + 8*SZFREG + 0*SZXREG
	xvst	xr1, sp, 9*SZREG + 8*SZFREG + 1*SZXREG
	xvst	xr2, sp, 9*SZREG + 8*SZFREG + 2*SZXREG
	xvst	xr3, sp, 9*SZREG + 8*SZFREG + 3*SZXREG
	xvst	xr4, sp, 9*SZREG + 8*SZFREG + 4*SZXREG
	xvst	xr5, sp, 9*SZREG + 8*SZFREG + 5*SZXREG
	xvst	xr6, sp, 9*SZREG + 8*SZFREG + 6*SZXREG
	xvst	xr7, sp, 9*SZREG + 8*SZFREG + 7*SZXREG
#elif defined USE_LSX
	vst	vr0, sp, 9*SZREG + 8*SZFREG + 0*SZVREG
	vst	vr1, sp, 9*SZREG + 8*SZFREG + 1*SZVREG
	vst	vr2, sp, 9*SZREG + 8*SZFREG + 2*SZVREG
	vst	vr3, sp, 9*SZREG + 8*SZFREG + 3*SZVREG
	vst	vr4, sp, 9*SZREG + 8*SZFREG + 4*SZVREG
	vst	vr5, sp, 9*SZREG + 8*SZFREG + 5*SZVREG
	vst	vr6, sp, 9*SZREG + 8*SZFREG + 6*SZVREG
	vst	vr7, sp, 9*SZREG + 8*SZFREG + 7*SZVREG
#elif !defined __loongarch_soft_float
	FREG_S	fa0, sp, 9*SZREG + 0*SZFREG
	FREG_S	fa1, sp, 9*SZREG + 1*SZFREG
	FREG_S	fa2, sp, 9*SZREG + 2*SZFREG
	FREG_S	fa3, sp, 9*SZREG + 3*SZFREG
	FREG_S	fa4, sp, 9*SZREG + 4*SZFREG
	FREG_S	fa5, sp, 9*SZREG + 5*SZFREG
	FREG_S	fa6, sp, 9*SZREG + 6*SZFREG
	FREG_S	fa7, sp, 9*SZREG + 7*SZFREG
#endif

	/* Update .got.plt and obtain runtime address of callee */
	SLLI	a1, t1, 1
	or	a0, t0, zero
	ADD	a1, a1, t1
	la	a2, _dl_fixup
	jirl	ra, a2, 0
	or	t1, v0, zero

	/* Restore arguments from stack. */
	REG_L	ra, sp, 0*SZREG
	REG_L	a0, sp, 1*SZREG
	REG_L	a1, sp, 2*SZREG
	REG_L	a2, sp, 3*SZREG
	REG_L	a3, sp, 4*SZREG
	REG_L	a4, sp, 5*SZREG
	REG_L	a5, sp, 6*SZREG
	REG_L	a6, sp, 7*SZREG
	REG_L	a7, sp, 8*SZREG

#ifdef USE_LASX
	xvld	xr0, sp, 9*SZREG + 8*SZFREG + 0*SZXREG
	xvld	xr1, sp, 9*SZREG + 8*SZFREG + 1*SZXREG
	xvld	xr2, sp, 9*SZREG + 8*SZFREG + 2*SZXREG
	xvld	xr3, sp, 9*SZREG + 8*SZFREG + 3*SZXREG
	xvld	xr4, sp, 9*SZREG + 8*SZFREG + 4*SZXREG
	xvld	xr5, sp, 9*SZREG + 8*SZFREG + 5*SZXREG
	xvld	xr6, sp, 9*SZREG + 8*SZFREG + 6*SZXREG
	xvld	xr7, sp, 9*SZREG + 8*SZFREG + 7*SZXREG
#elif defined USE_LSX
	vld	vr0, sp, 9*SZREG + 8*SZFREG + 0*SZVREG
	vld	vr1, sp, 9*SZREG + 8*SZFREG + 1*SZVREG
	vld	vr2, sp, 9*SZREG + 8*SZFREG + 2*SZVREG
	vld	vr3, sp, 9*SZREG + 8*SZFREG + 3*SZVREG
	vld	vr4, sp, 9*SZREG + 8*SZFREG + 4*SZVREG
	vld	vr5, sp, 9*SZREG + 8*SZFREG + 5*SZVREG
	vld	vr6, sp, 9*SZREG + 8*SZFREG + 6*SZVREG
	vld	vr7, sp, 9*SZREG + 8*SZFREG + 7*SZVREG
#elif !defined __loongarch_soft_float
	FREG_L	fa0, sp, 9*SZREG + 0*SZFREG
	FREG_L	fa1, sp, 9*SZREG + 1*SZFREG
	FREG_L	fa2, sp, 9*SZREG + 2*SZFREG
	FREG_L	fa3, sp, 9*SZREG + 3*SZFREG
	FREG_L	fa4, sp, 9*SZREG + 4*SZFREG
	FREG_L	fa5, sp, 9*SZREG + 5*SZFREG
	FREG_L	fa6, sp, 9*SZREG + 6*SZFREG
	FREG_L	fa7, sp, 9*SZREG + 7*SZFREG
#endif

	ADDI	sp, sp, FRAME_SIZE

	/* Invoke the callee. */
	jirl	zero, t1, 0
END (_dl_runtime_resolve)
