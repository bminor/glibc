/* Support for reading /etc/ld.so.cache files written by Linux ldconfig.
   Copyright (C) 1996-2014 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <assert.h>
#include <unistd.h>
#include <ldsodefs.h>
#include <sys/mman.h>
#include <dl-cache.h>
#include <dl-procinfo.h>
#include <stdint.h>
#include <_itoa.h>

#ifndef _DL_PLATFORMS_COUNT
# define _DL_PLATFORMS_COUNT 0
#endif

/* This struct describes a single ld.so.cache (there could be several,
   as we want to look in $prefix/etc/ld.so.cache and in the system default
   /etc/ld.so.cache). See b/2471323.  */
struct cache_info {
  const char *ld_so_cache;
  /* This is the starting address and the size of the mmap()ed file.  */
  struct cache_file *cache;
  struct cache_file_new *cache_new;
  size_t cachesize;
};

/* Dynamically allocated; last entry is sentinel with ld_so_cache == NULL.  */
static struct cache_info *cache_info;

/* 1 if cache_data + PTR points into the cache.  */
#define _dl_cache_verify_ptr(ptr) (ptr < cache_data_size)

#define SEARCH_CACHE(cache) \
/* We use binary search since the table is sorted in the cache file.	      \
   The first matching entry in the table is returned.			      \
   It is important to use the same algorithm as used while generating	      \
   the cache file.  */							      \
do									      \
  {									      \
    left = 0;								      \
    right = cache->nlibs - 1;						      \
									      \
    while (left <= right)						      \
      {									      \
	__typeof__ (cache->libs[0].key) key;				      \
									      \
	middle = (left + right) / 2;					      \
									      \
	key = cache->libs[middle].key;					      \
									      \
	/* Make sure string table indices are not bogus before using	      \
	   them.  */							      \
	if (! _dl_cache_verify_ptr (key))				      \
	  {								      \
	    cmpres = 1;							      \
	    break;							      \
	  }								      \
									      \
	/* Actually compare the entry with the key.  */			      \
	cmpres = _dl_cache_libcmp (name, cache_data + key);		      \
	if (__builtin_expect (cmpres == 0, 0))				      \
	  {								      \
	    /* Found it.  LEFT now marks the last entry for which we	      \
	       know the name is correct.  */				      \
	    left = middle;						      \
									      \
	    /* There might be entries with this name before the one we	      \
	       found.  So we have to find the beginning.  */		      \
	    while (middle > 0)						      \
	      {								      \
		__typeof__ (cache->libs[0].key) key;			      \
									      \
		key = cache->libs[middle - 1].key;			      \
		/* Make sure string table indices are not bogus before	      \
		   using them.  */					      \
		if (! _dl_cache_verify_ptr (key)			      \
		    /* Actually compare the entry.  */			      \
		    || _dl_cache_libcmp (name, cache_data + key) != 0)	      \
		  break;						      \
		--middle;						      \
	      }								      \
									      \
	    do								      \
	      {								      \
		int flags;						      \
		__typeof__ (cache->libs[0]) *lib = &cache->libs[middle];      \
									      \
		/* Only perform the name test if necessary.  */		      \
		if (middle > left					      \
		    /* We haven't seen this string so far.  Test whether the  \
		       index is ok and whether the name matches.  Otherwise   \
		       we are done.  */					      \
		    && (! _dl_cache_verify_ptr (lib->key)		      \
			|| (_dl_cache_libcmp (name, cache_data + lib->key)    \
			    != 0)))					      \
		  break;						      \
									      \
		flags = lib->flags;					      \
		if (_dl_cache_check_flags (flags)			      \
		    && _dl_cache_verify_ptr (lib->value))		      \
		  {							      \
		    if (best == NULL || flags == GLRO(dl_correct_cache_id))   \
		      {							      \
			HWCAP_CHECK;					      \
			best = cache_data + lib->value;			      \
									      \
			if (flags == GLRO(dl_correct_cache_id))		      \
			  /* We've found an exact match for the shared	      \
			     object and no general `ELF' release.  Stop	      \
			     searching.  */				      \
			  break;					      \
		      }							      \
		  }							      \
	      }								      \
	    while (++middle <= right);					      \
	    break;							      \
	}								      \
									      \
	if (cmpres < 0)							      \
	  left = middle + 1;						      \
	else								      \
	  right = middle - 1;						      \
      }									      \
  }									      \
