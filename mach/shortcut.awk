# Icky intimate knowledge of MiG output.

BEGIN { args=""; argsnext=0; echo=1; }

$NF == rpc \
  {
    for (i = 1; i < NF; ++i) printf "%s ", $i;
    print call;
    next;
  }

args == "" && $1 == "#else" { argsnext=1; print $0; next; }

argsnext == 1 { args=$0; }

{ argsnext=0; }

/^{/ { echo=0; }

echo == 1 { print $0; }

/^}/ \
  {
    print "{";
    print "  kern_return_t err;";
    print "  err = " syscall args ";";
    print "  if (err == MACH_SEND_INTERRUPTED)";
    print "    err = " rpc args ";";
    print "  return err;"
    print "}";
    echo = 1;
  }
