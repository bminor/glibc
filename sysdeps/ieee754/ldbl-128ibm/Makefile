# The`long double' type is a distinct type we support if
# -mlong-double-128 option is used (or when it becomes a default
# when -mlong-double-64 is not used).
long-double-fcts = yes
ifneq ($(with-clang),yes)
sysdep-CFLAGS += -mlong-double-128
endif

ifeq ($(subdir),stdlib)
tests += tst-strtold-ldbl-128ibm
$(objpfx)tst-strtold-ldbl-128ibm: $(libm)
endif

ifeq ($(subdir),math)
tests += test-fmodl-ldbl-128ibm test-remainderl-ldbl-128ibm \
	 test-remquol-ldbl-128ibm test-canonical-ldbl-128ibm \
	 test-totalorderl-ldbl-128ibm
endif

# clang crashing a lot trying to optimize any of this
ifeq ($(with-clang),yes)
CFLAGS-test-canonical-ldbl-128ibm.c = -O0
CFLAGS-test-totalorderl-ldbl-128ibm.c = -O0

CFLAGS-e_acoshl.c = -O0
CFLAGS-e_acosl.c = -O0
CFLAGS-e_asinl.c = -O0
CFLAGS-e_atan2l.c = -O0
CFLAGS-e_atanhl.c = -O0
CFLAGS-e_coshl.c = -O0
CFLAGS-e_exp10l.c = -O0
CFLAGS-e_expl.c = -O0
CFLAGS-e_fmodl.c = -O0
CFLAGS-e_gammal_r.c = -O0
CFLAGS-e_hypotl.c = -O0
CFLAGS-e_ilogbl.c = -O0
CFLAGS-e_j0l.c = -O0
CFLAGS-e_j1l.c = -O0
CFLAGS-e_jnl.c = -O0
CFLAGS-e_lgammal_r.c = -O0
CFLAGS-e_log10l.c = -O0
CFLAGS-e_log2l.c = -O0
CFLAGS-e_logl.c = -O0
CFLAGS-e_powl.c = -O0
CFLAGS-e_remainderl.c = -O0
CFLAGS-e_rem_pio2l.c = -O0
CFLAGS-e_sinhl.c = -O0
CFLAGS-e_sqrtl.c = -O0
CFLAGS-gamma_productl.c = -O0
CFLAGS-k_cosl.c = -O0
CFLAGS-k_sincosl.c = -O0
CFLAGS-k_sinl.c = -O0
CFLAGS-k_tanl.c = -O0
CFLAGS-ldbl2mpn.c = -O0
CFLAGS-lgamma_negl.c = -O0
CFLAGS-lgamma_productl.c = -O0
CFLAGS-mpn2ldbl.c = -O0
CFLAGS-printf_fphex.c = -O0
CFLAGS-s_asinhl.c = -O0
CFLAGS-s_atanl.c = -O0
CFLAGS-s_cbrtl.c = -O0
CFLAGS-s_ceill.c = -O0
CFLAGS-s_copysignl.c = -O0
CFLAGS-s_cosl.c = -O0
CFLAGS-s_erfl.c = -O0
CFLAGS-s_expm1l.c = -O0
CFLAGS-s_fabsl.c = -O0
CFLAGS-s_finitel.c = -O0
CFLAGS-s_floorl.c = -O0
CFLAGS-s_fmal.c = -O0
CFLAGS-s_fpclassifyl.c = -O0
CFLAGS-s_frexpl.c = -O0
CFLAGS-s_fromfpl.c = -O0
CFLAGS-s_fromfpl_main.c = -O0
CFLAGS-s_fromfpxl.c = -O0
CFLAGS-s_getpayloadl.c = -O0
CFLAGS-s_iscanonicall.c = -O0
CFLAGS-s_isinfl.c = -O0
CFLAGS-s_isnanl.c = -O0
CFLAGS-s_issignalingl.c = -O0
CFLAGS-s_llrintl.c = -O0
CFLAGS-s_llroundl.c = -O0
CFLAGS-s_log1pl.c = -O0
CFLAGS-s_logbl.c = -O0
CFLAGS-s_lrintl.c = -O0
CFLAGS-s_lroundl.c = -O0
CFLAGS-s_modfl.c = -O0
CFLAGS-s_nearbyintl.c = -O0
CFLAGS-s_nextafterl.c = -O0
CFLAGS-s_nexttoward.c = -O0
CFLAGS-s_nexttowardf.c = -O0
CFLAGS-s_nextupl.c = -O0
CFLAGS-s_remquol.c = -O0
CFLAGS-s_rintl.c = -O0
CFLAGS-s_roundevenl.c = -O0
CFLAGS-s_roundl.c = -O0
CFLAGS-s_scalblnl.c = -O0
CFLAGS-s_scalbnl.c = -O0
CFLAGS-s_setpayloadl.c = -O0
CFLAGS-s_setpayloadl_main.c = -O0
CFLAGS-s_setpayloadsigl.c = -O0
CFLAGS-s_signbitl.c = -O0
CFLAGS-s_sincosl.c = -O0
CFLAGS-s_sinl.c = -O0
CFLAGS-s_tanhl.c = -O0
CFLAGS-s_tanl.c = -O0
CFLAGS-s_totalorderl.c = -O0
CFLAGS-s_totalordermagl.c = -O0
CFLAGS-s_truncl.c = -O0
CFLAGS-s_ufromfpl.c = -O0
CFLAGS-s_ufromfpxl.c = -O0
CFLAGS-t_sincosl.c = -O0
endif
