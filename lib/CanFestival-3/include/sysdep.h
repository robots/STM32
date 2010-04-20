#ifndef __sysdep_h__
#define __sysdep_h__

#include "config.h"

#ifdef CANOPEN_BIG_ENDIAN

/* Warning: the argument must not update pointers, e.g. *p++ */

#define UNS16_LE(v)  ((((UNS16)(v) & 0xff00) >> 8) | \
		      (((UNS16)(v) & 0x00ff) << 8))

#define UNS32_LE(v)  ((((UNS32)(v) & 0xff000000) >> 24) |	\
		      (((UNS32)(v) & 0x00ff0000) >> 8)  |	\
		      (((UNS32)(v) & 0x0000ff00) << 8)  |	\
		      (((UNS32)(v) & 0x000000ff) << 24))

#else

#define UNS16_LE(v)  (v)

#define UNS32_LE(v)  (v)

#endif

#endif /* __sysdep_h__ */

