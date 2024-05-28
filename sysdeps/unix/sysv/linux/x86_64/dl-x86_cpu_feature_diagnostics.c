/* CPU diagnostics probing.  Linux/x86-64 version.
   Copyright (C) 2024 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <cpu-features.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/wait.h>
#include <sysdep.h>

static void
_dl_x86_probe (struct x86_cpu_feature_diagnostics *diag, bool reported,
               void (*probe) (void))
{
  if (reported)
    diag->reported |= 1ULL << diag->count;

  /* Use fork/waitid for crash handling.  This is simpler than using
     signal handling: it does not need global data to communicate with
     the handler, nor building out-of-line helper functions to the
     baseline ISA, and it avoids dealing differences in sigset_t size.  */
  long int ret = INTERNAL_SYSCALL_CALL (fork);
  if (ret == 0)
    {
      /* New process that runs the probe.  This may trigger a crash.  */
      probe ();

      INTERNAL_SYSCALL_CALL (exit_group, 0);
    }
  else if (ret > 0)
    {
      siginfo_t si;
      ret = INTERNAL_SYSCALL_CALL (waitid, P_PID, ret, &si, WEXITED, NULL);
      if (ret >=0 && si.si_status == 0)
        /* Probe was successful.   */
        diag->probed |= 1ULL << diag->count;
    }

  ++diag->count;
}

void
_dl_x86_cpu_feature_diagnostics_run (const struct cpu_features *cpu_features,
                                     struct x86_cpu_feature_diagnostics *diag)
{
  /* x86-64-v2 features.  */
  extern void _dl_x86_probe_cmpxchg16b (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, CMPXCHG16B),
                 _dl_x86_probe_cmpxchg16b);

  extern void _dl_x86_probe_sahf (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, LAHF64_SAHF64),
                 _dl_x86_probe_sahf);

  extern void _dl_x86_probe_popcnt (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, POPCNT),
                 _dl_x86_probe_popcnt);

  extern void _dl_x86_probe_sse3 (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, SSE3),
                 _dl_x86_probe_sse3);

  extern void _dl_x86_probe_sse4_1 (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, SSE4_1),
                 _dl_x86_probe_sse4_1);

  extern void _dl_x86_probe_sse4_2 (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, SSE4_2),
                 _dl_x86_probe_sse4_2);

  extern void _dl_x86_probe_ssse3 (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, SSSE3),
                 _dl_x86_probe_ssse3);

  /* x86-64-v3 features.  */
  extern void _dl_x86_probe_avx (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AVX),
                 _dl_x86_probe_avx);

  /* AVX probe using xmm registers.  */
  extern void _dl_x86_probe_avx_xmm (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AVX),
                 _dl_x86_probe_avx_xmm);

  extern void _dl_x86_probe_avx2 (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AVX2),
                 _dl_x86_probe_avx2);

  extern void _dl_x86_probe_bmi1 (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, BMI1),
                 _dl_x86_probe_bmi1);

  /* Alternative BMI1 probe.  Perhaps harder to mask.  */
  extern void _dl_x86_probe_bmi1_tzcnt (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, BMI1),
                 _dl_x86_probe_bmi1_tzcnt);

  extern void _dl_x86_probe_bmi2 (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, BMI2),
                 _dl_x86_probe_bmi2);

  extern void _dl_x86_probe_f16c (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, F16C),
                 _dl_x86_probe_f16c);

  extern void _dl_x86_probe_fma (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, FMA),
                 _dl_x86_probe_fma);

  /* FMA4 is not part of x86-64-v3, but may produce a useful hint.  */
  extern void _dl_x86_probe_fma4 (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, FMA4),
                 _dl_x86_probe_fma4);

  extern void _dl_x86_probe_lzcnt (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, LZCNT),
                 _dl_x86_probe_lzcnt);

  extern void _dl_x86_probe_movbe (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, MOVBE),
                 _dl_x86_probe_movbe);

  extern void _dl_x86_probe_osxsave (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, OSXSAVE),
                 _dl_x86_probe_osxsave);

  /* x86-64-v4 features.  */
  extern void _dl_x86_probe_avx512f (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AVX512F),
                 _dl_x86_probe_avx512f);

  extern void _dl_x86_probe_avx512bw (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AVX512BW),
                 _dl_x86_probe_avx512bw);

  extern void _dl_x86_probe_avx512bw_ymm (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AVX512BW),
                 _dl_x86_probe_avx512bw_ymm);

  extern void _dl_x86_probe_avx512cd (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AVX512CD),
                 _dl_x86_probe_avx512cd);

  extern void _dl_x86_probe_avx512cd_ymm0 (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AVX512CD),
                 _dl_x86_probe_avx512cd_ymm0);

  extern void _dl_x86_probe_avx512vl (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AVX512VL),
                 _dl_x86_probe_avx512vl);

  /* Other CPU features, not part of microarchitecture levels.  */
  extern void _dl_x86_probe_adx (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, ADX),
                 _dl_x86_probe_adx);

  extern void _dl_x86_probe_aes (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AES),
                 _dl_x86_probe_aes);

  extern void _dl_x86_probe_aes_avx (void) attribute_hidden;
  _dl_x86_probe (diag,
                 CPU_FEATURE_USABLE_P (cpu_features, AES)
                 && CPU_FEATURE_USABLE_P (cpu_features, AVX),
                 _dl_x86_probe_aes_avx);

  extern void _dl_x86_probe_vaes (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, VAES),
                 _dl_x86_probe_vaes);

  extern void _dl_x86_probe_sha (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, SHA),
                 _dl_x86_probe_sha);

  extern void _dl_x86_probe_avx512_vbmi (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AVX512_VBMI),
                 _dl_x86_probe_avx512_vbmi);

  extern void _dl_x86_probe_avx512_vbmi_xmm (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AVX512_VBMI),
                 _dl_x86_probe_avx512_vbmi_xmm);

  extern void _dl_x86_probe_avx512_vbmi2 (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AVX512_VBMI2),
                 _dl_x86_probe_avx512_vbmi2);

  extern void _dl_x86_probe_avx512_vbmi2_xmm (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AVX512_VBMI2),
                 _dl_x86_probe_avx512_vbmi2_xmm);

  extern void _dl_x86_probe_avx_vnni (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AVX_VNNI),
                 _dl_x86_probe_avx_vnni);

  extern void _dl_x86_probe_avx512_vnni (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AVX512_VNNI),
                 _dl_x86_probe_avx512_vnni);

  extern void _dl_x86_probe_avx512_ifma (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, AVX512_IFMA),
                 _dl_x86_probe_avx512_ifma);

  serialize
  tpause
  ptwrite
  xsusldtrk
  clmul
  crc32

  extern void _dl_x86_probe_apx_f (void) attribute_hidden;
  _dl_x86_probe (diag, CPU_FEATURE_USABLE_P (cpu_features, APX_F),
                 _dl_x86_probe_apx_f);
}
