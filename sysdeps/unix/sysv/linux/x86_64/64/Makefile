# The default ABI is 64.
default-abi := 64

ifeq ($(subdir),elf)

tests-map-32bit = \
  tst-map-32bit-1a \
  tst-map-32bit-1b \
# tests-map-32bit
tst-map-32bit-1a-no-pie = yes
tst-map-32bit-1b-no-pie = yes
tests += $(tests-map-32bit)

modules-map-32bit = \
  tst-map-32bit-mod \
# modules-map-32bit
modules-names += $(modules-map-32bit)

$(objpfx)tst-map-32bit-mod.so: $(libsupport)
tst-map-32bit-1a-ENV = LD_PREFER_MAP_32BIT_EXEC=1
$(objpfx)tst-map-32bit-1a: $(objpfx)tst-map-32bit-mod.so
tst-map-32bit-1b-ENV = GLIBC_TUNABLES=glibc.cpu.prefer_map_32bit_exec=1
$(objpfx)tst-map-32bit-1b: $(objpfx)tst-map-32bit-mod.so

endif
