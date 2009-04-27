#define _GNU_SOURCE
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../locale/hashval.h"
#define __LC_LAST 13
#include "../locale/locarchive.h"
#include "../crypt/md5.h"

const char *alias_file = DATADIR "/locale/locale.alias";
const char *locar_file = PREFIX "/lib/locale/locale-archive";
const char *tmpl_file = PREFIX "/lib/locale/locale-archive.tmpl";
const char *loc_path = PREFIX "/lib/locale/";
int be_quiet = 1;
int verbose = 0;
int max_locarchive_open_retry = 10;
const char *output_prefix;

static const char *locnames[] =
  {
#define DEFINE_CATEGORY(category, category_name, items, a) \
  [category] = category_name,
#include "../locale/categories.def"
#undef  DEFINE_CATEGORY
  };

static int
is_prime (unsigned long candidate)
{
  /* No even number and none less than 10 will be passed here.  */
  unsigned long int divn = 3;
  unsigned long int sq = divn * divn;

  while (sq < candidate && candidate % divn != 0)
    {
      ++divn;
      sq += 4 * divn;
      ++divn;
    }

  return candidate % divn != 0;
}

unsigned long
next_prime (unsigned long seed)
{
  /* Make it definitely odd.  */
  seed |= 1;

  while (!is_prime (seed))
    seed += 2;

  return seed;
}

void
error (int status, int errnum, const char *message, ...)
{
  va_list args;

  va_start (args, message);
  fflush (stdout);
  fprintf (stderr, "%s: ", program_invocation_name);
  vfprintf (stderr, message, args);
  va_end (args);
  if (errnum)
    fprintf (stderr, ": %s", strerror (errnum));
  putc ('\n', stderr);
  fflush (stderr);
  if (status)
    exit (errnum == EROFS ? 0 : status);
}

void *
xmalloc (size_t size)
{
  void *p = malloc (size);
  if (p == NULL)
    error (EXIT_FAILURE, errno, "could not allocate %zd bytes of memory", size);
  return p;
}

static void
open_tmpl_archive (struct locarhandle *ah)
{
  struct stat64 st;
  int fd;
  struct locarhead head;
  const char *archivefname = tmpl_file;

  /* Open the archive.  We must have exclusive write access.  */
  fd = open64 (archivefname, O_RDONLY);
  if (fd == -1)
    error (EXIT_FAILURE, errno, "cannot open locale archive template file \"%s\"",
	   archivefname);

  if (fstat64 (fd, &st) < 0)
    error (EXIT_FAILURE, errno, "cannot stat locale archive template file \"%s\"",
	   archivefname);

  /* Read the header.  */
  if (TEMP_FAILURE_RETRY (read (fd, &head, sizeof (head))) != sizeof (head))
    error (EXIT_FAILURE, errno, "cannot read archive header");

  ah->fd = fd;
  ah->mmaped = (head.sumhash_offset
		+ head.sumhash_size * sizeof (struct sumhashent));
  if (ah->mmaped > (unsigned long) st.st_size)
    error (EXIT_FAILURE, 0, "locale archite template file truncated");
  ah->mmaped = st.st_size;
  ah->reserved = st.st_size;

  /* Now we know how large the administrative information part is.
     Map all of it.  */
  ah->addr = mmap64 (NULL, ah->mmaped, PROT_READ, MAP_SHARED, fd, 0);
  if (ah->addr == MAP_FAILED)
    error (EXIT_FAILURE, errno, "cannot map archive header");
}

/* Open the locale archive.  */
extern void open_archive (struct locarhandle *ah, bool readonly);

/* Close the locale archive.  */
extern void close_archive (struct locarhandle *ah);

/* Add given locale data to the archive.  */
extern int add_locale_to_archive (struct locarhandle *ah, const char *name,
				  locale_data_t data, bool replace);

