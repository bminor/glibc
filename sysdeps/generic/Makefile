ifeq	($(subdir),math)
ifndef	math-twiddled

routines:= $(filter-out acos asin cos sin ceil __rint hypot,$(routines)) \
	   sincos asincos
aux	:= $(aux) exp__E log__L

math-twiddled := t

endif

bsdmath_dirs := $(bsdmath_dirs) common common_source

endif

ifeq	($(subdir),string)
dont_distribute := $(dont_distribute) endian.h	# Generated.
endif

$(objpfx)endian.h: $(objpfx)det_endian
	$(dir $<)$(notdir $<) > $@-tmp
	mv $@-tmp $@

$(objpfx)det_endian: $(sysdep_dir)/generic/det_endian.c
	$(CC) $< -o $@

before-compile := $(before-compile) $(objpfx)endian.h
