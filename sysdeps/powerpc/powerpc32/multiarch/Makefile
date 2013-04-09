ifeq ($(subdir),string)
sysdep_routines += memcpy-power7 memcpy-a2 memcpy-power6 memcpy-cell \
		   memcmp-power7 memset-power4 memset-power6 memset-power7 \
		   bzero-power4 bzero-power6 bzero-power7 \
		   strncmp-power7 strncmp-power4 strlen-power7 \
                   strcasecmp-power7 strcasecmp_l-power7 \
                   strncase-power7 strncase_l-power7 \
		   strnlen-power7 strnlen-c \
		   strchr-power7 \
		   mempcpy-power7 mempcpy-c \
		   memchr-power7 memchr-c \
		   memrchr-power7 memrchr-c \
		   rawmemchr-power7 rawmemchr-c \
		   strchrnul-power7 strchrnul-c \
		   wordcopy-power7 wordcopy-power6 wordcopy-power5 \
		   wordcopy-power4 wordcopy-c \
		   wcschr-power7 wcschr-power6 wcschr-c \
		   wcsrchr-power7 wcsrchr-power6 wcsrchr-c \
		   wcscpy-power7 wcscpy-power6 wcscpy-c

CFLAGS-strncase-power7.c += -funroll-loops
CFLAGS-strncase_l-power7.c += -funroll-loops
endif