while (0)


int
internal_function
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


/* Look up NAME in ld.so.cache described by INFO and return the file name
   stored there, or null if none is found.  The cache is loaded if it was not
   already.  If loading the cache previously failed there will be no more
   attempts to load it.  */
static
const char *
internal_function
_dl_load_cache_lookup_2 (const char *name, struct cache_info *info)
{
  int left, right, middle;
  int cmpres;
  const char *cache_data;
  uint32_t cache_data_size;
  const char *best;

  /* Print a message if the loading of libs is traced.  */
  if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_LIBS, 0))
    _dl_debug_printf (" search cache=%s\n", info->ld_so_cache);

  if (info->cache == NULL)
    {
      /* Read the contents of the file.  */
      void *file = _dl_sysdep_read_whole_file (info->ld_so_cache,
					       &info->cachesize,
					       PROT_READ);

      /* We can handle three different cache file formats here:
	 - the old libc5/glibc2.0/2.1 format
	 - the old format with the new format in it
	 - only the new format
	 The following checks if the cache contains any of these formats.  */
      if (file != MAP_FAILED && info->cachesize > sizeof *info->cache
	  && memcmp (file, CACHEMAGIC, sizeof CACHEMAGIC - 1) == 0)
	{
	  size_t offset;
	  /* Looks ok.  */
	  info->cache = file;

	  /* Check for new version.  */
	  offset =
	    ALIGN_CACHE (sizeof (struct cache_file)
			 + info->cache->nlibs * sizeof (struct file_entry));

	  info->cache_new =
	    (struct cache_file_new *) ((void *) info->cache + offset);
	  if (info->cachesize < (offset + sizeof (struct cache_file_new))
	      || memcmp (info->cache_new->magic, CACHEMAGIC_VERSION_NEW,
			 sizeof CACHEMAGIC_VERSION_NEW - 1) != 0)
	    info->cache_new = (void *) -1;
	}
      else if (file != MAP_FAILED && info->cachesize > sizeof *info->cache_new
	       && memcmp (file, CACHEMAGIC_VERSION_NEW,
			  sizeof CACHEMAGIC_VERSION_NEW - 1) == 0)
	{
	  info->cache_new = file;
	  info->cache = file;
	}
      else
	{
	  if (file != MAP_FAILED)
	    __munmap (file, info->cachesize);
	  info->cache = (void *) -1;
	}

      assert (info->cache != NULL);
    }

  if (info->cache == (void *) -1)
    /* Previously looked for the cache file and didn't find it.  */
    return NULL;

  best = NULL;

  if (info->cache_new != (void *) -1)
    {
      uint64_t platform;

      /* This is where the strings start.  */
      cache_data = (const char *) info->cache_new;

      /* Now we can compute how large the string table is.  */
      cache_data_size =
	(const char *) info->cache + info->cachesize - cache_data;

      platform = _dl_string_platform (GLRO(dl_platform));
      if (platform != (uint64_t) -1)
	platform = 1ULL << platform;

#define _DL_HWCAP_TLS_MASK (1LL << 63)
      uint64_t hwcap_exclude = ~((GLRO(dl_hwcap) & GLRO(dl_hwcap_mask))
				 | _DL_HWCAP_PLATFORM | _DL_HWCAP_TLS_MASK);

      /* Only accept hwcap if it's for the right platform.  */
#define HWCAP_CHECK \
      if (lib->hwcap & hwcap_exclude)					      \
	continue;							      \
      if (GLRO(dl_osversion) && lib->osversion > GLRO(dl_osversion))	      \
	continue;							      \
      if (_DL_PLATFORMS_COUNT						      \
	  && (lib->hwcap & _DL_HWCAP_PLATFORM) != 0			      \
	  && (lib->hwcap & _DL_HWCAP_PLATFORM) != platform)		      \
	continue
      SEARCH_CACHE (info->cache_new);
    }
  else
    {
      /* This is where the strings start.  */
      cache_data = (const char *) &info->cache->libs[info->cache->nlibs];

      /* Now we can compute how large the string table is.  */
      cache_data_size =
	(const char *) info->cache + info->cachesize - cache_data;

#undef HWCAP_CHECK
#define HWCAP_CHECK do {} while (0)
      SEARCH_CACHE (info->cache);
    }

  /* Print our result if wanted.  */
  if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_LIBS, 0)
      && best != NULL)
    _dl_debug_printf ("  trying file=%s\n", best);

  return best;
}

