#if defined __sparc__ && defined __arch64__
register void *__thread_self __asm ("g7");
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <dirent.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

int main(void)
{
  pid_t pid;
  int status, rerun_ldconfig = 0, rerun_cnt = 0;
  char initpath[256];

#ifdef __i386__
  char buffer[4096];
  struct pref {
    char *p;
    int len;
  } prefix[] = { { "libc-", 5 }, { "libm-", 5 },
		 { "librt-", 6 }, { "libpthread-", 11 },
		 { "librtkaio-", 10 }, { "libthread_db-", 13 } };
  int i, j, fd;
  off_t base;
  ssize_t ret;
#ifdef ARCH_386
  const char *remove_dirs[] = { "/lib/i686", "/lib/tls" };
#else
  const char *remove_dirs[] = { "/lib/tls/i686" };
#endif
  for (j = 0; j < sizeof (remove_dirs) / sizeof (remove_dirs[0]); ++j)
    {
      size_t rmlen = strlen (remove_dirs[j]);
      fd = open (remove_dirs[j], O_RDONLY);
      if (fd >= 0
	  && (ret = getdirentries (fd, buffer, sizeof (buffer), &base))
	     >= (ssize_t) offsetof (struct dirent, d_name))
	{
	  for (base = 0; base + offsetof (struct dirent, d_name) < ret; )
	    {
	      struct dirent *d = (struct dirent *) (buffer + base);

	      for (i = 0; i < sizeof (prefix) / sizeof (prefix[0]); i++)
		if (! strncmp (d->d_name, prefix[i].p, prefix[i].len))
		  {
		    char *p = d->d_name + prefix[i].len;

		    while (*p == '.' || (*p >= '0' && *p <= '9')) p++;
		    if (p[0] == 's' && p[1] == 'o' && p[2] == '\0'
			&& p + 3 - d->d_name
			   < sizeof (initpath) - rmlen - 1)
		      {
			memcpy (initpath, remove_dirs[j], rmlen);
			initpath[rmlen] = '/';
			strcpy (initpath + rmlen + 1, d->d_name);
			unlink (initpath);
			break;
		      }
		  }
	      base += d->d_reclen;
	    }
	  close (fd);
	}
    }
#endif

  int ldsocfd = open (LD_SO_CONF, O_RDONLY);
  struct stat ldsocst;
  if (ldsocfd >= 0 && fstat (ldsocfd, &ldsocst) >= 0)
    {
      char p[ldsocst.st_size + 1];
      if (read (ldsocfd, p, ldsocst.st_size) == ldsocst.st_size)
        {
	  p[ldsocst.st_size] = '\0';
	  if (strstr (p, "include ld.so.conf.d/*.conf") == NULL)
	    {
	      close (ldsocfd);
	      ldsocfd = open (LD_SO_CONF, O_WRONLY | O_TRUNC);
	      if (ldsocfd >= 0)
		{
		  size_t slen = strlen ("include ld.so.conf.d/*.conf\n");
		  if (write (ldsocfd, "include ld.so.conf.d/*.conf\n", slen)
		      != slen
		      || write (ldsocfd, p, ldsocst.st_size) != ldsocst.st_size)
		    _exit (109);
		}
	    }
        }
      if (ldsocfd >= 0)
	close (ldsocfd);
    }

  do
    {
      char linkbuf[64], *linkp;
      int linklen;

      pid = vfork ();
      if (pid == 0)
	{
	  execl ("/sbin/ldconfig", "/sbin/ldconfig", NULL);
	  _exit (110);
	}
      else if (pid < 0)
	_exit (111);
      if (waitpid (0, &status, 0) != pid || !WIFEXITED (status))
	_exit (112);
      if (WEXITSTATUS (status))
	_exit (WEXITSTATUS (status));

      rerun_ldconfig = 0;
#ifdef LIBTLS
      linkp = linkbuf + strlen (LIBTLS);
      linklen = readlink (LIBTLS "librt.so.1", linkp,
			  sizeof (linkbuf) - 1 - strlen (LIBTLS));
      if (linklen == strlen ("librtkaio-2.3.X.so")
	  && memcmp (linkp, "librtkaio-2.3.", 14) == 0
	  && strchr ("23", linkp[14])
	  && memcmp (linkp + 15, ".so", 4) == 0)
	{
	  memcpy (linkbuf, LIBTLS, strlen (LIBTLS));
	  unlink (linkbuf);
	  rerun_ldconfig = 1;
	}
#endif

#ifdef __i386__
      linkp = linkbuf + strlen ("/lib/i686/");
      linklen = readlink ("/lib/i686/librt.so.1", linkp,
			  sizeof (linkbuf) - 1 - strlen ("/lib/i686/"));
      if (linklen == strlen ("librtkaio-2.3.X.so")
	  && memcmp (linkp, "librtkaio-2.3.", 14) == 0
	  && strchr ("23", linkp[14])
	  && memcmp (linkp + 15, ".so", 4) == 0)
	{
	  memcpy (linkbuf, "/lib/i686/", strlen ("/lib/i686/"));
	  unlink (linkbuf);
	  rerun_ldconfig = 1;
	}
#endif
     }
  while (rerun_ldconfig && ++rerun_cnt < 2);

  if (! utimes (GCONV_MODULES_CACHE, NULL))
    {
      pid = vfork ();
      if (pid == 0)
	{
	  execl ("/usr/sbin/iconvconfig", "/usr/sbin/iconvconfig", NULL);
	  _exit (113);
	}
      else if (pid < 0)
	_exit (114);
      if (waitpid (0, &status, 0) != pid || !WIFEXITED (status))
	_exit (115);
      if (WEXITSTATUS (status))
	_exit (WEXITSTATUS (status));
    }

  /* Check if telinit is available and the init fifo as well.  */
  if (access ("/sbin/telinit", X_OK) || access ("/dev/initctl", F_OK))
    _exit (0);
  /* Check if we are not inside of some chroot, because we'd just
     timeout and leave /etc/initrunlvl.  */
  if (readlink ("/proc/1/exe", initpath, 256) <= 0 ||
      readlink ("/proc/1/root", initpath, 256) <= 0)
    _exit (0);

  pid = vfork ();
  if (pid == 0)
    {
      execl ("/sbin/telinit", "/sbin/telinit", "u", NULL);
      _exit (116);
    }
  else if (pid < 0)
    _exit (117);
  if (waitpid (0, &status, 0) != pid || !WIFEXITED (status))
    _exit (118);

  /* Check if we can safely condrestart sshd.  */
  if (access ("/sbin/service", X_OK) == 0
      && access ("/usr/sbin/sshd", X_OK) == 0
      && access ("/bin/bash", X_OK) == 0)
    {
      pid = vfork ();
      if (pid == 0)
	{
	  execl ("/sbin/service", "/sbin/service", "sshd", "condrestart", NULL);
	  _exit (119);
	}
      else if (pid < 0)
	_exit (120);
      if (waitpid (0, &status, 0) != pid || !WIFEXITED (status))
	_exit (121);
    }

  _exit(0);
}

