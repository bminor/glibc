#include <stdio.h>
#include <stdlib.h>

#include "tst-strtod.h"

#define TEST_STRTOD(FSUF, FTYPE, FTOSTR, LSUF, CSUF)			\
struct test_strto ## FSUF						\
{									\
  const char *str;							\
  FTYPE result;								\
  size_t offset;							\
} tests_strto ## FSUF[] =						\
{									\
  { "0xy", 0.0 ## LSUF, 1 },						\
  { "0x.y", 0.0 ## LSUF, 1 },						\
  { "0x0.y", 0.0 ## LSUF, 4 },						\
  { "0x.0y", 0.0 ## LSUF, 4 },						\
  { ".y", 0.0 ## LSUF, 0 },						\
  { "0.y", 0.0 ## LSUF, 2 },						\
  { ".0y", 0.0 ## LSUF, 2 }						\
};									\
									\
static int								\
test_strto ## FSUF (void)						\
{									\
  int status = 0;							\
  for (size_t i = 0;							\
       i < sizeof (tests_strto ## FSUF) / sizeof (tests_strto ## FSUF[0]); \
       ++i)								\
    {									\
      char *ep;								\
      FTYPE r = strto ## FSUF (tests_strto ## FSUF[i].str, &ep);	\
      if (r != tests_strto ## FSUF[i].result)				\
	{								\
	  char buf1[FSTRLENMAX], buf2[FSTRLENMAX];			\
	  FTOSTR (buf1, sizeof (buf1), "%g", r);			\
	  FTOSTR (buf2, sizeof (buf2), "%g", tests_strto ## FSUF[i].result); \
	  printf ("test %zu r = %s, expect %s\n", i, buf1, buf2);	\
	  status = 1;							\
	}								\
      if (ep != tests_strto ## FSUF[i].str + tests_strto ## FSUF[i].offset) \
	{								\
	  printf ("test %zu strto" #FSUF				\
		  " parsed %tu characters, expected %zu\n",		\
		  i, ep - tests_strto ## FSUF[i].str,			\
		  tests_strto ## FSUF[i].offset);			\
	  status = 1;							\
	}								\
    }									\
  return status;							\
}

GEN_TEST_STRTOD_FOREACH (TEST_STRTOD)

static int
do_test (void)
{
  return STRTOD_TEST_FOREACH (test_strto);
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
