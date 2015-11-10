ifeq ($(subdir),elf)
CFLAGS-.os += $(if $(filter $(@F),$(patsubst %,%.os,$(all-rtld-routines))),\
		   -mno-sse -mno-mmx)

tests-special += $(objpfx)tst-ld-sse-use.out
$(objpfx)tst-ld-sse-use.out: ../sysdeps/x86/tst-ld-sse-use.sh $(objpfx)ld.so
	@echo "Checking ld.so for SSE register use.  This will take a few seconds..."
	$(BASH) $< $(objpfx) '$(NM)' '$(OBJDUMP)' '$(READELF)' > $@; \
	$(evaluate-test)

tests += tst-prelink
tst-prelink-ENV = LD_TRACE_PRELINKING=1

$(objpfx)tst-prelink-conflict.out: $(objpfx)tst-prelink.out
	grep stdout $< | grep conflict | $(AWK) '{ print $$10, $$11 }' > $@

$(objpfx)tst-prelink-cmp.out: $(..)sysdeps/x86/tst-prelink.exp \
			      $(objpfx)tst-prelink-conflict.out
	cmp $^ > $@; \
	$(evaluate-test)

ifeq ($(run-built-tests),yes)
tests-special += $(objpfx)tst-prelink-cmp.out
endif
endif
