/* Internal routines for nss_files.
   Copyright (C) 2020 Free Software Foundation, Inc.
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

#ifndef _NSS_FILES_H
#define _NSS_FILES_H

#include <stdio.h>

/* Open PATH for reading, as a data source for nss_files.  */
FILE *__nss_files_fopen (const char *path);
libc_hidden_proto (__nss_files_fopen)

/* Read a line from FP, storing it BUF.  Strip leading blanks and skip
   comments.  Sets errno and returns error code on failure.  Special
   failure: ERANGE means the buffer is too small.  The function writes
   the original offset to *POFFSET (which can be negative in the case
   of non-seekable input).  */
int __nss_readline (FILE *fp, char *buf, size_t len, off64_t *poffset);
libc_hidden_proto (__nss_readline)

/* Seek FP to OFFSET.  Sets errno and returns error code on failure.
   On success, sets errno to ERANGE and returns ERANGE (to indicate
   re-reading of the same input line to the caller).  If OFFSET is
   negative, fail with ESPIPE without seeking.  Intended to be used
   after parsing data read by __nss_readline failed with ERANGE.  */
int __nss_readline_seek (FILE *fp, off64_t offset) attribute_hidden;

/* Handles the result of a parse_line call (as defined by
   nss/nss_files/files-parse.c).  Adjusts the file offset of FP as
   necessary.  Returns 0 on success, and updates errno on failure (and
   returns that error code).  */
int __nss_parse_line_result (FILE *fp, off64_t offset, int parse_line_result);
libc_hidden_proto (__nss_parse_line_result)

struct parser_data;

/* Instances of the parse_line function from
   nss/nss_files/files-parse.c.  */
typedef int nss_files_parse_line (char *line, void *result,
                                  struct parser_data *data,
                                  size_t datalen, int *errnop);
extern nss_files_parse_line _nss_files_parse_etherent;
extern nss_files_parse_line _nss_files_parse_grent;
extern nss_files_parse_line _nss_files_parse_netent;
extern nss_files_parse_line _nss_files_parse_protoent;
extern nss_files_parse_line _nss_files_parse_pwent;
extern nss_files_parse_line _nss_files_parse_rpcent;
extern nss_files_parse_line _nss_files_parse_servent;
extern nss_files_parse_line _nss_files_parse_sgent;
extern nss_files_parse_line _nss_files_parse_spent;

libnss_files_hidden_proto (_nss_files_parse_etherent)
libc_hidden_proto (_nss_files_parse_grent)
libnss_files_hidden_proto (_nss_files_parse_netent)
libnss_files_hidden_proto (_nss_files_parse_protoent)
libc_hidden_proto (_nss_files_parse_pwent)
libnss_files_hidden_proto (_nss_files_parse_rpcent)
libnss_files_hidden_proto (_nss_files_parse_servent)
libc_hidden_proto (_nss_files_parse_sgent)
libc_hidden_proto (_nss_files_parse_spent)

/* Generic implementation of fget*ent_r.  Reads lines from FP until
   EOF or a successful parse into *RESULT using PARSER.  Returns 0 on
   success, ENOENT on EOF, ERANGE on too-small buffer.  */
int __nss_fgetent_r (FILE *fp, void *result,
                     char *buffer, size_t buffer_length,
                     nss_files_parse_line parser) attribute_hidden;

#endif /* _NSS_FILES_H */
