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

#endif /* _NSS_FILES_H */
