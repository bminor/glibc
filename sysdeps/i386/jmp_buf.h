/* Define the machine-dependent type `jmp_buf'.  Intel 386 version.  */

typedef struct
  {
    long int __bx, __si, __di, __bp, __sp, __dx;
  } __jmp_buf[1];
