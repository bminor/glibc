/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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

#ifndef	__GNU_STABS_H

#define	__GNU_STABS_H	1

#ifdef	HAVE_GNU_LD

/* Alias a function:
   	function_alias(creat, _creat, int, (file, mode),
		       DEFUN(creat, (file, mode),
		             CONST char *file AND int mode))
   Yes, this is very repetitive.  Nothing you can do about it, so shut up.  */
#define	function_alias(name, _name, type, args, defun) \
  symbol_alias (_name, name);

#define function_alias_void(name, _name, args, defun) \
  symbol_alias (_name, name);

/* Make references to ALIAS refer to SYMBOL.  */
#ifdef	__STDC__
#define	symbol_alias(symbol, alias)	\
  asm(".stabs \"" "_" #alias "\",11,0,0,0\n"\
      ".stabs \"" "_" #symbol "\",1,0,0,0")
#else
/* Your assembler better grok this right!  */
#define	symbol_alias(symbol, alias)	\
  asm(".stabs \"_/**/alias\",11,0,0,0\n.stabs \"_/**/symbol\",1,0,0,0")
#endif

/* Issue a warning message from the linker whenever SYMBOL is referenced.  */
#ifdef	__STDC__
#define	warn_references(symbol, msg)	\
  asm(".stabs \"" msg "\",30,0,0,0\n"	\
      ".stabs \"_" #symbol "\",1,0,0,0")
#else
#define	warn_references(symbol, msg)	\
  asm(".stabs msg,30,0,0,0\n.stabs \"_/**/symbol\",1,0,0,0")
#endif

#ifdef	__STDC__
#define	stub_warning(name) \
  warn_references(name, \
		  "warning: " #name " is not implemented and will always fail")
#else
#define	stub_warning(name) \
  warn_references(name, \
		  "warning: name is not implemented and will always fail")
#endif

#ifdef	__STDC__
#define	text_set_element(set, symbol)	\
  asm(".stabs \"_" #set "\",23,0,0,_" #symbol)
#define	data_set_element(set, symbol)	\
  asm(".stabs \"_" #set "\",25,0,0,_" #symbol)
#else
#define	text_set_element(set, symbol)	\
  asm(".stabs \"_/**/set\",23,0,0,_/**/symbol")
#define	data_set_element(set, symbol)	\
  asm(".stabs \"_/**/set\",25,0,0,_/**/symbol")
#endif

#else	/* No GNU stabs.  */

#define	function_alias(name, _name, type, args, defun) \
  type defun { return _name args; }

#define function_alias_void(name, _name, args, defun) \
  void defun { _name args; }

#endif	/* GNU stabs.  */

#endif	/* gnu-stabs.h  */
