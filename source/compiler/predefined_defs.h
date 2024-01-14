/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Keywords
#define __FUNCTION__ __func__
#define __thread _Thread_local

// Type definitions
#ifdef __LP64__
#define __SIZE_TYPE__ long unsigned int
#define __PTRDIFF_TYPE__ long int
#define __WCHAR_TYPE__ int
#define __WINT_TYPE__ unsigned int
#define __INTMAX_TYPE__ long int
#define __UINTMAX_TYPE__ long unsigned int
#define __INT8_TYPE__ signed char
#define __INT16_TYPE__ short
#define __INT32_TYPE__ int
#define __INT64_TYPE__ long int
#define __UINT8_TYPE__ unsigned char
#define __UINT16_TYPE__ unsigned short
#define __UINT32_TYPE__ unsigned int
#define __UINT64_TYPE__ long unsigned int
#define __INT_LEAST8_TYPE__ signed char
#define __INT_LEAST16_TYPE__ short
#define __INT_LEAST32_TYPE__ int
#define __INT_LEAST64_TYPE__ long int
#define __UINT_LEAST8_TYPE__ unsigned char
#define __UINT_LEAST16_TYPE__ unsigned short
#define __UINT_LEAST32_TYPE__ unsigned int
#define __UINT_LEAST64_TYPE__ long unsigned int
#define __INT_FAST8_TYPE__ signed char
#define __INT_FAST16_TYPE__ long int
#define __INT_FAST32_TYPE__ long int
#define __INT_FAST64_TYPE__ long int
#define __UINT_FAST8_TYPE__ unsigned char
#define __UINT_FAST16_TYPE__ long unsigned int
#define __UINT_FAST32_TYPE__ long unsigned int
#define __UINT_FAST64_TYPE__ long unsigned int
#define __INTPTR_TYPE__ long int
#define __UINTPTR_TYPE__ long unsigned int
#endif

// Runtime functions
extern _Noreturn void __kefirrt_trap(void);
extern void *__kefirrt_return_address(int);
extern void *__kefirrt_frame_address(int);

extern int __kefirrt_ffs(int);
extern int __kefirrt_clz(unsigned int);
extern int __kefirrt_ctz(unsigned int);
extern int __kefirrt_clrsb(int);
extern int __kefirrt_popcount(unsigned int);
extern int __kefirrt_parity(int);

extern unsigned short __kefirrt_bswap16(unsigned short);
extern unsigned int __kefirrt_bswap32(unsigned int);
extern unsigned long __kefirrt_bswap64(unsigned long);

extern int __kefirrt_ffsl(long);
extern int __kefirrt_clzl(unsigned long);
extern int __kefirrt_ctzl(unsigned long);
extern int __kefirrt_clrsbl(long);
extern int __kefirrt_popcountl(unsigned long);
extern int __kefirrt_parityl(long);

extern int __kefirrt_ffsll(long);
extern int __kefirrt_clzll(unsigned long);
extern int __kefirrt_ctzll(unsigned long);
extern int __kefirrt_clrsbll(long);
extern int __kefirrt_popcountll(unsigned long);
extern int __kefirrt_parityll(long);

extern double __kefirrt_huge_val(void);
extern float __kefirrt_huge_valf(void);
extern long double __kefirrt_huge_vall(void);

extern double __kefirrt_inf(void);
extern float __kefirrt_inff(void);
extern long double __kefirrt_infl(void);

// Builtins
#define __builtin_expect(_exp, _c) ((_c), (_exp))
#define __builtin_expect_with_probability(_expr, _c, _prob) ((_prob), (_c), (_exp))
#define __builtin_prefetch(_addr, ...) \
    do {                               \
        (void) (_addr);                \
    } while (0)
#define __builtin_assume_aligned(_exp, ...) (__VA_ARGS__, (_exp))

#define __builtin_trap() __kefirrt_trap()
#define __builtin_unreachable() __kefirrt_trap()
#define __builtin_return_address(_level) __kefirrt_return_address((_level))
#define __builtin_frame_address(_level) __kefirrt_frame_address((_level))
#define __builtin_extract_return_addr(_addr) (_addr)
#define __builtin_frob_return_addr(_addr) (_addr)

#define __builtin_ffs(_x) __kefirrt_ffs((_x))
#define __builtin_clz(_x) __kefirrt_clz((_x))
#define __builtin_ctz(_x) __kefirrt_ctz((_x))
#define __builtin_clrsb(_x) __kefirrt_clrsb((_x))
#define __builtin_popcount(_x) __kefirrt_popcount((_x))
#define __builtin_parity(_x) __kefirrt_parity((_x))

#define __builtin_bswap16(_x) __kefirrt_bswap16((_x))
#define __builtin_bswap32(_x) __kefirrt_bswap32((_x))
#define __builtin_bswap64(_x) __kefirrt_bswap64((_x))

#define __builtin_ffsl(_x) __kefirrt_ffsl((_x))
#define __builtin_clzl(_x) __kefirrt_clzl((_x))
#define __builtin_ctzl(_x) __kefirrt_ctzl((_x))
#define __builtin_clrsbl(_x) __kefirrt_clrsbl((_x))
#define __builtin_popcountl(_x) __kefirrt_popcountl((_x))
#define __builtin_parityl(_x) __kefirrt_parityl((_x))

#define __builtin_ffsll(_x) __kefirrt_ffsll((_x))
#define __builtin_clzll(_x) __kefirrt_clzll((_x))
#define __builtin_ctzll(_x) __kefirrt_ctzll((_x))
#define __builtin_clrsbll(_x) __kefirrt_clrsbll((_x))
#define __builtin_popcountll(_x) __kefirrt_popcountll((_x))
#define __builtin_parityll(_x) __kefirrt_parityll((_x))

#define __builtin_huge_val(_x) __kefirrt_huge_val()
#define __builtin_huge_valf(_x) __kefirrt_huge_valf()
#define __builtin_huge_vall(_x) __kefirrt_huge_vall()

#define __builtin_inf(_x) __kefirrt_inf()
#define __builtin_inff(_x) __kefirrt_inff()
#define __builtin_infl(_x) __kefirrt_infl()

#define __builtin_LINE() __LINE__
#define __builtin_FILE() __FILE__
#define __builtin_FUNCTION() __FUNCTION__
