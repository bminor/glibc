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

subdir := mach

distribute = Machrules syscalls.awk shortcut.awk

headers = mach_init.h mach.h mach-shortcuts.h \
	  $(interface-headers)
generated = mach-shortcuts.h


# Install the Mach header files directly out of the Mach kernel source.
mach-srcdir = /usr1/gnu/mach # XXX

# When compiling, use the Mach header files directly from the kernel sources.
includes += -I$(mach-srcdir)

mach-src-headers := $(wildcard $(addprefix $(mach-srcdir)/mach/,\
					   *.defs *.h
					   */*.defs */*.h))
mach-headers = $(filter-out mach/machine/%,\
			    $(mach-src-headers:$(mach-srcdir)/%=%))
install-others += $(addprefix $(includedir),$(mach-headers))
$(includedir)/mach/%: $(mach-srcdir)/%; $(do-install)


user-interfaces := $(addprefix mach/,mach_interface mach_port mach_host	\
				     memory_object_user			\
				     memory_object_default		\
	            )\
		   $(addprefix device/,device device_request)
# device_reply is giving me headaches because of userprefix
server-interfaces := # mach/__exc mach/exc

routines = $(interface-routines) \
	   $(filter-out syscall_%,$(mach-syscalls)) \
	   $(addprefix __,$(mach-syscalls)) \
	   $(mach-shortcuts) $(addprefix __,$(mach-shortcuts)) \
	   mach_init vm_page_size \
	   mig_strncpy __msg msg \
	   mig_support mig_syms \
	   msg_destroy msg_server msg_server_t \
	   __msg_dest __msg_server \
	   devstream

tests := hello


all:

include ../Makeconfig	# To get objpfx defined.

# Define mach-syscalls and sysno-*.
ifneq ($(no_deps),t)
include $(objpfx)mach-syscalls.mk
endif
$(objpfx)mach-syscalls.mk: mach/syscall_sw.h syscalls.awk
# Go kludges!!!
	sed -n -e '/Unix server implement them/,$$d' \
	       -e 's/^kernel_trap(\(.*\),\([-0-9]*\),[0-9]*)$$/\1 \2/p' \
	< $< | awk -f $(word 2,$^) > $@-new
	mv $@-new $@
generated += mach-syscalls.mk

ifndef mach-syscalls
# We cannot make the deps properly until we know how to make the system
# call functions, and for that we must know what they all are.
no_deps=t
else
$(mach-syscalls:%=$(objpfx)__%.S): $(objpfx)__%.S: $(objpfx)mach-syscalls.mk
	(echo '#include <sysdep.h>'; \
	 echo 'SYSCALL_TRAP (__$*, $(sysno-$*))') > $@-new
	 mv $@-new $@
$(mach-syscalls:%=$(objpfx)%.c): $(objpfx)%.c:
	(echo '#include <gnu-stabs.h>'; \
	 echo 'symbol_alias (__$*, $*)') > $@-new
	mv $@-new $@
generated += $(mach-syscalls:%=__%.S) \
	     $(mach-syscalls:%=%.c)
endif	# mach-syscalls

mach-shortcuts := $(patsubst syscall_%,%,$(filter syscall_%,$(mach-syscalls)))

ifndef mach-shortcuts
# Forget about mach_interface.defs for this run.  On the next run,
# $(mach-shortcuts) will be set, and that will change how
# mach_interface.defs is processed: it will get the -D flags below.
user-interfaces := $(filter-out mach/mach_interface \
				mach/mach_port mach/mach_host,\
				$(user-interfaces))
endif

# Make the MiG stubs for $(mach-shortcuts) be CALL_rpc.
migdefines += $(foreach call,$(mach-shortcuts),-D$(call)=$(call)_rpc)
mach/mach_interface.uh mach/mach_port.uh: $(objpfx)mach-syscalls.mk

ifdef mach-shortcuts
$(mach-shortcuts:%=$(objpfx)__%.c): $(objpfx)__%.c: shortcut.awk \
						    $(objpfx)__%_rpc.c
	gawk -v call=__$* -v rpc=__$*_rpc -v syscall=__syscall_$* \
	     -f $^ > $@-new
	mv $@-new $@

# The shortcut calls have had their names changed to CALL_rpc, so
# CALL_rpc.c is being generated with a symbol alias to __CALL_rpc.  
# We also want CALL.c with an alias to __CALL.
$(mach-shortcuts:%=$(objpfx)%.c): $(objpfx)%.c:
	(echo '#include <gnu-stabs.h>'; \
	 echo 'symbol_alias (__$*, $*)') > $@-new
	mv $@-new $@
generated += $(mach-shortcuts:%=%.c)
endif	# mach-shortcuts

# Generate mach-shortcuts.h, which will contain the prototypes for the
# shortcutted kernel RPC functions.
$(objpfx)mach-shortcuts.h: $(objpfx)mach/mach_interface.h \
			   $(objpfx)mach/mach_port.h
# The first line gets us one paragraph per line, with @s separating real lines.
# The second line selects paragraphs for the shortcutted functions.
# The third line removes `_rpc' from the names and rerealifies the lines.
	cat $^ | tr \\012 @ | sed s/@@/@%/g | tr % \\012 \
	| grep '^/\* Routine [a-z0-9_]*_rpc \*/' \
	| sed 's/_rpc//g' | tr @ \\012 > $@-new
	mv $@-new $@

before-compile += $(objpfx)mach-shortcuts.h


include Machrules
include ../Rules


# There is already a mach.h, so mach.defs generates mach_interface.h.
mach/mach_interface.defs: mach/mach.defs
	ln $< $@ || cp $< $@
# There is already a memory_object.h,
# so memory_object.defs generates memory_object_user.h.
mach/memory_object_user.defs: mach/memory_object.defs
	ln $< $@ || cp $< $@
generated += mach/mach.defs mach/memory_object_user.defs

# Be sure not to make these with implicit rules from foo.defs.
mach.h mach/memory_object.h: ;
