/* Atfork handling.  Hurd pthread version.
   Copyright (C) 2002-2020 Free Software Foundation, Inc.
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
   License along with the GNU C Library;  if not, see
   <https://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stdlib.h>
#include <libc-lock.h>
#include <fork.h>

struct atfork
{
  void (*prepare) (void);
  void (*parent) (void);
  void (*child) (void);
  void *dso_handle;
  struct atfork *prev;
  struct atfork *next;
};

/* TODO: better locking */
__libc_lock_define_initialized (static, atfork_lock);
static struct atfork *fork_handlers, *fork_last_handler;

static void
atfork_pthread_prepare (void)
{
  struct atfork *handlers, *last_handler;

  __libc_lock_lock (atfork_lock);
  handlers = fork_handlers;
  last_handler = fork_last_handler;
  __libc_lock_unlock (atfork_lock);

  if (last_handler == NULL)
    return;

  while (1)
    {
      if (last_handler->prepare != NULL)
	last_handler->prepare ();
      if (last_handler == handlers)
	break;
      last_handler = last_handler->prev;
    }
}
text_set_element (_hurd_atfork_prepare_hook, atfork_pthread_prepare);

static void
atfork_pthread_parent (void)
{
  struct atfork *handlers;

  __libc_lock_lock (atfork_lock);
  handlers = fork_handlers;
  __libc_lock_unlock (atfork_lock);

  while (handlers != NULL)
    {
      if (handlers->parent != NULL)
	handlers->parent ();
      handlers = handlers->next;
    }
}
text_set_element (_hurd_atfork_parent_hook, atfork_pthread_parent);

static void
atfork_pthread_child (void)
{
  struct atfork *handlers;

  __libc_lock_lock (atfork_lock);
  handlers = fork_handlers;
  __libc_lock_unlock (atfork_lock);

  while (handlers != NULL)
    {
      if (handlers->child != NULL)
	handlers->child ();
      handlers = handlers->next;
    }
}
text_set_element (_hurd_atfork_child_hook, atfork_pthread_child);

int
__register_atfork (void (*prepare) (void),
		   void (*parent) (void),
		   void (*child) (void),
		   void *dso_handle)
{
  struct atfork *new = malloc (sizeof (*new));
  if (new == NULL)
    return errno;

  new->prepare = prepare;
  new->parent = parent;
  new->child = child;
  new->dso_handle = dso_handle;
  new->prev = NULL;

  __libc_lock_lock (atfork_lock);
  new->next = fork_handlers;
  if (fork_handlers != NULL)
    fork_handlers->prev = new;
  fork_handlers = new;
  if (fork_last_handler == NULL)
    fork_last_handler = new;
  __libc_lock_unlock (atfork_lock);

  return 0;
}
libc_hidden_def (__register_atfork)

void
__unregister_atfork (void *dso_handle)
{
  struct atfork **handlers, *prev = NULL, *next;
  __libc_lock_lock (atfork_lock);
  handlers = &fork_handlers;
  while (*handlers != NULL)
    {
      if ((*handlers)->dso_handle == dso_handle)
	{
	  /* Drop this handler from the list.  */
	  if (*handlers == fork_last_handler)
	    {
	      /* Was last, new last is prev, if any.  */
	      fork_last_handler = prev;
	    }

	  next = (*handlers)->next;
	  if (next != NULL)
	    next->prev = prev;
	  *handlers = next;
	}
      else
	{
	  /* Just proceed to next handler.  */
	  prev = *handlers;
	  handlers = &prev->next;
	}
    }
  __libc_lock_unlock (atfork_lock);
}
