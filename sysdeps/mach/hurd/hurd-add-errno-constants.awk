# Add Hurd-specific constants to errno_constants.pysym.
# Copyright (C) 2017 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

# On the Hurd, errno.h defines E-constants corresponding to a number of
# Mach low-level errors.  Add these to the set of values recognized by
# stdlib/errno-printer.py.  This script must be kept in sync with errnos.awk.

BEGIN {
  in_mach_errors = "";
  in_mig_errors = 0;
  in_device_errors = 0;
}

function emit_subhead()
{
  header = FILENAME;
  sub(/.*include\//, "", header);
  printf("\n-- Errors from <%s>\n", header);
}

NF == 3 && $1 == "#define" && $2 == "MACH_SEND_IN_PROGRESS" \
  {
    in_mach_errors = FILENAME;
    emit_subhead();
  }
NF == 3 && $1 == "#define" && $2 == "KERN_SUCCESS" \
  {
    in_mach_errors = FILENAME;
    emit_subhead();
    next;
  }
in_mach_errors != "" && $2 == "MACH_IPC_COMPAT" \
  {
    in_mach_errors = "";
    next;
  }

$1 == "#define" && $2 == "_MACH_MIG_ERRORS_H_" \
  {
    in_mig_errors = 1;
    emit_subhead();
    next;
  }
in_mig_errors && $1 == "#endif" && $3 == "_MACH_MIG_ERRORS_H_" \
  {
    in_mig_errors = 0;
  }

$1 == "#define" && $2 == "D_SUCCESS" \
  {
    in_device_errors = 1;
    emit_subhead();
    next;
  }
in_device_errors && $1 == "#endif" \
  {
    in_device_errors = 0;
  }

(in_mach_errors == FILENAME && NF == 3 && $1 == "#define") || \
(in_mig_errors && $1 == "#define" && $3 <= -300) || \
(in_device_errors && $1 == "#define" && $2 ~ /D_/ && NF > 3) \
  {
    print "E" $2;
  }
