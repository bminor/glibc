ifeq ($(subdir),elf)
CFLAGS-.os += $(if $(filter $(@F),$(patsubst %,%.os,$(all-rtld-routines))),\
		   -mno-sse -mno-mmx)

tests: $(objpfx)tst-xmmymm.out
$(objpfx)tst-xmmymm.out: ../sysdeps/x86/tst-xmmymm.sh $(objpfx)ld.so
	@echo "Checking ld.so for SSE register use.  This will take a few seconds..."
	$(SHELL) $< $(objpfx) '$(NM)' '$(OBJDUMP)' '$(READELF)' > $@

ifeq (yesyes,$(build-shared)$(multi-arch))
tests += tst-ifunc-cpu1-main
modules-names += tst-ifunc-cpu1-mod

tst-ifunc-cpu1-mod.so-no-z-defs = yes

$(objpfx)tst-ifunc-cpu1-main: $(objpfx)tst-ifunc-cpu1-mod.so
endif
endif
