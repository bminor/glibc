/* Static assert for __blkcnt64_t when __INO_T_MATCHES_INO64_T is defined.
   Copyright (C) 2019-2020 Free Software Foundation, Inc.
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

#include <sys/stat.h>
#include <bits/typesizes.h>

/* This is due to the layout code in bits/stat.h and the overflow handling in
   wordsize-32/overflow.h, requiring either all or none of the three types
   concerned to have padding.  */

#if defined __INO_T_MATCHES_INO64_T
_Static_assert (sizeof (__blkcnt_t) == sizeof (__blkcnt64_t),
                "__blkcnt_t and __blkcnt64_t must match");
#endif
