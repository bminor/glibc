// An NSS module that extends local user account lookup to the files
// /etc/passwd.borg and /etc/passwd.borg.base
// passwd.borg.base is a subset of passwd.borg that is used as a fallback.

#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <nss.h>
#include <errno.h>
#include <string.h>

#include <libc-lock.h>
__libc_lock_define_initialized (static, lock)
#define NSSBORG_LOCK  __libc_lock_lock (lock)
#define NSSBORG_UNLOCK  __libc_lock_unlock (lock);

static FILE *f;
static FILE *fb;

#define DEBUG(fmt, ...)

// _nss_borg_setpwent_locked()
// Internal setup routine

static enum nss_status _nss_borg_setpwent_locked(void) {

  DEBUG("Opening passwd.borg\n");
  f = fopen("/etc/passwd.borg", "r");

  DEBUG("Opening passwd.borg.base\n");
  fb = fopen("/etc/passwd.borg.base", "r");

  if (f||fb) {
    return NSS_STATUS_SUCCESS;
  } else {
    return NSS_STATUS_UNAVAIL;
  }
}

// _nss_borg_setpwent()
// Called by NSS to open the passwd file
// Oddly, NSS passes a boolean saying whether to keep the database file open; ignore it

enum nss_status _nss_borg_setpwent(int stayopen) {
  enum nss_status ret;
  NSSBORG_LOCK;
  ret = _nss_borg_setpwent_locked();
  NSSBORG_UNLOCK;
  return ret;
}

// _nss_borg_endpwent_locked()
// Internal close routine

static enum nss_status _nss_borg_endpwent_locked(void) {

  DEBUG("Closing passwd.borg\n");
  if (f) {
    fclose(f);
    f = NULL;
  }
  DEBUG("Closing passwd.borg.base\n");
  if (fb) {
    fclose(fb);
    fb = NULL;
  }
  return NSS_STATUS_SUCCESS;
}

// _nss_borg_endpwent()
// Called by NSS to close the passwd file

enum nss_status _nss_borg_endpwent() {
  enum nss_status ret;
  NSSBORG_LOCK;
  ret = _nss_borg_endpwent_locked();
  NSSBORG_UNLOCK;
  return ret;
}

// _nss_borg_getpwent_r_locked()
// Called internally to return the next entry from the passwd file

static enum nss_status _nss_borg_getpwent_r_locked(struct passwd *result,
                                                   char *buffer, size_t buflen,
                                                   int *errnop) {

  enum nss_status ret;
  // Save a copy of the buffer address, in case first call errors
  // and sets it to 0.
  struct passwd *sparecopy = result;
  if (
      f != NULL && (fgetpwent_r(f, result, buffer, buflen, &result) == 0)) {
    DEBUG("Returning borg user %d:%s\n", result->pw_uid, result->pw_name);
    ret = NSS_STATUS_SUCCESS;
  } else if (
	     // Yes, this is one of those cases where an assign makes sense.
	     fb != NULL && (result = sparecopy) && (fgetpwent_r(fb, result, buffer, buflen, &result) == 0)) {
    DEBUG("Returning base user %d:%s\n", result->pw_uid, result->pw_name);
    ret = NSS_STATUS_SUCCESS;
  } else {
    *errnop = errno;
    switch (*errnop) {
      case ERANGE:
        ret = NSS_STATUS_TRYAGAIN;
        break;
      case ENOENT:
      default:
        ret = NSS_STATUS_NOTFOUND;
    }
  }

  return ret;
}

// _nss_borg_getpwent_r()
// Called by NSS (I think) to look up next entry in passwd file
enum nss_status _nss_borg_getpwent_r(struct passwd *result,
                                     char *buffer, size_t buflen,
                                     int *errnop) {
  enum nss_status ret;
  NSSBORG_LOCK;
  ret = _nss_borg_getpwent_r_locked(result, buffer, buflen, errnop);
  NSSBORG_UNLOCK;
  return ret;
}

// _nss_borg_getpwuid_r()
// Find a user account by uid

enum nss_status _nss_borg_getpwuid_r(uid_t uid, struct passwd *result,
                                     char *buffer, size_t buflen,
                                     int *errnop) {

  enum nss_status ret;

  NSSBORG_LOCK;
  ret = _nss_borg_setpwent_locked();
  DEBUG("Looking for uid %d\n", uid);

  if (ret == NSS_STATUS_SUCCESS) {
    while ((ret = _nss_borg_getpwent_r_locked(result, buffer, buflen, errnop))
           == NSS_STATUS_SUCCESS) {
      if (result->pw_uid == uid)
        break;
    }
  }

  _nss_borg_endpwent_locked();
  NSSBORG_UNLOCK;

  return ret;
}

// _nss_borg_getpwnam_r()
// Find a user account by name

enum nss_status _nss_borg_getpwnam_r(const char *name, struct passwd *result,
                                     char *buffer, size_t buflen,
                                     int *errnop) {

  enum nss_status ret;

  NSSBORG_LOCK;
  ret = _nss_borg_setpwent_locked();
  DEBUG("Looking for user %s\n", name);

  if (ret == NSS_STATUS_SUCCESS) {
    while ((ret = _nss_borg_getpwent_r_locked(result, buffer, buflen, errnop))
           == NSS_STATUS_SUCCESS) {
      if (!strcmp(result->pw_name, name))
        break;
    }
  }

  _nss_borg_endpwent_locked();
  NSSBORG_UNLOCK;

  return ret;
}
