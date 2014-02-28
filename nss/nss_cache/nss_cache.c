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
static FILE *s_file = NULL;
static char p_filename[NSS_CACHE_PATH_LENGTH] = "/etc/passwd.cache";
static char g_filename[NSS_CACHE_PATH_LENGTH] = "/etc/group.cache";
static char s_filename[NSS_CACHE_PATH_LENGTH] = "/etc/shadow.cache";

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

// _nss_cache_bsearch_lookup()
// Binary search through a sorted nss file for a single record.

enum nss_status _nss_cache_bsearch_lookup(FILE *file,
                                         struct nss_cache_args *args,
                                         int *errnop) {
  enum nss_cache_match (*lookup)(
                                  FILE *,
                                  struct nss_cache_args *
                                 ) = args->lookup_function;
  long min = 0;
  long max;
  long pos;

  // get the size of the file
  if (fseek(file, 0, SEEK_END) != 0) {
    DEBUG("fseek fail\n");
    return NSS_STATUS_UNAVAIL;
  }
  max = ftell(file);

  // binary search until we are within 100 chars of the right line
  while (min + 100 < max) {
    pos = (min + max) / 2;

    if (fseek(file, pos, SEEK_SET) != 0) {
      DEBUG("fseek fail\n");
      return NSS_STATUS_UNAVAIL;
    }

    // scan forward to the start of the next line.
    for (;;) {
      int c = getc_unlocked(file);
      if (c == EOF) break;
      ++pos;
      if (c == '\n') break;
    }

    // break if we've stopped making progress in this loop (long lines)
    if (pos <= min || pos >= max) {
      break;
    }

    // see if this line matches
    switch (lookup(file, args)) {
      case NSS_CACHE_EXACT:
        return NSS_STATUS_SUCCESS;  // done!
      case NSS_CACHE_HIGH:
        max = pos;
        continue;  // search again
      case NSS_CACHE_LOW:
        min = pos;
        continue;  // search again
      case NSS_CACHE_ERROR:
        if (errno == ERANGE) {
          // let the caller retry
          *errnop = errno;
          return _nss_cache_ent_bad_return_code(*errnop);
        }
        DEBUG("expected error %s [errno=%d] from lookup function\n",
              strerror(errno), errno);
        return NSS_STATUS_UNAVAIL;
    }
  }

  // fall back on a linear search in the remaining space
  DEBUG("Switching to linear scan\n");
  pos = min - 100;  // back 100, might be in the middle of the right line
  if (fseek(file, pos, SEEK_SET) != 0) {
    DEBUG("fseek fail\n");
    return NSS_STATUS_UNAVAIL;
  }

  while (pos < max) {
    switch (lookup(file, args)) {
      case NSS_CACHE_EXACT:
        return NSS_STATUS_SUCCESS;
      case NSS_CACHE_HIGH:
      case NSS_CACHE_LOW:
        pos = ftell(file);
        continue;
      case NSS_CACHE_ERROR:
        if (errno == ERANGE) {
          // let the caller retry
          *errnop = errno;
          return _nss_cache_ent_bad_return_code(*errnop);
        }
        break;
    }
    break;
  }

  return NSS_STATUS_NOTFOUND;
}

// _nss_cache_bsearch()
// If a sorted nss file is present, attempt a binary search on it.

enum nss_status _nss_cache_bsearch(struct nss_cache_args *args, int *errnop) {
  FILE *file = NULL;
  struct stat system_file;
  struct stat sorted_file;
  enum nss_status ret;

  file =  fopen(args->sorted_filename, "r");
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

  ret = _nss_cache_bsearch_lookup(file, args, errnop);

  fclose(file);
  return ret;

}

//
// Routines for passwd map defined below here
//

// _nss_cache_setpwent_path()
// Helper function for testing

extern char* _nss_cache_setpwent_path(const char *path) {

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
      *errnop = errno;
      ret = _nss_cache_ent_bad_return_code(*errnop);
    }
  }

  return ret;
}

// _nss_cache_getpwent_r()
// Called by NSS to look up next entry in passwd file

