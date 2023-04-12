#!/usr/bin/python3
# Copyright (C) 2023 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
#
# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <https://www.gnu.org/licenses/>.

import sys

TEMPLATE = """
#include <math.h>
#include <arm_neon.h>

#define STRIDE {stride}

#define CALL_BENCH_FUNC(v, i) (__extension__ ({{                         \\
   {rtype} mx0 = {fname}(vld1q_f{prec_short} (variants[v].in[i].arg0));  \\
   mx0; }}))

struct args
{{
  {stype} arg0[STRIDE];
  double timing;
}};

struct _variants
{{
  const char *name;
  int count;
  const struct args *in;
}};

static const struct args in0[{rowcount}] = {{
{in_data}
}};

static const struct _variants variants[1] = {{
  {{"", {rowcount}, in0}},
}};

#define NUM_VARIANTS 1
#define NUM_SAMPLES(i) (variants[i].count)
#define VARIANT(i) (variants[i].name)

static {rtype} volatile ret;

#define BENCH_FUNC(i, j) ({{ ret = CALL_BENCH_FUNC(i, j); }})
#define FUNCNAME "{fname}"
#include <bench-libmvec-skeleton.c>
"""

def main(name):
    _, prec, _, func = name.split("-")
    scalar_to_advsimd_type = {"double": "float64x2_t", "float": "float32x4_t"}

    stride = {"double": 2, "float": 4}[prec]
    rtype = scalar_to_advsimd_type[prec]
    atype = scalar_to_advsimd_type[prec]
    fname = f"_ZGVnN{stride}v_{func}{'f' if prec == 'float' else ''}"
    prec_short = {"double": 64, "float": 32}[prec]

    with open(f"../benchtests/libmvec/{func}-inputs") as f:
        in_vals = [l.strip() for l in f.readlines() if l and not l.startswith("#")]
    in_vals = [in_vals[i:i+stride] for i in range(0, len(in_vals), stride)]
    rowcount= len(in_vals)
    in_data = ",\n".join("{{" + ", ".join(row) + "}, 0}" for row in in_vals)

    print(TEMPLATE.format(stride=stride,
                          rtype=rtype,
                          atype=atype,
                          fname=fname,
                          prec_short=prec_short,
                          in_data=in_data,
                          rowcount=rowcount,
                          stype=prec))


if __name__ == "__main__":
    main(sys.argv[1])
