# Copyright (C) 1991, 1992, 1994 Free Software Foundation, Inc.
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

#
#	Makefile for math.
#
subdir	:= math

headers	:= math.h __math.h huge_val.h nan.h


routines := acos asin atan cos sin tan cosh sinh tanh exp fabs ldexp	\
	    log log10 floor sqrt fmod frexp pow atan2 ceil modf		\
	    isinf isnan finite infnan copysign scalb drem logb		\
	    __isinf __isnan __finite __infnan __copysign __scalb __drem __logb\
	    __rint rint hypot cabs cbrt __expm1 expm1 log1p acosh asinh atanh 
tests := # test-math
install-lib := libm.a


include ../Rules

$(objpfx)libm.a:
	$(AR) cr $@ /dev/null

lib: $(objpfx)libm.a

ifneq (,)     # the code is now distributed through ../sysdeps/

# Other dirs to look for source files (for dist).
export source_dirs = $(filter-out unused,$(shell find bsd -type d -print))

include $(objpfx)BSDmath-files
$(objpfx)BSDmath-files:
	(echo define +ansideclificate-bsd;				\
	 echo "(echo '#include <ansidecl.h>';				\
	        echo '#include \"\$$<\"') > \$$@-tmp;			\
		mv \$$@-tmp \$$@";					\
	 echo endef;			    				\
	 for dir in $(source_dirs); do					\
	   echo "\$$(objpfx)%.c: $${dir}/%.c;\$$(+ansideclificate-bsd)";\
	 done) > $@-tmp
	mv $@-tmp $@


ifdef bsdmath_dirs

override CPPFLAGS := $(CPPFLAGS) -Ibsd $(addprefix -Ibsd/,$(bsdmath_dirs))

+bsdpath := $(subst $+ ,:,bsd $(addprefix bsd/,$(bsdmath_dirs)))
vpath %.s $(+bsdpath)
vpath %.h $(+bsdpath)

+bsdfiles := $(wildcard $(patsubst %,bsd/%/*.c,$(bsdmath_dirs)))

ifdef +bsdfiles

# Find all the files which have both BSD and sysdep versions.
+sysdeps := $(notdir $(wildcard \
		       $(foreach dir, \
				 $(filter-out %/generic %/stub, \
					      $(+sysdep_dirs)), \
				 $(addprefix $(dir)/, \
					     $(notdir $(+bsdfiles))))))

# Filter these out of the list of BSD files.
+bsdfiles := $(filter-out $(addprefix %/,$(+sysdeps)),$(+bsdfiles))

ifdef +bsdfiles
# Assert that all the BSD C sources exist in the object directory,
# so VPATH will find them there first.
$(addprefix $(objpfx),$(notdir $(+bsdfiles))):
endif

# See how easy this would be in make v4?
ifneq (,)
define bsd-files
$(foreach dir,$(bsdmath_dirs),
$(objpfx)%.c: bsd/$(dir)/%.c
	(echo '#include <ansidecl.h>'; echo '#include "$<") > $@-tmp
	mv $@-tmp $@
endef
$(bsd-files)
endif

ifneq ($(findstring gcc,$(CC)),)
# Disable GCC warnings for grody BSD code.
override CFLAGS := $(filter-out -W%,$(CFLAGS))
# In make v4, put "$(+bsdfiles): " in front of that.
endif

endif	# +bsdfiles

endif	# bsdmath_dirs

endif
