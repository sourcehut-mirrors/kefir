#define NO_TRAMPOLINES

// Missing built-ins
#define __builtin_memset(p, v, n) memset(p, v, n)
#define __builtin_memcpy(d, s, n) memcpy(d, s, n)
#define __builtin_memcmp(d, s, n) memcmp(d, s, n)
#define __builtin_memmove(d, s, n) memmove(d, s, n)
#define __builtin_strcpy(d, s) strcpy(d, s)
#define __builtin_strncpy(d, s, n) strncpy(d, s, n)
#define __builtin_strcmp(d, s) strcmp(d, s)
#define __builtin_strlen(s) strlen(s)
#define __builtin_signbit(n) signbit(n)
#define __builtin_abort(...) abort(__VA_ARGS__)
#define __builtin_printf printf
#define __builtin_sprintf sprintf
#define __builtin_longjmp(env, status) longjmp(env, status)
#define __builtin_setjmp(env) setjmp(env, status)
#define __builtin_exit(...) exit(__VA_ARGS__)
#define __builtin_abs(n) abs(n)
#define __builtin_malloc(...) malloc(__VA_ARGS__)

// Missing declarations
void *malloc(__SIZE_TYPE__);
void *memset(void *, int, __SIZE_TYPE__);
void *memcpy(void *, const void *, __SIZE_TYPE__);
void *memmove(void *, const void *, __SIZE_TYPE__);