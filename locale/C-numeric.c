#include <ansidecl.h>
#include <localeinfo.h>
#include <stddef.h>


CONST struct numeric_info __numeric_C =
  {
    (char *) ".", (char *) "",
    (char *) "\177"
  };

CONST struct numeric_info *_numeric_info = &__numeric_C;
