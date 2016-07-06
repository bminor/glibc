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

/* An NSS module which adds supports for file maps with a trailing .cache
 * suffix (/etc/passwd.cache, /etc/group.cache, and /etc/shadow.cache)
 */

#include "nss_cache.h"

#include <sys/mman.h>

// Locking implementation: use pthreads.
#include <pthread.h>
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#pragma weak pthread_mutex_lock
#define NSS_CACHE_LOCK()    do { \
    if (&pthread_mutex_lock != NULL) pthread_mutex_lock(&mutex); \
  } while (0)

#pragma weak pthread_mutex_unlock
#define NSS_CACHE_UNLOCK()  do { \
    if (&pthread_mutex_unlock != NULL) pthread_mutex_unlock(&mutex); \
  } while (0)

static FILE *p_file = NULL;
static FILE *g_file = NULL;
static char p_filename[NSS_CACHE_PATH_LENGTH] = "/etc/passwd.cache";
static char g_filename[NSS_CACHE_PATH_LENGTH] = "/etc/group.cache";
#ifndef BSD
static FILE *s_file = NULL;
static char s_filename[NSS_CACHE_PATH_LENGTH] = "/etc/shadow.cache";
#else
extern int fgetpwent_r(FILE *, struct passwd *, char *, size_t, struct passwd **);
extern int fgetgrent_r(FILE *, struct group *, char *, size_t, struct group **);
#endif // ifndef BSD

/* Common return code routine for all *ent_r_locked functions.
 * We need to return TRYAGAIN if the underlying files guy raises ERANGE,
 * so that our caller knows to try again with a bigger buffer.
 */

static inline enum nss_status _nss_cache_ent_bad_return_code(int errnoval) {
  enum nss_status ret;

  switch (errnoval) {
  case ERANGE:
    DEBUG("ERANGE: Try again with a bigger buffer\n");
    ret = NSS_STATUS_TRYAGAIN;
    break;
  case ENOENT:
  default:
    DEBUG("ENOENT or default case: Not found\n");
    ret = NSS_STATUS_NOTFOUND;
  };
  return ret;
}

//
// Binary search routines below here
//

int _nss_cache_bsearch2_compare(const void *key, const void *value) {
  struct nss_cache_args *args = (struct nss_cache_args *)key;
  const char *value_text = (const char *)value;

  // Using strcmp as the generation of the index sorts without
  // locale awareness.
  return strcmp(args->lookup_key, value_text);
}

enum nss_status _nss_cache_bsearch2(struct nss_cache_args *args, int *errnop) {
  enum nss_cache_match (*lookup)(FILE *, struct nss_cache_args *) =
      args->lookup_function;
  FILE *file = NULL;
  FILE *system_file_stream = NULL;
  struct stat system_file;
  struct stat sorted_file;
  enum nss_status ret = 100;
  long offset = 0;
  void *mapped_data = NULL;

  file = fopen(args->sorted_filename, "r");
  if (file == NULL) {
    DEBUG("error opening %s\n", args->sorted_filename);
    return NSS_STATUS_UNAVAIL;
  }

  // if the sorted file is older than the system file, do not risk stale
  // data and abort
  // TODO(vasilios):  should be a compile or runtime option
  if (stat(args->system_filename, &system_file) != 0) {
    DEBUG("failed to stat %s\n", args->system_filename);
    fclose(file);
    return NSS_STATUS_UNAVAIL;
  }
  if (fstat(fileno(file), &sorted_file) != 0) {
    DEBUG("failed to stat %s\n", args->sorted_filename);
    fclose(file);
    return NSS_STATUS_UNAVAIL;
  }
  if (difftime(system_file.st_mtime, sorted_file.st_mtime) > 0) {
    DEBUG("%s may be stale, aborting lookup\n", args->sorted_filename);
    fclose(file);
    return NSS_STATUS_UNAVAIL;
  }

  mapped_data =
      mmap(NULL, sorted_file.st_size, PROT_READ, MAP_PRIVATE, fileno(file), 0);
  if (mapped_data == MAP_FAILED) {
    DEBUG("mmap failed\n");
    fclose(file);
    return NSS_STATUS_UNAVAIL;
  }

