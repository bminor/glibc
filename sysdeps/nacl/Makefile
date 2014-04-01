# Copyright (C) 2014 Free Software Foundation, Inc.
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
# <http://www.gnu.org/licenses/>.

# The (required) --with-headers option to configure sets sysheaders to the
# location of the native_client/.. source directory.  We'll get necessary
# headers directly from there.
naclsrc = $(sysheaders)/native_client/src

# How to find the directory containing this Makefile.
nacl = $(..)nacl-port/sysdeps/nacl

# Generate our bits/errno.h with the numbers from NaCl's sys/errno.h file.
nacl-errno = $(naclsrc)/trusted/service_runtime/include/sys/errno.h

bits-errno = $(common-objpfx)bits/errno.h
$(bits-errno): $(common-objpfx)stamp-errnos ;
$(common-objpfx)stamp-errnos: $(nacl)/errnos.awk $(..)manual/errno.texi \
			      $(nacl-errno)
	$(make-target-directory)
	$(AWK) -f $^ > $(bits-errno)-tmp
# Make it unwritable so noone will edit it by mistake.
	-chmod a-w $(bits-errno)-tmp
	$(move-if-change) $(bits-errno)-tmp $(bits-errno)
	touch $@
common-generated += stamp-errnos bits/errno.h
before-compile += $(bits-errno)

nacl-irt.h = $(common-objpfx)nacl-irt.h
$(nacl-irt.h): $(nacl)/irt.sed $(naclsrc)/untrusted/irt/irt.h
	sed -f $^ > $@.new
	mv -f $@.new $@
common-generated += nacl-irt.h
before-compile += $(nacl-irt.h)

ifeq ($(subdir),misc)

# We reuse the Linux file since the bits match.  The file lives in the
# top-level source tree so we can use it without reference to any
# sysdeps/.../linux/ directories, but it's still a sysdeps decision to
# install it.
sysdep_headers += bits/mman-linux.h

$(objpfx)nacl-interfaces.v.i: $(nacl)/nacl-interfaces.mk.in \
			      $(nacl)/nacl-interface-list.h
-include $(objpfx)nacl-interfaces.v

nacl-all-interfaces = $(nacl-mandatory-interfaces) $(nacl-optional-interfaces)
nacl-interface-routines = $(nacl-all-interfaces:%=nacl-interface-%)

define nacl-interface-table-command
(echo '#define INTERFACE_CATEGORY $1'; \
 echo '#define INTERFACE_TYPE $*'; \
 echo '#define INTERFACE_STRING $(nacl-$*-string)'; \
 echo '#include "nacl-interface-table.c"' \
) > $@T
mv -f $@T $@
endef

nacl-interface-pattern = $(objpfx)nacl-interface-%.c

$(nacl-mandatory-interfaces:%=$(nacl-interface-pattern)): \
  $(nacl-interface-pattern): $(nacl)/Makefile $(objpfx)nacl-interfaces.v
	$(make-target-directory)
	$(call nacl-interface-table-command,mandatory)
$(nacl-optional-interfaces:%=$(nacl-interface-pattern)): \
  $(nacl-interface-pattern): $(nacl)/Makefile $(objpfx)nacl-interfaces.v
	$(make-target-directory)
	$(call nacl-interface-table-command,optional)

sysdep_routines += nacl_interface_query \
		   nacl-interfaces $(nacl-interface-routines)

# XXX temp test
others += hello hellow
others-static += hello

endif

ifeq ($(subdir),io)
sysdep_routines += xstatconv
endif
