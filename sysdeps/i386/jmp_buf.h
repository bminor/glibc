/* Define the machine-dependent type `jmp_buf'.  Intel 386 version.  */

typedef struct
  {
    long int __bx, __si, __di;
    PTR __bp;
    PTR __sp;
    PTR __pc;
  } __jmp_buf[1];