  const char *data = (const char *)mapped_data;
  while (*data != '\n') {
    ++data;
  }
  long entry_size = data - (const char *)mapped_data + 1;
  long entry_count = sorted_file.st_size / entry_size;

  void *entry = bsearch(args, mapped_data, entry_count, entry_size,
                        &_nss_cache_bsearch2_compare);
  if (entry != NULL) {
    const char *entry_text = entry;
    sscanf(entry_text + strlen(entry_text) + 1, "%ld", &offset);
  }

  if (munmap(mapped_data, sorted_file.st_size) == -1) {
    DEBUG("munmap failed\n");
  }
  fclose(file);

  if (entry == NULL) {
    return NSS_STATUS_NOTFOUND;
  }

  system_file_stream = fopen(args->system_filename, "r");
  if (system_file_stream == NULL) {
    DEBUG("error opening %s\n", args->system_filename);
    return NSS_STATUS_UNAVAIL;
  }

  if (fseek(system_file_stream, offset, SEEK_SET) != 0) {
    DEBUG("fseek fail\n");
    return NSS_STATUS_UNAVAIL;
  }

  switch (lookup(system_file_stream, args)) {
  case NSS_CACHE_EXACT:
    ret = NSS_STATUS_SUCCESS;
    break;
  case NSS_CACHE_ERROR:
    if (errno == ERANGE) {
      // let the caller retry
      *errnop = errno;
      ret = _nss_cache_ent_bad_return_code(*errnop);
    }
    break;
  default:
    ret = NSS_STATUS_UNAVAIL;
    break;
  }

  fclose(system_file_stream);
  return ret;
}

//
// Routines for passwd map defined below here
//

// _nss_cache_setpwent_path()
// Helper function for testing

extern char *_nss_cache_setpwent_path(const char *path) {

  DEBUG("%s %s\n", "Setting p_filename to", path);
  return strncpy(p_filename, path, NSS_CACHE_PATH_LENGTH - 1);
}

// _nss_cache_pwuid_wrap()
// Internal wrapper for binary searches, using uid-specific calls.

static enum nss_cache_match _nss_cache_pwuid_wrap(FILE *file,
                                                  struct nss_cache_args *args) {
  struct passwd *result = args->lookup_result;
  uid_t *uid = args->lookup_value;

  if (fgetpwent_r(file, result, args->buffer, args->buflen, &result) == 0) {
    if (result->pw_uid == *uid) {
      DEBUG("SUCCESS: found user %d:%s\n", result->pw_uid, result->pw_name);
      return NSS_CACHE_EXACT;
    }
    DEBUG("Failed match at uid %d\n", result->pw_uid);
    if (result->pw_uid > *uid) {
      return NSS_CACHE_HIGH;
    } else {
      return NSS_CACHE_LOW;
    }
  }

  return NSS_CACHE_ERROR;
}

// _nss_cache_pwnam_wrap()
// Internal wrapper for binary searches, using username-specific calls.

static enum nss_cache_match _nss_cache_pwnam_wrap(FILE *file,
                                                  struct nss_cache_args *args) {
  struct passwd *result = args->lookup_result;
  char *name = args->lookup_value;
  int ret;

  if (fgetpwent_r(file, result, args->buffer, args->buflen, &result) == 0) {
    ret = strcoll(result->pw_name, name);
    if (ret == 0) {
      DEBUG("SUCCESS: found user %s\n", result->pw_name);
      return NSS_CACHE_EXACT;
    }
    DEBUG("Failed match at name %s\n", result->pw_name);
    if (ret > 0) {
      return NSS_CACHE_HIGH;
    } else {
      return NSS_CACHE_LOW;
    }
  }

  return NSS_CACHE_ERROR;
}

// _nss_cache_setpwent_locked()
// Internal setup routine

static enum nss_status _nss_cache_setpwent_locked(void) {

  DEBUG("%s %s\n", "Opening", p_filename);
  p_file = fopen(p_filename, "r");