extern void add_alias (struct locarhandle *ah, const char *alias,
		       bool replace, const char *oldname,
		       uint32_t *locrec_offset_p);

extern struct namehashent *
insert_name (struct locarhandle *ah,
	     const char *name, size_t name_len, bool replace);

struct nameent
{
  char *name;
  struct locrecent *locrec;
};

struct dataent
{
  const unsigned char *sum;
  uint32_t file_offset;
};

static int
nameentcmp (const void *a, const void *b)
{
  struct locrecent *la = ((const struct nameent *) a)->locrec;
  struct locrecent *lb = ((const struct nameent *) b)->locrec;
  uint32_t start_a = -1, end_a = 0;
  uint32_t start_b = -1, end_b = 0;
  int cnt;

  for (cnt = 0; cnt < __LC_LAST; ++cnt)
    if (cnt != LC_ALL)
      {
	if (la->record[cnt].offset < start_a)
	  start_a = la->record[cnt].offset;
	if (la->record[cnt].offset + la->record[cnt].len > end_a)
	  end_a = la->record[cnt].offset + la->record[cnt].len;
      }
  assert (start_a != (uint32_t)-1);
  assert (end_a != 0);

  for (cnt = 0; cnt < __LC_LAST; ++cnt)
    if (cnt != LC_ALL)
      {
	if (lb->record[cnt].offset < start_b)
	  start_b = lb->record[cnt].offset;
	if (lb->record[cnt].offset + lb->record[cnt].len > end_b)
	  end_b = lb->record[cnt].offset + lb->record[cnt].len;
      }
  assert (start_b != (uint32_t)-1);
  assert (end_b != 0);

  if (start_a != start_b)
    return (int)start_a - (int)start_b;
  return (int)end_a - (int)end_b;
}

static int
dataentcmp (const void *a, const void *b)
{
  if (((const struct dataent *) a)->file_offset
      < ((const struct dataent *) b)->file_offset)
    return -1;

  if (((const struct dataent *) a)->file_offset
      > ((const struct dataent *) b)->file_offset)
    return 1;

  return 0;
}

static int
sumsearchfn (const void *key, const void *ent)
{
  uint32_t keyn = *(uint32_t *)key;
  uint32_t entn = ((struct dataent *)ent)->file_offset;

  if (keyn < entn)
    return -1;
  if (keyn > entn)
    return 1;
  return 0;
}

static void
compute_data (struct locarhandle *ah, struct nameent *name, size_t sumused,
	      struct dataent *files, locale_data_t data)
{
  int cnt;
  struct locrecent *locrec = name->locrec;
  struct dataent *file;
  data[LC_ALL].addr = ((char *) ah->addr) + locrec->record[LC_ALL].offset;
  data[LC_ALL].size = locrec->record[LC_ALL].len;
  for (cnt = 0; cnt < __LC_LAST; ++cnt)
    if (cnt != LC_ALL)
      {
	data[cnt].addr = ((char *) ah->addr) + locrec->record[cnt].offset;
	data[cnt].size = locrec->record[cnt].len;
	if (data[cnt].addr >= data[LC_ALL].addr
	    && data[cnt].addr + data[cnt].size
	       <= data[LC_ALL].addr + data[LC_ALL].size)
	  __md5_buffer (data[cnt].addr, data[cnt].size, data[cnt].sum);
	else
	  {
	    file = bsearch (&locrec->record[cnt].offset, files, sumused,
			    sizeof (*files), sumsearchfn);
	    if (file == NULL)
	      error (EXIT_FAILURE, 0, "inconsistent template file");
	    memcpy (data[cnt].sum, file->sum, sizeof (data[cnt].sum));
	  }
      }
}

