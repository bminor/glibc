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

subdir := mach

distribute = Machrules interface.awk mach-syscalls.awk # $(interfaces:%=%.defs)

interface-header-prefix = mach/
headers = mach_init.h \
	  $(nope mach.h) \
	  $(addprefix mach/,$(mach-headers)) \
	  $(interface-headers)

ifneq (,)
mach-headers := host_info.h kern_return.h \
		mach_param.h mach_types.h \
		memory_object.h \
		message.h mig_errors.h msg_type.h \
		policy.h port.h processor_info.h \
		std_types.h task_info.h task_special_ports.h \
		thread_info.h thread_special_ports.h \
		thread_status.h thread_switch.h time_value.h \
		vm_attributes.h vm_inherit.h vm_prot.h \
		vm_statistics.h

user-interfaces := mach_interface mach_port mach_host \
		   device device_request device_reply \
		   default_pager_object \
		   memory_object_user memory_object_default
server-interfaces := __exc
endif

routines = $(interface-routines) \
	   mach_init vm_page_size \
	   mig_strncpy mig_support msg \
	   msg_destroy msg_server msg_server_t \
	   __msg_destroy __msg_server \
	   devstream

tests := hello


all:


# Define mach-syscalls and sysno-*.
#include mach-syscalls.mk
dont_distribute := syscall_sw.h
mach-syscalls.mk: syscall_sw.h syscalls.awk
	sed -n -e '/STANDALONE/,$$d' \
	       -e 's/^kernel_trap(\(.*\),\([-0-9]*\),[0-9]*)$$/\1 \2/p' \
	< $< | awk -f $(word 2,$^) > $@-new		      
	mv $@-new $@

ifdef mach-syscalls
$(mach-syscalls:%=__%.S): __%.S: mach-syscalls.mk
	(echo '#include <sysdep.h>'; \
	 echo 'SYSCALL_TRAP (__$*, $(sysno-$*))') > $@
endif

mach-shortcuts := $(patsubst syscall_%,%,$(filter syscall_%,$(mach-syscalls)))

# Make the MiG stubs for $(mach-shortcuts) be CALL_rpc.
migdefines := $(migdefines) \
	      $(foreach call,$(mach-shortcuts),-D$(call)=$(call)_rpc)

# This rule needs to come before the implicit rules in Machrules.
__%.c: shortcut.awk __%_rpc.c
	gawk -v call=__$* -v rpc=__$*_rpc -v syscall=__syscall_$* \
	     -f $^ > $@-new
	mv $@-new $@

include Machrules

interface-routines := $(filter-out %_rpc,$(interface-routines)) \
		      $(foreach call,$(mach-shortcuts),\
				__$(call) $(call) __syscall_$(call))
dont_distribute := $(dont_distribute) \
		   $(interface-routines:%=%.c) $(interface-headers)

include ../Rules


# There is already a mach.h, so mach.defs generates mach_interface.h.
mach_interface.defs: mach.defs
	ln -s $< $@ || cp $< $@
# There is already a memory_object.h,
# so memory_object.defs generates memory_object_user.h.
memory_object_user.defs: memory_object.defs
	ln -s $< $@ || cp $< $@

# Be sure not to make these with implicit rules from foo.defs.
ifdef mach-headers
$(mach-headers): ;
endif
