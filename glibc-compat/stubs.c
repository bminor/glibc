/*
 * STAT stuff that breaks Applix
 */

#include <sys/stat.h>

/* 1 of 3: _xstat */
int
_xstat (int vers, const char *name, struct stat *buf)
{
    return __xstat (vers, name, buf);
}

/* 2 of 3: _fxstat */
int
_fxstat (int vers, int fd, struct stat *buf)
{
    return __fxstat (vers, fd, buf);
}

/* 3 of 3: _lxstat */
int
_lxstat (int vers, const char *name, struct stat *buf)
{
    return __lxstat (vers, name, buf);
}


/*
 * __setjmp stuff that breaks again Applix
 */
#include <setjmp.h>

int __setjmp(jmp_buf env)
{
    return _setjmp(env);
}


/*
 * __setfpucw break several math packages that ahve not heard of
 * the standard _FPU_SETCW() way of setting the control word for the FPU
 */
#include <fpu_control.h>
void __setfpucw(fpu_control_t cw)
{
    _FPU_SETCW(cw);
}


/* Register FUNC to be executed by `exit'.  */
int
atexit (void (*func) (void))
{
  int __cxa_atexit (void (*func) (void *), void *arg, void *d);
  return __cxa_atexit ((void (*) (void *)) func, 0, 0);
}
