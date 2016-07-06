/* Copyright 2009 Google Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA
 */

#include <errno.h>
#include <grp.h>
#include <nss.h>
#include <stdlib.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <time.h>
#include <unistd.h>

#ifndef BSD
#include <shadow.h>
#endif // ifndef BSD

#ifndef NSS_CACHE_H
#define NSS_CACHE_H

#ifdef DEBUG
#undef DEBUG
#define DEBUG(fmt, args...)                                                    \
  do {                                                                         \
    fprintf(stderr, fmt, ##args);                                              \
  } while (0)
#else
#define DEBUG(fmt, ...)                                                        \
  do {                                                                         \
  } while (0)
#endif /* DEBUG */

#define NSS_CACHE_PATH_LENGTH 255
extern char *_nss_cache_setpwent_path(const char *path);
extern char *_nss_cache_setgrent_path(const char *path);
#ifndef BSD
extern char *_nss_cache_setspent_path(const char *path);
#endif // ifndef BSD

enum nss_cache_match {
  NSS_CACHE_EXACT = 0,
  NSS_CACHE_HIGH = 1,
  NSS_CACHE_LOW = 2,
  NSS_CACHE_ERROR = 3,
};

struct nss_cache_args {
  char *system_filename;
  char *sorted_filename;
  void *lookup_function;
  void *lookup_value;
  void *lookup_result;
  char *buffer;
  size_t buflen;
  char *lookup_key;
  size_t lookup_key_length;
};

#endif /* NSS_CACHE_H */
