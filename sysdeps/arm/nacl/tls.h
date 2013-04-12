/* XXX */

#ifndef _TLS_H
#define _TLS_H	1

#ifndef __ASSEMBLER__
# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include <nacl-interfaces.h>

/* Type for the dtv.  */
typedef union dtv
{
  size_t counter;
  struct
  {
    void *val;
    bool is_static;
  } pointer;
} dtv_t;

/* The TP points to the start of the thread blocks.  */
# define TLS_DTV_AT_TP	1

typedef struct
{
  dtv_t *dtv;
  void *private;
} tcbhead_t;

/* This is the size of the initial TCB.  */
# define TLS_INIT_TCB_SIZE	sizeof (tcbhead_t)

/* Alignment requirements for the initial TCB.  */
# define TLS_INIT_TCB_ALIGN	16

/* This is the size of the TCB.  */
# define TLS_TCB_SIZE		sizeof (tcbhead_t)

/* This is the size we need before TCB.  */
# define TLS_PRE_TCB_SIZE	0       /* XXX */

/* Return the thread descriptor for the current thread.  */
# define THREAD_SELF	__builtin_thread_pointer ()

/* Alignment requirements for the TCB.  */
# define TLS_TCB_ALIGN		16

/* Install the dtv pointer.  The pointer passed is to the element with
   index -1 which contain the length.  */
# define INSTALL_DTV(tcbp, dtvp) \
  (((tcbhead_t *) (tcbp))->dtv = (dtvp) + 1)

/* Install new dtv for current thread.  */
# define INSTALL_NEW_DTV(dtv) \
  (THREAD_DTV() = (dtv))

/* Return dtv of given thread descriptor.  */
# define GET_DTV(tcbp) \
  (((tcbhead_t *) (tcbp))->dtv)

/* Return the address of the dtv for the current thread.  */
# define THREAD_DTV() \
  (((tcbhead_t *) __builtin_thread_pointer ())->dtv)

#define TLS_INIT_TP_EXPENSIVE 1

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.  */
# define TLS_INIT_TP(tcbp, secondcall) \
  ((*__nacl_irt_tls.tls_init) (tcbp) == 0 ? NULL : "tls_init call failed")

/* Get and set the global scope generation counter in struct pthread.  */
#define THREAD_GSCOPE_FLAG_UNUSED 0
#define THREAD_GSCOPE_FLAG_USED   1
#define THREAD_GSCOPE_FLAG_WAIT   2
#define THREAD_GSCOPE_RESET_FLAG() \
  do									     \
    {}									     \
  while (0)
#define THREAD_GSCOPE_SET_FLAG() \
  do									     \
    {}									     \
  while (0)
#define THREAD_GSCOPE_WAIT() ((void) 0)


#endif /* __ASSEMBLER__ */

#endif	/* tls.h */
