/*
 * This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
 * MD5 Message-Digest Algorithm (RFC 1321).
 *
 * Homepage:
 * http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
 *
 * Author:
 * Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
 *
 * This software was written by Alexander Peslyak in 2001.  No copyright is
 * claimed, and the software is hereby placed in the public domain.
 * In case this attempt to disclaim copyright and place the software in the
 * public domain is deemed null and void, then the software is
 * Copyright (c) 2001 Alexander Peslyak and it is hereby released to the
 * general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 *
 * See md5.c for more information.
 */

#ifndef _LOCALE_PROGS_MD5_H
#define _LOCALE_PROGS_MD5_H 1

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint64_t buflen;
	uint32_t a, b, c, d;
	uint32_t block[16];
	uint8_t buffer[64];
} MD5_CTX;

extern void MD5_Init(MD5_CTX *ctx);
extern void MD5_Update(MD5_CTX *ctx, const void *data, size_t size);
extern void MD5_Final(uint8_t result[16], MD5_CTX *ctx);

extern void MD5_Buffer(const void *data, size_t size, uint8_t result[16]);

#endif /* alg-md5.h */
