/* bits/types.h -- definitions of __*_t types underlying *_t types.
   Copyright (C) 2002-2020 Free Software Foundation, Inc.
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

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

#ifndef	_BITS_TYPES_H
#define	_BITS_TYPES_H	1

#include <features.h>
#include <bits/wordsize.h>
#include <bits/timesize.h>

/* Fixed-size types, underlying types depend on word size and compiler.  */
typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;
#if __WORDSIZE == 64
typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;
#elif __WORDSIZE == 32
__extension__ typedef signed long long int __int64_t;
__extension__ typedef unsigned long long int __uint64_t;
#else
# error "Unsupported value for __WORDSIZE"
#endif

/* Smallest types with at least a given width.  */
typedef __int8_t __int_least8_t;
typedef __uint8_t __uint_least8_t;
typedef __int16_t __int_least16_t;
typedef __uint16_t __uint_least16_t;
typedef __int32_t __int_least32_t;
typedef __uint32_t __uint_least32_t;
typedef __int64_t __int_least64_t;
typedef __uint64_t __uint_least64_t;

/* Largest integral types.  */
typedef __int64_t __intmax_t;
typedef __uint64_t __uintmax_t;

/* The machine-dependent file <bits/typesizes.h> defines __*_T_TYPE
   macros for each of the OS types we define below.  The definitions
   of those macros must use the following macros for underlying types.
   We define __S<SIZE>_TYPE and __U<SIZE>_TYPE for the signed and unsigned
   variants of each of the following integer types on this machine.

	16		-- "natural" 16-bit type (always short)
	32		-- "natural" 32-bit type (always int)
	64		-- "natural" 64-bit type (long or long long)
	LONG32		-- 32-bit type, traditionally long
	WORD		-- natural type of __WORDSIZE bits (int or long)
	LONGWORD	-- type of __WORDSIZE bits, traditionally long

   We distinguish WORD/LONGWORD and 64/QUAD so that the conventional
   uses of `long' or `long long' type modifiers match the types we
   define, even when a less-adorned type would be the same size.
   This matters for (somewhat) portably writing printf/scanf formats
   for these types, where using the appropriate l or ll format
   modifiers can make the typedefs and the formats match up across all
   GNU platforms.  If we used `long' when it's 64 bits where `long
   long' is expected, then the compiler would warn about the formats
   not matching the argument types, and the programmer changing them
   to shut up the compiler would break the program's portability.

   Here we assume what is presently the case in all the GCC configurations
   we support: long long is always 64 bits, long is always word/address size,
   and int is always 32 bits.  */

#define	__S16_TYPE		__int16_t
#define __U16_TYPE		__uint16_t
#define	__S32_TYPE		__int32_t
#define __U32_TYPE		__uint32_t
#define __S64_TYPE		__int64_t
#define __U64_TYPE		__uint64_t

#define __SLONGWORD_TYPE	long int
#define __ULONGWORD_TYPE	unsigned long int
#if __WORDSIZE == 32
# define __SWORD_TYPE		int
# define __UWORD_TYPE		unsigned int
# define __SLONG32_TYPE		long int
# define __ULONG32_TYPE		unsigned long int
#else /* __WORDSIZE == 64 */
# define __SWORD_TYPE		long int
# define __UWORD_TYPE		unsigned long int
# define __SLONG32_TYPE		int
# define __ULONG32_TYPE		unsigned int
#endif
#include <bits/typesizes.h>	/* Defines __*_T_TYPE macros.  */
#include <bits/time64.h>	/* Defines __TIME*_T_TYPE macros.  */

/* Types from core POSIX:  */
typedef __BLKCNT_T_TYPE __blkcnt_t;	/* Disk block counts.  */
typedef __CLOCKID_T_TYPE __clockid_t;	/* Clock IDs.  */
typedef __DEV_T_TYPE __dev_t;		/* Device numbers.  */
typedef __FSBLKCNT_T_TYPE __fsblkcnt_t;	/* File system block counts.  */
typedef __FSFILCNT_T_TYPE __fsfilcnt_t;	/* File system inode counts.  */
typedef __GID_T_TYPE __gid_t;		/* Group IDs.  */
typedef __INO_T_TYPE __ino_t;		/* File serial numbers.  */
typedef __MODE_T_TYPE __mode_t;		/* File attribute bitmasks.  */
typedef __NLINK_T_TYPE __nlink_t;	/* File link counts.  */
typedef __OFF_T_TYPE __off_t;		/* File sizes and offsets.  */
typedef __PID_T_TYPE __pid_t;		/* Process IDs.  */
typedef __RLIM_T_TYPE __rlim_t;		/* Resource measurements.  */
typedef __SSIZE_T_TYPE __ssize_t;	/* Byte count, or error.  */
typedef __TIME_T_TYPE __time_t;		/* Seconds since the Epoch.  */
typedef __TIMER_T_TYPE __timer_t;	/* Timer IDs.  */
typedef __UID_T_TYPE __uid_t;		/* Type of user identifications.  */

/* Types added by LFS:  */
typedef __BLKCNT64_T_TYPE __blkcnt64_t;
typedef __FSBLKCNT64_T_TYPE __fsblkcnt64_t;
typedef __FSFILCNT64_T_TYPE __fsfilcnt64_t;
typedef __INO64_T_TYPE __ino64_t;
typedef __OFF64_T_TYPE __off64_t;
typedef __RLIM64_T_TYPE __rlim64_t;

/* Types added by Y2038:  */
typedef __TIME64_T_TYPE __time64_t;

/* Types added by XSI:  */
typedef __BLKSIZE_T_TYPE __blksize_t;	  /* Size of a disk block.  */
typedef __CLOCK_T_TYPE __clock_t;	  /* CPU usage counts.  */
typedef __ID_T_TYPE __id_t;		  /* General IDs.  */
typedef __KEY_T_TYPE __key_t;		  /* "SysV" IPC keys.  */
typedef __SUSECONDS_T_TYPE __suseconds_t; /* Signed count of microseconds.  */
typedef __USECONDS_T_TYPE __useconds_t;	  /* Count of microseconds.  */

/* Types added by BSD:  */
typedef __FSID_T_TYPE __fsid_t;		/* File system IDs.  */
typedef __REGISTER_T_TYPE __register_t;	/* Integer register.  */

/* Types added by Linux:  */
typedef __FSWORD_T_TYPE __fsword_t;	/* Miscellaneous file system fields.  */
typedef __SYSCALL_SLONG_TYPE __syscall_slong_t;
typedef __SYSCALL_ULONG_TYPE __syscall_ulong_t;

/* These few don't really vary by system, they always correspond
   to one of the other defined types.  */

/* ISO C:  */
typedef __SWORD_TYPE __intptr_t;	/* Signed type that can hold void*.  */
typedef __UWORD_TYPE __uintptr_t;	/* Unsigned type that can hold void*.  */
typedef int __sig_atomic_t;		/* Atomic access relative to signals.  */

/* POSIX:  */
typedef __U32_TYPE __socklen_t;		/* Size of a socket address.  */

/* BSD:  */
typedef char *__caddr_t;                /* Memory address.  */
typedef __S32_TYPE __daddr_t;		/* Disk address.  */
typedef __off64_t __loff_t;		/* File sizes and offsets.  */

#endif /* bits/types.h */
