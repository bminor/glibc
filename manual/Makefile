# Makefile for the GNU C Library manual.

# Copyright (C) 1992, 1993 Free Software Foundation, Inc.
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

subdir := manual

.PHONY: all dvi info
all: dvi info
dvi: libc.dvi
info: libc.info

# Set chapters and chapters-incl.
include $(objpfx)chapters
$(objpfx)chapters: libc.texinfo
	$(find-includes)
chapters := $(filter-out summary.texi,$(chapters))
ifdef chapters
include $(objpfx)chapters-incl
$(objpfx)chapters-incl: $(chapters)
	$(find-includes)
endif

define find-includes
(echo '$@ :=' \\	;\
 awk '$$1 == "@include" { print $$2 " \\" }' $^) > $@.new
mv $@.new $@
endef

$(objpfx)libc.dvi $(objpfx)libc.info: $(chapters) summary.texi $(chapters-incl)
$(objpfx)libc.dvi: texinfo.tex

# Generate the summary from the Texinfo source files for each chapter.
$(objpfx)summary.texi: stamp-summary ;
$(objpfx)stamp-summary: summary.awk $(chapters)
	awk -f $^ \
	| sort +1 -2 | tr '\014' '\012' > summary-tmp
	./move-if-change summary-tmp $(objpfx)summary.texi
# touch is broken on our machines.  Sigh.
	date > $@

# Generate Texinfo files from the C source for the example programs.
$(objpfx)%.c.texi: examples/%.c
	sed -e 's,[{}],@&,g'				\
	    -e 's,/\*\(@.*\)\*/,\1,g'			\
	    -e 's,/\* *\(.*\) *\*/,/* @r{\1} */,g'	\
	    -e 's/\(@[a-z][a-z]*\)@{\([^}]*\)@}/\1{\2}/'\
	    $< > $@.new
	mv $@.new $@


minimal-dist = COPYING.LIB summary.awk Makefile move-if-change		\
	       libc.texinfo $(chapters)					\
	       $(patsubst %.c.texi,examples/%.c,			\
			  $(filter-out summary.texi,$(chapters-incl)))
distribute = $(minimal-dist)	       					\
	     $(patsubst examples/%.c,%.c.texi,$(filter examples/%.c,	\
			$(minimal-dist)))				\
	     libc.?? libc.??s texinfo.tex summary.texi			\
	     stamp-summary chapters chapters-incl

tar-it = tar chovf $@ $^

manual.tar: $(minimal-dist) ; $(tar-it)
mandist.tar: $(distribute) ; $(tar-it)

edition := $(shell sed -n 's/^@set EDITION \(.*\)$$/\1/p' libc.texinfo)

glibc-doc-$(edition).tar: $(distribute)
	@rm -f glibc-doc-$(edition)
	ln -s . glibc-doc-$(edition)
	tar chvf $@ $(addprefix glibc-doc-$(edition)/,$^)
	rm -f glibc-doc-$(edition)

%.Z: %
	compress -c $< > $@.new
	mv $@.new $@
%.z: %
	gzip -9 -c $< > $@.new
	mv $@.new $@
%.uu: %
	uuencode $< < $< > $@.new
	mv $@.new $@

.PHONY: dist
dist: glibc-doc-$(edition).tar.z

ifneq (,)
dist: ../Make-dist
	$(MAKE) -f $< $(Make-dist-args)
endif

ifndef ETAGS
ETAGS = etags -T
endif
TAGS: $(minimal-dist)
	$(ETAGS) -o $@ $^
