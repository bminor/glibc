/* Check that _dl_find_object behavior matches up with gaps.
   Copyright (C) 2025-2026 Free Software Foundation, Inc.
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
#include <gnu/lib-names.h>
#include <link.h>
#include <stdbool.h>
#include <stdio.h>
#include <support/check.h>
#include <support/xdlfcn.h>
#include <support/xunistd.h>
#include <sys/mman.h>
#include <unistd.h>

static int
do_test (void)
{
  struct link_map *l = xdlopen (LD_SO, RTLD_NOW);
  if (!l->l_contiguous)
    {
      puts ("info: ld.so link map is not contiguous");

      /* Try to find holes by probing with mmap.  */
      int pagesize = getpagesize ();
      bool gap_found = false;
      ElfW(Addr) addr = l->l_map_start;
      TEST_COMPARE (addr % pagesize, 0);
      while (addr < l->l_map_end)
        {
          void *expected = (void *) addr;
          void *ptr = xmmap (expected, 1, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS, -1);
          struct dl_find_object dlfo;
          int dlfo_ret = _dl_find_object (expected, &dlfo);
          if (ptr == expected)
            {
              if (dlfo_ret < 0)
                {
                  TEST_COMPARE (dlfo_ret, -1);
                  printf ("info: hole without mapping data found at %p\n", ptr);
                }
              else
                FAIL ("object \"%s\" found in gap at %p",
                      dlfo.dlfo_link_map->l_name, ptr);
              gap_found = true;
            }
          else if (dlfo_ret == 0)
            {
              if ((void *) dlfo.dlfo_link_map != (void *) l)
                {
                  printf ("info: object \"%s\" found at %p\n",
                          dlfo.dlfo_link_map->l_name, ptr);
                  gap_found = true;
                }
            }
          else
            TEST_COMPARE (dlfo_ret, -1);
          xmunmap (ptr, 1);
          addr += pagesize;
        }
      if (!gap_found)
        FAIL ("no ld.so gap found");
    }
  else
    {
      puts ("info: ld.so link map is contiguous");

      /* Assert that ld.so is truly contiguous in memory.  */
      volatile long int *p = (volatile long int *) l->l_map_start;
      volatile long int *end = (volatile long int *) l->l_map_end;
      while (p < end)
        {
          *p;
          ++p;
        }
    }

  xdlclose (l);

  return 0;
}

#include <support/test-driver.c>
