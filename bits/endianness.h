/* This file must be overridden for each supported CPU architecture.
   It should define __BYTE_ORDER to one of the constants defined in
   string/bits/endian.h, as appropriate for the machine in
   question.  If floating-point quantities are not stored in the
   same byte order as integer quantities, it should also define
   __FLOAT_WORD_ORDER as appropriate.  */

#ifndef _BITS_ENDIANNESS_H
#define _BITS_ENDIANNESS_H 1

#ifndef _BITS_ENDIAN_H
# error "Never use <bits/endianness.h> directly; include <endian.h> instead."
#endif

#error "Machine byte order unknown."

#endif /* bits/endianness.h */
