#define NO_TRAMPOLINES

// Missing constants
#define __INT_MAX__ 0x7FFFFFFF
#define __LONG_MAX__ 0x7FFFFFFFFFFFFFFFl
#define __LONG_LONG_MAX__ 0x7FFFFFFFFFFFFFFFl
#define __CHAR_BIT__ 8
#define __FLT_MIN__    \
    (((union {         \
         long l;       \
         float f;      \
     }){.l = 8388608}) \
         .f)
#define __FLT_MAX__       \
    (((union {            \
         long l;          \
         float f;         \
     }){.l = 2139095039}) \
         .f)
#define __SIZEOF_SHORT__ 2
#define __SIZEOF_INT__ 4
#define __SIZEOF_LONG__ 8
#define __SIZEOF_LONG_LONG__ 8
#define __SIZEOF_POINTER__ 8
#define __SIZEOF_SIZE_T__ 8
#define __SIZEOF_PTRDIFF_T__ 8

// Missing types
#define __label__ void *

// Missing built-ins
#define __builtin_memset(p, v, n) memset(p, v, n)
#define __builtin_memcpy(d, s, n) memcpy(d, s, n)
#define __builtin_memcmp(d, s, n) memcmp(d, s, n)
#define __builtin_strcpy(d, s) strcpy(d, s)
#define __builtin_strncpy(d, s, n) strncpy(d, s, n)
#define __builtin_strcmp(d, s) strcmp(d, s)
#define __builtin_strlen(s) strlen(s)
#define __builtin_signbit(n) signbit(n)
#define __builtin_abort() abort()
#define __builtin_printf printf
#define __builtin_sprintf sprintf
#define __builtin_longjmp(env, status) longjmp(env, status)
#define __builtin_setjmp(env) setjmp(env, status)
#define __builtin_exit() exit()
#define __builtin_abs(n) abs(n)

// Missing declarations
void *malloc(__SIZE_TYPE__);
void *memset(void *, int, __SIZE_TYPE__);