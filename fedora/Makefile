# Makefile for maintaining glibc fedora-branch and creating Fedora source RPMs.

tag-prefix := fedora-

.PHONY: update commit-merge commit patch tag archive finish_archive srpm rpm
FORCE:

snapshot-date-fmt := +'%Y-%m-%d %H:%M %Z'
snapshot-name-fmt := +'%Y%m%dT%H%M'

include branch.mk


on-branch = $(filter-out HEAD,$(glibc-base))

snapshot-date := $($(glibc-branch)-sync-date)
snapshot-name := $(shell date -u -d '$(snapshot-date)' $(snapshot-name-fmt))

ifeq ($(glibc-base),HEAD)
tar-name := glibc
select-snapshot = -D '$(snapshot-date)'
snapshot-id = $(glibc-base):$(snapshot-date)
else
tar-name := glibc-$(subst _,.,$(patsubst glibc-%-branch,%,$(glibc-base)))
select-snapshot = -r '$($(glibc-branch)-sync-tag)'
snapshot-id = $($(glibc-branch)-sync-tag)
endif


update:
	now=`date -u -d $(snapshot-date-fmt)`; \
	name=`date -u -d "$$now" $(snapshot-name-fmt)`; \
	$(if $(on-branch),\
	  old="$(tag-prefix)$(tar-name)-$$name; \
	  new="$(tag-prefix)$(tar-name)$(snapshot-name)"; \
	  cvs -Q rtag -r $(glibc-base) $$new libc; \
	, old="$(glibc-base):$(snapshot-date)"; \
	  new="$(glibc-base):$$now"; \
	) \
	(echo '# This file is updated automatically by Makefile.'; \
	 echo 'glibc-branch := $(glibc-branch)'; \
	 echo 'glibc-base := $(glibc-base)'; \
	 echo "$(glibc-branch)-sync-date := $$now"; \
	 $(if $(on-branch),echo "$(glibc-branch)-sync-tag := $$new";) \
	) > branch.mk; \
	cd ..; cvs -q update -d -P -j"$$old" -j"$$new" \
		   2>&1 | tee redhat/update.log

commit-merge:
	cd ..; cvs -Q commit -m"Updated to $(snapshot-id)"

glibc.spec: glibc.spec.in ../version.h branch.mk
	(echo '%define glibcdate $(snapshot-name)'; \
	 sed -n '/VERSION/s/^.*"\([^"]*\)"$$/%define glibcversion \1/p' \
	     ../version.h; \
	 cat $<) > $@.new
	mv -f $@.new $@

spec-nvr := $(shell rpm -q --qf '%{NAME}-%{VERSION}-%{RELEASE}\n' \
			--specfile glibc.spec | sed 1q)
spec-tag = $(subst .,_,$(spec-nvr))

tag: glibc.spec
	cd ..; cvs -Q tag -c $(tag-prefix)$(spec-tag)

$(tar-name)-$(glibc-branch).patch: makepatch.awk glibc.spec FORCE
	(cd ..; cvs -q diff -upN \
		    $(select-snapshot) -r $(tag-prefix)$(spec-tag)) | \
	awk -v OLDVER=$(tar-name)-$(snapshot-name) \
	    -v NEWVER=$(tar-name)-$(snapshot-name)-$(glibc-branch) \
	    -f $< > patch.tmp
	mv -f patch.tmp $@

# makepatch.awk omits these files from the patch; we put them in a tar file.
outside-patch = fedora c_stubs rtkaio glibc-compat \
		localedata/charmaps/GB18030 iconvdata/gb18030.c

$(tar-name)-$(glibc-branch)-$(snapshot-name).tar.bz2: FORCE
	@rm -rf libc
	cvs -Q export -r$(tag-prefix)$(spec-tag) \
		      $(addprefix libc/,$(outside-patch))
	tar cf - -C libc $(outside-patch) | bzip2 -9 > $@.new
	rm -rf libc
	mv -f $@.new $@

$(tar-name)-$(snapshot-name).tar.bz2: Makefile branch.mk
	@rm -rf $(tar-name)-$(snapshot-name)
	cvs -Q export -d $(tar-name)-$(snapshot-name) $(select-snapshot) libc
	tar cf - $(tar-name)-$(snapshot-name) | bzip2 -9 > $@.new
	rm -rf $(tar-name)-$(snapshot-name)
	mv -f $@.new $@

archives = $(tar-name)-$(snapshot-name).tar.bz2 \
	   $(tar-name)-$(glibc-branch)-$(snapshot-name).tar.bz2 \
	   $(tar-name)-$(glibc-branch).patch
finish_archive: $(archives)

archive: glibc.spec
	$(MAKE) tag finish_archive

rpm srpm: $(spec-nvr).src.rpm
$(spec-nvr).src.rpm: glibc.spec $(archives)
	rpmbuild --define "_sourcedir `pwd`" \
		 --define "_specdir `pwd`" \
		 --define "_srcrpmdir `pwd`" \
		 --nodeps -bs $<
