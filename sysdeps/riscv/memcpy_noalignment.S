/* memcpy for RISC-V, ignoring buffer alignment
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <sys/asm.h>

/* void *memcpy(void *, const void *, size_t) */
ENTRY (__memcpy_noalignment)
	move t6, a0  /* Preserve return value */

	/* Bail if 0 */
	beqz a2, 7f

	/* Jump to byte copy if size < SZREG */
	li a4, SZREG
	bltu a2, a4, 5f

	/* Round down to the nearest "page" size */
	andi a4, a2, ~((16*SZREG)-1)
	beqz a4, 2f
	add a3, a1, a4

	/* Copy the first word to get dest word aligned */
	andi a5, t6, SZREG-1
	beqz a5, 1f
	REG_L a6, (a1)
	REG_S a6, (t6)

	/* Align dst up to a word, move src and size as well. */
	addi t6, t6, SZREG-1
	andi t6, t6, ~(SZREG-1)
	sub a5, t6, a0
	add a1, a1, a5
	sub a2, a2, a5

	/* Recompute page count */
	andi a4, a2, ~((16*SZREG)-1)
	beqz a4, 2f

1:
	/* Copy "pages" (chunks of 16 registers) */
	REG_L a4,       0(a1)
	REG_L a5,   SZREG(a1)
	REG_L a6, 2*SZREG(a1)
	REG_L a7, 3*SZREG(a1)
	REG_L t0, 4*SZREG(a1)
	REG_L t1, 5*SZREG(a1)
	REG_L t2, 6*SZREG(a1)
	REG_L t3, 7*SZREG(a1)
	REG_L t4, 8*SZREG(a1)
	REG_L t5, 9*SZREG(a1)
	REG_S a4,       0(t6)
	REG_S a5,   SZREG(t6)
	REG_S a6, 2*SZREG(t6)
	REG_S a7, 3*SZREG(t6)
	REG_S t0, 4*SZREG(t6)
	REG_S t1, 5*SZREG(t6)
	REG_S t2, 6*SZREG(t6)
	REG_S t3, 7*SZREG(t6)
	REG_S t4, 8*SZREG(t6)
	REG_S t5, 9*SZREG(t6)
	REG_L a4, 10*SZREG(a1)
	REG_L a5, 11*SZREG(a1)
	REG_L a6, 12*SZREG(a1)
	REG_L a7, 13*SZREG(a1)
	REG_L t0, 14*SZREG(a1)
	REG_L t1, 15*SZREG(a1)
	addi a1, a1, 16*SZREG
	REG_S a4, 10*SZREG(t6)
	REG_S a5, 11*SZREG(t6)
	REG_S a6, 12*SZREG(t6)
	REG_S a7, 13*SZREG(t6)
	REG_S t0, 14*SZREG(t6)
	REG_S t1, 15*SZREG(t6)
	addi t6, t6, 16*SZREG
	bltu a1, a3, 1b
	andi a2, a2, (16*SZREG)-1  /* Update count */

2:
	/* Remainder is smaller than a page, compute native word count */
	beqz a2, 7f
	andi a5, a2, ~(SZREG-1)
	andi a2, a2, (SZREG-1)
	add a3, a1, a5
	/* Jump directly to last word if no words. */
	beqz a5, 4f

3:
	/* Use single native register copy */
	REG_L a4, 0(a1)
	addi a1, a1, SZREG
	REG_S a4, 0(t6)
	addi t6, t6, SZREG
	bltu a1, a3, 3b

	/* Jump directly out if no more bytes */
	beqz a2, 7f

4:
	/* Copy the last word unaligned */
	add a3, a1, a2
	add a4, t6, a2
	REG_L a5, -SZREG(a3)
	REG_S a5, -SZREG(a4)
	ret

5:
	/* Copy bytes when the total copy is <SZREG */
	add a3, a1, a2

6:
	lb a4, 0(a1)
	addi a1, a1, 1
	sb a4, 0(t6)
	addi t6, t6, 1
	bltu a1, a3, 6b

7:
	ret

END (__memcpy_noalignment)
