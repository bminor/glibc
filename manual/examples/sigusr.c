#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

/* When a @code{SIGUSR1} signal arrives, set this variable.  */

volatile sig_atomic_t usr_interrupt = 0;

void 
synch_signal (int sig)
{
  usr_interrupt = 1;
}


/* The child process executes this function. */

void 
child_function (void)
{
  /* Perform initialization. */
  printf ("I'm here!!!  My pid is %d.\n", (int) getpid ());

  /* Let parent know you're done. */
  kill (getppid (), SIGUSR1);

  /* Continue with execution. */
  printf ("Bye, now....\n");
  exit (0);
}


int
main (void)
{
  struct sigaction usr_action;
  sigset_t block_mask;
  pid_t child_id;

  /* Establish the signal handler. */
  sigfillset (&block_mask);
  usr_action.sa_handler = synch_signal;
  usr_action.sa_mask = block_mask;
  usr_action.sa_flags = 0;
  sigaction (SIGUSR1, &usr_action, NULL);

  /* Create the child process. */
  child_id = fork ();
  if (child_id == 0)
    child_function ();		/* Does not return */

  /* Busy wait for child to send a signal. */
  while (!usr_interrupt)
    ;

  /* Now continue execution. */
  printf ("That's all, folks!\n");
  return 0;
}
