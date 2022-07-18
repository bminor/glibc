/* Copyright (C) 2022 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _AARCH64_MORELLO_CHERI_PERMS_H
#define _AARCH64_MORELLO_CHERI_PERMS_H

/* Capability permission bits.  These are defined in the Arm Architecture
   Reference Manual Suplement- Morello for A-Profile Architecture:
   https://developer.arm.com/documentation/ddi0606/latest  */
#define CAP_PERM_LOAD (1 << 17)
#define CAP_PERM_STORE (1 << 16)
#define CAP_PERM_EXECUTE (1 << 15)
#define CAP_PERM_LOAD_CAP (1 << 14)
#define CAP_PERM_STORE_CAP (1 << 13)
#define CAP_PERM_STORE_LOCAL (1 << 12)
#define CAP_PERM_SEAL (1 << 11)
#define CAP_PERM_UNSEAL (1 << 10)
#define CAP_PERM_COMPARTMENT_ID (1 << 7)
#define CAP_PERM_MUTABLE_LOAD (1 << 6)
#define CAP_PERM_EXECUTIVE (1 << 1)
#define CAP_PERM_GLOBAL (1 << 0)

/* Used with __builtin_cheri_perms_and.  */
#define CAP_PERM_MASK_BASE (-1UL ^ ( \
  CAP_PERM_LOAD | \
  CAP_PERM_STORE | \
  CAP_PERM_EXECUTE | \
  CAP_PERM_LOAD_CAP | \
  CAP_PERM_STORE_CAP | \
  CAP_PERM_SEAL | \
  CAP_PERM_UNSEAL | \
  CAP_PERM_COMPARTMENT_ID))
#define CAP_PERM_MASK_R (CAP_PERM_MASK_BASE | CAP_PERM_LOAD | CAP_PERM_LOAD_CAP)
#define CAP_PERM_MASK_RW (CAP_PERM_MASK_R | CAP_PERM_STORE | CAP_PERM_STORE_CAP)
#define CAP_PERM_MASK_RX (CAP_PERM_MASK_R | CAP_PERM_EXECUTE)

#define STACK_CAP_PERM_PCS ( \
  CAP_PERM_LOAD | \
  CAP_PERM_STORE | \
  CAP_PERM_LOAD_CAP | \
  CAP_PERM_STORE_CAP | \
  CAP_PERM_MUTABLE_LOAD)

/* Check if stack is valid according to stack PCS rules.  Only permissions
   are checked, seal and invalid tag cause fault on access.  */
#define STACK_CAP_CHECK(stack, size) \
  ({unsigned long __perm = __builtin_cheri_perms_get (stack); \
    (__perm & (CAP_PERM_GLOBAL|CAP_PERM_STORE_LOCAL)) \
    && (__perm & STACK_CAP_PERM_PCS) == STACK_CAP_PERM_PCS;})

#endif
