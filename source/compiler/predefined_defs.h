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
#define __extension__

#define __USER_LABEL_PREFIX__

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
#define __CHAR8_TYPE__ unsigned char
#define __CHAR16_TYPE__ unsigned short int
#define __CHAR32_TYPE__ unsigned int
#endif

#define __FLOAT_WORD_ORDER__ __BYTE_ORDER__
#define __label__ void *

// Type width
#ifdef __LP64__
#define __PTRDIFF_WIDTH__ __LONG_WIDTH__
#define __SIZE_WIDTH__ __LONG_WIDTH__
#define __WCHAR_WIDTH__ __INT_WIDTH__
#define __WINT_WIDTH__ __INT_WIDTH__
#define __INT_LEAST8_WIDTH__ __SCHAR_WIDTH__
#define __INT_LEAST16_WIDTH__ __SHRT_WIDTH__
#define __INT_LEAST32_WIDTH__ __INT_WIDTH__
#define __INT_LEAST64_WIDTH__ __LONG_WIDTH__
#define __INT_FAST8_WIDTH__ __SCHAR_WIDTH__
#define __INT_FAST16_WIDTH__ __SHRT_WIDTH__
#define __INT_FAST32_WIDTH__ __INT_WIDTH__
#define __INT_FAST64_WIDTH__ __LONG_WIDTH__
#define __INTPTR_WIDTH__ __LONG_WIDTH__
#define __INTMAX_WIDTH__ __LONG_WIDTH__
#endif

// Type size
#ifdef __LP64__
#define __SIZEOF_POINTER__ __SIZEOF_LONG__
#define __SIZEOF_SIZE_T__ __SIZEOF_LONG__
#define __SIZEOF_PTRDIFF_T__ __SIZEOF_LONG__
#define __SIZEOF_WCHAR_T__ __SIZEOF_INT__
#define __SIZEOF_WINT_T__ __SIZEOF_INT__
#endif

// Atomics
#define __ATOMIC_RELAXED 0
#define __ATOMIC_CONSUME 1
#define __ATOMIC_ACQUIRE 2
#define __ATOMIC_RELEASE 3
#define __ATOMIC_ACQ_REL 4
#define __ATOMIC_SEQ_CST 5

#define __atomic_store(_ptr, _val, _memorder)                           \
    ({                                                                  \
        extern void __atomic_store(__SIZE_TYPE__, void *, void *, int); \
        __atomic_store(sizeof(*(_ptr)), (_ptr), (_val), (_memorder));   \
    })

#define __atomic_store_n(_ptr, _val, _memorder)           \
    ({                                                    \
        __typeof__((void) 0, (_val)) __copy_val = (_val); \
        __atomic_store((_ptr), &__copy_val, (_memorder)); \
    })

#define __atomic_load(_ptr, _mem, _memorder)                           \
    ({                                                                 \
        extern void __atomic_load(__SIZE_TYPE__, void *, void *, int); \
        __atomic_load(sizeof(*(_ptr)), (_ptr), (_mem), (_memorder));   \
    })

#define __atomic_load_n(_ptr, _memorder)                 \
    ({                                                   \
        __typeof__((void) 0, *(_ptr)) __target;          \
        __atomic_load((_ptr), &(__target), (_memorder)); \
        __target;                                        \
    })

#define __atomic_exchange(_ptr, _val, _ret, _memorder)                             \
    ({                                                                             \
        extern void __atomic_exchange(__SIZE_TYPE__, void *, void *, void *, int); \
        __atomic_exchange(sizeof(*(_ptr)), (_ptr), (_val), (_ret), (_memorder));   \
    })

#define __atomic_exchange_n(_ptr, _val, _memorder)                           \
    ({                                                                       \
        __typeof__((void) 0, (_val)) __copy_val = (_val);                    \
        __typeof__((void) 0, *(_ptr)) __ret_val;                             \
        __atomic_exchange((_ptr), &(__copy_val), &(__ret_val), (_memorder)); \
        __ret_val;                                                           \
    })

#define __atomic_compare_exchange(_ptr, _expected, _desired, _weak, _success_memorder, _failure_memorder) \
    ({                                                                                                    \
        extern _Bool __atomic_compare_exchange(__SIZE_TYPE__, void *, void *, void *, int, int);          \
        (void) (_weak);                                                                                   \
        __atomic_compare_exchange(sizeof(*(_ptr)), (_ptr), (_expected), (_desired), (_success_memorder),  \
                                  (_failure_memorder));                                                   \
    })

#define __atomic_compare_exchange_n(_ptr, _expected, _desired, _weak, _success_memorder, _failure_memorder) \
    ({                                                                                                      \
        __typeof__((void) 0, (_desired)) __copy_desired = (_desired);                                       \
        (void) (_weak);                                                                                     \
        __atomic_compare_exchange((_ptr), (_expected), &(__copy_desired), (_weak), (_success_memorder),     \
                                  (_failure_memorder));                                                     \
    })

