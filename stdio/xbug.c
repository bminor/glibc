#include <stdio.h>

main()
{
    char * filename = "xbug.c";
    FILE *input;
    int i;
    char buf[256];


    if (!freopen (filename, "r", stdin))
        fprintf(stderr, "cannot open file\n");

    if (!(input = popen("/bin/cat", "r")))
        fprintf(stderr, "cannot run \n");


    while(gets(buf) != NULL) {
	fprintf(stdout, "read: %s\n", buf);
    }
}
