/* This file precedes db-symbols.awk before preprocessing when making
   rtld-db-symbols.v.i for the test.  We just set this macro here so
   that we get the list of ld.so symbols instead of libpthread ones.  */

%define IS_IN_rtld
