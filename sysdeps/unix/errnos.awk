BEGIN { special = 0 }

/ERRNOS/ { nerrnos = split(errnos, errs)
	     for (i = 1; i <= nerrnos; ++i)
	       printf "  DO(\"%s\", %s);\n", errs[i], errs[i]
	     special = 1 }


{ if (special == 0) print $0; special = 0 }
