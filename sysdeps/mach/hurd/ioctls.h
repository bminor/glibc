/*
  ioctls are:
  1 bit set if it has input
  1 bit set if it has output
  2 bits of type[0]
  5 bits of count[0]
  2 bits of type[1]
  5 bits of count[1]
  2 bits of type[2]
  3 bits of count[2]
  4 bits of group
  7 bits of command
    */
union __ioctl
  {
    struct
      {
	enum
	  {
	    IOC_VOID = 0,	/* No parameters.  */
	    IOC_OUT = 1,	/* Copy in parameters.  */
	    IOC_IN = 2,		/* Copy out parameters.  */
	    IOC_INOUT = (IOC_IN|IOC_OUT)
	    } inout:2;
	enum { IOC_8, IOC_16, IOC_32 } type0:2, type1:2, type2:2;
	unsigned int count0:5, count1:5, count2:3;
	unsigned int group:4, command:7;
      } s;
    int i;
  };

#define	_IOC(inout, group, num, t0, c0, t1, c1, t2, c2)			      \
  (((union __ioctl)							      \
    { { (inout), (t0), (t1), (t2), (c0), (c1), (c2),			      \
	  ((group) - 'a') >> 2, (num) } }).__i)

