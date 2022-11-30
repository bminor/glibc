/* Support for allocations with narrow capability bounds.  Morello version.
   Copyright (C) 2022 Free Software Foundation, Inc.
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

#ifndef _AARCH64_MORELLO_LIBC_CAP_H
#define _AARCH64_MORELLO_LIBC_CAP_H 1

#include <stdint.h>
#include <sys/mman.h>
#include <libc-lock.h>

/* Hash table for __libc_cap_widen.  */

#define HT_MIN_LEN (65536 / sizeof (struct htentry))
#define HT_MAX_LEN (1UL << 58)

struct htentry
{
  uint64_t key;
  uint64_t unused;
  void *value;
};

struct ht
{
  __libc_lock_define(,mutex);
  size_t mask;    /* Length - 1, note: length is powerof2.  */
  size_t fill;    /* Used + deleted entries.  */
  size_t used;
  size_t reserve; /* Planned adds.  */
  struct htentry *tab;
};

static inline bool
htentry_isempty (struct htentry *e)
{
  return e->key == 0;
}

static inline bool
htentry_isdeleted (struct htentry *e)
{
  return e->key == -1;
}

static inline bool
htentry_isused (struct htentry *e)
{
  return e->key != 0 && e->key != -1;
}

static inline uint64_t
ht_key_hash (uint64_t key)
{
  return (key >> 4) ^ (key >> 18);
}

