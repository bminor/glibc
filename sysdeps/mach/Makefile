# If we were configured with `--with-mach=DIR', then config.make sets
# mach-srcdir to DIR.  Otherwise guess we are in a big source tree.
ifndef mach-srcdir
mach-srcdir = ../../mach
endif

# When compiling, use the Mach header files directly from the kernel sources.
includes += -I$(mach-srcdir)

# Find Mach header files in the kernel source.
vpath mach/%.h $(mach-srcdir)
vpath device/%.h $(mach-srcdir)
