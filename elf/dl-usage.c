/* Print usage information and help for ld.so.
   Copyright (C) 1995-2020 Free Software Foundation, Inc.
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

#include <dl-cache.h>
#include <dl-main.h>
#include <ldsodefs.h>
#include <unistd.h>
#include "version.h"

void
_dl_usage (const char *argv0, const char *wrong_option)
{
  if (wrong_option != NULL)
    _dl_error_printf ("%s: unrecognized option '%s'\n", argv0, wrong_option);
  else
    _dl_error_printf ("%s: missing program name\n", argv0);
  _dl_error_printf ("Try '%s --help' for more information.\n", argv0);
  _exit (EXIT_FAILURE);
}

void
_dl_version (void)
{
  _dl_printf ("\
ld.so " PKGVERSION RELEASE " release version " VERSION ".\n\
Copyright (C) 2020 Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.\n\
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A\n\
PARTICULAR PURPOSE.\n\
");
  _exit (EXIT_SUCCESS);
}

void
_dl_help (const char *argv0, struct dl_main_state *state)
{
  _dl_printf ("\
Usage: %s [OPTION]... EXECUTABLE-FILE [ARGS-FOR-PROGRAM...]\n\
You have invoked `ld.so', the helper program for shared library executables.\n\
This program usually lives in the file `/lib/ld.so', and special directives\n\
in executable files using ELF shared libraries tell the system's program\n\
loader to load the helper program from this file.  This helper program loads\n\
the shared libraries needed by the program executable, prepares the program\n\
to run, and runs it.  You may invoke this helper program directly from the\n\
command line to load and run an ELF executable file; this is like executing\n\
that file itself, but always uses this helper program from the file you\n\
specified, instead of the helper program file specified in the executable\n\
file you run.  This is mostly of use for maintainers to test new versions\n\
of this helper program; chances are you did not intend to run this program.\n\
\n\
  --list                list all dependencies and how they are resolved\n\
  --verify              verify that given object really is a dynamically linked\n\
                        object we can handle\n\
  --inhibit-cache       Do not use " LD_SO_CACHE "\n\
  --library-path PATH   use given PATH instead of content of the environment\n\
                        variable LD_LIBRARY_PATH\n\
  --inhibit-rpath LIST  ignore RUNPATH and RPATH information in object names\n\
                        in LIST\n\
  --audit LIST          use objects named in LIST as auditors\n\
  --preload LIST        preload objects named in LIST\n\
  --argv0 STRING        set argv[0] to STRING before running\n\
  --help                display this help and exit\n\
  --version             output version information and exit\n\
",
              argv0);
  _exit (EXIT_SUCCESS);
}
