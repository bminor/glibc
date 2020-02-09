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
	 tst-attr1 \
	 tst-basic1 tst-basic2 tst-basic3 tst-basic4 tst-basic5 tst-basic6 \
	 tst-basic7 \
	 tst-cond1 tst-cond2 tst-cond3 tst-cond4 tst-cond5 tst-cond6 tst-cond7 \
	 tst-cond8 tst-cond9 tst-cond10 tst-cond12 tst-cond13 \
	 tst-cond14 tst-cond15 tst-cond16 tst-cond17 tst-cond18 tst-cond19 \
	 tst-cond23 tst-cond24 tst-cond25 \
	 tst-cond-except \
	 tst-key1 tst-key2 tst-key3 tst-key4 \
	 tst-mutex1 tst-mutex2 tst-mutex3 tst-mutex4 tst-mutex6 tst-mutex10 \
	 tst-once1 tst-once2 tst-once3 tst-once4 \
	 tst-rwlock1 tst-rwlock4 tst-rwlock5 tst-rwlock13 tst-rwlock16 \
	 tst-rwlock-tryrdlock-stall tst-rwlock-trywrlock-stall \
	 tst-sem1 tst-sem2 tst-sem3 tst-sem4 tst-sem6 tst-sem7 \
	 tst-sem8 tst-sem9 tst-sem10 tst-sem14 tst-sem15 tst-sem16 \
	 tst-spin1 tst-spin2 tst-spin3 tst-spin4

tests += tst-oncex3 tst-oncex4

ifeq ($(build-shared),yes)
tests-static += tst-cond8-static
tests += tst-cond8-static
endif

CFLAGS-tst-oncex3.c += -fexceptions
CFLAGS-tst-oncex4.c += -fexceptions

endif