#define __kefir_atomic_fetch_op(_op, _ptr, _val, _memorder)                                                        \
    ({                                                                                                             \
        typedef __typeof__((void) 0, *(_ptr)) __result_t;                                                          \
        typedef __result_t (*__fn_t)(void *, __result_t, int);                                                     \
        extern __UINT8_TYPE__ __atomic_fetch_##_op##_1(void *, __UINT8_TYPE__, int);                               \
        extern __UINT16_TYPE__ __atomic_fetch_##_op##_2(void *, __UINT16_TYPE__, int);                             \
        extern __UINT32_TYPE__ __atomic_fetch_##_op##_4(void *, __UINT32_TYPE__, int);                             \
        extern __UINT64_TYPE__ __atomic_fetch_##_op##_8(void *, __UINT64_TYPE__, int);                             \
        __builtin_choose_expr(                                                                                     \
            sizeof(__result_t) == 1, ((__fn_t) __atomic_fetch_##_op##_1)((_ptr), (_val), (_memorder)),             \
            __builtin_choose_expr(                                                                                 \
                sizeof(__result_t) == 2, ((__fn_t) __atomic_fetch_##_op##_2)((_ptr), (_val), (_memorder)),         \
                __builtin_choose_expr(                                                                             \
                    sizeof(__result_t) <= 4, ((__fn_t) __atomic_fetch_##_op##_4)((_ptr), (_val), (_memorder)),     \
                    __builtin_choose_expr(                                                                         \
                        sizeof(__result_t) <= 8, ((__fn_t) __atomic_fetch_##_op##_8)((_ptr), (_val), (_memorder)), \
                        ({ _Static_assert(0, "Atomic fetch operation of specified size is not supported"); }))))); \
    })

#define __atomic_fetch_add(_ptr, _val, _memorder) __kefir_atomic_fetch_op(add, (_ptr), (_val), (_memorder))
#define __atomic_fetch_sub(_ptr, _val, _memorder) __kefir_atomic_fetch_op(sub, (_ptr), (_val), (_memorder))
#define __atomic_fetch_or(_ptr, _val, _memorder) __kefir_atomic_fetch_op(or, (_ptr), (_val), (_memorder))
#define __atomic_fetch_xor(_ptr, _val, _memorder) __kefir_atomic_fetch_op(xor, (_ptr), (_val), (_memorder))
#define __atomic_fetch_and(_ptr, _val, _memorder) __kefir_atomic_fetch_op(and, (_ptr), (_val), (_memorder))
#define __atomic_fetch_nand(_ptr, _val, _memorder) __kefir_atomic_fetch_op(nand, (_ptr), (_val), (_memorder))

#define __atomic_thread_fence(_memorder)                        \
    ({                                                          \
        extern void __kefir_builtin_atomic_seq_cst_fence(void); \
        (void) (_memorder);                                     \
        __kefir_builtin_atomic_seq_cst_fence();                 \
    })
#define __atomic_signal_fence(_memorder)                        \
    ({                                                          \
        extern void __kefir_builtin_atomic_seq_cst_fence(void); \
        (void) (_memorder);                                     \
        __kefir_builtin_atomic_seq_cst_fence();                 \
    })
#define __atomic_is_lock_free(_size, _ptr)                                        \
    ({                                                                            \
        extern _Bool __atomic_is_lock_free(__SIZE_TYPE__, volatile void *);       \
        __atomic_is_lock_free((__SIZE_TYPE__) (_size), (volatile void *) (_ptr)); \
    })
#define __atomic_test_and_set(_ptr, _memorder)                                     \
    ({                                                                             \
        extern _Bool __kefir_builtin_atomic_seq_cst_test_and_set(volatile void *); \
        __kefir_builtin_atomic_seq_cst_test_and_set((volatile void *) (_ptr));     \
    })
#define __atomic_clear(_ptr, _memorder)                                    \
    ({                                                                     \
        extern void __kefir_builtin_atomic_seq_cst_clear(volatile void *); \
        (void) (_memorder);                                                \
        __kefir_builtin_atomic_seq_cst_clear((volatile void *) (_ptr));    \
    })

#define __KEFIR_ATOMIC_ALWAYS_LOCK_FREE 2
#ifdef __LP64__
#define __GCC_ATOMIC_BOOL_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __GCC_ATOMIC_CHAR_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __GCC_ATOMIC_CHAR8_T_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __GCC_ATOMIC_CHAR16_T_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __GCC_ATOMIC_CHAR32_T_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __GCC_ATOMIC_WCHAR_T_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __GCC_ATOMIC_SHORT_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __GCC_ATOMIC_INT_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __GCC_ATOMIC_LONG_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __GCC_ATOMIC_LLONG_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __GCC_ATOMIC_POINTER_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE

#define __CLANG_ATOMIC_BOOL_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __CLANG_ATOMIC_CHAR_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __CLANG_ATOMIC_CHAR8_T_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __CLANG_ATOMIC_CHAR16_T_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __CLANG_ATOMIC_CHAR32_T_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __CLANG_ATOMIC_WCHAR_T_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __CLANG_ATOMIC_SHORT_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __CLANG_ATOMIC_INT_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __CLANG_ATOMIC_LONG_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __CLANG_ATOMIC_LLONG_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE
#define __CLANG_ATOMIC_POINTER_LOCK_FREE __KEFIR_ATOMIC_ALWAYS_LOCK_FREE

#define __GCC_ATOMIC_TEST_AND_SET_TRUEVAL 1
#endif

// Clang atomic builtins
#define __c11_atomic_init(_ptr, _desired) __atomic_store_n((_ptr), (_desired), __ATOMIC_SEQ_CST)

#define __KEFIR_C11_ATOMIC_MEMORDER1(...)                                                         \
    ({                                                                                            \
        int __memorder[] = {0, __VA_ARGS__};                                                      \
        __builtin_choose_expr(sizeof(__memorder) == sizeof(int), __ATOMIC_SEQ_CST, ({             \
                                  _Static_assert(sizeof(__memorder) == sizeof(int) * 2,           \
                                                 "Expected one optional memory order parameter"); \
                                  __memorder[1];                                                  \
                              }));                                                                \
    })
#define __KEFIR_C11_ATOMIC_MEMORDER2(...)                                                          \
    ({                                                                                             \
        int __memorder[] = {0, __VA_ARGS__};                                                       \
        __builtin_choose_expr(sizeof(__memorder) == sizeof(int), __ATOMIC_SEQ_CST, ({              \
                                  _Static_assert(sizeof(__memorder) == sizeof(int) * 3,            \
                                                 "Expected two optional memory order parameters"); \
                                  __memorder[1];                                                   \
                              }));                                                                 \
    })
#define __KEFIR_C11_ATOMIC_MEMORDER3(...)                                                          \
    ({                                                                                             \
        int __memorder[] = {0, __VA_ARGS__};                                                       \
        __builtin_choose_expr(sizeof(__memorder) == sizeof(int), __ATOMIC_SEQ_CST, ({              \
                                  _Static_assert(sizeof(__memorder) == sizeof(int) * 3,            \
                                                 "Expected two optional memory order parameters"); \
                                  __memorder[2];                                                   \
                              }));                                                                 \
    })

#define __c11_atomic_load(_ptr, ...) __atomic_load_n((_ptr), __KEFIR_C11_ATOMIC_MEMORDER1(__VA_ARGS__))
#define __c11_atomic_store(_ptr, _value, ...) \
    __atomic_store_n((_ptr), (_value), __KEFIR_C11_ATOMIC_MEMORDER1(__VA_ARGS__))
#define __c11_atomic_exchange(_ptr, _value, ...) \
    __atomic_exchange_n((_ptr), (_value), __KEFIR_C11_ATOMIC_MEMORDER1(__VA_ARGS__))
#define __c11_atomic_compare_exchange_strong(_ptr, _expected, _desired, ...)                                   \
    __atomic_compare_exchange_n((_ptr), (_expected), (_desired), 0, __KEFIR_C11_ATOMIC_MEMORDER2(__VA_ARGS__), \
                                __KEFIR_C11_ATOMIC_MEMORDER3(__VA_ARGS__))
#define __c11_atomic_compare_exchange_weak(_ptr, _expected, _desired, ...)                                     \
    __atomic_compare_exchange_n((_ptr), (_expected), (_desired), 0, __KEFIR_C11_ATOMIC_MEMORDER2(__VA_ARGS__), \
                                __KEFIR_C11_ATOMIC_MEMORDER3(__VA_ARGS__))

#define __c11_atomic_fetch_add(_ptr, _value, ...) \
    __atomic_fetch_add((_ptr), (_value), __KEFIR_C11_ATOMIC_MEMORDER1(__VA_ARGS__))
#define __c11_atomic_fetch_sub(_ptr, _value, ...) \
    __atomic_fetch_sub((_ptr), (_value), __KEFIR_C11_ATOMIC_MEMORDER1(__VA_ARGS__))
#define __c11_atomic_fetch_or(_ptr, _value, ...) \
    __atomic_fetch_or((_ptr), (_value), __KEFIR_C11_ATOMIC_MEMORDER1(__VA_ARGS__))
#define __c11_atomic_fetch_xor(_ptr, _value, ...) \
    __atomic_fetch_xor((_ptr), (_value), __KEFIR_C11_ATOMIC_MEMORDER1(__VA_ARGS__))
#define __c11_atomic_fetch_and(_ptr, _value, ...) \
    __atomic_fetch_and((_ptr), (_value), __KEFIR_C11_ATOMIC_MEMORDER1(__VA_ARGS__))

#define __c11_atomic_is_lock_free(_sz) __atomic_is_lock_free((_sz), (const volatile void *) 0)
#define __c11_atomic_thread_fence(_order) __atomic_thread_fence((_order))
#define __c11_atomic_signal_fence(_order) __atomic_signal_fence((_order))

// Sync builtins
#define __sync_fetch_and_add(_ptr, _value, ...) __atomic_fetch_add((_ptr), (_value), __ATOMIC_SEQ_CST)
#define __sync_fetch_and_sub(_ptr, _value, ...) __atomic_fetch_sub((_ptr), (_value), __ATOMIC_SEQ_CST)
#define __sync_fetch_and_or(_ptr, _value, ...) __atomic_fetch_or((_ptr), (_value), __ATOMIC_SEQ_CST)
#define __sync_fetch_and_and(_ptr, _value, ...) __atomic_fetch_and((_ptr), (_value), __ATOMIC_SEQ_CST)
#define __sync_fetch_and_xor(_ptr, _value, ...) __atomic_fetch_xor((_ptr), (_value), __ATOMIC_SEQ_CST)
#define __sync_fetch_and_nand(_ptr, _value, ...) __atomic_fetch_nand((_ptr), (_value), __ATOMIC_SEQ_CST)

#define __kefir_sync_op_and_fetch(_op, _ptr, _value)                                                                  \
    ({                                                                                                                \
        typedef __typeof_unqual__((void) 0, *(_ptr)) __value_t;                                                       \
        __value_t __prev_value = __atomic_load_n((_ptr), __ATOMIC_ACQUIRE), __new_value;                              \
        do {                                                                                                          \
            __new_value = __prev_value _op(__value_t)(_value);                                                        \
        } while (                                                                                                     \
            !__atomic_compare_exchange_n((_ptr), &__prev_value, __new_value, 0, __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE)); \
        __new_value;                                                                                                  \
    })
#define __sync_add_and_fetch(_ptr, _value, ...) __kefir_sync_op_and_fetch(+, (_ptr), (_value))
#define __sync_sub_and_fetch(_ptr, _value, ...) __kefir_sync_op_and_fetch(-, (_ptr), (_value))
#define __sync_or_and_fetch(_ptr, _value, ...) __kefir_sync_op_and_fetch(|, (_ptr), (_value))
#define __sync_and_and_fetch(_ptr, _value, ...) __kefir_sync_op_and_fetch(&, (_ptr), (_value))
#define __sync_xor_and_fetch(_ptr, _value, ...) __kefir_sync_op_and_fetch(^, (_ptr), (_value))
#define __sync_nand_and_fetch(_ptr, _value, ...)                                                                      \
    ({                                                                                                                \
        typedef __typeof_unqual__((void) 0, *(_ptr)) __value_t;                                                       \
        __value_t __prev_value = __atomic_load_n((_ptr), __ATOMIC_ACQUIRE), __new_value;                              \
        do {                                                                                                          \
            __new_value = ~(__prev_value & (__value_t) (_value));                                                     \
        } while (                                                                                                     \
            !__atomic_compare_exchange_n((_ptr), &__prev_value, __new_value, 0, __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE)); \
        __new_value;                                                                                                  \
    })

#define __sync_bool_compare_and_swap(_ptr, _oldval, _newval, ...)                                                \
    ({                                                                                                           \
        __typeof__((void) 0, (_oldval)) __copy_oldval = (_oldval);                                               \
        __atomic_compare_exchange_n((_ptr), &(__copy_oldval), (_newval), 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); \
    })
#define __sync_val_compare_and_swap(_ptr, _oldval, _newval, ...)                                                      \
    ({                                                                                                                \
        __typeof__((void) 0, (_oldval)) __copy_oldval = (_oldval);                                                    \
        (void) __atomic_compare_exchange_n((_ptr), &__copy_oldval, (_newval), 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); \
        __copy_oldval;                                                                                                \
    })

#define __sync_synchronize(...) __atomic_thread_fence(__ATOMIC_SEQ_CST)

#define __sync_lock_test_and_set(_ptr, _value, ...) __atomic_exchange_n((_ptr), (_value), __ATOMIC_ACQUIRE)
#define __sync_lock_release(_ptr, ...) __atomic_store_n((_ptr), 0, __ATOMIC_RELEASE)

// Runtime functions
extern _Noreturn void __kefir_builtin_trap(void);
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

#define __builtin_trap() __kefir_builtin_trap()
#define __builtin_unreachable() __kefir_builtin_trap()
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
