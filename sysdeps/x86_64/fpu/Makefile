ifeq ($(subdir),mathvec)
libmvec-support += svml_d_cos2_core svml_d_cos4_core_avx \
		   svml_d_cos4_core svml_d_cos8_core \
		   svml_d_sin2_core svml_d_sin4_core_avx \
		   svml_d_sin4_core svml_d_sin8_core svml_d_trig_data \
		   svml_s_cosf4_core svml_s_cosf8_core_avx \
		   svml_s_cosf8_core svml_s_cosf16_core svml_s_trig_data \
		   svml_s_sinf4_core svml_s_sinf8_core_avx \
		   svml_s_sinf8_core svml_s_sinf16_core \
		   svml_d_sincos2_core svml_d_sincos4_core_avx \
		   svml_d_sincos4_core svml_d_sincos8_core \
		   svml_d_log2_core svml_d_log4_core_avx svml_d_log4_core \
		   svml_d_log8_core svml_d_log_data svml_s_logf4_core \
		   svml_s_logf8_core_avx svml_s_logf8_core svml_s_logf16_core \
		   svml_s_logf_data svml_d_exp2_core svml_d_exp4_core_avx \
		   svml_d_exp4_core svml_d_exp8_core svml_d_exp_data \
		   svml_s_expf4_core svml_s_expf8_core_avx svml_s_expf8_core \
		   svml_s_expf16_core svml_s_expf_data svml_d_pow2_core \
		   svml_d_pow4_core_avx svml_d_pow4_core svml_d_pow8_core \
		   svml_d_pow_data svml_s_powf4_core svml_s_powf8_core_avx \
		   svml_s_powf8_core svml_s_powf16_core svml_s_powf_data \
		   svml_s_sincosf4_core svml_s_sincosf8_core_avx \
		   svml_s_sincosf8_core svml_s_sincosf16_core
endif

# Variables for libmvec tests.
ifeq ($(subdir),math)
ifeq ($(build-mathvec),yes)
libmvec-tests += double-vlen2 double-vlen4 double-vlen4-avx2 \
		 float-vlen4 float-vlen8 float-vlen8-avx2
tests += test-double-libmvec-sincos test-double-libmvec-sincos-avx \
	 test-double-libmvec-sincos-avx2 test-float-libmvec-sincosf \
	 test-float-libmvec-sincosf-avx test-float-libmvec-sincosf-avx2
test-extras += test-libmvec test-libmvec-avx \
	       test-libmvec-avx2 test-libmvec-avx512f
extra-test-objs += test-libmvec.o test-libmvec-avx.o \
		   test-libmvec-avx2.o test-libmvec-avx512f.o

$(objpfx)test-double-libmvec-sincos: \
  $(objpfx)test-double-libmvec-sincos.o \
  $(objpfx)test-libmvec.o $(libmvec)

$(objpfx)test-double-libmvec-sincos-avx: \
  $(objpfx)test-double-libmvec-sincos-avx.o \
  $(objpfx)test-libmvec-avx.o $(libmvec)

$(objpfx)test-double-libmvec-sincos-avx2: \
  $(objpfx)test-double-libmvec-sincos-avx2.o \
  $(objpfx)test-libmvec-avx2.o $(libmvec)

$(objpfx)test-float-libmvec-sincosf: \
  $(objpfx)test-float-libmvec-sincosf.o \
  $(objpfx)test-libmvec.o $(libmvec)

$(objpfx)test-float-libmvec-sincosf-avx: \
  $(objpfx)test-float-libmvec-sincosf-avx.o \
  $(objpfx)test-libmvec-avx.o $(libmvec)

$(objpfx)test-float-libmvec-sincosf-avx2: \
  $(objpfx)test-float-libmvec-sincosf-avx2.o \
  $(objpfx)test-libmvec-avx2.o $(libmvec)

libmvec-tests += double-vlen8 float-vlen16
tests += test-double-libmvec-sincos-avx512f \
	 test-float-libmvec-sincosf-avx512f
test-extras += test-libmvec-avx512f
extra-test-objs += test-libmvec-avx512f.o

$(objpfx)test-double-libmvec-sincos-avx512f: \
  $(objpfx)test-double-libmvec-sincos-avx512f.o \
  $(objpfx)test-libmvec-avx512f.o $(libmvec)

$(objpfx)test-float-libmvec-sincosf-avx512f: \
  $(objpfx)test-float-libmvec-sincosf-avx512f.o \
  $(objpfx)test-libmvec-avx512f.o $(libmvec)

double-vlen2-funcs = cos exp log pow sin sincos
double-vlen4-funcs = cos exp log pow sin sincos
double-vlen4-avx2-funcs = cos exp log pow sin sincos
double-vlen8-funcs = cos exp log pow sin sincos
float-vlen4-funcs = cos exp log pow sin sincos
float-vlen8-funcs = cos exp log pow sin sincos
float-vlen8-avx2-funcs = cos exp log pow sin sincos
float-vlen16-funcs = cos exp log pow sin sincos

double-vlen4-arch-ext-cflags = -mavx
double-vlen4-arch-ext2-cflags = -mavx2
double-vlen8-arch-ext-cflags = -mavx512f

float-vlen8-arch-ext-cflags = -mavx
float-vlen8-arch-ext2-cflags = -mavx2
float-vlen16-arch-ext-cflags = -mavx512f

libmvec-sincos-cflags = $(libm-test-fast-math-cflags) -fno-inline -fopenmp -Wno-unknown-pragmas

CFLAGS-test-double-vlen4-avx2-wrappers.c = $(double-vlen4-arch-ext2-cflags)

CFLAGS-test-float-vlen8-avx2-wrappers.c = $(float-vlen8-arch-ext2-cflags)

CFLAGS-test-double-libmvec-sincos.c = $(libmvec-sincos-cflags)
CFLAGS-test-double-libmvec-sincos-avx.c = $(libmvec-sincos-cflags) $(double-vlen4-arch-ext-cflags)
CFLAGS-test-double-libmvec-sincos-avx2.c = $(libmvec-sincos-cflags) $(double-vlen4-arch-ext2-cflags)
CFLAGS-test-double-libmvec-sincos-avx512f.c = $(libmvec-sincos-cflags) $(double-vlen8-arch-ext-cflags)

CFLAGS-test-float-libmvec-sincosf.c = $(libmvec-sincos-cflags)
CFLAGS-test-float-libmvec-sincosf-avx.c = $(libmvec-sincos-cflags) $(float-vlen8-arch-ext-cflags)
CFLAGS-test-float-libmvec-sincosf-avx2.c = $(libmvec-sincos-cflags) $(float-vlen8-arch-ext2-cflags)
CFLAGS-test-float-libmvec-sincosf-avx512f.c = $(libmvec-sincos-cflags) $(float-vlen16-arch-ext-cflags)
endif
endif

ifeq ($(subdir)$(config-cflags-mprefer-vector-width),mathyes)
# When compiled with -O3 -march=skylake, GCC 8 and 9 optimize some loops
# in branred.c with 256-bit vector instructions, which leads to store
# forward stall:
#
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=90579
#
# Limit vector width to 128 bits to work around this issue.  It improves
# performance of sin and cos by more than 40% on Skylake.
CFLAGS-branred.c = -mprefer-vector-width=128
endif
