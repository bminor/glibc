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

ifeq ($(subdir),misc)

sysdep_headers := $(sysdep_headers) sys/param.h
dont_distribute := $(dont_distribute) sys/param.h	# Generated.

endif

before-compile := $(before-compile) $(common-objpfx)sys/param.h

$(common-objpfx)sys/param.h: $(sysincludedir)/sys/param.h $(common-objpfx)sys/
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

$(common-objpfx)param.h.c: $(sysincludedir)/sys/param.h
	rm -f $@
	cp $< $@
$(common-objpfx)param.h.dep: $(common-objpfx)param.h.c \
			     $(sysdep_dir)/unix/Makefile
	$(+mkdep) -I$(sysincludedir) $<		\
	| sed > $@				\
	      -e 's@^.*:@@'			\
	      -e 's@$<@@g'			\
	      -e 's@$(sysincludedir)/*@@g'	\
	      -e 's@\\$$@@'			\
	      -e 's@^@sys/param.h-includes := $$(sys/param.h-includes) @'

ifeq ($(subdir),misc)

# Install system headers the system sys/param.h uses.

include $(common-objpfx)param.h.dep

# Don't preempt our own headers. 
sys/param.h-includes := \
  $(filter-out $(patsubst $(..)%,%,\
			  $(wildcard $(addprefix $(..),\
				     $(sys/param.h-includes)))),\
	       $(sys/param.h-includes))

sysdep_headers := $(sysdep_headers) $(sys/param.h-includes)
dont_distribute := $(dont_distribute) $(sys/param.h-includes)	# Generated.

ifdef sys/param.h-includes
# Copy the system files to $(common-objdir).
$(addprefix $(common-objpfx),$(sys/param.h-includes)): $(common-objpfx)%: \
						       $(sysincludedir)/%
	-mkdir $(dir $@)
# Some of these files sometimes contain conflicting declarations
# for htons, ntohs, etc.
	sed '/[ 	]*[hn][sl]to[nh][sl][	 (]*/d' $< > $@-tmp
	mv $@-tmp $@
endif

endif # misc

ifeq "$(dir $(firstword $(wildcard $(+sysdep_dirs:%=%/errnos.h))))" \
     "$(sysdep_dir)/stub/"
# These need to exist before any compiling is done,
# so cpp doesn't instead find the stub versions.
before-compile := $(before-compile) $(common-objpfx)errnos.h 

$(common-objpfx)errnos.h: $(common-objpfx)make-errnos
	$(dir $<)$(notdir $<) > $@-tmp
	mv $@-tmp $@

$(common-objpfx)make-errnos: $(common-objpfx)make-errnos.c
	$(common-objdir-compile)

$(common-objpfx)make-errnos.c: $(sysdep_dir)/unix/errnos-tmpl.c \
			$(sysdep_dir)/unix/errnos.awk $(common-objpfx)errnos
	awk -f $(word 2,$^) errnos="`tr '\012' ' ' < $(word 3,$^)`" $< > $@-tmp
	mv $@-tmp $@

$(common-objpfx)errnos: $(wildcard $(sysincludedir)/errno.h	\
				   $(sysincludedir)/sys/errno.h)
	cat $^ | sed -n 's/^#define[ 	]*\(E[A-Z0-9]*\)[ 	]*[0-9][0-9]*[	]*.*$$/\1/p' > $@-tmp
	mv $@-tmp $@
endif

ifeq "$(dir $(firstword $(wildcard $(+sysdep_dirs:%=%/ioctls.h))))" \
     "$(sysdep_dir)/stub/"
before-compile := $(before-compile) $(common-objpfx)ioctls.h

$(common-objpfx)ioctls.h: $(common-objpfx)make-ioctls
	$(dir $<)$(notdir $<) > $@-tmp
	mv $@-tmp $@

$(common-objpfx)make-ioctls: $(common-objpfx)make-ioctls.c
	$(common-objdir-compile)

$(common-objpfx)make-ioctls.c: $(sysdep_dir)/unix/ioctls-tmpl.c \
			       $(sysdep_dir)/unix/ioctls.awk \
			       $(common-objpfx)ioctls
	awk -f $(word 2,$^) requests="`cat $(word 3,$^)`" $< > $@-tmp
	mv $@-tmp $@

$(common-objpfx)ioctls: $(sysdep_dir)/unix/snarf-ioctls \
			$(sysincludedir)/sys/ioctl.h
	$(dir $<)$(notdir $<) $(word 2,$^) | tr '\012' ' ' > $@-tmp
	mv $@-tmp $@
endif

ifeq ($(subdir),stdio)

before-compile: $(objpfx)errlist.c
$(objpfx)errlist.c: $(objpfx)make_errlist
	@rm -f $@
	$< > $@-tmp
	mv $@-tmp $@

$(objpfx)make_errlist: $(sysdep_dir)/unix/make_errlist.c
	$(native-compile)

endif

ifeq "$(dir $(firstword $(wildcard $(+sysdep_dirs:%=%/syscall.h))))" \
     "$(sysdep_dir)/stub/"

# The syscall code assumes a file <syscall.h> that defines macros
# `SYS_call' for syscall `call'.  Variations on this I have seen include:
# it's in <sys/syscall.h>;
# it's in <sys.s>;
# it defines `CALL' instead of `SYS_call'.

# Find a file that might have these.
syscall.h := $(firstword $(wildcard $(addprefix $(sysincludedir)/, \
						syscall.h sys/syscall.h \
						sys.s sys/sys.s \
						sys.S sys/sys.S)))
ifdef syscall.h

# Transmogrify any of several formats of the file into the one we want.
$(common-objpfx)syscall.h: $(syscall.h)
	tr '[A-Z]' '[a-z]' < $< | \
	sed -e 's/[ 	]sys_/ /' \
	    -e 's/^#define[ 	]*\([a-z0-9_]*\)[ 	]*/#define SYS_\1 /' \
	    -e 's/[ 	]sys_/ SYS_/' \
	    -e '/^#/!d' \
	    -e '/^#ident/d' \
	> $@-tmp
	mv $@-tmp $@

before-compile := $(before-compile) $(common-objpfx)syscall.h

endif

endif