  if (p_file) {
    return NSS_STATUS_SUCCESS;
  } else {
    return NSS_STATUS_UNAVAIL;
  }
}

// _nss_cache_setpwent()
// Called by NSS to open the passwd file
// 'stayopen' parameter is ignored.

enum nss_status _nss_cache_setpwent(int stayopen) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_setpwent_locked();
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_endpwent_locked()
// Internal close routine

static enum nss_status _nss_cache_endpwent_locked(void) {

  DEBUG("Closing passwd.cache\n");
  if (p_file) {
    fclose(p_file);
    p_file = NULL;
  }
  return NSS_STATUS_SUCCESS;
}

// _nss_cache_endpwent()
// Called by NSS to close the passwd file

enum nss_status _nss_cache_endpwent(void) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_endpwent_locked();
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_getpwent_r_locked()
// Called internally to return the next entry from the passwd file

static enum nss_status _nss_cache_getpwent_r_locked(struct passwd *result,
                                                    char *buffer, size_t buflen,
                                                    int *errnop) {
  enum nss_status ret = NSS_STATUS_SUCCESS;

  if (p_file == NULL) {
    DEBUG("p_file == NULL, going to setpwent\n");
    ret = _nss_cache_setpwent_locked();
  }

  if (ret == NSS_STATUS_SUCCESS) {
    if (fgetpwent_r(p_file, result, buffer, buflen, &result) == 0) {
      DEBUG("Returning user %d:%s\n", result->pw_uid, result->pw_name);
    } else {
      if (errno == ENOENT) {
        errno = 0;
      }
      *errnop = errno;
      ret = _nss_cache_ent_bad_return_code(*errnop);
    }
  }

  return ret;
}

// _nss_cache_getpwent_r()
// Called by NSS to look up next entry in passwd file

enum nss_status _nss_cache_getpwent_r(struct passwd *result, char *buffer,
                                      size_t buflen, int *errnop) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_getpwent_r_locked(result, buffer, buflen, errnop);
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_getpwuid_r()
// Find a user account by uid

enum nss_status _nss_cache_getpwuid_r(uid_t uid, struct passwd *result,
                                      char *buffer, size_t buflen,
                                      int *errnop) {
  char filename[NSS_CACHE_PATH_LENGTH];
  struct nss_cache_args args;
  enum nss_status ret;

  strncpy(filename, p_filename, NSS_CACHE_PATH_LENGTH - 1);
  if (strlen(filename) > NSS_CACHE_PATH_LENGTH - 7) {
    DEBUG("filename too long\n");
    return NSS_STATUS_UNAVAIL;
  }
  strncat(filename, ".ixuid", 6);

  args.sorted_filename = filename;
  args.system_filename = p_filename;
  args.lookup_function = _nss_cache_pwuid_wrap;
  args.lookup_value = &uid;
  args.lookup_result = result;
  args.buffer = buffer;
  args.buflen = buflen;
  char uid_text[11];
  snprintf(uid_text, sizeof(uid_text), "%d", uid);
  args.lookup_key = uid_text;
  args.lookup_key_length = strlen(uid_text);

  DEBUG("Binary search for uid %d\n", uid);
  NSS_CACHE_LOCK();
  ret = _nss_cache_bsearch2(&args, errnop);

  if (ret == NSS_STATUS_UNAVAIL) {
    DEBUG("Binary search failed, falling back to full linear search\n");
    ret = _nss_cache_setpwent_locked();

    if (ret == NSS_STATUS_SUCCESS) {
      while ((ret = _nss_cache_getpwent_r_locked(
                  result, buffer, buflen, errnop)) == NSS_STATUS_SUCCESS) {
        if (result->pw_uid == uid)
          break;
      }
    }
  }

  _nss_cache_endpwent_locked();
  NSS_CACHE_UNLOCK();

  return ret;
}

// _nss_cache_getpwnam_r()
// Find a user account by name

