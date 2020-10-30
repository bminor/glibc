/* Support for reading /etc/ld.so.cache files written by Linux ldconfig.
   Copyright (C) 1996-2020 Free Software Foundation, Inc.
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

#include <assert.h>
#include <unistd.h>
#include <ldsodefs.h>
#include <sys/mman.h>
#include <dl-cache.h>
#include <dl-procinfo.h>
#include <stdint.h>
#include <_itoa.h>
#include <dl-hwcaps.h>

#ifndef _DL_PLATFORMS_COUNT
# define _DL_PLATFORMS_COUNT 0
#endif

/* This is the starting address and the size of the mmap()ed file.  */
static struct cache_file *cache;
static struct cache_file_new *cache_new;
static size_t cachesize;

/* True if PTR is a valid string table index.  */
static inline bool
_dl_cache_verify_ptr (uint32_t ptr, size_t string_table_size)
{
  return ptr < string_table_size;
}

/* Compute the address of the element INDEX of the array at LIBS.
   Conceptually, this is &LIBS[INDEX], but use ENTRY_SIZE for the size
   of *LIBS.  */
static inline const struct file_entry *
_dl_cache_file_entry (const struct file_entry *libs, size_t entry_size,
		      size_t index)
{
  return (const void *) libs + index * entry_size;
}

/* We use binary search since the table is sorted in the cache file.
   The first matching entry in the table is returned.  It is important
   to use the same algorithm as used while generating the cache file.
   STRING_TABLE_SIZE indicates the maximum offset in STRING_TABLE at
   which data is mapped; it is not exact.  */
static const char *
search_cache (const char *string_table, uint32_t string_table_size,
	      struct file_entry *libs, uint32_t nlibs, uint32_t entry_size,
	      const char *name)
{
  /* Used by the HWCAP check in the struct file_entry_new case.  */
  uint64_t platform = _dl_string_platform (GLRO (dl_platform));
  if (platform != (uint64_t) -1)
    platform = 1ULL << platform;
  uint64_t hwcap_mask = GET_HWCAP_MASK ();
#define _DL_HWCAP_TLS_MASK (1LL << 63)
  uint64_t hwcap_exclude = ~((GLRO (dl_hwcap) & hwcap_mask)
			     | _DL_HWCAP_PLATFORM | _DL_HWCAP_TLS_MASK);

  int left = 0;
  int right = nlibs - 1;
  const char *best = NULL;

  while (left <= right)
    {
      int middle = (left + right) / 2;
      uint32_t key = _dl_cache_file_entry (libs, entry_size, middle)->key;

      /* Make sure string table indices are not bogus before using
	 them.  */
      if (!_dl_cache_verify_ptr (key, string_table_size))
	return NULL;

      /* Actually compare the entry with the key.  */
      int cmpres = _dl_cache_libcmp (name, string_table + key);
      if (__glibc_unlikely (cmpres == 0))
	{
	  /* Found it.  LEFT now marks the last entry for which we
	     know the name is correct.  */
	  left = middle;

	  /* There might be entries with this name before the one we
	     found.  So we have to find the beginning.  */
	  while (middle > 0)
	    {
	      key = _dl_cache_file_entry (libs, entry_size, middle - 1)->key;
	      /* Make sure string table indices are not bogus before
		 using them.  */
	      if (!_dl_cache_verify_ptr (key, string_table_size)
		  /* Actually compare the entry.  */
		  || _dl_cache_libcmp (name, string_table + key) != 0)
		break;
	      --middle;
	    }

	  do
	    {
	      int flags;
	      const struct file_entry *lib
		= _dl_cache_file_entry (libs, entry_size, middle);

	      /* Only perform the name test if necessary.  */
	      if (middle > left
		  /* We haven't seen this string so far.  Test whether the
		     index is ok and whether the name matches.  Otherwise
		     we are done.  */
		  && (! _dl_cache_verify_ptr (lib->key, string_table_size)
		      || (_dl_cache_libcmp (name, string_table + lib->key)
			  != 0)))
		break;

	      flags = lib->flags;
	      if (_dl_cache_check_flags (flags)
		  && _dl_cache_verify_ptr (lib->value, string_table_size))
		{
		  if (best == NULL || flags == GLRO (dl_correct_cache_id))
		    {
		      if (entry_size >= sizeof (struct file_entry_new))
			{
			  /* The entry is large enough to include
			     HWCAP data.  Check it.  */
			  struct file_entry_new *libnew
			    = (struct file_entry_new *) lib;

			  if (libnew->hwcap & hwcap_exclude)
			    continue;
			  if (GLRO (dl_osversion)
			      && libnew->osversion > GLRO (dl_osversion))
			    continue;
			  if (_DL_PLATFORMS_COUNT
			      && (libnew->hwcap & _DL_HWCAP_PLATFORM) != 0
			      && ((libnew->hwcap & _DL_HWCAP_PLATFORM)
				  != platform))
			    continue;
			}

		      best = string_table + lib->value;

		      if (flags == GLRO (dl_correct_cache_id))
			/* We've found an exact match for the shared
			   object and no general `ELF' release.  Stop
			   searching.  */
			break;
		    }
		}
	    }
	  while (++middle <= right);
	  break;
	}

      if (cmpres < 0)
	left = middle + 1;
      else
	right = middle - 1;
    }

  return best;
}

