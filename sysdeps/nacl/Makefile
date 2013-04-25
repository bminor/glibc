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

ifeq ($(subdir),misc)

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
  $(nacl-interface-pattern): $(nacl)/Makefile
	$(make-target-directory)
	$(call nacl-interface-table-command,mandatory)
$(nacl-optional-interfaces:%=$(nacl-interface-pattern)): \
  $(nacl-interface-pattern): $(nacl)/Makefile
	$(make-target-directory)
	$(call nacl-interface-table-command,optional)

sysdep_routines += nacl_interface_query \
		   nacl-interfaces $(nacl-interface-routines)

# The Linux bits are close enough for NaCl and we want to use the existing
# file rather than copying it into sysdeps/nacl and letting it bit rot.
# But we certainly don't want sysdeps/unix/sysv/linux in our sysdirs list!
# So copy the file to the build directory.
sysdep_headers += bits/mman-linux.h
before-compile += $(common-objpfx)bits/mman-linux.h
$(common-objpfx)bits/mman-linux.h: \
  $(..)sysdeps/unix/sysv/linux/bits/mman-linux.h
	$(make-target-directory)
	ln -f $< $@ || cp -f $< $@

# XXX temp test
others += hello

endif