enum nss_status _nss_cache_getpwnam_r(const char *name, struct passwd *result,
                                      char *buffer, size_t buflen,
                                      int *errnop) {
  char *pw_name;
  char filename[NSS_CACHE_PATH_LENGTH];
  struct nss_cache_args args;
  enum nss_status ret;

  NSS_CACHE_LOCK();

  // name is a const char, we need a non-const copy
  pw_name = malloc(strlen(name) + 1);
  if (pw_name == NULL) {
    DEBUG("malloc error\n");
    return NSS_STATUS_UNAVAIL;
  }
  strncpy(pw_name, name, strlen(name) + 1);

  strncpy(filename, p_filename, NSS_CACHE_PATH_LENGTH - 1);
  if (strlen(filename) > NSS_CACHE_PATH_LENGTH - 8) {
    DEBUG("filename too long\n");
    free(pw_name);
    return NSS_STATUS_UNAVAIL;
  }
  strncat(filename, ".ixname", 7);

  args.sorted_filename = filename;
  args.system_filename = p_filename;
  args.lookup_function = _nss_cache_pwnam_wrap;
  args.lookup_value = pw_name;
  args.lookup_result = result;
  args.buffer = buffer;
  args.buflen = buflen;
  args.lookup_key = pw_name;
  args.lookup_key_length = strlen(pw_name);

  DEBUG("Binary search for user %s\n", pw_name);
  ret = _nss_cache_bsearch2(&args, errnop);

  if (ret == NSS_STATUS_UNAVAIL) {
    DEBUG("Binary search failed, falling back to full linear search\n");
    ret = _nss_cache_setpwent_locked();

    if (ret == NSS_STATUS_SUCCESS) {
      while ((ret = _nss_cache_getpwent_r_locked(
                  result, buffer, buflen, errnop)) == NSS_STATUS_SUCCESS) {
        if (!strcmp(result->pw_name, name))
          break;
      }
    }
  }

  free(pw_name);
  _nss_cache_endpwent_locked();
  NSS_CACHE_UNLOCK();

  return ret;
}

//
//  Routines for group map defined here.
//

// _nss_cache_setgrent_path()
// Helper function for testing

extern char *_nss_cache_setgrent_path(const char *path) {

  DEBUG("%s %s\n", "Setting g_filename to", path);
  return strncpy(g_filename, path, NSS_CACHE_PATH_LENGTH - 1);
}

// _nss_cache_setgrent_locked()
// Internal setup routine

static enum nss_status _nss_cache_setgrent_locked(void) {

  DEBUG("%s %s\n", "Opening", g_filename);
  g_file = fopen(g_filename, "r");

  if (g_file) {
    return NSS_STATUS_SUCCESS;
  } else {
    return NSS_STATUS_UNAVAIL;
  }
}

// _nss_cache_grgid_wrap()
// Internal wrapper for binary searches, using gid-specific calls.

static enum nss_cache_match _nss_cache_grgid_wrap(FILE *file,
                                                  struct nss_cache_args *args) {
  struct group *result = args->lookup_result;
  gid_t *gid = args->lookup_value;

  if (fgetgrent_r(file, result, args->buffer, args->buflen, &result) == 0) {
    if (result->gr_gid == *gid) {
      DEBUG("SUCCESS: found group %d:%s\n", result->gr_gid, result->gr_name);
      return NSS_CACHE_EXACT;
    }
    DEBUG("Failed match at gid %d\n", result->gr_gid);
    if (result->gr_gid > *gid) {
      return NSS_CACHE_HIGH;
    } else {
      return NSS_CACHE_LOW;
    }
  }

  return NSS_CACHE_ERROR;
}

// _nss_cache_grnam_wrap()
// Internal wrapper for binary searches, using groupname-specific calls.

static enum nss_cache_match _nss_cache_grnam_wrap(FILE *file,
                                                  struct nss_cache_args *args) {
  struct group *result = args->lookup_result;
  char *name = args->lookup_value;
  int ret;

  if (fgetgrent_r(file, result, args->buffer, args->buflen, &result) == 0) {
    ret = strcoll(result->gr_name, name);
    if (ret == 0) {
      DEBUG("SUCCESS: found group %s\n", result->gr_name);
      return NSS_CACHE_EXACT;
    }
    DEBUG("Failed match at name %s\n", result->gr_name);
    if (ret > 0) {
      return NSS_CACHE_HIGH;
    } else {
      return NSS_CACHE_LOW;
    }
  }

  return NSS_CACHE_ERROR;
}

