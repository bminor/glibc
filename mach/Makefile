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

distribute = Machrules interface.awk # $(interfaces:%=%.defs)

headers = mach_init.h \
	  $(addprefix mach/,\
		      	    host_info.h kern_return.h \
			    mach_param.h mach_types.h \
			    memory_object.h \
			    message.h mig_errors.h msg_type.h \
			    policy.h port.h processor_info.h \
			    std_types.h task_info.h task_special_ports.h \
			    thread_info.h thread_special_ports.h \
			    thread_status.h thread_switch.h time_value.h \
			    vm_attributes.h vm_inherit.h vm_prot.h \
			    vm_statistics.h)

ifneq (,)
user-interfaces		:= mach_interface mach_port \
			   mach_host mach_debug \
			   device device_request device_reply \
			   default_pager_object \
			   memory_object_user \
			   memory_object_default
server-interfaces	:= __exc
endif

user-interfaces = foobar
server-interfaces = foobar __foobar

include Machrules

routines = $(interface-routines)
headers = $(interface-headers)
ifneq (,)
routines := mach_init mach_init_syms \
	    mig_strncpy mig_support msg \
	    mach_traps mach_syscalls mach_shortcuts \
	    mach_msg_destroy mach_msg_server mach_msg_server_timeout \
	    __mach_msg_destroy __mach_msg_server \
	    devstream
tests := hello
endif

include ../Rules


include mach-shortcuts
mach-shortcuts: mach_shortcuts.h
	(echo 'mach-shortcuts := ' \\ ; \
	 sed -n 's/SHORTCUT[^(]*(\([^,]*\),.*$$/  \1 '\\\\'/p' < $<) > $@

# Make the MiG stubs for $(mach-shortcuts) be mig_foo.
ifneq (,)
migdefines := $(migdefines) \
	      $(foreach call,$(mach-shortcuts),-D$(call)=mig_$(call))
endif


# There is already a mach.h, so mach.defs generates mach_interface.h.
mach_interface.defs: mach.defs
	ln -s $< $@ || cp $< $@
# There is already a memory_object.h,
# so memory_object.defs generates memory_object_user.h.
memory_object_user.defs: memory_object.defs
	ln -s $< $@ || cp $< $@