static struct htentry *
ht_tab_alloc (size_t n)
{
  size_t size = n * sizeof (struct htentry);
  assert (size && (size & 65535) == 0);
  void *p = __mmap (0, size, PROT_READ|PROT_WRITE,
		    MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
  if (p == MAP_FAILED)
    return NULL;
  return p;
}

static void
ht_tab_free (struct htentry *tab, size_t n)
{
  int r = __munmap (tab, n * sizeof (struct htentry));
  assert (r == 0);
}

static bool
ht_init (struct ht *ht)
{
  __libc_lock_init (ht->mutex);
  ht->mask = HT_MIN_LEN - 1;
  ht->fill = 0;
  ht->used = 0;
  ht->reserve = 0;
  ht->tab = ht_tab_alloc (ht->mask + 1);
  return ht->tab != NULL;
}

static struct htentry *
ht_lookup (struct ht *ht, uint64_t key, uint64_t hash)
{
  size_t mask = ht->mask;
  size_t i = hash;
  size_t j;
  struct htentry *e = ht->tab + (i & mask);
  struct htentry *del;

  if (e->key == key || htentry_isempty (e))
    return e;
  if (htentry_isdeleted (e))
    del = e;
  else
    del = NULL;

  /* Quadratic probing.  */
  for (j =1, i += j++; ; i += j++)
    {
      e = ht->tab + (i & mask);
      if (e->key == key)
	return e;
      if (htentry_isempty (e))
	return del != NULL ? del : e;
      if (del == NULL && htentry_isdeleted (e))
	del = e;
    }
}

static bool
ht_resize (struct ht *ht)
{
  size_t len;
  size_t used = ht->used;
  size_t n = ht->used + ht->reserve;
  size_t oldlen = ht->mask + 1;

  if (2 * n >= HT_MAX_LEN)
    len = HT_MAX_LEN;
  else
    for (len = HT_MIN_LEN; len < 2 * n; len *= 2);
  struct htentry *newtab = ht_tab_alloc (len);
  struct htentry *oldtab = ht->tab;
  struct htentry *e;
  if (newtab == NULL)
    return false;

  ht->tab = newtab;
  ht->mask = len - 1;
  ht->fill = ht->used;
  for (e = oldtab; used > 0; e++)
    {
      if (htentry_isused (e))
	{
	  uint64_t hash = ht_key_hash (e->key);
	  used--;
	  *ht_lookup (ht, e->key, hash) = *e;
	}
    }
  ht_tab_free (oldtab, oldlen);
  return true;
}

static bool
ht_reserve (struct ht *ht)
{
  bool r = true;
  __libc_lock_lock (ht->mutex);
  ht->reserve++;
  size_t future_fill = ht->fill + ht->reserve;
  size_t future_used = ht->used + ht->reserve;
  /* Resize at 3/4 fill or if there are many deleted entries.  */
  if (future_fill > ht->mask - ht->mask / 4
      || future_fill > 2 * future_used + ht->mask / 4)
    r = ht_resize (ht);
  if (!r)
    ht->reserve--;
  __libc_lock_unlock (ht->mutex);
  return r;
}

static void
ht_unreserve (struct ht *ht)
{
  __libc_lock_lock (ht->mutex);
  assert (ht->reserve > 0);
  ht->reserve--;
  __libc_lock_unlock (ht->mutex);
}

static bool
ht_add (struct ht *ht, uint64_t key, void *value)
{
  __libc_lock_lock (ht->mutex);
  assert (ht->reserve > 0);
  ht->reserve--;
  uint64_t hash = ht_key_hash (key);
  struct htentry *e = ht_lookup (ht, key, hash);
  bool r = false;
  if (!htentry_isused (e))
    {
      if (htentry_isempty (e))
        ht->fill++;
      ht->used++;
      e->key = key;
      r = true;
    }
  e->value = value;
  __libc_lock_unlock (ht->mutex);
  return r;
}

static bool
ht_del (struct ht *ht, uint64_t key)
{
  __libc_lock_lock (ht->mutex);
  struct htentry *e = ht_lookup (ht, key, ht_key_hash (key));
  bool r = htentry_isused (e);
  if (r)
    {
      ht->used--;
      e->key = -1;
    }
  __libc_lock_unlock (ht->mutex);
  return r;
}

static void *
ht_get (struct ht *ht, uint64_t key)
{
  __libc_lock_lock (ht->mutex);
  struct htentry *e = ht_lookup (ht, key, ht_key_hash (key));
  void *v = htentry_isused (e) ? e->value : NULL;
  __libc_lock_unlock (ht->mutex);
  return v;
}

/* Capability narrowing APIs.  */

static struct ht __libc_cap_ht;

static __always_inline bool
__libc_cap_init (void)
{
  return ht_init (&__libc_cap_ht);
}

static __always_inline void
__libc_cap_fork_lock (void)
{
  __libc_lock_lock (__libc_cap_ht.mutex);
}

static __always_inline void
__libc_cap_fork_unlock_parent (void)
{
  __libc_lock_unlock (__libc_cap_ht.mutex);
}

static __always_inline void
__libc_cap_fork_unlock_child (void)
{
  __libc_lock_init (__libc_cap_ht.mutex);
}

static __always_inline bool
__libc_cap_map_add (void *p)
{
  assert (p != NULL);
// TODO: depends on pcuabi
//  assert (__builtin_cheri_base_get (p) == (uint64_t) p);
  return true;
}

static __always_inline void
__libc_cap_map_del (void *p)
{
  assert (p != NULL);
//  assert (__builtin_cheri_base_get (p) == (uint64_t) p);
}

/* No special alignment is needed for n <= __CAP_ALIGN_THRESHOLD
   allocations, i.e. __libc_cap_align (n) <= MALLOC_ALIGNMENT.  */
#define __CAP_ALIGN_THRESHOLD 32759

/* Round up the allocation size so the allocated pointer bounds
   can be represented.  Note: this may be called before any
   checks on n, so it should work with all possible n values.  */
static __always_inline size_t
__libc_cap_roundup (size_t n)
{
  if (__glibc_unlikely (n > PTRDIFF_MAX))
    return n;
  return __builtin_cheri_round_representable_length (n);
}

/* Returns the alignment requirement for an allocation size n such
   that the allocated pointer bounds can be represented.  Note:
   same n should be used in __libc_cap_roundup and __libc_cap_align
   and the results used together for __libc_cap_narrow.  */
static __always_inline size_t
__libc_cap_align (size_t n)
{
  return -__builtin_cheri_representable_alignment_mask (n);
}

/* Narrow the bounds of p to be [p, p+n).  Note: the original bounds
   of p includes the entire mapping where p points to and that bound
   should be recoverable by __libc_cap_widen.  Called with p aligned
   and n sized according to __libc_cap_align and __libc_cap_roundup.  */
static __always_inline void *
__libc_cap_narrow (void *p, size_t n)
{
  assert (p != NULL);
  uint64_t key = (uint64_t)(uintptr_t) p;
  assert (ht_add (&__libc_cap_ht, key, p));
  void *narrow = __builtin_cheri_bounds_set_exact (p, n);
  return narrow;
}

/* Given a p with narrowed bound (output of __libc_cap_narrow) return
   the same pointer with the internal wide bound.  */
static __always_inline void *
__libc_cap_widen (void *p)
{
  assert (__builtin_cheri_tag_get (p) && __builtin_cheri_offset_get (p) == 0);
  uint64_t key = (uint64_t)(uintptr_t) p;
  void *cap = ht_get (&__libc_cap_ht, key);
  assert (cap == p);
  return cap;
}

static __always_inline bool
__libc_cap_reserve (void)
{
  return ht_reserve (&__libc_cap_ht);
}

static __always_inline void
__libc_cap_unreserve (void)
{
  ht_unreserve (&__libc_cap_ht);
}

static __always_inline void
__libc_cap_drop (void *p)
{
  assert (p != NULL);
  uint64_t key = (uint64_t)(uintptr_t) p;
  assert (ht_del (&__libc_cap_ht, key));
}

#endif
