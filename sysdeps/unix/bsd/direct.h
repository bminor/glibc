#ifndef	MAXNAMLEN
#define	MAXNAMLEN	255
#endif

struct direct
  {
    unsigned long int d_fileno;
    unsigned short int d_reclen;
    unsigned short int d_namlen;
    char d_name[MAXNAMLEN + 1];
  };

#define D_NAMLEN(d) ((d)->d_namlen)
