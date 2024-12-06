/* Basic tests for sealing.
   Copyright (C) 2025 Free Software Foundation, Inc.
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

#define LIB_PRELOAD              "tst-dl_mseal-preload.so"

#define LIB_AUDIT                "tst-dl_mseal-auditmod.so"

#define LIB_MODULE1              "tst-dl_mseal-mod-1.so"
#define LIB_MODULE1_DEP          "tst-dl_mseal-mod-2.so"

#define LIB_DLOPEN_DEFAULT       "tst-dl_mseal-dlopen-1.so"
#define LIB_DLOPEN_DEFAULT_DEP   "tst-dl_mseal-dlopen-1-1.so"
#define LIB_DLOPEN_NODELETE      "tst-dl_mseal-dlopen-2.so"
#define LIB_DLOPEN_NODELETE_DEP  "tst-dl_mseal-dlopen-2-1.so"