int __libc_multiple_threads __attribute__((nocommon));
int __libc_enable_asynccancel (void) { return 0; }
void __libc_disable_asynccancel (int x) { }
void __libc_csu_init (void) { }
void __libc_csu_fini (void) { }
pid_t __fork (void) { return -1; }
char thr_buf[65536];

#ifndef __powerpc__
int __libc_start_main (int (*main) (void), int argc, char **argv,
		       void (*init) (void), void (*fini) (void),
		       void (*rtld_fini) (void), void * stack_end)
#else
struct startup_info
{
  void *sda_base;
  int (*main) (int, char **, char **, void *);
  int (*init) (int, char **, char **, void *);
  void (*fini) (void);
};

int __libc_start_main (int argc, char **ubp_av,
		       char **ubp_ev,
		       void *auxvec, void (*rtld_fini) (void),
		       struct startup_info *stinfo,
		       char **stack_on_entry)
#endif
{
#if defined __ia64__ || defined __powerpc64__
  register void *r13 __asm ("r13") = thr_buf + 32768;
  __asm ("" : : "r" (r13));
#elif defined __sparc__
  register void *g6 __asm ("g6") = thr_buf + 32768;
# ifdef __arch64__
  __thread_self = thr_buf + 32768;
# else
  register void *__thread_self __asm ("g7") = thr_buf + 32768;
# endif
  __asm ("" : : "r" (g6), "r" (__thread_self));
#elif defined __s390__ && !defined __s390x__
  __asm ("sar %%a0,%0" : : "d" (thr_buf + 32768));
#elif defined __s390x__
  __asm ("sar %%a1,%0; srlg 0,%0,32; sar %%a0,0" : : "d" (thr_buf + 32768) : "0");
#elif defined __powerpc__ && !defined __powerpc64__
  register void *r2 __asm ("r2") = thr_buf + 32768;
  __asm ("" : : "r" (r2));
#endif
  main();
  return 0;
}