// _nss_cache_setgrent()
// Called by NSS to open the group file
// 'stayopen' parameter is ignored.

enum nss_status _nss_cache_setgrent(int stayopen) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_setgrent_locked();
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_endgrent_locked()
// Internal close routine

static enum nss_status _nss_cache_endgrent_locked(void) {

  DEBUG("Closing group.cache\n");
  if (g_file) {
    fclose(g_file);
    g_file = NULL;
  }
  return NSS_STATUS_SUCCESS;
}

// _nss_cache_endgrent()
// Called by NSS to close the group file

enum nss_status _nss_cache_endgrent(void) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_endgrent_locked();
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_getgrent_r_locked()
// Called internally to return the next entry from the group file

static enum nss_status _nss_cache_getgrent_r_locked(struct group *result,
                                                    char *buffer, size_t buflen,
                                                    int *errnop) {
  enum nss_status ret = NSS_STATUS_SUCCESS;

  if (g_file == NULL) {
    DEBUG("g_file == NULL, going to setgrent\n");
    ret = _nss_cache_setgrent_locked();
  }

  if (ret == NSS_STATUS_SUCCESS) {
    fpos_t position;

    fgetpos(g_file, &position);
    if (fgetgrent_r(g_file, result, buffer, buflen, &result) == 0) {
      DEBUG("Returning group %s (%d)\n", result->gr_name, result->gr_gid);
    } else {
      /* Rewind back to where we were just before, otherwise the data read
       * into the buffer is probably going to be lost because there's no
       * guarantee that the caller is going to have preserved the line we
       * just read.  Note that glibc's nss/nss_files/files-XXX.c does
       * something similar in CONCAT(_nss_files_get,ENTNAME_r) (around
       * line 242 in glibc 2.4 sources).
       */
      if (errno == ENOENT) {
        errno = 0;
      } else {
        fsetpos(g_file, &position);
      }
      *errnop = errno;
      ret = _nss_cache_ent_bad_return_code(*errnop);
    }
  }

  return ret;
}

// _nss_cache_getgrent_r()
// Called by NSS to look up next entry in group file

enum nss_status _nss_cache_getgrent_r(struct group *result, char *buffer,
                                      size_t buflen, int *errnop) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_getgrent_r_locked(result, buffer, buflen, errnop);
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_getgrgid_r()
// Find a group by gid

enum nss_status _nss_cache_getgrgid_r(gid_t gid, struct group *result,
                                      char *buffer, size_t buflen,
                                      int *errnop) {
  char filename[NSS_CACHE_PATH_LENGTH];
  struct nss_cache_args args;
  enum nss_status ret;

  // Since we binary search over the groups using the user provided
  // buffer, we do not start searching before we have a buffer that
  // is big enough to have a high chance of succeeding.
  if (buflen < (1 << 20)) {
    *errnop = ERANGE;
    return NSS_STATUS_TRYAGAIN;
  }

  strncpy(filename, g_filename, NSS_CACHE_PATH_LENGTH - 1);
  if (strlen(filename) > NSS_CACHE_PATH_LENGTH - 7) {
    DEBUG("filename too long\n");
    return NSS_STATUS_UNAVAIL;
  }
  strncat(filename, ".ixgid", 6);

  args.sorted_filename = filename;
  args.system_filename = g_filename;
  args.lookup_function = _nss_cache_grgid_wrap;
  args.lookup_value = &gid;
  args.lookup_result = result;
  args.buffer = buffer;
  args.buflen = buflen;
  char gid_text[11];
  snprintf(gid_text, sizeof(gid_text), "%d", gid);
  args.lookup_key = gid_text;
  args.lookup_key_length = strlen(gid_text);

  DEBUG("Binary search for gid %d\n", gid);
  NSS_CACHE_LOCK();
  ret = _nss_cache_bsearch2(&args, errnop);

  if (ret == NSS_STATUS_UNAVAIL) {
    DEBUG("Binary search failed, falling back to full linear search\n");
    ret = _nss_cache_setgrent_locked();

    if (ret == NSS_STATUS_SUCCESS) {
      while ((ret = _nss_cache_getgrent_r_locked(
                  result, buffer, buflen, errnop)) == NSS_STATUS_SUCCESS) {
        if (result->gr_gid == gid)
          break;
      }
    }
  }

  _nss_cache_endgrent_locked();
  NSS_CACHE_UNLOCK();

  return ret;
}

