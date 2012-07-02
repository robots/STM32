#ifndef PLATFORM_h_
#define PLATFORM_h_


#define LIKELY(x)               __builtin_expect(!!(x), 1)
#define UNLIKELY(x)             __builtin_expect(!!(x), 0)
#define ARRAY_SIZE(x) ((sizeof(x)/sizeof(x[0])))
#define BV(x) (1 << (x))

#ifndef NULL
#define NULL 0
#endif

#endif
