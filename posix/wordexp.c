/* Copyright (C) 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <sys/types.h>
#include <wordexp.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int
DEFUN(wordexp, (string, pwordexp, flags),
      CONST char *string AND wordexp_t *pwordexp AND int flags)
{
  int error;
  pid_t pid;
  int d[2];
  int status;

  struct word
    {
      struct word *next;
      char *word;
    } *words;
  size_t nwords, wordc, i;
  FILE *f;

  if (pipe (d) < 0)
    return -1;

  pid = fork ();
  if (pid < 0)
    return -1;

  if (pid == 0)
    {
      /* Child.  Run the shell.  */

      CONST char fmt[]
	= "for word in %s; do echo -n \"$word\"; echo -e \\\\000; done";
      char flags[] = "-Puc", *f = &flags[1];
      char *buffer = (char *) __alloca (fmt - 2 + strlen (string));
      if (flags & WRDE_NOCMD)
	*f++ = 'P';
      if (flags & WRDE_UNDEF)
	*f++ = 'u';
      *f++ = 'c';
      *f = '\0';
      sprintf (buffer, fmt, string);
      close (d[STDIN_FILENO]);
      dup2 (d[STDOUT_FILENO], STDOUT_FILENO);
      if (!(flags & WRDE_SHOWERR))
	close (STDERR_FILENO);
      execl (SHELL_PATH, SHELL_NAME, flags, buffer, (char *) NULL);
      _exit (WRDE_ABEND);
    }

  /* Parent.  */

  words = NULL;
  nwords = 0;
  lose = WRDE_NOSPACE;

  close (d[STDOUT_FILENO]);
  f = fdopen (d[STDIN_FILENO]);
  if (f == NULL)
    goto lose;

  while (!feof (f))
    {
      struct word *new = (struct word *) __alloca (sizeof (struct word));
      size_t len = 0;
      new->word = NULL;
      if (__getdelim (&new->word, &len, '\0', f) == -1)
	goto lose;
      new->next = words;
      words = new;
      ++nwords;
    }

  if (waitpid (pid, &status, 0) != pid)
    goto lose;

  if (WIFEXITED (status))
    {
      if (WEXITSTATUS (status) != 0)
	{
	  error = WEXITSTATUS (status);
	  goto lose;
	}
    }
  else
    goto lose;

  wordc = nwords + 1;
  if (flags & WRDE_DOOFFS)
    words += pwordexp->we_offs;
  if (flags & WRDE_APPEND)
    wordc += pwordexp->we_wordc;

  if (flags & WRDE_APPEND)
    wordv = (char **) realloc ((PTR) pwordexp->we_wordv,
			       wordc * sizeof (char *));
  else
    wordv = (char **) malloc (wordc * sizeof (char *));
  if (wordv == NULL)
    goto lose;

  if (flags & WRDE_DOOFFS)
    for (i = 0; i < pwordexp->we_offs; ++i)
      wordv[i] = NULL;

  i = wordc;
  while (words != NULL)
    {
      wordv[--i] = words->word;
      words = words->next;
    }

  if (flags & WRDE_REUSE)
    wordfree (pwordexp);

  pwordexp->we_wordc = wordc;
  pwordexp->we_wordv = wordv;

  return 0;

 lose:
  {
    int save;
    save = errno;
    (void) kill (pid, SIGKILL);
    while (words != NULL)
      {
	free (words->word);
	words = words->next;
      }
    errno = save;
    return error;
  }
}
