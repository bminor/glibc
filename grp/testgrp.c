#include <ansidecl.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

void main (void)
{
  uid_t me;
  struct passwd *my_passwd;
  struct group *my_group;
  char **members;

  me = getuid ();
  my_passwd = getpwuid (me);

  printf ("My login name is %s.\n", my_passwd->pw_name);
  printf ("My uid is %d.\n", (int)(my_passwd->pw_uid));
  printf ("My home directory is %s.\n", my_passwd->pw_dir);
  printf ("My default shell is %s.\n", my_passwd->pw_shell);

  my_group = getgrgid (my_passwd->pw_gid);
  if (!my_group) {
    printf ("Couldn't find out about group %d.\n", (int)(my_passwd->pw_gid));
    exit (EXIT_FAILURE);
    }

  printf ("My default group is %s (%d).\n",
	  my_group->gr_name, (int)(my_passwd->pw_gid));
  printf ("The members of this group are:\n");
  members = my_group->gr_mem;
  while (members) {
    printf ("  %s\n", *(members));
    members++;
    }
  exit (EXIT_SUCCESS);
}



