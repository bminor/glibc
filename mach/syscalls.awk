BEGIN { calls="" }

{
  calls = calls " " $1;
  print "sysno-" $1 " = " $2;
}

END { print "mach-syscalls := " calls }
