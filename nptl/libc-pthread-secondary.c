/* Copyright (C) 2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifdef HAVE_ASM_SECONDARY_DIRECTIVE
# define pthread_mutex_lock __rename_pthread_mutex_lock
# define pthread_mutex_unlock __rename_pthread_mutex_unlock
# include <safe-fatal.h>
# include <errno.h>
# undef pthread_mutex_lock
# undef pthread_mutex_unlock

static void __attribute__ ((unused))
pthread_secondary_void (void)
{
}

static int __attribute__ ((unused))
pthread_secondary_zero (void)
{
  return 0;
}

static int __attribute__ ((unused))
pthread_secondary_einval (void)
{
  return EINVAL;
}

/* Use STB_SECONDARY on pthread functions in libc so that they are used
   only when libpthread is not used.  */

asm (".secondary _pthread_cleanup_push_defer");
strong_alias (pthread_secondary_void, _pthread_cleanup_push_defer)

asm (".secondary _pthread_cleanup_pop_restore");

void
_pthread_cleanup_pop_restore (struct _pthread_cleanup_buffer *buffer,
			      int execute)
{
  if (execute)
    buffer->__routine (buffer->__arg);
}

asm (".secondary __pthread_cleanup_upto");
strong_alias (pthread_secondary_void, __pthread_cleanup_upto)

asm (".secondary __pthread_getspecific");
strong_alias (pthread_secondary_zero, __pthread_getspecific)

asm (".secondary __pthread_setspecific");
strong_alias (pthread_secondary_einval, __pthread_setspecific)

asm (".secondary __pthread_key_create");
strong_alias (pthread_secondary_einval, __pthread_key_create)

asm (".secondary __pthread_mutex_lock");
strong_alias (pthread_secondary_zero, __pthread_mutex_lock)

asm (".secondary __pthread_mutex_unlock");
strong_alias (pthread_secondary_zero, __pthread_mutex_unlock)

asm (".secondary __pthread_once");
asm (".secondary pthread_once");

int
__pthread_once (pthread_once_t *once_control,
		void (*init_routine) (void))
{
  if (*once_control == PTHREAD_ONCE_INIT)
    {
      init_routine ();
      *once_control |= 2;
    }
  return 0;
}
strong_alias (__pthread_once, pthread_once)

asm (".secondary __pthread_rwlock_rdlock");
strong_alias (pthread_secondary_zero, __pthread_rwlock_rdlock)

asm (".secondary __pthread_rwlock_unlock");
strong_alias (pthread_secondary_zero, __pthread_rwlock_unlock)

asm (".secondary __pthread_rwlock_wrlock");
strong_alias (pthread_secondary_zero, __pthread_rwlock_wrlock)

asm (".secondary __pthread_unwind");

void
__attribute ((noreturn))
__cleanup_fct_attribute
attribute_compat_text_section
__pthread_unwind (__pthread_unwind_buf_t *buf)
{
  /* We cannot call abort() here.  */
  typedef __typeof (__safe_fatal) *fn_noreturn __attribute ((noreturn));
  fn_noreturn fn = (fn_noreturn) __safe_fatal;
  fn ();
}

# ifndef SHARED
asm (".secondary __pthread_setcancelstate");
strong_alias (pthread_secondary_zero, __pthread_setcancelstate)

asm (".secondary pthread_mutex_lock");
strong_alias (__pthread_mutex_lock, pthread_mutex_lock)

asm (".secondary pthread_mutex_unlock");
strong_alias (__pthread_mutex_unlock, pthread_mutex_unlock)

asm (".secondary __pthread_rwlock_destroy");
strong_alias (pthread_secondary_zero, __pthread_rwlock_destroy)

asm (".secondary __pthread_rwlock_init");
strong_alias (pthread_secondary_zero, __pthread_rwlock_init)
# endif
#endif
