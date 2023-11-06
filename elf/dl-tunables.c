/* The tunable framework.  See the README.tunables to know how to use the
   tunable in a glibc module.

   Copyright (C) 2016-2023 Free Software Foundation, Inc.
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

/* Mark symbols hidden in static PIE for early self relocation to work.  */
#if BUILD_PIE_DEFAULT
# pragma GCC visibility push(hidden)
#endif
#include <startup.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sysdep.h>
#include <fcntl.h>
#include <ldsodefs.h>
#include <array_length.h>
#include <dl-minimal-malloc.h>

#define TUNABLES_INTERNAL 1
#include "dl-tunables.h"

#include <not-errno.h>

static char *
tunables_strdup (const char *in)
{
  size_t i = 0;

  while (in[i++] != '\0');
  char *out = __minimal_malloc (i + 1);

  /* For most of the tunables code, we ignore user errors.  However,
     this is a system error - and running out of memory at program
     startup should be reported, so we do.  */
  if (out == NULL)
    _dl_fatal_printf ("failed to allocate memory to process tunables\n");

  while (i-- > 0)
    out[i] = in[i];

  return out;
}

static char **
get_next_env (char **envp, char **name, size_t *namelen, char **val,
	      char ***prev_envp)
{
  while (envp != NULL && *envp != NULL)
    {
      char **prev = envp;
      char *envline = *envp++;
      int len = 0;

      while (envline[len] != '\0' && envline[len] != '=')
	len++;

      /* Just the name and no value, go to the next one.  */
      if (envline[len] == '\0')
	continue;

      *name = envline;
      *namelen = len;
      *val = &envline[len + 1];
      *prev_envp = prev;

      return envp;
    }

  return NULL;
}

static void
do_tunable_update_val (tunable_t *cur, const tunable_val_t *valp,
		       const tunable_num_t *minp,
		       const tunable_num_t *maxp)
{
  tunable_num_t val, min, max;

  if (cur->type.type_code == TUNABLE_TYPE_STRING)
    {
      cur->val.strval = valp->strval;
      cur->initialized = true;
      return;
    }

  bool unsigned_cmp = unsigned_tunable_type (cur->type.type_code);

  val = valp->numval;
  min = minp != NULL ? *minp : cur->type.min;
  max = maxp != NULL ? *maxp : cur->type.max;

  /* We allow only increasingly restrictive bounds.  */
  if (tunable_val_lt (min, cur->type.min, unsigned_cmp))
    min = cur->type.min;

  if (tunable_val_gt (max, cur->type.max, unsigned_cmp))
    max = cur->type.max;

  /* Skip both bounds if they're inconsistent.  */
  if (tunable_val_gt (min, max, unsigned_cmp))
    {
      min = cur->type.min;
      max = cur->type.max;
    }

  /* Bail out if the bounds are not valid.  */
  if (tunable_val_lt (val, min, unsigned_cmp)
      || tunable_val_lt (max, val, unsigned_cmp))
    return;

  cur->val.numval = val;
  cur->type.min = min;
  cur->type.max = max;
  cur->initialized = true;
}

/* Validate range of the input value and initialize the tunable CUR if it looks
   good.  */
static void
tunable_initialize (tunable_t *cur, const char *strval)
{
  tunable_val_t val;

  if (cur->type.type_code != TUNABLE_TYPE_STRING)
    val.numval = (tunable_num_t) _dl_strtoul (strval, NULL);
  else
    val.strval = strval;
  do_tunable_update_val (cur, &val, NULL, NULL);
}

void
__tunable_set_val (tunable_id_t id, tunable_val_t *valp, tunable_num_t *minp,
		   tunable_num_t *maxp)
{
  tunable_t *cur = &tunable_list[id];

  do_tunable_update_val (cur, valp, minp, maxp);
}

struct tunable_toset_t
{
  tunable_t *t;
  const char *value;
};

enum { tunables_list_size = array_length (tunable_list) };

/* Parse the tunable string VALSTRING and set TUNABLES with the found tunables
   and their respective strings.  VALSTRING is a duplicated values,  where
   delimiters ':' are replaced with '\0', so string tunables are null
   terminated.
   Return the number of tunables found (including 0 if the string is empty)
   or -1 if for an ill-formatted definition.  */
