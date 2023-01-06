/* Data for s390 version of processor capability information.
   Copyright (C) 2006-2023 Free Software Foundation, Inc.
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

#include <dl-procinfo.h>

const char _dl_s390_cap_flags[_DL_HWCAP_COUNT][9] =
  {
    "esan3", "zarch", "stfle", "msa", "ldisp", "eimm", "dfp", "edat", "etf3eh",
    "highgprs", "te", "vx", "vxd", "vxe", "gs", "vxe2", "vxp", "sort", "dflt",
    "vxp2", "nnpa", "pcimio", "sie"
  };

const char _dl_s390_platforms[_DL_PLATFORMS_COUNT][7] =
  {
    "g5", "z900", "z990", "z9-109", "z10", "z196", "zEC12", "z13", "z14", "z15",
    "z16"
  };
