BEGIN {
  FS=" \t[";
  print "#include <mach/error.h>\n#include <errorlib.h>";
  print "#define static static const"
  nsubs = split(subsys, subs);
  for (sub in subs) printf "#includes \"%s\"\n", sub;
  print "\n\n\
static const struct error_system __mach_error_systems[err_max_system + 1] =";
  print "  {";
}
{
  for (i = 1; i <= NF; ++i)
    if ($i ~ /err_[a-z0-9A-Z_]+_sub/) {
      sub = substr ($i, 0, length ($i) - 4);
      printf "    [err_get_system (%s)] = { errlib_count (%s),\n", $i, sub;
      printf "    \"(system %s) error with unknown subsystem\", %s },\n", 
             sub, $i;
      break;
    }
}
END {
  print "  };";
  printf "\n\
const int __mach_error_system_count = errlib_count (__mach_error_systems);";
}