int
_dl_cache_libcmp (const char *p1, const char *p2)
{
  while (*p1 != '\0')
    {
      if (*p1 >= '0' && *p1 <= '9')
        {
          if (*p2 >= '0' && *p2 <= '9')
            {
	      /* Must compare this numerically.  */
	      int val1;
	      int val2;

	      val1 = *p1++ - '0';
	      val2 = *p2++ - '0';
	      while (*p1 >= '0' && *p1 <= '9')
	        val1 = val1 * 10 + *p1++ - '0';
	      while (*p2 >= '0' && *p2 <= '9')
	        val2 = val2 * 10 + *p2++ - '0';
	      if (val1 != val2)
		return val1 - val2;
	    }
	  else
            return 1;
        }
      else if (*p2 >= '0' && *p2 <= '9')
        return -1;
      else if (*p1 != *p2)
        return *p1 - *p2;
      else
	{
	  ++p1;
	  ++p2;
	}
    }
  return *p1 - *p2;
}


/* Look up NAME in ld.so.cache and return the file name stored there, or null
   if none is found.  The cache is loaded if it was not already.  If loading
   the cache previously failed there will be no more attempts to load it.
   The caller is responsible for freeing the returned string.  The ld.so.cache
   may be unmapped at any time by a completing recursive dlopen and
   this function must take care that it does not return references to
   any data in the mapping.  */
char *
_dl_load_cache_lookup (const char *name)
{
  /* Print a message if the loading of libs is traced.  */
  if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_LIBS))
    _dl_debug_printf (" search cache=%s\n", LD_SO_CACHE);

  if (cache == NULL)
    {
      /* Read the contents of the file.  */
      void *file = _dl_sysdep_read_whole_file (LD_SO_CACHE, &cachesize,
					       PROT_READ);

      /* We can handle three different cache file formats here:
	 - only the new format
	 - the old libc5/glibc2.0/2.1 format
	 - the old format with the new format in it
	 The following checks if the cache contains any of these formats.  */
      if (file != MAP_FAILED && cachesize > sizeof *cache_new
	  && memcmp (file, CACHEMAGIC_VERSION_NEW,
		     sizeof CACHEMAGIC_VERSION_NEW - 1) == 0
	  /* Check for corruption, avoiding overflow.  */
	  && ((cachesize - sizeof *cache_new) / sizeof (struct file_entry_new)
	      >= ((struct cache_file_new *) file)->nlibs))
	{
	  cache_new = file;
	  cache = file;
	}
      else if (file != MAP_FAILED && cachesize > sizeof *cache
	       && memcmp (file, CACHEMAGIC, sizeof CACHEMAGIC - 1) == 0
	       /* Check for corruption, avoiding overflow.  */
	       && ((cachesize - sizeof *cache) / sizeof (struct file_entry)
		   >= ((struct cache_file *) file)->nlibs))
	{
	  size_t offset;
	  /* Looks ok.  */
	  cache = file;

	  /* Check for new version.  */
	  offset = ALIGN_CACHE (sizeof (struct cache_file)
				+ cache->nlibs * sizeof (struct file_entry));

	  cache_new = (struct cache_file_new *) ((void *) cache + offset);
	  if (cachesize < (offset + sizeof (struct cache_file_new))
	      || memcmp (cache_new->magic, CACHEMAGIC_VERSION_NEW,
			 sizeof CACHEMAGIC_VERSION_NEW - 1) != 0)
	    cache_new = (void *) -1;
	}
      else
	{
	  if (file != MAP_FAILED)
	    __munmap (file, cachesize);
	  cache = (void *) -1;
	}

      assert (cache != NULL);
    }

  if (cache == (void *) -1)
    /* Previously looked for the cache file and didn't find it.  */
    return NULL;

  const char *best;
  if (cache_new != (void *) -1)
    {
      const char *string_table = (const char *) cache_new;
      best = search_cache (string_table, cachesize,
			   &cache_new->libs[0].entry, cache_new->nlibs,
			   sizeof (cache_new->libs[0]), name);
    }
  else
    {
      const char *string_table = (const char *) &cache->libs[cache->nlibs];
      uint32_t string_table_size
	= (const char *) cache + cachesize - string_table;
      best = search_cache (string_table, string_table_size,
			   &cache->libs[0], cache->nlibs,
			   sizeof (cache->libs[0]), name);
    }

  /* Print our result if wanted.  */
  if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_LIBS, 0)
      && best != NULL)
    _dl_debug_printf ("  trying file=%s\n", best);

  if (best == NULL)
    return NULL;

  /* The double copy is *required* since malloc may be interposed
     and call dlopen itself whose completion would unmap the data
     we are accessing. Therefore we must make the copy of the
     mapping data without using malloc.  */
  char *temp;
  temp = alloca (strlen (best) + 1);
  strcpy (temp, best);
  return __strdup (temp);
}

#ifndef MAP_COPY
/* If the system does not support MAP_COPY we cannot leave the file open
   all the time since this would create problems when the file is replaced.
   Therefore we provide this function to close the file and open it again
   once needed.  */
void
_dl_unload_cache (void)
{
  if (cache != NULL && cache != (struct cache_file *) -1)
    {
      __munmap (cache, cachesize);
      cache = NULL;
    }
}
#endif
