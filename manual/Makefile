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

export subdir := manual

.PHONY: all dvi info
all: dvi info
dvi: libc.dvi
info: libc.info

# Get glibc's configuration info.
ifneq (,$(wildcard ../Makeconfig))
include ../Makeconfig
endif

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
(echo '$(@F) :=' \\	;\
 awk '$$1 == "@include" { print $$2 " \\" }' $^) > $@.new
mv $@.new $@
endef

$(objpfx)libc.dvi $(objpfx)libc.info: $(chapters) summary.texi $(chapters-incl)
$(objpfx)libc.dvi: texinfo.tex

$(objpfx)%.info: %.texinfo
	$(MAKEINFO) $< -o $@
$(objpfx)%.dvi: %.texinfo
	$(TEXI2DVI) $<

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
export distribute := $(distribute)

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

.PHONY: mostlyclean distclean realclean clean subdir_clean
subdir_clean: clean
distclean clean: mostlyclean
mostlyclean:
	-rm -f $(addprefix $(objpfx),libc.dvi libc.info*)
realclean: clean
	-rm -f $(addprefix $(objpfx),chapters chapters-incl \
				     summary.texi stamp-summary \
				     *.c.texi)

.PHONY: install subdir_install
subdir_install: install
install: $(infodir)/libc.info

ifndef infodir
infodir = $(prefix)/info
endif
ifndef prefix
prefix = /usr/local
endif

ifndef INSTALL_DATA
INSTALL_DATA = $(INSTALL) -m 644
endif
ifndef INSTALL
INSTALL = install
endif

$(infodir)/libc.info: $(objpfx)libc.info
	for file in $<*; do \
	  name=`basename $$file`; \
	  $(INSTALL_DATA) $$file \
	    `echo $@ | sed "s,$(<F)\$$,$$name,"`; \
	done

.PHONY: dist
dist: glibc-doc-$(edition).tar.z

ifneq (,$(wildcard ../Make-dist))
dist: ../Make-dist
	$(MAKE) -f $< $(Make-dist-args)
endif

ifndef ETAGS
ETAGS = etags -T
endif
TAGS: $(minimal-dist)
	$(ETAGS) -o $@ $^

# These are targets that each glibc subdirectory is expected to understand.
# ../Rules defines them for code subdirectories; for us, they are no-ops.
glibc-targets	:= subdir_lib objects objs others tests subdir_lint.out \
		   subdir_echo-headers subdir_echo-distinfo stubs
.PHONY: $(glibc-targets)
$(glibc-targets):

stubs: $(common-objpfx)stub-manual
$(common-objpfx)stub-manual:
	cp /dev/null $@


# Get rid of these variables if they came from the parent.
routines =
aux =
sources =
objects =
headers =