// _nss_cache_getgrnam_r()
// Find a group by name

enum nss_status _nss_cache_getgrnam_r(const char *name, struct group *result,
                                      char *buffer, size_t buflen,
                                      int *errnop) {
  char *gr_name;
  char filename[NSS_CACHE_PATH_LENGTH];
  struct nss_cache_args args;
  enum nss_status ret;

  NSS_CACHE_LOCK();

  // name is a const char, we need a non-const copy
  gr_name = malloc(strlen(name) + 1);
  if (gr_name == NULL) {
    DEBUG("malloc error\n");
    return NSS_STATUS_UNAVAIL;
  }
  strncpy(gr_name, name, strlen(name) + 1);

  strncpy(filename, g_filename, NSS_CACHE_PATH_LENGTH - 1);
  if (strlen(filename) > NSS_CACHE_PATH_LENGTH - 8) {
    DEBUG("filename too long\n");
    free(gr_name);
    return NSS_STATUS_UNAVAIL;
  }
  strncat(filename, ".ixname", 7);

  args.sorted_filename = filename;
  args.system_filename = g_filename;
  args.lookup_function = _nss_cache_grnam_wrap;
  args.lookup_value = gr_name;
  args.lookup_result = result;
  args.buffer = buffer;
  args.buflen = buflen;
  args.lookup_key = gr_name;
  args.lookup_key_length = strlen(gr_name);

  DEBUG("Binary search for group %s\n", gr_name);
  ret = _nss_cache_bsearch2(&args, errnop);

  if (ret == NSS_STATUS_UNAVAIL) {
    DEBUG("Binary search failed, falling back to full linear search\n");
    ret = _nss_cache_setgrent_locked();

    if (ret == NSS_STATUS_SUCCESS) {
      while ((ret = _nss_cache_getgrent_r_locked(
                  result, buffer, buflen, errnop)) == NSS_STATUS_SUCCESS) {
        if (!strcmp(result->gr_name, name))
          break;
      }
    }
  }

  free(gr_name);
  _nss_cache_endgrent_locked();
  NSS_CACHE_UNLOCK();

  return ret;
}

//
//  Routines for shadow map defined here.
//
#ifndef BSD

// _nss_cache_setspent_path()
// Helper function for testing

extern char *_nss_cache_setspent_path(const char *path) {

  DEBUG("%s %s\n", "Setting s_filename to", path);
  return strncpy(s_filename, path, NSS_CACHE_PATH_LENGTH - 1);
}

// _nss_cache_setspent_locked()
// Internal setup routine

static enum nss_status _nss_cache_setspent_locked(void) {

  DEBUG("%s %s\n", "Opening", g_filename);
  s_file = fopen(s_filename, "r");

  if (s_file) {
    return NSS_STATUS_SUCCESS;
  } else {
    return NSS_STATUS_UNAVAIL;
  }
}

// _nss_cache_spnam_wrap()
// Internal wrapper for binary searches, using shadow-specific calls.

static enum nss_cache_match _nss_cache_spnam_wrap(FILE *file,
                                                  struct nss_cache_args *args) {
  struct spwd *result = args->lookup_result;
  char *name = args->lookup_value;
  int ret;

  if (fgetspent_r(file, result, args->buffer, args->buflen, &result) == 0) {
    ret = strcoll(result->sp_namp, name);
    if (ret == 0) {
      DEBUG("SUCCESS: found user %s\n", result->sp_namp);
      return NSS_CACHE_EXACT;
    }
    DEBUG("Failed match at name %s\n", result->sp_namp);
    if (ret > 0) {
      return NSS_CACHE_HIGH;
    } else {
      return NSS_CACHE_LOW;
    }
  }

  return NSS_CACHE_ERROR;
}

