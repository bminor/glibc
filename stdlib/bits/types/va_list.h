/* This guard macro needs to match the one used by at least gcc and
   clang's stdarg.h to indicate that va_list, specifically, has been
   defined.  */
#ifndef _VA_LIST

#include <bits/types/__va_list.h>

/* Check again, __va_list.h may not have been able to avoid including
   all of stdarg.h.  */
# ifndef _VA_LIST
typedef __gnuc_va_list va_list;
# endif
# define _VA_LIST

#endif
