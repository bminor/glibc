# Copyright (C) 1991 Free Software Foundation, Inc.
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

all:

subdir := mach

headers = mach_init.h
ifneq (,)
	  $(addprefix mach/,$(interface-headers) \
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
endif

user-interfaces		:= mach_interface mach_port \
			   mach_host mach_debug \
			   device device_request \
			   default_page_object \
			   memory_object_user \
			   memory_object_default
server-interfaces	:= exc device_reply

include Machrules

routines := mach_init mach_init_syms \
	    mig_strncpy mig_support msg \
	    mach_traps mach_syscalls mach_shortcuts \
	    mach_msg_destroy mach_msg_server mach_msg_server_timeout \
	    __mach_msg_destroy __mach_msg_server \
	    $(interface-routines) \
	    devstream
tests := hello


include ../Rules


mach_interface.defs: mach.defs
	ln $< $@ || cp $< $@
memory_object_user.defs: memory_object.defs
	ln $< $@ || cp $< $@
