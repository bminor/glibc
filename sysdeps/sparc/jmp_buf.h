/* Define the machine-dependent type `jmp_buf'.  Sun 4 version.  */

typedef struct
  {
    /* Return PC (register o7).  */
    PTR __pc;
    /* Saved FP.  */
    PTR __fp;
  } jmp_buf[1];
