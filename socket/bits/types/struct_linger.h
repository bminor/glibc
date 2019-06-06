#ifndef _BITS_TYPES_STRUCT_LINGER_H
#define _BITS_TYPES_STRUCT_LINGER_H 1

/* Structure used to manipulate the SO_LINGER option.  */
struct linger
  {
    int l_onoff;		/* Nonzero to linger on close.  */
    int l_linger;		/* Time to linger.  */
  };

#endif
