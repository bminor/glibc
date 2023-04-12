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
#include <arm_sve.h>

#define MAX_STRIDE {max_stride}
#define STRIDE {stride}
#define PTRUE svptrue_b{prec_short}
#define SV_LOAD svld1_f{prec_short}
#define SV_STORE svst1_f{prec_short}
#define REQUIRE_SVE

#define CALL_BENCH_FUNC(v, i) (__extension__ ({{                              \\
   {rtype} mx0 = {fname}(SV_LOAD (PTRUE(), variants[v].in[i].arg0), PTRUE()); \\
   mx0; }}))

struct args
{{
  {stype} arg0[MAX_STRIDE];
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

// Cannot pass volatile pointer to svst1. This still does not appear to get optimised out.
static {stype} /*volatile*/ ret[MAX_STRIDE];

#define BENCH_FUNC(i, j) ({{ SV_STORE(PTRUE(), ret, CALL_BENCH_FUNC(i, j)); }})
#define FUNCNAME "{fname}"
#include <bench-libmvec-skeleton.c>
"""

def main(name):
    _, prec, _, func = name.split("-")
    scalar_to_sve_type = {"double": "svfloat64_t", "float": "svfloat32_t"}

    stride = {"double": "svcntd()", "float": "svcntw()"}[prec]
    rtype = scalar_to_sve_type[prec]
    atype = scalar_to_sve_type[prec]
    fname = f"_ZGVsMxv_{func}{'f' if prec == 'float' else ''}"
    prec_short = {"double": 64, "float": 32}[prec]
    # Max SVE vector length is 2048 bits. To ensure benchmarks are
    # vector-length-agnostic, but still use as wide vectors as
    # possible on any given target, divide input data into 2048-bit
    # rows, then load/store as many elements as the target will allow.
    max_stride = 2048 // prec_short

    with open(f"../benchtests/libmvec/{func}-inputs") as f:
        in_vals = [l.strip() for l in f.readlines() if l and not l.startswith("#")]
    in_vals = [in_vals[i:i+max_stride] for i in range(0, len(in_vals), max_stride)]
    rowcount= len(in_vals)
    in_data = ",\n".join("{{" + ", ".join(row) + "}, 0}" for row in in_vals)

    print(TEMPLATE.format(stride=stride,
                          rtype=rtype,
                          atype=atype,
                          fname=fname,
                          prec_short=prec_short,
                          in_data=in_data,
                          rowcount=rowcount,
                          stype=prec,
                          max_stride=max_stride))


if __name__ == "__main__":
    main(sys.argv[1])
