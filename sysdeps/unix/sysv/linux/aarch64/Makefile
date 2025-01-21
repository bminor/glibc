ifeq ($(subdir),misc)
sysdep_headers += sys/elf.h
tests += \
  tst-aarch64-pkey \
  # tests

ifneq (no,$(findstring no,$(have-cc-gcs) $(have-test-cc-gcs) $(have-ld-gcs)))

gcs-tests-dynamic = \
  tst-gcs-disabled \
  tst-gcs-enforced \
  tst-gcs-enforced-abort \
  tst-gcs-optional-off \
  tst-gcs-optional-on \
  tst-gcs-override \
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