static int
fill_archive (struct locarhandle *tmpl_ah, size_t nlist, char *list[],
	      const char *primary)
{
  struct locarhandle ah;
  struct locarhead *head;
  int result = 0;
  struct nameent *names;
  struct namehashent *namehashtab;
  size_t cnt, used;
  struct dataent *files;
  struct sumhashent *sumhashtab;
  size_t sumused;
  struct locrecent *primary_locrec = NULL;
  struct nameent *primary_nameent = NULL;

  head = tmpl_ah->addr;
  names = (struct nameent *) malloc (head->namehash_used
				     * sizeof (struct nameent));
  files = (struct dataent *) malloc (head->sumhash_used
				     * sizeof (struct dataent));
  if (names == NULL || files == NULL)
    error (EXIT_FAILURE, errno, "could not allocate tables");

  namehashtab = (struct namehashent *) ((char *) tmpl_ah->addr
					+ head->namehash_offset);
  sumhashtab = (struct sumhashent *) ((char *) tmpl_ah->addr
				      + head->sumhash_offset);

  for (cnt = used = 0; cnt < head->namehash_size; ++cnt)
    if (namehashtab[cnt].locrec_offset != 0)
      {
	assert (used < head->namehash_used);
	names[used].name = tmpl_ah->addr + namehashtab[cnt].name_offset;
	names[used++].locrec
	  = (struct locrecent *) ((char *) tmpl_ah->addr +
				  namehashtab[cnt].locrec_offset);
      }

  /* Sort the names.  */
  qsort (names, used, sizeof (struct nameent), nameentcmp);

  for (cnt = sumused = 0; cnt < head->sumhash_size; ++cnt)
    if (sumhashtab[cnt].file_offset != 0)
      {
	assert (sumused < head->sumhash_used);
	files[sumused].sum = (const unsigned char *) sumhashtab[cnt].sum;
	files[sumused++].file_offset = sumhashtab[cnt].file_offset;
      }

  /* Sort by file locations.  */
  qsort (files, sumused, sizeof (struct dataent), dataentcmp);

  /* Open the archive.  This call never returns if we cannot
     successfully open the archive.  */
  open_archive (&ah, false);

  if (primary != NULL)
    {
      for (cnt = 0; cnt < used; ++cnt)
	if (strcmp (names[cnt].name, primary) == 0)
	  break;
      if (cnt < used)
	{
	  locale_data_t data;

	  compute_data (tmpl_ah, &names[cnt], sumused, files, data);
	  result |= add_locale_to_archive (&ah, primary, data, 0);
	  primary_locrec = names[cnt].locrec;
	  primary_nameent = &names[cnt];
	}
    }

  for (cnt = 0; cnt < used; ++cnt)
    if (&names[cnt] == primary_nameent)
      continue;
    else if ((cnt > 0 && names[cnt - 1].locrec == names[cnt].locrec)
	     || names[cnt].locrec == primary_locrec)
      {
	const char *oldname;
	struct namehashent *namehashent;
	uint32_t locrec_offset;

	if (names[cnt].locrec == primary_locrec)
	  oldname = primary;
	else
	  oldname = names[cnt - 1].name;
	namehashent = insert_name (&ah, oldname, strlen (oldname), true);
	assert (namehashent->name_offset != 0);
	assert (namehashent->locrec_offset != 0);
	locrec_offset = namehashent->locrec_offset;
	add_alias (&ah, names[cnt].name, 0, oldname, &locrec_offset);
      }
    else
      {
	locale_data_t data;

	compute_data (tmpl_ah, &names[cnt], sumused, files, data);
	result |= add_locale_to_archive (&ah, names[cnt].name, data, 0);
      }

  while (nlist-- > 0)
    {
      const char *fname = *list++;
      size_t fnamelen = strlen (fname);
      struct stat64 st;
      DIR *dirp;
      struct dirent64 *d;
      int seen;
      locale_data_t data;
      int cnt;

      /* First see whether this really is a directory and whether it
	 contains all the require locale category files.  */
      if (stat64 (fname, &st) < 0)
	{
	  error (0, 0, "stat of \"%s\" failed: %s: ignored", fname,
		 strerror (errno));
	  continue;
	}
      if (!S_ISDIR (st.st_mode))
	{
	  error (0, 0, "\"%s\" is no directory; ignored", fname);
	  continue;
	}

      dirp = opendir (fname);
      if (dirp == NULL)
	{
	  error (0, 0, "cannot open directory \"%s\": %s: ignored",
		 fname, strerror (errno));
	  continue;
	}

      seen = 0;
      while ((d = readdir64 (dirp)) != NULL)
	{
	  for (cnt = 0; cnt < __LC_LAST; ++cnt)
	    if (cnt != LC_ALL)
	      if (strcmp (d->d_name, locnames[cnt]) == 0)
		{
		  unsigned char d_type;

		  /* We have an object of the required name.  If it's
		     a directory we have to look at a file with the
		     prefix "SYS_".  Otherwise we have found what we
		     are looking for.  */
#ifdef _DIRENT_HAVE_D_TYPE
		  d_type = d->d_type;

		  if (d_type != DT_REG)
#endif
		    {
		      char fullname[fnamelen + 2 * strlen (d->d_name) + 7];

#ifdef _DIRENT_HAVE_D_TYPE
		      if (d_type == DT_UNKNOWN)
#endif
			{
			  strcpy (stpcpy (stpcpy (fullname, fname), "/"),
				  d->d_name);

			  if (stat64 (fullname, &st) == -1)
			    /* We cannot stat the file, ignore it.  */
			    break;

			  d_type = IFTODT (st.st_mode);
			}

		      if (d_type == DT_DIR)
			{
			  /* We have to do more tests.  The file is a
			     directory and it therefore must contain a
			     regular file with the same name except a
			     "SYS_" prefix.  */
			  char *t = stpcpy (stpcpy (fullname, fname), "/");
			  strcpy (stpcpy (stpcpy (t, d->d_name), "/SYS_"),
				  d->d_name);

			  if (stat64 (fullname, &st) == -1)
			    /* There is no SYS_* file or we cannot
			       access it.  */
			    break;

			  d_type = IFTODT (st.st_mode);
			}
		    }

		  /* If we found a regular file (eventually after
		     following a symlink) we are successful.  */
		  if (d_type == DT_REG)
		    ++seen;
		  break;
		}
	}

      closedir (dirp);

      if (seen != __LC_LAST - 1)
	{
	  /* We don't have all locale category files.  Ignore the name.  */
	  error (0, 0, "incomplete set of locale files in \"%s\"",
		 fname);
	  continue;
	}

      /* Add the files to the archive.  To do this we first compute
	 sizes and the MD5 sums of all the files.  */
      for (cnt = 0; cnt < __LC_LAST; ++cnt)
	if (cnt != LC_ALL)
	  {
	    char fullname[fnamelen + 2 * strlen (locnames[cnt]) + 7];
	    int fd;

	    strcpy (stpcpy (stpcpy (fullname, fname), "/"), locnames[cnt]);
	    fd = open64 (fullname, O_RDONLY);
	    if (fd == -1 || fstat64 (fd, &st) == -1)
	      {
		/* Cannot read the file.  */
		if (fd != -1)
		  close (fd);
		break;
	      }

	    if (S_ISDIR (st.st_mode))
	      {
		char *t;
		close (fd);
		t = stpcpy (stpcpy (fullname, fname), "/");
		strcpy (stpcpy (stpcpy (t, locnames[cnt]), "/SYS_"),
			locnames[cnt]);

		fd = open64 (fullname, O_RDONLY);
		if (fd == -1 || fstat64 (fd, &st) == -1
		    || !S_ISREG (st.st_mode))
		  {
		    if (fd != -1)
		      close (fd);
		    break;
		  }
	      }

	    /* Map the file.  */
	    data[cnt].addr = mmap64 (NULL, st.st_size, PROT_READ, MAP_SHARED,
				     fd, 0);
	    if (data[cnt].addr == MAP_FAILED)
	      {
		/* Cannot map it.  */
		close (fd);
		break;
	      }

	    data[cnt].size = st.st_size;
	    __md5_buffer (data[cnt].addr, st.st_size, data[cnt].sum);

	    /* We don't need the file descriptor anymore.  */
	    close (fd);
	  }

      if (cnt != __LC_LAST)
	{
	  while (cnt-- > 0)
	    if (cnt != LC_ALL)
	      munmap (data[cnt].addr, data[cnt].size);

	  error (0, 0, "cannot read all files in \"%s\": ignored", fname);

	  continue;
	}

      result |= add_locale_to_archive (&ah, basename (fname), data, 0);

      for (cnt = 0; cnt < __LC_LAST; ++cnt)
	if (cnt != LC_ALL)
	  munmap (data[cnt].addr, data[cnt].size);
    }

  /* We are done.  */
  close_archive (&ah);

  return result;
}

