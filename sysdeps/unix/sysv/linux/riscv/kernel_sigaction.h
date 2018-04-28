/* This is the sigaction structure from the RISC-V Linux 4.15 kernel.  */

struct kernel_sigaction {
	__sighandler_t k_sa_handler;
	unsigned long sa_flags;
	sigset_t sa_mask;
};
