// This file is a modified excerpt from glibc 2.42 sys/cdefs.h header.
//
// Refer to <https://sourceware.org/glibc/> website for more information,
// license, etc.
//
// Kefir author makes no claims with respect to this file, and provides it only
// for successful compilation of an external test

// clang-format off
#include_next <sys/cdefs.h>

#  define __REDIRECT_NTH(name, proto, alias) \
     name proto __asm__ (__ASMNAME (#alias)) __THROW
#  define __REDIRECT_NTHNL(name, proto, alias) \
     name proto __asm__ (__ASMNAME (#alias)) __THROWNL
# define __ASMNAME(cname)  __ASMNAME2 (__USER_LABEL_PREFIX__, cname)
# define __ASMNAME2(prefix, cname) __STRING (prefix) cname

// clang-format on
