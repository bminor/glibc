/* Update p_align of the first PT_LOAD segment.
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
   <https://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <link.h>
#include <error.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>

const char *file_name;

static size_t update_p_align (size_t);

int
main (int argc, char ** argv)
{
  if (argc != 2)
    {
      printf ("Usage: %s: file\n", argv[0]);
      return 0;
    }

  file_name = argv[1];
  struct stat statbuf;
  int errno_saved;

  if (stat (file_name, &statbuf) < 0)
    error (1, errno, "%s: not exist", file_name);

  ElfW(Ehdr) *ehdr;

  if (statbuf.st_size < sizeof (*ehdr))
    error (1, 0, "%s: too small", file_name);

  int fd = open (file_name, O_RDWR);
  if (fd < 0)
    error (1, errno, "%s: can't open", file_name);

  /* Map in the whole file.  */
  void *base = mmap (NULL, statbuf.st_size, PROT_READ | PROT_WRITE,
		     MAP_SHARED, fd, 0);
  if (base == MAP_FAILED)
    {
      errno_saved = errno;
      close (fd);
      error (1, errno_saved, "%s: failed to map", file_name);
    }

  ehdr = (ElfW(Ehdr) *) base;
  if (ehdr->e_ident[EI_MAG0] != ELFMAG0
      || ehdr->e_ident[EI_MAG1] != ELFMAG1
      || ehdr->e_ident[EI_MAG2] != ELFMAG2
      || ehdr->e_ident[EI_MAG3] != ELFMAG3)
    {
      close (fd);
      error (1, 0, "%s: bad ELF header", file_name);
    }

  if (ehdr->e_type != ET_DYN)
    {
      close (fd);
      error (1, 0, "%s: not shared library", file_name);
    }

  bool unsupported_class = true;
  switch (ehdr->e_ident[EI_CLASS])
    {
    default:
      break;

    case ELFCLASS32:
      unsupported_class = __ELF_NATIVE_CLASS != 32;
      break;

    case ELFCLASS64:
      unsupported_class = __ELF_NATIVE_CLASS != 64;
      break;
    }

  if (unsupported_class)
    {
      close (fd);
      error (1, 0, "%s: unsupported ELF class: %d",
	     file_name, ehdr->e_ident[EI_CLASS]);
    }

  size_t phdr_size = sizeof (ElfW(Phdr)) * ehdr->e_phentsize;
  if (statbuf.st_size < (ehdr->e_phoff + phdr_size))
    {
      close (fd);
      error (1, 0, "%s: too small", file_name);
    }

  ElfW(Phdr) *phdr = (ElfW(Phdr) *) (base + ehdr->e_phoff);
  for (int i = 0; i < ehdr->e_phnum; i++, phdr++)
    if (phdr->p_type == PT_LOAD)
      {
	/* Update p_align of the first PT_LOAD segment.  */
	phdr->p_align = update_p_align (phdr->p_align);
	break;
      }

  munmap (base, statbuf.st_size);
  close (fd);

  return 0;
}
