NACL_MANDATORY_INTERFACE (rtld,
                          NACL_IRT_BASIC_v0_1, nacl_irt_basic)
NACL_MANDATORY_INTERFACE (rtld,
                          NACL_IRT_FDIO_v0_1, nacl_irt_fdio)
NACL_MANDATORY_INTERFACE (rtld,
                          NACL_IRT_FILENAME_v0_1, nacl_irt_filename)
NACL_MANDATORY_INTERFACE (rtld,
                          NACL_IRT_MEMORY_v0_3, nacl_irt_memory)
NACL_MANDATORY_INTERFACE (libc,
                          NACL_IRT_THREAD_v0_1, nacl_irt_thread)
NACL_MANDATORY_INTERFACE (rtld,
                          NACL_IRT_FUTEX_v0_1, nacl_irt_futex)
NACL_MANDATORY_INTERFACE (rtld,
                          NACL_IRT_TLS_v0_1, nacl_irt_tls)
NACL_MANDATORY_INTERFACE (libc,
                          NACL_IRT_RESOURCE_OPEN_v0_1, nacl_irt_resource_open)
NACL_MANDATORY_INTERFACE (rtld,
                          NACL_IRT_CODE_DATA_ALLOC_v0_1,
                          nacl_irt_code_data_alloc)
NACL_OPTIONAL_INTERFACE (libc,
                         NACL_IRT_CLOCK_v0_1, nacl_irt_clock)
NACL_OPTIONAL_INTERFACE (rtld,
                         NACL_IRT_DYNCODE_v0_1, nacl_irt_dyncode)
NACL_OPTIONAL_INTERFACE (rtld,
                         NACL_IRT_DEV_GETPID_v0_1, nacl_irt_dev_getpid)
NACL_OPTIONAL_INTERFACE (rtld,
                         NACL_IRT_DEV_FILENAME_v0_3, nacl_irt_dev_filename)
