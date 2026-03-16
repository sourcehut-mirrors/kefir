// This file is a modified excerpt from glibc 2.42 sys/cdefs.h header.
//
// Refer to <https://sourceware.org/glibc/> website for more information,
// license, etc.
//
// Kefir author makes no claims with respect to this file, and provides it only
// for successful compilation of an external test

// clang-format off
#include_next <sys/cdefs.h>

# define __REDIRECT(name, proto, alias) name proto __asm__ (__ASMNAME (#alias))
# ifdef __cplusplus
#  define __REDIRECT_NTH(name, proto, alias) \
     name proto __THROW __asm__ (__ASMNAME (#alias))
#  define __REDIRECT_NTHNL(name, proto, alias) \
     name proto __THROWNL __asm__ (__ASMNAME (#alias))
# else
#  define __REDIRECT_NTH(name, proto, alias) \
     name proto __asm__ (__ASMNAME (#alias)) __THROW
#  define __REDIRECT_NTHNL(name, proto, alias) \
     name proto __asm__ (__ASMNAME (#alias)) __THROWNL
# endif
# define __ASMNAME(cname)  __ASMNAME2 (__USER_LABEL_PREFIX__, cname)
# define __ASMNAME2(prefix, cname) __STRING (prefix) cname

#ifndef __REDIRECT_FORTIFY
#define __REDIRECT_FORTIFY __REDIRECT
#endif

#ifndef __REDIRECT_FORTIFY_NTH
#define __REDIRECT_FORTIFY_NTH __REDIRECT_NTH
#endif
// clang-format on