int main ()
{
  char path[4096];
  DIR *dirp;
  struct dirent64 *d;
  struct stat64 st;
  char *list[16384], *primary;
  unsigned int cnt = 0;
  struct locarhandle tmpl_ah;
  size_t loc_path_len = strlen (loc_path);

  dirp = opendir (loc_path);
  if (dirp == NULL)
    error (EXIT_FAILURE, errno, "cannot open directory \"%s\"", loc_path);

  open_tmpl_archive (&tmpl_ah);

  unlink (locar_file);
  primary = getenv ("LC_ALL");
  if (primary == NULL)
    primary = getenv ("LANG");
  if (primary != NULL)
    {
      if (strncmp (primary, "ja", 2) != 0
	  && strncmp (primary, "ko", 2) != 0
	  && strncmp (primary, "zh", 2) != 0)
	{
	  char *ptr = malloc (strlen (primary) + strlen (".utf8") + 1), *p, *q;

	  if (ptr)
	    {
	      p = ptr;
	      q = primary;
	      while (*q && *q != '.' && *q != '@')
		*p++ = *q++;
	      if (*q == '.')
		while (*q && *q != '@')
		  q++;
	      p = stpcpy (p, ".utf8");
	      strcpy (p, q);
	      primary = ptr;
	    }
	  else
	    primary = NULL;
	}
    }

  memcpy (path, loc_path, loc_path_len);

  while ((d = readdir64 (dirp)) != NULL)
    {
      if (strcmp (d->d_name, ".") == 0 || strcmp (d->d_name, "..") == 0)
	continue;

      size_t d_name_len = strlen (d->d_name);
      if (loc_path_len + d_name_len + 1 > sizeof (path))
	{
	  error (0, 0, "too long filename \"%s\"", d->d_name);
	  continue;
	}

      memcpy (path + loc_path_len, d->d_name, d_name_len + 1);
      if (stat64 (path, &st) < 0)
	{
	  error (0, errno, "cannot stat \"%s\"", path);
	  continue;
	}
      if (! S_ISDIR (st.st_mode))
	continue;
      if (cnt == 16384)
	error (EXIT_FAILURE, 0, "too many directories in \"%s\"", loc_path);
      list[cnt] = strdup (path);
      if (list[cnt] == NULL)
	{
	  error (0, errno, "cannot add file to list \"%s\"", path);
	  continue;
	}
      if (primary != NULL && cnt > 0 && strcmp (primary, d->d_name) == 0)
	{
	  char *p = list[0];
	  list[0] = list[cnt];
	  list[cnt] = p;
	}
      cnt++;
    }
  closedir (dirp);
  fill_archive (&tmpl_ah, cnt, list, primary);
  close_archive (&tmpl_ah);
  truncate (tmpl_file, 0);
  char *argv[] = { "/usr/sbin/tzdata-update", NULL };
  execve (argv[0], (char *const *)argv, (char *const *)&argv[1]);
  exit (0);
}
