/* Define the machine-dependent type `jmp_buf'.  Sun 4 version.  */

typedef struct
  {
    /* Return PC (register o7).  */
    __ptr_t __pc;
    /* Saved FP.  */
    __ptr_t __fp;
  } __jmp_buf[1];
