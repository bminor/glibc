/* dlinfo -- Get information from the dynamic linker.
   Copyright (C) 2003-2020 Free Software Foundation, Inc.
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

#include <dlfcn.h>
#include <link.h>
#include <ldsodefs.h>
#include <libintl.h>

#if !defined SHARED && IS_IN (libdl)

int
dlinfo (void *handle, int request, void *arg)
{
  return __dlinfo (handle, request, arg);
}

#else

# include <dl-tls.h>

struct dlinfo_args
{
  void *handle;
  int request;
  void *arg;
};

static void
dlinfo_doit (void *argsblock)
{
  struct dlinfo_args *const args = argsblock;
  struct link_map *l = args->handle;

  switch (args->request)
    {
    case RTLD_DI_CONFIGADDR:
    default:
      _dl_signal_error (0, NULL, NULL, N_("unsupported dlinfo request"));
      break;

    case RTLD_DI_LMID:
      *(Lmid_t *) args->arg = l->l_ns;
      break;

    case RTLD_DI_LINKMAP:
      *(struct link_map **) args->arg = l;
      break;

    case RTLD_DI_SERINFO:
      _dl_rtld_di_serinfo (l, args->arg, false);
      break;
    case RTLD_DI_SERINFOSIZE:
      _dl_rtld_di_serinfo (l, args->arg, true);
      break;

    case RTLD_DI_ORIGIN:
      strcpy (args->arg, l->l_origin);
      break;

    case RTLD_DI_TLS_MODID:
      *(size_t *) args->arg = 0;
      *(size_t *) args->arg = l->l_tls_modid;
      break;

    case RTLD_DI_TLS_DATA:
      {
	void *data = NULL;
	if (l->l_tls_modid != 0)
	  data = GLRO(dl_tls_get_addr_soft) (l);
	*(void **) args->arg = data;
	break;
      }
    }
}

int
__dlinfo (void *handle, int request, void *arg)
{
# ifdef SHARED
  if (!rtld_active ())
    return _dlfcn_hook->dlinfo (handle, request, arg);
# endif

  struct dlinfo_args args = { handle, request, arg };
  return _dlerror_run (&dlinfo_doit, &args) ? -1 : 0;
}
# ifdef SHARED
strong_alias (__dlinfo, dlinfo)
# endif
#endif