enum nss_status _nss_cache_getpwent_r(struct passwd *result,
                                     char *buffer, size_t buflen,
                                     int *errnop) {
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
  strncat(filename, ".byuid", 6);

  args.sorted_filename = filename;
  args.system_filename = p_filename;
  args.lookup_function = _nss_cache_pwuid_wrap;
  args.lookup_value = &uid;
  args.lookup_result = result;
  args.buffer = buffer;
  args.buflen = buflen;

  DEBUG("Binary search for uid %d\n", uid);
  NSS_CACHE_LOCK();
  ret = _nss_cache_bsearch(&args, errnop);

  // TODO(vasilios): make this a runtime or compile-time option, as this slows
  // down legitimate misses as the trade off for safety.
  if (ret == NSS_STATUS_NOTFOUND) {
    DEBUG("Binary search returned nothing.\n");
    ret = NSS_STATUS_UNAVAIL;
  }

  if (ret == NSS_STATUS_UNAVAIL) {
    DEBUG("Binary search failed, falling back to full linear search\n");
    ret = _nss_cache_setpwent_locked();

    if (ret == NSS_STATUS_SUCCESS) {
      while ((ret = _nss_cache_getpwent_r_locked(result,
                                                buffer,
                                                buflen,
                                                errnop))
             == NSS_STATUS_SUCCESS) {
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
  strncat(filename, ".byname", 7);

  args.sorted_filename = filename;
  args.system_filename = p_filename;
  args.lookup_function = _nss_cache_pwnam_wrap;
  args.lookup_value = pw_name;
  args.lookup_result = result;
  args.buffer = buffer;
  args.buflen = buflen;

  DEBUG("Binary search for user %s\n", pw_name);
  ret = _nss_cache_bsearch(&args, errnop);

  // TODO(vasilios): make this a runtime or compile-time option, as this slows
  // down legitimate misses as the trade off for safety.
  if (ret == NSS_STATUS_NOTFOUND) {
    DEBUG("Binary search returned nothing.\n");
    ret = NSS_STATUS_UNAVAIL;
  }

  if (ret == NSS_STATUS_UNAVAIL) {
    DEBUG("Binary search failed, falling back to full linear search\n");
    ret = _nss_cache_setpwent_locked();

    if (ret == NSS_STATUS_SUCCESS) {
      while ((ret = _nss_cache_getpwent_r_locked(result,
                                                buffer,
                                                buflen,
                                                errnop))
             == NSS_STATUS_SUCCESS) {
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

extern char* _nss_cache_setgrent_path(const char *path) {

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
      fsetpos(g_file, &position);
      *errnop = errno;
      ret = _nss_cache_ent_bad_return_code(*errnop);
    }
  }

  return ret;
}

// _nss_cache_getgrent_r()
// Called by NSS to look up next entry in group file

enum nss_status _nss_cache_getgrent_r(struct group *result,
                                     char *buffer, size_t buflen,
                                     int *errnop) {
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

  strncpy(filename, g_filename, NSS_CACHE_PATH_LENGTH - 1);
  if (strlen(filename) > NSS_CACHE_PATH_LENGTH - 7) {
    DEBUG("filename too long\n");
    return NSS_STATUS_UNAVAIL;
  }
  strncat(filename, ".bygid", 6);

  args.sorted_filename = filename;
  args.system_filename = g_filename;
  args.lookup_function = _nss_cache_grgid_wrap;
  args.lookup_value = &gid;
  args.lookup_result = result;
  args.buffer = buffer;
  args.buflen = buflen;

  DEBUG("Binary search for gid %d\n", gid);
  NSS_CACHE_LOCK();
  ret = _nss_cache_bsearch(&args, errnop);

  // TODO(vasilios): make this a runtime or compile-time option, as this slows
  // down legitimate misses as the trade off for safety.
  if (ret == NSS_STATUS_NOTFOUND) {
    DEBUG("Binary search returned nothing.\n");
    ret = NSS_STATUS_UNAVAIL;
  }

  if (ret == NSS_STATUS_UNAVAIL) {
    DEBUG("Binary search failed, falling back to full linear search\n");
    ret = _nss_cache_setgrent_locked();

    if (ret == NSS_STATUS_SUCCESS) {
      while ((ret = _nss_cache_getgrent_r_locked(result,
                                                buffer,
                                                buflen,
                                                errnop))
             == NSS_STATUS_SUCCESS) {
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
  strncat(filename, ".byname", 7);

  args.sorted_filename = filename;
  args.system_filename = g_filename;
  args.lookup_function = _nss_cache_grnam_wrap;
  args.lookup_value = gr_name;
  args.lookup_result = result;
  args.buffer = buffer;
  args.buflen = buflen;

  DEBUG("Binary search for group %s\n", gr_name);
  ret = _nss_cache_bsearch(&args, errnop);

  // TODO(vasilios): make this a runtime or compile-time option, as this slows
  // down legitimate misses as the trade off for safety.
  if (ret == NSS_STATUS_NOTFOUND) {
    DEBUG("Binary search returned nothing.\n");
    ret = NSS_STATUS_UNAVAIL;
  }

  if (ret == NSS_STATUS_UNAVAIL) {
    DEBUG("Binary search failed, falling back to full linear search\n");
    ret = _nss_cache_setgrent_locked();

    if (ret == NSS_STATUS_SUCCESS) {
      while ((ret = _nss_cache_getgrent_r_locked(result,
                                                buffer,
                                                buflen,
                                                errnop))
             == NSS_STATUS_SUCCESS) {
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

// _nss_cache_setspent_path()
// Helper function for testing

extern char* _nss_cache_setspent_path(const char *path) {

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
      *errnop = errno;
      ret = _nss_cache_ent_bad_return_code(*errnop);
    }
  }

  return ret;
}

// _nss_cache_getspent_r()
// Called by NSS to look up next entry in the shadow file

enum nss_status _nss_cache_getspent_r(struct spwd *result,
                                     char *buffer, size_t buflen,
                                     int *errnop) {
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
  strncat(filename, ".byname", 7);

  args.sorted_filename = filename;
  args.system_filename = s_filename;
  args.lookup_function = _nss_cache_spnam_wrap;
  args.lookup_value = sp_namp;
  args.lookup_result = result;
  args.buffer = buffer;
  args.buflen = buflen;

  DEBUG("Binary search for user %s\n", sp_namp);
  ret = _nss_cache_bsearch(&args, errnop);

  // TODO(vasilios): make this a runtime or compile-time option, as this slows
  // down legitimate misses as the trade off for safety.
  if (ret == NSS_STATUS_NOTFOUND) {
    DEBUG("Binary search returned nothing.\n");
    ret = NSS_STATUS_UNAVAIL;
  }

  if (ret == NSS_STATUS_UNAVAIL) {
    DEBUG("Binary search failed, falling back to full linear search\n");
    ret = _nss_cache_setspent_locked();

    if (ret == NSS_STATUS_SUCCESS) {
      while ((ret = _nss_cache_getspent_r_locked(result,
                                                buffer,
                                                buflen,
                                                errnop))
             == NSS_STATUS_SUCCESS) {
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
