# Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
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

all:

# Some things below (but before including Rules) use configuration variables.
include ../Makeconfig


headers = hurd.h $(interface-headers)
distribute := intr_rpc.awk hurdintr.awk

user-interfaces		:= $(addprefix hurd/,\
				       auth process startup msg \
				       exec core interrupt \
				       fs fsys io term socket )
server-interfaces	:= hurd/msg

routines = $(interface-routines) \
	   hurdid hurdpath hurdpid hurdports \
	   hurdexec fexecve \
	   __setauth setauth \
	   __pid2task __task2pid pid2task task2pid \
	   getuids setuids getumask fchroot \
	   dtable __getdport getdport openport setdtsz \
#	   hurdsig hurdmsg hurdsock hurdauth \
	   privports \
	   msgportdemux startsig \
	   fopenport __fopenport \
	   vpprintf
aux := hurdsyms hurdinit

# Get the proper definition of `hurd-srcdir'.
include ../sysdeps/mach/hurd/Makefile

# Use and install the Hurd header files directly out of the Hurd source.

# Find the MiG defs files in the Hurd source.
vpath %.defs $(hurd-srcdir)

# Install all .h and .defs files we find in the Hurd's hurd/ directory.
hurd-headers := $(patsubst $(hurd-srcdir)/%,%,\
			   $(wildcard $(addprefix $(hurd-srcdir)/hurd/,\
						  *.defs *.h)))

# Don't distribute the Hurd headers; they are in the Hurd distribution.
dont_distribute = $(hurd-headers)

# DO NOT try to remake these in any way!!!
$(addprefix $(hurd-srcdir)/,$(hurd-headers)) : ;
install-others += $(addprefix $(includedir)/,$(hurd-headers))
$(includedir)/hurd/%: $(hurd-srcdir)/hurd/%; $(do-install)

# Insert comment here.

ifdef intrhack # XXX
# This rule needs to come before the implicit rules in Machrules.
__%.c: intr_rpc.awk %_rpc.c
	gawk -v call=__$* -v rpc=__$*_rpc -f $^ > $@-new
	mv $@-new $@
# __%_rpc.c is made with mig by Machrules.
%_rpc.c: __%_rpc.c
	sed s/MACH_MSG_OPTION_NONE/MACH_SEND_INTERRUPT/ $< > $@
endif

include ../mach/Machrules

ifdef intrhack # XXX
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
endif

include ../Rules
