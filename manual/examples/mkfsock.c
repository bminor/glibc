#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <errno.h>
#include <stdlib.h>

int 
make_named_socket (const char *filename)
{
  struct sockaddr_un name;
  int sock, status;
  size_t size;

  /* Create the socket.  */
  
  sock = socket (PF_UNIX, SOCK_DGRAM, 0);
  if (sock < 0)
    {
      perror ("socket");
      exit (EXIT_FAILURE);
    }

  /* Bind a name to the socket.  */

  name.sun_family = AF_FILE;
  strcpy (name.sun_path, filename);
  size = offsetof (name.sun_path) + strlen (name.sun_path) + 1;
  status = bind (sock, (struct sockaddr *) &name, size);
  if (status < 0)
    {
      perror ("bind");
      exit (EXIT_FAILURE);
    }

  return sock;
}
