#ifndef	_PRINTF_H

/* Workaround PR90731 with GCC 9 when using ldbl redirects in C++.  */
#include <bits/floatn.h>
#if defined __cplusplus && __LDOUBLE_REDIRECTS_TO_FLOAT128_ABI == 1
# if __GNUC_PREREQ (9, 0) && !__GNUC_PREREQ (9, 3)
#   pragma GCC system_header
# endif
#endif

#include <stdio-common/printf.h>

# ifndef _ISOMAC

/* Internal interfaces for registered specifiers.  */
extern printf_arginfo_size_function **__printf_arginfo_table attribute_hidden;
extern printf_function **__printf_function_table attribute_hidden;
extern printf_va_arg_function **__printf_va_arg_table attribute_hidden;
int __register_printf_specifier (int, printf_function,
				 printf_arginfo_size_function);
libc_hidden_proto (__register_printf_specifier)

/* The various kinds of arguments that can be passed to printf.  */
union printf_arg
  {
    wchar_t pa_wchar;
    int pa_int;
    long int pa_long_int;
    long long int pa_long_long_int;
    unsigned int pa_u_int;
    unsigned long int pa_u_long_int;
    unsigned long long int pa_u_long_long_int;
    double pa_double;
    long double pa_long_double;
#if __HAVE_FLOAT128_UNLIKE_LDBL
    _Float128 pa_float128;
#endif
    const char *pa_string;
    const wchar_t *pa_wstring;
    void *pa_pointer;
    void *pa_user;
};

#include <bits/types/locale_t.h>

/* Now define the internal interfaces.  */
extern int __printf_fphex (FILE *, const struct printf_info *,
			   const void *const *) attribute_hidden;
extern int __printf_fp (FILE *, const struct printf_info *,
			const void *const *);
libc_hidden_proto (__printf_fp)
extern int __printf_fp_l (FILE *, locale_t, const struct printf_info *,
			  const void *const *);
libc_hidden_proto (__printf_fp_l)

extern unsigned int __guess_grouping (unsigned int intdig_max,
				      const char *grouping)
     attribute_hidden;

# endif /* !_ISOMAC */
#endif
