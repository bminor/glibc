# Makefile for the GNU C Library manual.

# Copyright (C) 1992 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

chapters=archive.texinfo assert.texinfo conf.texinfo copying.texinfo	\
ctype.texinfo database.texinfo errno.texinfo filesys.texinfo		\
header.texinfo intro.texinfo io.texinfo job.texinfo			\
lgpl.texinfo limits.texinfo llio.texinfo				\
locale.texinfo maint.texinfo math.texinfo mbyte.texinfo memory.texinfo	\
pipe.texinfo process.texinfo search.texinfo setjmp.texinfo		\
signal.texinfo socket.texinfo stdarg.texinfo stddef.texinfo		\
stdio.texinfo string.texinfo sysinfo.texinfo terminal.texinfo		\
time.texinfo


.PHONY: all
all: library.dvi library.info

library.dvi library.info: $(chapters) summary.texinfo texinfo.tex

library.info:
	makeinfo library.texinfo

summary.texinfo: summary.awk $(chapters)
	awk -f $^ \
	| sort +1 -2 | tr '\014' '\012' > $@.new
	mv $@.new $@

headers :=
sources :=
subdir := manual
distribute := library.texinfo $(chapters) texinfo.tex \
	      summary.texinfo summary.awk
.PHONY: dist
dist: ../Make-dist
	$(MAKE) -f $< $(Make-dist-args)
