#include_next <stdbool.h>

#ifdef bool
#undef bool
#define bool _Bool
#endif
