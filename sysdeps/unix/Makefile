# Copyright (C) 1991, 1992 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

# These need to exist before any compiling is done,
# so cpp doesn't instead find the stub versions.
before-compile := $(before-compile) $(objpfx)errnos.h $(objpfx)ioctls.h

$(objpfx)errnos.h: $(objpfx)make-errnos
	$(dir $<)$(notdir $<) > $@-tmp
	mv $@-tmp $@

$(objpfx)make-errnos: $(objpfx)make-errnos.c
	cd $(objdir); $(CC) $(<:$(objpfx)%=%) -o $(@:$(objpfx)%=%)

$(objpfx)make-errnos.c: $(sysdep_dir)/unix/errnos-tmpl.c \
			$(sysdep_dir)/unix/errnos.awk $(objpfx)errnos
	awk -f $(word 2,$^) errnos="`tr '\012' ' ' < $(word 3,$^)`" $< > $@-tmp
	mv $@-tmp $@

$(objpfx)errnos: $(wildcard $(sysincludedir)/errno.h	\
			    $(sysincludedir)/sys/errno.h)
	cat $^ | sed -n 's/^#define[ 	]*\(E[A-Z0-9]*\)[ 	]*[0-9][0-9]*[	]*.*$$/\1/p' > $@-tmp
	mv $@-tmp $@

ifeq ($(subdir),misc)

sysdep_headers := $(sysdep_headers) sys/param.h
dont_distribute := $(dont_distribute) sys/param.h	# Generated.

endif

before-compile := $(before-compile) $(objpfx)sys/param.h

$(objpfx)sys/param.h: $(sysincludedir)/sys/param.h 
	-mkdir $(dir $@)
	(echo '#ifndef _GNU_SYS_PARAM_H';	\
	 echo '#define _GNU_SYS_PARAM_H 1';	\
	 awk < $<			\
	   '/^#define[ 	]*NULL/ { print "#ifndef NULL";	\
				  print $$0;		\
				  print "#endif";	\
				  was_null = 1 }	\
	    { if (!was_null) print $$0;			\
	      was_null = 0 }';		\
	 echo '#endif	/* sys/param.h */') > $@-tmp
	mv $@-tmp $@

$(objpfx)param.h.c: $(sysincludedir)/sys/param.h
	rm -f $@
	cp $< $@
$(objpfx)param.h.dep: $(objpfx)param.h.c $(sysdep_dir)/unix/Makefile
	$(+mkdep) -I$(sysincludedir) $<		\
	| sed > $@				\
	      -e 's@^.*:@@'			\
	      -e 's@$<@@g'			\
	      -e 's@$(sysincludedir)/*@@g'	\
	      -e 's@\\$$@@'			\
	      -e 's@^@sys/param.h-includes := $$(sys/param.h-includes) @'

ifeq ($(subdir),misc)

# Install system headers the system sys/param.h uses.

include $(objpfx)param.h.dep

# Don't preempt our own headers. 
sys/param.h-includes := \
  $(filter-out $(wildcard $(addprefix $(..),$(sys/param.h-includes))),\
	       $(sys/param.h-includes))

headers := $(headers) $(sys/param.h-includes)
dont_distribute := $(dont_distribute) $(sys/param.h-includes)	# Generated.

ifdef sys/param.h-includes
# Copy the system files to $(objdir).
$(addprefix $(objpfx),$(sys/param.h-includes)): $(objpfx)%: $(sysincludedir)/%
	-mkdir $(dir $@)
	cp $< $@
endif

endif # misc

$(objpfx)ioctls.h: $(objpfx)make-ioctls
	$(dir $<)$(notdir $<) > $@-tmp
	mv $@-tmp $@

$(objpfx)make-ioctls: $(objpfx)make-ioctls.c
	cd $(objdir); $(CC) $(<:$(objpfx)%=%) -o $(@:$(objpfx)%=%)

$(objpfx)make-ioctls.c: $(sysdep_dir)/unix/ioctls-tmpl.c \
			$(sysdep_dir)/unix/ioctls.awk $(objpfx)ioctls
	awk -f $(word 2,$^) requests="`cat $(word 3,$^)`" $< > $@-tmp
	mv $@-tmp $@

$(objpfx)ioctls: $(sysdep_dir)/unix/snarf-ioctls $(sysincludedir)/sys/ioctl.h
	$(dir $<)$(notdir $<) $(word 2,$^) | tr '\012' ' ' > $@-tmp
	mv $@-tmp $@

ifeq ($(subdir),stdio)

before-compile: $(objpfx)errlist.c
$(objpfx)errlist.c: $(objpfx)make_errlist
	@rm -f $@
	$< > $@-tmp
	mv $@-tmp $@

$(objpfx)make_errlist: $(sysdep_dir)/unix/make_errlist.c
	$(native-compile)

endif
