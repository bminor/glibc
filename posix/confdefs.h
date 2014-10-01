#ifndef __CONFDEFS_H__
#define __CONFDEFS_H__

#include <posix/confdefs-defs.h>

#define CONF_DEF_UNDEFINED 1
#define CONF_DEF_DEFINED_SET 2
#define CONF_DEF_DEFINED_UNSET 3

#define CONF_IS_DEFINED_SET(conf) (conf##_DEF == CONF_DEF_DEFINED_SET)
#define CONF_IS_DEFINED_UNSET(conf) (conf##_DEF == CONF_DEF_DEFINED_UNSET)
#define CONF_IS_UNDEFINED(conf) (conf##_DEF == CONF_DEF_UNDEFINED)
#define CONF_IS_DEFINED(conf) (conf##_DEF != CONF_DEF_UNDEFINED)

#endif