// _nss_cache_setspent()
// Called by NSS to open the shadow file
// 'stayopen' parameter is ignored.

enum nss_status _nss_cache_setspent(int stayopen) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_setspent_locked();
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_endspent_locked()
// Internal close routine

static enum nss_status _nss_cache_endspent_locked(void) {

  DEBUG("Closing shadow.cache\n");
  if (s_file) {
    fclose(s_file);
    s_file = NULL;
  }
  return NSS_STATUS_SUCCESS;
}

// _nss_cache_endspent()
// Called by NSS to close the shadow file

enum nss_status _nss_cache_endspent(void) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_endspent_locked();
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_getspent_r_locked()
// Called internally to return the next entry from the shadow file

static enum nss_status _nss_cache_getspent_r_locked(struct spwd *result,
                                                    char *buffer, size_t buflen,
                                                    int *errnop) {

  enum nss_status ret = NSS_STATUS_SUCCESS;

  if (s_file == NULL) {
    DEBUG("s_file == NULL, going to setspent\n");
    ret = _nss_cache_setspent_locked();
  }

  if (ret == NSS_STATUS_SUCCESS) {
    if (fgetspent_r(s_file, result, buffer, buflen, &result) == 0) {
      DEBUG("Returning shadow entry %s\n", result->sp_namp);
    } else {
      if (errno == ENOENT) {
        errno = 0;
      }
      *errnop = errno;
      ret = _nss_cache_ent_bad_return_code(*errnop);
    }
  }

  return ret;
}

// _nss_cache_getspent_r()
// Called by NSS to look up next entry in the shadow file

enum nss_status _nss_cache_getspent_r(struct spwd *result, char *buffer,
                                      size_t buflen, int *errnop) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_getspent_r_locked(result, buffer, buflen, errnop);
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_getspnam_r()
// Find a user by name

enum nss_status _nss_cache_getspnam_r(const char *name, struct spwd *result,
                                      char *buffer, size_t buflen,
                                      int *errnop) {
  char *sp_namp;
  char filename[NSS_CACHE_PATH_LENGTH];
  struct nss_cache_args args;
  enum nss_status ret;

  NSS_CACHE_LOCK();

  // name is a const char, we need a non-const copy
  sp_namp = malloc(strlen(name) + 1);
  if (sp_namp == NULL) {
    DEBUG("malloc error\n");
    return NSS_STATUS_UNAVAIL;
  }
  strncpy(sp_namp, name, strlen(name) + 1);

  strncpy(filename, s_filename, NSS_CACHE_PATH_LENGTH - 1);
  if (strlen(filename) > NSS_CACHE_PATH_LENGTH - 8) {
    DEBUG("filename too long\n");
    free(sp_namp);
    return NSS_STATUS_UNAVAIL;
  }
  strncat(filename, ".ixname", 7);

  args.sorted_filename = filename;
  args.system_filename = s_filename;
  args.lookup_function = _nss_cache_spnam_wrap;
  args.lookup_value = sp_namp;
  args.lookup_result = result;
  args.buffer = buffer;
  args.buflen = buflen;
  args.lookup_key = sp_namp;
  args.lookup_key_length = strlen(sp_namp);

  DEBUG("Binary search for user %s\n", sp_namp);
  ret = _nss_cache_bsearch2(&args, errnop);

  if (ret == NSS_STATUS_UNAVAIL) {
    DEBUG("Binary search failed, falling back to full linear search\n");
    ret = _nss_cache_setspent_locked();

    if (ret == NSS_STATUS_SUCCESS) {
      while ((ret = _nss_cache_getspent_r_locked(
                  result, buffer, buflen, errnop)) == NSS_STATUS_SUCCESS) {
        if (!strcmp(result->sp_namp, name))
          break;
      }
    }
  }

  free(sp_namp);
  _nss_cache_endspent_locked();
  NSS_CACHE_UNLOCK();

  return ret;
}
#else
#include "bsdnss.c"
#endif // ifndef BSD
