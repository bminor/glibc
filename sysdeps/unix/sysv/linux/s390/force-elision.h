/* Automatic enabling of elision for mutexes
   Copyright (C) 2014-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* Automatically enable elision for existing user lock kinds.  */
#define FORCE_ELISION(m, s)						\
  if (__pthread_force_elision)						\
    {									\
      /* See concurrency notes regarding __kind in			\
	 struct __pthread_mutex_s in					\
	 sysdeps/nptl/bits/thread-shared-types.h.			\
									\
	 There are the following cases for the kind of a mutex		\
	 (The mask PTHREAD_MUTEX_ELISION_FLAGS_NP covers the flags	\
	 PTHREAD_MUTEX_ELISION_NP and PTHREAD_MUTEX_NO_ELISION_NP where	\
	 only one of both flags can be set):				\
	 - both flags are not set:					\
	 This is the first lock operation for this mutex.  Enable	\
	 elision as it is not enabled so far.				\
	 Note: It can happen that multiple threads are calling e.g.	\
	 pthread_mutex_lock at the same time as the first lock		\
	 operation for this mutex.  Then elision is enabled for this	\
	 mutex by multiple threads.  Storing with relaxed MO is enough	\
	 as all threads will store the same new value for the kind of	\
	 the mutex.  But we have to ensure that we always use the	\
	 elision path regardless if this thread has enabled elision or	\
	 another one.							\
									\
	 - PTHREAD_MUTEX_ELISION_NP flag is set:			\
	 Elision was already enabled for this mutex by a previous lock	\
	 operation.  See case above.  Just use the elision path.	\
									\
	 - PTHREAD_MUTEX_NO_ELISION_NP flag is set:			\
	 Elision was explicitly disabled by pthread_mutexattr_settype.	\
	 Do not use the elision path.					\
	 Note: The flag PTHREAD_MUTEX_NO_ELISION_NP will never be	\
	 changed after mutex initialization.  */			\
      int mutex_kind = atomic_load_relaxed (&((m)->__data.__kind));	\
      if ((mutex_kind & PTHREAD_MUTEX_ELISION_FLAGS_NP) == 0)		\
	{								\
	  mutex_kind |= PTHREAD_MUTEX_ELISION_NP;			\
	  atomic_store_relaxed (&((m)->__data.__kind), mutex_kind);	\
	}								\
      if ((mutex_kind & PTHREAD_MUTEX_ELISION_NP) != 0)			\
	{								\
	  s;								\
	}								\
    }