/* Parse CACHE_LIST, and build cache_info array.  */
static
struct cache_info *
_dl_alloc_cache_info (const char *const cache_list)
{
  struct cache_info *info;
  const char *begin;
  int num_caches, i;

  begin = cache_list;
  num_caches = 0;

  /* cache_list is a colon-separated list of absolute pathnames of
     ld.so.cache files.  We stop at first non-absolute path (if any).  */
  while (begin[0] == '/')
    {
      ++num_caches;
      begin = strchr (begin, ':');
      if (begin == NULL)
	break;
      ++begin;
    }

  /* Allocate one extra for the sentinel.  */
  info = (struct cache_info *) calloc (num_caches + 1, sizeof (*cache_info));

  begin = cache_list;
  for (i = 0; i < num_caches; ++i)
    {
      const char *const end = strchr(begin, ':');

      if (end == NULL)
	info[i].ld_so_cache = strdup (begin);
      else
	{
	  info[i].ld_so_cache = strndup (begin, end - begin);
	  begin = end + 1;
	}
    }
  return info;
}

/* Look up NAME in each of ld.so.cache caches in turn, and return the
   file name stored there, or null if none is found.  */
const char *
internal_function
_dl_load_cache_lookup (const char *name, int mode)
{
  struct cache_info *info;

  /* This runs at startup, or during dlopen with loader lock held.  */
  if (cache_info == NULL)
    /* Caches have not been initialized yet.  */
    cache_info = _dl_alloc_cache_info (GLRO(google_ld_so_cache_list));

  if (mode & __RTLD_GOOGLE_IGNORE_HOST_LD_SO_CACHE)
    {
      /* We are loading a component of glibc itself (e.g. nss or iconv).
	 Such components must not be loaded from host, only from this
	 glibc compilation (which must always be the first component of
	 the cache list). See b/3133396 */

      return _dl_load_cache_lookup_2 (name, cache_info);
    }

  for (info = cache_info; info->ld_so_cache != NULL; ++info)
    {
      const char *result;

      result = _dl_load_cache_lookup_2 (name, info);
      if (result != NULL)
	return result;
    }

  return NULL;
}

#ifndef MAP_COPY
/* If the system does not support MAP_COPY we cannot leave the file open
   all the time since this would create problems when the file is replaced.
   Therefore we provide this function to close the file described by INFO
   and open it again once needed.  */
static
void
_dl_unload_cache_2 (struct cache_info *info)
{
  if (info->cache != NULL && info->cache != (struct cache_file *) -1)
    {
      __munmap (info->cache, info->cachesize);
      info->cache = NULL;
    }
}

/* Unload all loaded ld.so.cache caches.  */
void
_dl_unload_cache (void)
{
  struct cache_info *info;

  if (cache_info == NULL)
    /* No caches loaded yet.  */
    return;

  for (info = cache_info; info->ld_so_cache != NULL; ++info)
    _dl_unload_cache_2 (info);
}
#endif