static int
parse_tunables_string (char *valstring, struct tunable_toset_t *tunables)
{
  if (valstring == NULL || *valstring == '\0')
    return 0;

  char *p = valstring;
  bool done = false;
  int ntunables = 0;

  while (!done)
    {
      char *name = p;

      /* First, find where the name ends.  */
      while (*p != '=' && *p != ':' && *p != '\0')
	p++;

      /* If we reach the end of the string before getting a valid name-value
	 pair, bail out.  */
      if (*p == '\0')
	return -1;

      /* We did not find a valid name-value pair before encountering the
	 colon.  */
      if (*p == ':')
	{
	  p++;
	  continue;
	}

      /* Skip the '='.  */
      p++;

      char *value = p;

      while (*p != '=' && *p != ':' && *p != '\0')
	p++;

      if (*p == '=')
	return -1;
      else if (*p == '\0')
	done = true;
      else
	*p++ = '\0';

      /* Add the tunable if it exists.  */
      for (size_t i = 0; i < tunables_list_size; i++)
	{
	  tunable_t *cur = &tunable_list[i];

	  if (tunable_is_name (cur->name, name))
	    {
	      tunables[ntunables++] = (struct tunable_toset_t) { cur, value };
	      break;
	    }
	}
    }

  return ntunables;
}

static void
parse_tunables (char *valstring)
{
  struct tunable_toset_t tunables[tunables_list_size];
  int ntunables = parse_tunables_string (valstring, tunables);
  if (ntunables == -1)
    {
      _dl_error_printf (
        "WARNING: ld.so: invalid GLIBC_TUNABLES `%s': ignored.\n", valstring);
      return;
    }

  for (int i = 0; i < ntunables; i++)
    tunable_initialize (tunables[i].t, tunables[i].value);
}

/* Initialize the tunables list from the environment.  For now we only use the
   ENV_ALIAS to find values.  Later we will also use the tunable names to find
   values.  */
void
__tunables_init (char **envp)
{
  char *envname = NULL;
  char *envval = NULL;
  size_t len = 0;
  char **prev_envp = envp;

  /* Ignore tunables for AT_SECURE programs.  */
  if (__libc_enable_secure)
    return;

  while ((envp = get_next_env (envp, &envname, &len, &envval,
			       &prev_envp)) != NULL)
    {
      if (tunable_is_name ("GLIBC_TUNABLES", envname))
	{
	  parse_tunables (tunables_strdup (envval));
	  continue;
	}

      for (int i = 0; i < tunables_list_size; i++)
	{
	  tunable_t *cur = &tunable_list[i];

	  /* Skip over tunables that have either been set already or should be
	     skipped.  */
	  if (cur->initialized || cur->env_alias[0] == '\0')
	    continue;

	  const char *name = cur->env_alias;

	  /* We have a match.  Initialize and move on to the next line.  */
	  if (tunable_is_name (name, envname))
	    {
	      tunable_initialize (cur, envval);
	      break;
	    }
	}
    }
}

void
__tunables_print (void)
{
  for (int i = 0; i < array_length (tunable_list); i++)
    {
      const tunable_t *cur = &tunable_list[i];
      if (cur->type.type_code == TUNABLE_TYPE_STRING
	  && cur->val.strval == NULL)
	_dl_printf ("%s:\n", cur->name);
      else
	{
	  _dl_printf ("%s: ", cur->name);
	  switch (cur->type.type_code)
	    {
	    case TUNABLE_TYPE_INT_32:
	      _dl_printf ("%d (min: %d, max: %d)\n",
			  (int) cur->val.numval,
			  (int) cur->type.min,
			  (int) cur->type.max);
	      break;
	    case TUNABLE_TYPE_UINT_64:
	      _dl_printf ("0x%lx (min: 0x%lx, max: 0x%lx)\n",
			  (long int) cur->val.numval,
			  (long int) cur->type.min,
			  (long int) cur->type.max);
	      break;
	    case TUNABLE_TYPE_SIZE_T:
	      _dl_printf ("0x%zx (min: 0x%zx, max: 0x%zx)\n",
			  (size_t) cur->val.numval,
			  (size_t) cur->type.min,
			  (size_t) cur->type.max);
	      break;
	    case TUNABLE_TYPE_STRING:
	      _dl_printf ("%s\n", cur->val.strval);
	      break;
	    default:
	      __builtin_unreachable ();
	    }
	}
    }
}

/* Set the tunable value.  This is called by the module that the tunable exists
   in. */
void
__tunable_get_val (tunable_id_t id, void *valp, tunable_callback_t callback)
{
  tunable_t *cur = &tunable_list[id];

  switch (cur->type.type_code)
    {
    case TUNABLE_TYPE_UINT_64:
	{
	  *((uint64_t *) valp) = (uint64_t) cur->val.numval;
	  break;
	}
    case TUNABLE_TYPE_INT_32:
	{
	  *((int32_t *) valp) = (int32_t) cur->val.numval;
	  break;
	}
    case TUNABLE_TYPE_SIZE_T:
	{
	  *((size_t *) valp) = (size_t) cur->val.numval;
	  break;
	}
    case TUNABLE_TYPE_STRING:
	{
	  *((const char **)valp) = cur->val.strval;
	  break;
	}
    default:
      __builtin_unreachable ();
    }

  if (cur->initialized && callback != NULL)
    callback (&cur->val);
}

rtld_hidden_def (__tunable_get_val)
