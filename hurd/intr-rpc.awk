# Icky intimate knowledge of MiG output.

BEGIN { args=""; argsnext=0; echo=1; print "#include <hurd.h>"; }

$NF == rpc \
  {
    for (i = 1; i < NF; ++i) printf "%s ", $i;
    print call;
    next;
  }

args == "" && $1 == "#else" { argsnext=1; print $0; next; }

argsnext == 1 { args=$0; firstarg=substr($1, 2, length($1)-2); }

{ argsnext=0; }

/^{/ { echo=0; }

echo == 1 { print $0; }

/^}/ \
  {
    print "{";
    print "  return HURD_EINTR_RPC (" firstarg ", " rpc args ");";
    print "}";
    echo = 1;
  }
