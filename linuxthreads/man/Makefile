SOURCES= pthread_cleanup_push.man \
  pthread_kill_other_threads_np.man pthread_mutexattr_setkind_np.man

MANPAGES=$(SOURCES:.man=.3thr)

PREPRO=perl troffprepro

MANDIR=/usr/man/man3

all: $(MANPAGES)

.SUFFIXES: .man .3thr

.man.3thr:
	$(PREPRO) $*.man $*.3thr

$(MANPAGES): troffprepro

clean:
	rm -f *.3thr
	rm -f *~

install:
	install *.3thr $(MANDIR)
	@echo "*** Remember to run /usr/sbin/makewhatis `dirname $(MANDIR)` at some point"
