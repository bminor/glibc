/* Finalizer module for ELF shared C library.  This provides terminating
   null pointer words in the `.ctors' and `.dtors' sections.  */

#ifndef NO_CTORS_DTORS_SECTIONS
static void (*const __CTOR_END__[1]) (void)
     __attribute__ ((used, section (".ctors")))
     = { 0 };
static void (*const __DTOR_END__[1]) (void)
     __attribute__ ((used, section (".dtors")))
     = { 0 };
#endif
