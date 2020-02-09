# sysdeps makefile fragment for all pthread-supporting configurations.
# Copyright (C) 2014-2020 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <https://www.gnu.org/licenses/>.

ifeq ($(subdir),rt)

ifeq (yes,$(build-shared))
$(objpfx)tst-timer: $(objpfx)librt.so $(shared-thread-library)
else
$(objpfx)tst-timer: $(objpfx)librt.a $(static-thread-library)
endif

endif

ifneq (,$(filter $(subdir),htl nptl))
headers += threads.h

routines += thrd_current thrd_equal thrd_sleep thrd_yield

libpthread-routines += thrd_create thrd_detach thrd_exit thrd_join \
		       call_once \
		       mtx_destroy mtx_init mtx_lock mtx_timedlock \
		       mtx_trylock mtx_unlock \
		       cnd_broadcast \
		       cnd_destroy cnd_init cnd_signal cnd_timedwait cnd_wait \
		       tss_create tss_delete tss_get tss_set

tests += tst-cnd-basic tst-mtx-trylock tst-cnd-broadcast \
	 tst-cnd-timedwait tst-thrd-detach tst-mtx-basic tst-thrd-sleep \
	 tst-mtx-recursive tst-tss-basic tst-call-once tst-mtx-timedlock \
	 tst-basic1 tst-basic2 tst-basic3 tst-basic4 tst-basic5 tst-basic6 \
	 tst-basic7 \
	 tst-spin1 tst-spin2 tst-spin3 tst-spin4 \

endif
