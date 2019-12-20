// An NSS module that extends local user account lookup to the files
// /etc/passwd.borg and /etc/passwd.borg.base
// passwd.borg.base is a subset of passwd.borg that is used as a fallback.

#include <errno.h>
#include <nss.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <syslog.h>

#include <libc-lock.h>
__libc_lock_define_initialized(static, lock)
#define NSSBORG_LOCK __libc_lock_lock(lock)
#define NSSBORG_UNLOCK __libc_lock_unlock(lock);

#define EXEC_NAME_SIZE 4096

static FILE *f;
static FILE *fb;
static bool fb_eof;
static FILE *fbr;
static bool fbr_eof;
static char exec_name[EXEC_NAME_SIZE];

// This library will log into syslog any attempt to fetch a username from
// /etc/passwd.borg file, but only if /etc/passwd.borg.real file is present.
// This module makes the user lookup in following order:
// /etc/passwd.borg.real
// /etc/passwd.borg.base
// /etc/passwd.borg


// non_borg_user_lookup_warn is supposed to prevent log spew from a single
// process.
static int non_borg_user_lookup_warn = 0;

#define LOGNSS(priority, fmt, ...)                   \
  do {                                               \
    openlog("nss_borg", LOG_PID, 0);                 \
    syslog(LOG_USER | priority, fmt, ##__VA_ARGS__); \
    closelog();                                      \
  } while (0)
#define DEBUG(fmt, ...)
#define WARN(fmt, ...) LOGNSS(LOG_WARNING, fmt, ##__VA_ARGS__)

// _nss_borg_setpwent_locked()
// Internal setup routine
static enum nss_status _nss_borg_setpwent_locked(void) {
  int readlink_ret;
  DEBUG("Opening passwd.borg\n");
  f = fopen("/etc/passwd.borg", "r");

  DEBUG("Opening passwd.borg.base\n");
  fb = fopen("/etc/passwd.borg.base", "r");
  fb_eof = false;

  DEBUG("Opening passwd.borg.real\n");
  fbr = fopen("/etc/passwd.borg.real", "r");
  fbr_eof = false;

  DEBUG("Reading /proc/self/exe");
  // readlink does not append null-byte, so we reserve last byte for it.
  readlink_ret = readlink("/proc/self/exe", exec_name,
                          sizeof(exec_name) - 1);
  if (readlink_ret == -1) {
    DEBUG("Failed to readlink /proc/self/exe error: %d", errno);
    strncpy(exec_name, "__unknown__", sizeof(exec_name));
  } else {
    exec_name[readlink_ret] = '\0';
  }

  if (f || fb || fbr) {
    return NSS_STATUS_SUCCESS;
  } else {
    return NSS_STATUS_UNAVAIL;
  }
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
  DEBUG("Closing passwd.borg.real\n");
  if (fbr) {
    fclose(fbr);
    fbr = NULL;
  }
  return NSS_STATUS_SUCCESS;
}

// Called internally. It's a wrapper around fgetpwent_r that prevents from
// multiple read calls once EOF was reached (b/72036184)
static int _nss_borg_fgetpwent_r_eof(FILE *fp, struct passwd *pwbuf,
                                           char *buf, size_t buflen,
                                           struct passwd **pwbufp, bool *eof) {
  if (*eof) {
    *pwbufp = NULL;
    return ENOENT;
  } else {
    int ret = fgetpwent_r(fp, pwbuf, buf, buflen, pwbufp);
    if (ret == ENOENT) {
      *eof = true;
    }
    return ret;
  }
}

// _nss_borg_getpwent_r_locked()
// Called internally to return the next entry from the passwd file
static enum nss_status _nss_borg_getpwent_r_locked(struct passwd *result,
                                                   char *buffer, size_t buflen,
                                                   int *errnop,
                                                   int *non_borg_user) {
  enum nss_status ret;
  // Save a copy of the buffer address, in case first call errors
  // and sets it to 0.
  struct passwd *sparecopy = result;
  if (fbr != NULL && (_nss_borg_fgetpwent_r_eof(fbr, result, buffer, buflen,
                                                &result, &fbr_eof) == 0)) {
    DEBUG("Returning real borg user %d:%s\n", result->pw_uid, result->pw_name);
    ret = NSS_STATUS_SUCCESS;
  } else if (
      // Yes, this is one of those cases where an assign makes sense.
      fb != NULL && (result = sparecopy) &&
      (_nss_borg_fgetpwent_r_eof(fb, result, buffer, buflen, &result,
                                 &fb_eof) == 0)) {
    DEBUG("Returning base borg user %d:%s\n", result->pw_uid, result->pw_name);
    ret = NSS_STATUS_SUCCESS;
  } else if (
    // Yes, this is one of those cases where an assign makes sense.
    // NB: passwd.borg.base is not ordered by UID as of cl/201005022.
      f != NULL && (result = sparecopy) &&
          (fgetpwent_r(f, result, buffer, buflen, &result) == 0)) {
    DEBUG("Returning non borg user %d:%s\n", result->pw_uid, result->pw_name);
    // Log only if passwd.borg.real file is present.
    if (fbr) {
      *non_borg_user = 1;
    }
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

// All methods below are public interface of this library.

// _nss_borg_setpwent()
// Called by NSS to open the passwd file
// Oddly, NSS passes a boolean saying whether to keep the database file open;
// ignore it
enum nss_status _nss_borg_setpwent(int stayopen) {
  enum nss_status ret;
  NSSBORG_LOCK;
  ret = _nss_borg_setpwent_locked();
  NSSBORG_UNLOCK;
  return ret;
}

// _nss_borg_endpwent()
// Called by NSS to close the passwd file
enum nss_status _nss_borg_endpwent(void) {
  enum nss_status ret;
  NSSBORG_LOCK;
  ret = _nss_borg_endpwent_locked();
  NSSBORG_UNLOCK;
  return ret;
}

// _nss_borg_getpwent_r()
// Called by NSS (I think) to look up next entry in passwd file
enum nss_status _nss_borg_getpwent_r(struct passwd *result, char *buffer,
                                     size_t buflen, int *errnop) {
  enum nss_status ret;
  int non_borg_user = 0;
  NSSBORG_LOCK;
  ret = _nss_borg_getpwent_r_locked(result, buffer, buflen, errnop,
                                    &non_borg_user);
  if (non_borg_user && !non_borg_user_lookup_warn) {
    non_borg_user_lookup_warn = 1;
    WARN("Reached non borg section in getpwent call "
         "in process: %s running as: %d\n", exec_name, getuid());
  }
  NSSBORG_UNLOCK;
  return ret;
}

// _nss_borg_getpwuid_r()
// Called by NSS to find a user account by uid
enum nss_status _nss_borg_getpwuid_r(uid_t uid, struct passwd *result,
                                     char *buffer, size_t buflen, int *errnop) {
  enum nss_status ret;
  int non_borg_user = 0;

  NSSBORG_LOCK;
  ret = _nss_borg_setpwent_locked();
  DEBUG("Looking for uid %d\n", uid);

  if (ret == NSS_STATUS_SUCCESS) {
    while ((ret = _nss_borg_getpwent_r_locked(result, buffer, buflen, errnop,
                                              &non_borg_user)) ==
           NSS_STATUS_SUCCESS) {
      if (result->pw_uid == uid) {
        if (non_borg_user) {
          WARN("Returning non borg user %d:%s in process: %s running as: %d\n",
               result->pw_uid, result->pw_name, exec_name, getuid());
        }
        break;
      }
    }
  }

  _nss_borg_endpwent_locked();
  NSSBORG_UNLOCK;

  return ret;
}

// _nss_borg_getpwnam_r()
// Called by NSS to find a user account by name
enum nss_status _nss_borg_getpwnam_r(const char *name, struct passwd *result,
                                     char *buffer, size_t buflen, int *errnop) {
  enum nss_status ret;
  int non_borg_user = 0;

  NSSBORG_LOCK;
  ret = _nss_borg_setpwent_locked();
  DEBUG("Looking for user %s\n", name);

  if (ret == NSS_STATUS_SUCCESS) {
    while ((ret = _nss_borg_getpwent_r_locked(result, buffer, buflen, errnop,
                                              &non_borg_user)) ==
           NSS_STATUS_SUCCESS) {
      if (!strcmp(result->pw_name, name)) {
        if (non_borg_user) {
          WARN("Returning non borg user %d:%s in process: %s running as: %d\n",
               result->pw_uid, result->pw_name, exec_name, getuid());
        }
        break;
      }
    }
  }

  _nss_borg_endpwent_locked();
  NSSBORG_UNLOCK;

  return ret;
}
