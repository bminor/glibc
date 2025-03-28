ifeq ($(subdir),elf)
tests += \
  tst-tlsdesc-pac \
  # tests
modules-names += \
  tst-tlsdesc-pac-mod \
  # modules-names

LDFLAGS-tst-tlsdesc-pac = -rdynamic

$(objpfx)tst-tlsdesc-pac.out: $(objpfx)tst-tlsdesc-pac-mod.so
endif

ifeq ($(subdir),misc)
sysdep_headers += sys/elf.h
tests += \
  tst-aarch64-pkey \
  # tests

ifneq (no,$(findstring no,$(have-cc-gcs) $(have-test-cc-gcs) $(have-ld-gcs)))

gcs-tests-dynamic = \
  tst-gcs-disabled \
  tst-gcs-dlopen-disabled \
  tst-gcs-dlopen-enforced \
  tst-gcs-dlopen-optional-off \
  tst-gcs-dlopen-optional-on \
  tst-gcs-dlopen-override \
  tst-gcs-enforced \
  tst-gcs-enforced-abort \
  tst-gcs-noreturn \
  tst-gcs-optional-off \
  tst-gcs-optional-on \
  tst-gcs-override \
  tst-gcs-shared-disabled \
  tst-gcs-shared-enforced-abort \
  tst-gcs-shared-optional \
  tst-gcs-shared-override \
  # gcs-tests-dynamic

gcs-tests-static = \
  tst-gcs-disabled-static \
  tst-gcs-enforced-static \
  tst-gcs-enforced-static-abort \
  tst-gcs-optional-static-off \
  tst-gcs-optional-static-on \
  tst-gcs-override-static \
  # gcs-tests-static

tests += \
  $(gcs-tests-dynamic) \
  $(gcs-tests-static) \
  # tests

tests-static += \
  $(gcs-tests-static) \
  # tests-static

define run-gcs-abort-test
  $(test-wrapper-env) $(run-program-env) \
  $(tst-gcs-$*-abort-ENV) $(host-test-program-cmd)
endef

$(objpfx)tst-gcs-%-abort.out: $(..)sysdeps/unix/sysv/linux/aarch64/tst-gcs-abort.sh \
	$(objpfx)tst-gcs-%-abort
	$(SHELL) $< $(common-objpfx) $(test-name) '$(run-gcs-abort-test)'; \
	$(evaluate-test)

LDFLAGS-tst-gcs-disabled += -Wl,-z gcs=always
LDFLAGS-tst-gcs-enforced += -Wl,-z gcs=always
LDFLAGS-tst-gcs-enforced-abort += -Wl,-z gcs=never
LDFLAGS-tst-gcs-optional-on += -Wl,-z gcs=always
LDFLAGS-tst-gcs-optional-off += -Wl,-z gcs=never
LDFLAGS-tst-gcs-override += -Wl,-z gcs=never

LDFLAGS-tst-gcs-disabled-static += -Wl,-z gcs=always
LDFLAGS-tst-gcs-enforced-static += -Wl,-z gcs=always
LDFLAGS-tst-gcs-enforced-static-abort += -Wl,-z gcs=never
LDFLAGS-tst-gcs-optional-static-on += -Wl,-z gcs=always
LDFLAGS-tst-gcs-optional-static-off += -Wl,-z gcs=never
LDFLAGS-tst-gcs-override-static += -Wl,-z gcs=never

tst-gcs-disabled-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=0
tst-gcs-enforced-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=1
tst-gcs-enforced-abort-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=1
tst-gcs-optional-on-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=2
tst-gcs-optional-off-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=2
tst-gcs-override-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=3

tst-gcs-disabled-static-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=0
tst-gcs-enforced-static-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=1
tst-gcs-enforced-static-abort-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=1
tst-gcs-optional-static-on-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=2
tst-gcs-optional-static-off-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=2
tst-gcs-override-static-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=3

# force one of the dependencies to be unmarked
LDFLAGS-tst-gcs-mod2.so += -Wl,-z gcs=never

LDFLAGS-tst-gcs-shared-disabled = -Wl,-z gcs=always
LDFLAGS-tst-gcs-shared-enforced-abort = -Wl,-z gcs=always
LDFLAGS-tst-gcs-shared-optional = -Wl,-z gcs=always
LDFLAGS-tst-gcs-shared-override = -Wl,-z gcs=always

modules-names += \
  tst-gcs-mod1 \
  tst-gcs-mod2 \
  tst-gcs-mod3 \
  # modules-names

$(objpfx)tst-gcs-shared-disabled: $(objpfx)tst-gcs-mod1.so $(objpfx)tst-gcs-mod3.so
$(objpfx)tst-gcs-shared-enforced-abort: $(objpfx)tst-gcs-mod1.so $(objpfx)tst-gcs-mod3.so
$(objpfx)tst-gcs-shared-optional: $(objpfx)tst-gcs-mod1.so $(objpfx)tst-gcs-mod3.so
$(objpfx)tst-gcs-shared-override: $(objpfx)tst-gcs-mod1.so $(objpfx)tst-gcs-mod3.so
$(objpfx)tst-gcs-mod1.so: $(objpfx)tst-gcs-mod2.so

tst-gcs-shared-disabled-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=0
tst-gcs-shared-enforced-abort-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=1
tst-gcs-shared-optional-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=2
tst-gcs-shared-override-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=3

LDFLAGS-tst-gcs-dlopen-disabled = -Wl,-z gcs=always
LDFLAGS-tst-gcs-dlopen-enforced = -Wl,-z gcs=always
LDFLAGS-tst-gcs-dlopen-optional-on = -Wl,-z gcs=always
LDFLAGS-tst-gcs-dlopen-optional-off = -Wl,-z gcs=never
LDFLAGS-tst-gcs-dlopen-override = -Wl,-z gcs=always

tst-gcs-dlopen-disabled-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=0
tst-gcs-dlopen-enforced-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=1
tst-gcs-dlopen-optional-on-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=2
tst-gcs-dlopen-optional-off-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=2
tst-gcs-dlopen-override-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=3

$(objpfx)tst-gcs-dlopen-disabled.out: $(objpfx)tst-gcs-mod2.so
$(objpfx)tst-gcs-dlopen-enforced.out: $(objpfx)tst-gcs-mod2.so
$(objpfx)tst-gcs-dlopen-optional-on.out: $(objpfx)tst-gcs-mod2.so
$(objpfx)tst-gcs-dlopen-optional-off.out: $(objpfx)tst-gcs-mod2.so
$(objpfx)tst-gcs-dlopen-override.out: $(objpfx)tst-gcs-mod2.so

LDFLAGS-tst-gcs-noreturn = -Wl,-z gcs=always

tst-gcs-noreturn-ENV = GLIBC_TUNABLES=glibc.cpu.aarch64_gcs=0

endif # ifeq ($(have-test-cc-gcs),yes)

endif # ifeq ($(subdir),misc)

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif

abi-variants := lp64
abi-variants += lp64_be

ifeq (,$(filter $(default-abi),$(abi-variants)))
Unknown ABI, must be one of $(abi-variants)
endif

abi-lp64-condition := !defined __AARCH64EB__
abi-lp64_be-condition := defined __AARCH64EB__
