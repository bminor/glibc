# If we were configured with `--with-mach=DIR', then config.make sets
# mach-srcdir to DIR.  Otherwise guess we are in a big source tree.
ifndef mach-srcdir
mach-srcdir = ../../mach
endif

# Look for header files in mach/ under the top-level library source directory.
includes += -I$(..)mach

# When compiling, use the Mach header files directly from the kernel sources.
includes += -I$(mach-srcdir)

# Find Mach header files in the kernel source.
vpath mach/%.h $(mach-srcdir)
vpath device/%.h $(mach-srcdir)

ifneq (mach,$(subdir))
# Subdirectories other than mach/ might use the generated Mach headers.
# So make sure we get a chance to run in mach/ to make them before all else.

ifdef objpfx
mach_interface.h = $(objpfx)mach/mach_interface.h
else
mach_interface.h = $(..)mach/mach/mach_interface.h
endif

before-compile += $(mach_interface.h)
$(mach_interface.h):	# Run only if it doesn't exist.
	$(MAKE) -C $(..)mach $(patsubst $(..)mach/%,%,$@)
endif
