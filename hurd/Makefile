# Copyright (C) 1991, 1992 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

subdir := hurd

interface-header-prefix = hurd/
headers = hurd.h # $(interface-headers)

ifneq (,)
user-interfaces		:= auth process \
			   exec core interrupt \
			   fs io socket term
server-interfaces	:= misc
endif

routines = __pid2task __task2pid pid2task task2pid \
	   __setauth setauth hurdports \
	   fchroot \
	   hurdauth hurdid hurdpath hurdpid hurdsig hurdsock hurdfrob \
	   dtable __getdport getdport openport setdtablesize \
	   msgportdemux _hurd_start_sigthread \
	   getuids getumask \
	   _hurd_dead_recv \
	   fopenport __fopenport \
	   $(interface-routines)
aux := hurdsyms

all:


# This rule needs to come before the implicit rules in Machrules.
__%.c: intr_rpc.awk %_rpc.c
	gawk -v call=__$* -v rpc=__$*_rpc -f $^ > $@-new
	mv $@-new $@
# __%_rpc.c is made with mig by Machrules.
%_rpc.c: __%_rpc.c
	sed s/MACH_MSG_OPTION_NONE/MACH_SEND_INTERRUPT/ $< > $@

include ../mach/Machrules

ifdef user-interfaces
include hurdintr
endif
hurdintr: hurdintr.awk $(user-interfaces:%=%.defs)
	awk -f $^ varname=intr-calls > $@-new
	mv $@-new $@

# Make the INTR user stubs be defined as CALL_rpc.
migdefines := $(migdefines) $(foreach call,$(intr-calls),-D$(call)=$(call)_rpc)

interface-routines := $(filter-out %_rpc,$(interface-routines)) \
		      $(intr-calls:%=__% %)
dont_distribute = $(interface-routines:%=%.c) $(interface-headers)

distribute := intr_rpc.awk hurdintr.awk

include ../Rules
