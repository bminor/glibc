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
struct etherent;
struct group;
struct netent;
struct passwd;
struct protoent;
struct rpcent;
struct servent;
struct sgrp;
struct spwd;

/* Instances of the parse_line function from
   nss/nss_files/files-parse.c.  */
extern int _nss_files_parse_etherent (char *line, struct etherent *result,
                                      struct parser_data *data,
                                      size_t datalen, int *errnop);
extern int _nss_files_parse_grent (char *line, struct group *result,
                                   struct parser_data *data,
                                   size_t datalen, int *errnop);
extern int _nss_files_parse_netent (char *line, struct netent *result,
                                    struct parser_data *data,
                                    size_t datalen, int *errnop);
extern int _nss_files_parse_protoent (char *line, struct protoent *result,
                                      struct parser_data *data,
                                      size_t datalen, int *errnop);
extern int _nss_files_parse_pwent (char *line, struct passwd *result,
                                   struct parser_data *data,
                                   size_t datalen, int *errnop);
extern int _nss_files_parse_rpcent (char *line, struct rpcent *result,
                                    struct parser_data *data,
                                    size_t datalen, int *errnop);
extern int _nss_files_parse_servent (char *line, struct servent *result,
                                     struct parser_data *data,
                                     size_t datalen, int *errnop);
extern int _nss_files_parse_sgent (char *line, struct sgrp *result,
                                   struct parser_data *data,
                                   size_t datalen, int *errnop);
extern int _nss_files_parse_spent (char *line, struct spwd *result,
                                   struct parser_data *data,
                                   size_t datalen, int *errnop);

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
