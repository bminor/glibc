/* Detecting file changes using modification times.
   Copyright (C) 2017-2020 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Items for identifying a particular file version.  Excerpt from
   struct stat64.  */
struct file_change_detection
{
  /* Special values: 0 if file does not exist.  -1 to force mismatch
     with the next comparison.  */
  off64_t size;

  ino64_t ino;
  struct timespec mtime;
  struct timespec ctime;
};

/* Returns true if *LEFT and *RIGHT describe the same version of the
   same file.  */
static bool __attribute__ ((unused))
file_is_unchanged (const struct file_change_detection *left,
                   const struct file_change_detection *right)
{
  if (left->size < 0 || right->size < 0)
    /* Negative sizes are used as markers and never match.  */
    return false;
  else if (left->size == 0 && right->size == 0)
    /* Both files are empty or do not exist, so they have the same
       content, no matter what the other fields indicate.  */
    return true;
  else
    return left->size == right->size
      && left->ino == right->ino
      && left->mtime.tv_sec == right->mtime.tv_sec
      && left->mtime.tv_nsec == right->mtime.tv_nsec
      && left->ctime.tv_sec == right->ctime.tv_sec
      && left->ctime.tv_nsec == right->ctime.tv_nsec;
}

/* Extract file change information to *FILE from the stat buffer
   *ST.  */
static void __attribute__ ((unused))
file_change_detection_for_stat (struct file_change_detection *file,
                                const struct stat64 *st)
{
  if (S_ISDIR (st->st_mode))
    /* Treat as empty file.  */
    file->size = 0;
  else if (!S_ISREG (st->st_mode))
    /* Non-regular files cannot be cached.  */
    file->size = -1;
  else
    {
      file->size = st->st_size;
      file->ino = st->st_ino;
      file->mtime = st->st_mtim;
      file->ctime = st->st_ctim;
    }
}

/* Writes file change information for PATH to *FILE.  Returns true on
   success.  For benign errors, *FILE is cleared, and true is
   returned.  For errors indicating resource outages and the like,
   false is returned.  */
static bool __attribute__ ((unused))
file_change_detection_for_path (struct file_change_detection *file,
                                const char *path)
{
  struct stat64 st;
  if (stat64 (path, &st) != 0)
    switch (errno)
      {
      case EACCES:
      case EISDIR:
      case ELOOP:
      case ENOENT:
      case ENOTDIR:
      case EPERM:
        /* Ignore errors due to file system contents.  Instead, treat
           the file as empty.  */
        file->size = 0;
        return true;
      default:
        /* Other errors are fatal.  */
        return false;
      }
  else /* stat64 was successfull.  */
    {
      file_change_detection_for_stat (file, &st);
      return true;
    }
}

/* Writes file change information for the stream FP to *FILE.  Returns
   ture on success, false on failure.  If FP is NULL, treat the file
   as non-existing.  */
static bool __attribute__ ((unused))
file_change_detection_for_fp (struct file_change_detection *file,
                              FILE *fp)
{
  if (fp == NULL)
    {
      /* The file does not exist.  */
      file->size = 0;
      return true;
    }
  else
    {
      struct stat64 st;
      if (fstat64 (__fileno (fp), &st) != 0)
        /* If we already have a file descriptor, all errors are fatal.  */
        return false;
      else
        {
          file_change_detection_for_stat (file, &st);
          return true;
        }
    }
}
