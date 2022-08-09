#define __PTRDIFF_TYPE__ long
#define __SIZE_TYPE__ unsigned long
#define __WCHAR_TYPE__ int
#define __WINT_TYPE__ unsigned int
#define __builtin_constant_p(x) 0
#define __builtin_memcpy(d, s, c) memcpy((d), (s), (c))
void *memcpy(void *__restrict, const void *__restrict, __SIZE_TYPE__);