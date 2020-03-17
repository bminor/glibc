/* Support for GNU properties.  AArch64 version.
   Copyright (C) 2018-2020 Free Software Foundation, Inc.
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

#ifndef _DL_PROP_H
#define _DL_PROP_H

#include <not-cancel.h>

extern void _dl_bti_check (struct link_map *, const char *)
    attribute_hidden;

static inline void __attribute__ ((always_inline))
_rtld_main_check (struct link_map *m, const char *program)
{
  _dl_bti_check (m, program);
}

static inline void __attribute__ ((always_inline))
_dl_open_check (struct link_map *m)
{
  _dl_bti_check (m, NULL);
}

static inline void __attribute__ ((unused))
_dl_process_aarch64_property (struct link_map *l,
			      const ElfW(Nhdr) *note,
			      const ElfW(Addr) size,
			      const ElfW(Addr) align)
{
  /* The NT_GNU_PROPERTY_TYPE_0 note must be aligned to 4 bytes in
     32-bit objects and to 8 bytes in 64-bit objects.  Skip notes
     with incorrect alignment.  */
  if (align != (__ELF_NATIVE_CLASS / 8))
    return;

  const ElfW(Addr) start = (ElfW(Addr)) note;

  unsigned int feature_1 = 0;
  unsigned int last_type = 0;

  while ((ElfW(Addr)) (note + 1) - start < size)
    {
      /* Find the NT_GNU_PROPERTY_TYPE_0 note.  */
      if (note->n_namesz == 4
	  && note->n_type == NT_GNU_PROPERTY_TYPE_0
	  && memcmp (note + 1, "GNU", 4) == 0)
	{
	  /* Check for invalid property.  */
	  if (note->n_descsz < 8
	      || (note->n_descsz % sizeof (ElfW(Addr))) != 0)
	    return;

	  /* Start and end of property array.  */
	  unsigned char *ptr = (unsigned char *) (note + 1) + 4;
	  unsigned char *ptr_end = ptr + note->n_descsz;

	  do
	    {
	      unsigned int type = *(unsigned int *) ptr;
	      unsigned int datasz = *(unsigned int *) (ptr + 4);

	      /* Property type must be in ascending order.  */
	      if (type < last_type)
		return;

	      ptr += 8;
	      if ((ptr + datasz) > ptr_end)
		return;

	      last_type = type;

	      if (type == GNU_PROPERTY_AARCH64_FEATURE_1_AND)
		{
		  /* The size of GNU_PROPERTY_AARCH64_FEATURE_1_AND is 4
		     bytes.  When seeing GNU_PROPERTY_AARCH64_FEATURE_1_AND,
		     we stop the search regardless if its size is correct
		     or not.  There is no point to continue if this note
		     is ill-formed.  */
		  if (datasz != 4)
		    return;

		  feature_1 = *(unsigned int *) ptr;
		  if ((feature_1 & GNU_PROPERTY_AARCH64_FEATURE_1_BTI))
		    l->l_mach.bti = true;

		  /* Stop if we found the property note.  */
		  return;
		}
	      else if (type > GNU_PROPERTY_AARCH64_FEATURE_1_AND)
		{
		  /* Stop since property type is in ascending order.  */
		  return;
		}

	      /* Check the next property item.  */
	      ptr += ALIGN_UP (datasz, sizeof (ElfW(Addr)));
	    }
	  while ((ptr_end - ptr) >= 8);
	}

      note = ((const void *) note
	      + ELF_NOTE_NEXT_OFFSET (note->n_namesz, note->n_descsz,
				      align));
    }
}

#ifdef FILEBUF_SIZE
static inline int __attribute__ ((always_inline))
_dl_process_pt_note (struct link_map *l, const ElfW(Phdr) *ph,
		     int fd, struct filebuf *fbp)
{
  return 0;
}
#endif

static inline int __attribute__ ((always_inline))
_rtld_process_pt_note (struct link_map *l, const ElfW(Phdr) *ph)
{
  return 0;
}

static inline int
_dl_process_pt_gnu_property (struct link_map *l, const ElfW(Phdr) *ph)
{
  const ElfW(Nhdr) *note = (const void *) (ph->p_vaddr + l->l_addr);
  _dl_process_aarch64_property (l, note, ph->p_memsz, ph->p_align);
  return 0;
}

#endif /* _DL_PROP_H */
