ifeq ($(subdir),mathvec)
libmvec-sysdep_routines += vec_d_cos2_vsx vec_s_cosf4_vsx \
			   vec_d_sin2_vsx vec_s_sinf4_vsx \
			   vec_d_log2_vsx vec_d_log_data \
			   vec_s_logf4_vsx vec_s_logf_data \
			   vec_s_expf4_vsx vec_s_exp2f_data \
			   vec_s_powf4_vsx e_powf_log2_data \
			   vec_math_errf vec_math_err \
			   vec_d_exp2_vsx vec_d_exp_data \
			   vec_d_sincos2_vsx vec_s_sincosf4_vsx
CFLAGS-vec_d_cos2_vsx.c += -mabi=altivec -maltivec -mvsx -mpower8-vector
CFLAGS-vec_d_log2_vsx.c += -mabi=altivec -maltivec -mvsx -mpower8-vector
CFLAGS-vec_d_log_data.c += -mabi=altivec -maltivec -mvsx -mpower8-vector
CFLAGS-vec_math_errf.c += -mabi=altivec -maltivec -mvsx
CFLAGS-vec_s_cosf4_vsx.c += -mabi=altivec -maltivec -mvsx
CFLAGS-vec_s_logf4_vsx.c += -mabi=altivec -maltivec -mvsx
CFLAGS-vec_s_logf_data.c += -mabi=altivec -maltivec -mvsx
CFLAGS-vec_d_sin2_vsx.c += -mabi=altivec -maltivec -mvsx -mpower8-vector
CFLAGS-vec_s_sinf4_vsx.c += -mabi=altivec -maltivec -mvsx
CFLAGS-vec_d_sincos2_vsx.c += -mabi=altivec -maltivec -mvsx -mpower8-vector
CFLAGS-vec_s_sincosf4_vsx.c += -mabi=altivec -maltivec -mvsx
CFLAGS-vec_s_expf4_vsx.c += -mabi=altivec -maltivec -mvsx
CFLAGS-vec_s_exp2f_data.c += -mabi=altivec -maltivec -mvsx
CFLAGS-vec_d_exp2_vsx.c += -mabi=altivec -maltivec -mvsx -mpower8-vector
CFLAGS-vec_d_exp_data.c += -mabi=altivec -maltivec -mvsx
CFLAGS-vec_s_powf4_vsx.c += -mabi=altivec -maltivec -mvsx -mpower8-vector
CFLAGS-vec_math_err.c += -mabi=altivec -maltivec -mvsx
endif

# Variables for libmvec tests.
ifeq ($(subdir),math)
ifeq ($(build-mathvec),yes)
libmvec-tests += double-vlen2 float-vlen4

double-vlen2-funcs = cos sin sincos log exp
float-vlen4-funcs = cos sin sincos log exp pow

double-vlen2-arch-ext-cflags = -mabi=altivec -maltivec -mvsx -DREQUIRE_VSX
float-vlen4-arch-ext-cflags = -mabi=altivec -maltivec -mvsx -DREQUIRE_VSX

CFLAGS-test-double-vlen2-wrappers.c += -mabi=altivec -maltivec -mvsx -DREQUIRE_VSX
CFLAGS-test-float-vlen4-wrappers.c += -mabi=altivec -maltivec -mvsx -DREQUIRE_VSX

endif
endif
