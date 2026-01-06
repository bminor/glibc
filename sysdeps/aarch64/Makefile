long-double-fcts = yes

ifeq ($(subdir),elf)
sysdep-dl-routines += \
  dl-bti \
  dl-gcs \
  # sysdep-dl-routines

tests += \
  tst-audit26 \
  tst-audit27 \
  # tests

modules-names += \
  tst-audit26mod \
  tst-audit27mod \
  tst-auditmod26 \
  tst-auditmod27 \
  # modules-names

$(objpfx)tst-audit26: $(objpfx)tst-audit26mod.so \
		      $(objpfx)tst-auditmod26.so
LDFLAGS-tst-audit26 += -Wl,-z,lazy
tst-audit26-ENV = LD_AUDIT=$(objpfx)tst-auditmod26.so

$(objpfx)tst-audit27: $(objpfx)tst-audit27mod.so \
		      $(objpfx)tst-auditmod27.so
$(objpfx)tst-audit27mod.so: $(libsupport)
LDFLAGS-tst-audit27 += -Wl,-z,lazy
tst-audit27-ENV = LD_AUDIT=$(objpfx)tst-auditmod27.so
endif

ifeq ($(subdir),elf)
sysdep-rtld-routines += dl-start
sysdep-dl-routines += tlsdesc dl-tlsdesc
gen-as-const-headers += \
  dl-link.sym \
  rtld-global-offsets.sym

tests-internal += \
  tst-ifunc-arg-1 \
  tst-ifunc-arg-2 \
  tst-ifunc-arg-3 \
  tst-ifunc-arg-4 \
  # tests-internal

tests += tst-vpcs
modules-names += tst-vpcs-mod
LDFLAGS-tst-vpcs-mod.so = -Wl,-z,lazy
$(objpfx)tst-vpcs: $(objpfx)tst-vpcs-mod.so
endif

ifeq ($(subdir),csu)
gen-as-const-headers += \
  tlsdesc.sym \
  rtld-global-offsets.sym
endif

ifeq ($(subdir),gmon)
CFLAGS-mcount.c += -mgeneral-regs-only
endif

ifeq ($(subdir),math)
CPPFLAGS += -I../soft-fp
endif

ifeq ($(subdir),misc)
sysdep_headers += sys/ifunc.h
sysdep_routines += \
  __mtag_tag_zero_region \
  __mtag_tag_region \
  __arm_za_disable \
  __alloc_gcs

tests += \
  tst-sme-jmp \
  tst-sme-signal \
  tst-sme-za-state \
  # tests
tests-internal += \
  tst-sme-clone \
  tst-sme-clone3 \
  tst-sme-fork \
  tst-sme-vfork \
  # tests-internal

$(objpfx)tst-sme-clone3: $(objpfx)clone3.o $(objpfx)__arm_za_disable.o

ifeq (yes,$(have-test-bti))

tests += \
  tst-bti-abort-imm \
  tst-bti-abort-static \
  tst-bti-abort-transitive \
  tst-bti-abort-unprot \
  tst-bti-dep-prot \
  tst-bti-dlopen-imm \
  tst-bti-dlopen-prot \
  tst-bti-dlopen-transitive \
  tst-bti-ld-debug-both \
  tst-bti-ld-debug-dlopen \
  tst-bti-ld-debug-exe \
  tst-bti-ld-debug-shared \
  tst-bti-permissive-dlopen \
  tst-bti-permissive-imm \
  tst-bti-permissive-transitive \
  # tests

modules-names += \
  tst-bti-mod \
  tst-bti-mod-prot \
  tst-bti-mod-unprot \
  # modules-names

$(objpfx)tst-bti-abort-imm: $(objpfx)tst-bti-mod-unprot.so
$(objpfx)tst-bti-abort-transitive: $(objpfx)tst-bti-mod.so
$(objpfx)tst-bti-abort-unprot: $(objpfx)tst-bti-mod-prot.so
$(objpfx)tst-bti-dep-prot: $(objpfx)tst-bti-mod-prot.so
$(objpfx)tst-bti-mod.so: $(objpfx)tst-bti-mod-unprot.so
$(objpfx)tst-bti-permissive-imm: $(objpfx)tst-bti-mod-unprot.so
$(objpfx)tst-bti-permissive-transitive: $(objpfx)tst-bti-mod.so
$(objpfx)tst-bti-ld-debug-shared: $(objpfx)tst-bti-mod.so
$(objpfx)tst-bti-ld-debug-both: $(objpfx)tst-bti-mod-unprot.so

CFLAGS-tst-bti-abort-unprot.o += -mbranch-protection=none
CFLAGS-tst-bti-ld-debug-exe.o += -mbranch-protection=none
CFLAGS-tst-bti-ld-debug-both.o += -mbranch-protection=none
CFLAGS-tst-bti-mod-unprot.os += -mbranch-protection=none

tst-bti-abort-imm-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_bti=1
tst-bti-abort-transitive-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_bti=1
tst-bti-abort-unprot-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_bti=1
tst-bti-dep-prot-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_bti=1
tst-bti-dlopen-imm-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_bti=1
tst-bti-dlopen-prot-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_bti=1
tst-bti-dlopen-transitive-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_bti=1

tst-bti-permissive-imm-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_bti=0
tst-bti-permissive-transitive-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_bti=0
tst-bti-permissive-dlopen-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_bti=0

define run-bti-abort-test
  $(test-wrapper-env) $(run-program-env) \
  $(tst-bti-abort-$*-ENV) $(host-test-program-cmd)
endef

$(objpfx)tst-bti-abort-%.out: $(..)sysdeps/aarch64/tst-bti-abort.sh \
	$(objpfx)tst-bti-abort-%
	$(SHELL) $< $(common-objpfx) $(test-name) '$(run-bti-abort-test)'; \
	$(evaluate-test)

tests-static += \
  tst-bti-abort-static \
  # tests-static

tst-bti-abort-static-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_bti=1
CFLAGS-tst-bti-abort-static.o += -mbranch-protection=none

$(objpfx)tst-bti-ld-debug-%.out: $(..)elf/tst-dl-debug-protect.sh $(objpfx)tst-bti-ld-debug-%
	$(SHELL) $< $(objpfx) '$(test-wrapper-env)' '$(rtld-prefix)' \
	  '$(run-program-env) GLIBC_TUNABLES=glibc.cpu.aarch64_bti=0' \
	  'security: not compatible with AArch64 BTI: $(objpfx)' \
	  $(objpfx)tst-bti-ld-debug-$* > $@; $(evaluate-test)

endif # ifeq (yes,$(have-test-bti))

endif

ifeq ($(subdir),malloc)
sysdep_malloc_debug_routines = __mtag_tag_zero_region __mtag_tag_region
endif # malloc directory
