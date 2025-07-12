/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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

#define __KEFIR_PREDEFINED_AREA__
#ifdef __KEFIRCC__
__kefir_define_builtin_prefix(__builtin_) __kefir_define_builtin_prefix(__atomic_)
    __kefir_define_builtin_prefix(__sync_)
#endif

// Keywords
#define __FUNCTION__ __func__
#define __thread _Thread_local
#define __extension__
#define __label__ void *

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

        typedef struct {
    __UINT32_TYPE__ gp_offset;
    __UINT32_TYPE__ fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} __builtin_va_list[1];
#endif

// Floating-point
#define __FLOAT_WORD_ORDER__ __BYTE_ORDER__
#define __FLT_RADIX__ 2
#define __FLT_MANT_DIG__ 24
#define __DBL_MANT_DIG__ 53
#define __LDBL_MANT_DIG__ 64
#define __FLT_DIG__ 6
#define __DBL_DIG__ 15
#define __LDBL_DIG__ 18
#define __FLT_DECIMAL_DIG__ 9
#define __DBL_DECIMAL_DIG__ 17
#define __LDBL_DECIMAL_DIG__ 21
#define __FLT_MIN_EXP__ (-125)
#define __DBL_MIN_EXP__ (-1021)
#define __LDBL_MIN_EXP__ (-16381)
#define __FLT_MIN_10_EXP__ (-37)
#define __DBL_MIN_10_EXP__ (-307)
#define __LDBL_MIN_10_EXP__ (-4931)
#define __FLT_MAX_EXP__ 128
#define __DBL_MAX_EXP__ 1024
#define __LDBL_MAX_EXP__ 16384
#define __FLT_MAX_10_EXP__ 38
#define __DBL_MAX_10_EXP__ 308
#define __LDBL_MAX_10_EXP__ 4932
#define __FLT_MIN__ 1.17549435082228750797e-38F
#define __DBL_MIN__ 2.22507385850720138309e-308
#define __LDBL_MIN__ 3.3621031431120935063e-4932L
#define __FLT_EPSILON__ 1.1920928955078125e-07F
#define __DBL_EPSILON__ 2.22044604925031308085e-16
#define __LDBL_EPSILON__ 1.0842021724855044340e-19L
#define __FLT_MAX__ 3.40282346638528859812e+38F
#define __DBL_MAX__ 1.79769313486231570815e+308
#define __LDBL_MAX__ 1.1897314953572317650e+4932L

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

#define __BITINT_MAXWIDTH__ 65535

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

#define __atomic_store(_ptr, _val, _memorder)                                                                         \
    ({                                                                                                                \
        extern void __kefir_builtin_atomic_store8(void *, void *, int);                                               \
        extern void __kefir_builtin_atomic_store16(void *, void *, int);                                              \
        extern void __kefir_builtin_atomic_store32(void *, void *, int);                                              \
        extern void __kefir_builtin_atomic_store64(void *, void *, int);                                              \
        extern void __atomic_store(__SIZE_TYPE__, void *, void *, int);                                               \
        typedef __typeof__((void) 0, *(_ptr)) ptr_type_t;                                                             \
        __builtin_choose_expr(                                                                                        \
            sizeof(*(_ptr)) == 1, __kefir_builtin_atomic_store8((_ptr), (ptr_type_t) * (_val), (_memorder)),          \
            __builtin_choose_expr(                                                                                    \
                sizeof(*(_ptr)) == 2, __kefir_builtin_atomic_store16((_ptr), (ptr_type_t) * (_val), (_memorder)),     \
                __builtin_choose_expr(                                                                                \
                    sizeof(*(_ptr)) == 4, __kefir_builtin_atomic_store32((_ptr), (ptr_type_t) * (_val), (_memorder)), \
                    __builtin_choose_expr(sizeof(*(_ptr)) == 8,                                                       \
                                          __kefir_builtin_atomic_store64((_ptr), (ptr_type_t) * (_val), (_memorder)), \
                                          __atomic_store(sizeof(*(_ptr)), (void *) (_ptr), (_val), (_memorder))))));  \
    })

#define __atomic_store_n(_ptr, _val, _memorder)           \
    ({                                                    \
        __typeof__((void) 0, (_val)) __copy_val = (_val); \
        __atomic_store((_ptr), &__copy_val, (_memorder)); \
    })

#define __atomic_load(_ptr, _mem, _memorder)                                                                        \
    ({                                                                                                              \
        extern __UINT8_TYPE__ __kefir_builtin_atomic_load8(void *, int);                                            \
        extern __UINT16_TYPE__ __kefir_builtin_atomic_load16(void *, int);                                          \
        extern __UINT32_TYPE__ __kefir_builtin_atomic_load32(void *, int);                                          \
        extern __UINT64_TYPE__ __kefir_builtin_atomic_load64(void *, int);                                          \
        extern void __atomic_load(__SIZE_TYPE__, void *, void *, int);                                              \
        typedef __typeof_unqual__((void) 0, *(_mem)) * __mem_type_t;                                                \
        __builtin_choose_expr(                                                                                      \
            sizeof(*(_ptr)) == 1, ({                                                                                \
                *((__mem_type_t) (_mem)) = __kefir_builtin_atomic_load8((void *) (_ptr), (_memorder));              \
                (void) 0;                                                                                           \
            }),                                                                                                     \
            __builtin_choose_expr(                                                                                  \
                sizeof(*(_ptr)) == 2, ({                                                                            \
                    *((__mem_type_t) (_mem)) = __kefir_builtin_atomic_load16((void *) (_ptr), (_memorder));         \
                    (void) 0;                                                                                       \
                }),                                                                                                 \
                __builtin_choose_expr(                                                                              \
                    sizeof(*(_ptr)) == 4, ({                                                                        \
                        *((__mem_type_t) (_mem)) = __kefir_builtin_atomic_load32((void *) (_ptr), (_memorder));     \
                        (void) 0;                                                                                   \
                    }),                                                                                             \
                    __builtin_choose_expr(sizeof(*(_ptr)) == 8, ({                                                  \
                                              *((__mem_type_t) (_mem)) =                                            \
                                                  __kefir_builtin_atomic_load64((void *) (_ptr), (_memorder));      \
                                              (void) 0;                                                             \
                                          }),                                                                       \
                                          __atomic_load(sizeof(*(_ptr)), (void *) (_ptr), (_mem), (_memorder)))))); \
    })

#define __atomic_load_n(_ptr, _memorder)                 \
    ({                                                   \
        __typeof__((void) 0, *(_ptr)) __target;          \
        __atomic_load((_ptr), &(__target), (_memorder)); \
        __target;                                        \
    })

#define __atomic_exchange(_ptr, _val, _ret, _memorder)                                                               \
    ({                                                                                                               \
        extern __UINT8_TYPE__ __kefir_builtin_atomic_exchange8(void *, __UINT8_TYPE__, int);                         \
        extern __UINT16_TYPE__ __kefir_builtin_atomic_exchange16(void *, __UINT16_TYPE__, int);                      \
        extern __UINT32_TYPE__ __kefir_builtin_atomic_exchange32(void *, __UINT32_TYPE__, int);                      \
        extern __UINT64_TYPE__ __kefir_builtin_atomic_exchange64(void *, __UINT64_TYPE__, int);                      \
        extern void __atomic_exchange(__SIZE_TYPE__, void *, void *, void *, int);                                   \
        typedef __typeof_unqual__((void) 0, *(_ret)) * __ret_type_t;                                                 \
        __builtin_choose_expr(                                                                                       \
            sizeof(*(_ptr)) == 1, ({                                                                                 \
                *((__ret_type_t) (_ret)) =                                                                           \
                    __kefir_builtin_atomic_exchange8((void *) (_ptr), (__UINT8_TYPE__) * (_val), (_memorder));       \
                (void) 0;                                                                                            \
            }),                                                                                                      \
            __builtin_choose_expr(                                                                                   \
                sizeof(*(_ptr)) == 2, ({                                                                             \
                    *((__ret_type_t) (_ret)) =                                                                       \
                        __kefir_builtin_atomic_exchange16((void *) (_ptr), (__UINT16_TYPE__) * (_val), (_memorder)); \
                    (void) 0;                                                                                        \
                }),                                                                                                  \
                __builtin_choose_expr(                                                                               \
                    sizeof(*(_ptr)) == 4, ({                                                                         \
                        *((__ret_type_t) (_ret)) = __kefir_builtin_atomic_exchange32(                                \
                            (void *) (_ptr), (__UINT32_TYPE__) * (_val), (_memorder));                               \
                        (void) 0;                                                                                    \
                    }),                                                                                              \
                    __builtin_choose_expr(sizeof(*(_ptr)) == 8, ({                                                   \
                                              *((__ret_type_t) (_ret)) = __kefir_builtin_atomic_exchange64(          \
                                                  (void *) (_ptr), (__UINT64_TYPE__) * (_val), (_memorder));         \
                                              (void) 0;                                                              \
                                          }),                                                                        \
                                          __atomic_exchange(sizeof(*(_ptr)), (void *) (_ptr), (void *) (_val),       \
                                                            (void *) (_ret), (_memorder))))));                       \
    })

#define __atomic_exchange_n(_ptr, _val, _memorder)                           \
    ({                                                                       \
        __typeof__((void) 0, (_val)) __copy_val = (_val);                    \
        __typeof__((void) 0, *(_ptr)) __ret_val;                             \
        __atomic_exchange((_ptr), &(__copy_val), &(__ret_val), (_memorder)); \
        __ret_val;                                                           \
    })

#define __atomic_compare_exchange(_ptr, _expected, _desired, _weak, _success_memorder, _failure_memorder)              \
    ({                                                                                                                 \
        extern _Bool __atomic_compare_exchange(__SIZE_TYPE__, void *, void *, void *, int, int);                       \
        extern _Bool __kefir_builtin_atomic_compare_exchange8(void *, void *, __UINT8_TYPE__, int, int);               \
        extern _Bool __kefir_builtin_atomic_compare_exchange16(void *, void *, __UINT16_TYPE__, int, int);             \
        extern _Bool __kefir_builtin_atomic_compare_exchange32(void *, void *, __UINT32_TYPE__, int, int);             \
        extern _Bool __kefir_builtin_atomic_compare_exchange64(void *, void *, __UINT64_TYPE__, int, int);             \
        (void) (_weak);                                                                                                \
        __builtin_choose_expr(                                                                                         \
            sizeof(*(_ptr)) == 1,                                                                                      \
            __kefir_builtin_atomic_compare_exchange8((void *) (_ptr), (void *) (_expected),                            \
                                                     (__UINT8_TYPE__) * (_desired), (_success_memorder),               \
                                                     (_failure_memorder)),                                             \
            __builtin_choose_expr(                                                                                     \
                sizeof(*(_ptr)) == 2,                                                                                  \
                __kefir_builtin_atomic_compare_exchange16((void *) (_ptr), (void *) (_expected),                       \
                                                          (__UINT16_TYPE__) * (_desired), (_success_memorder),         \
                                                          (_failure_memorder)),                                        \
                __builtin_choose_expr(                                                                                 \
                    sizeof(*(_ptr)) == 4,                                                                              \
                    __kefir_builtin_atomic_compare_exchange32((void *) (_ptr), (void *) (_expected),                   \
                                                              (__UINT32_TYPE__) * (_desired), (_success_memorder),     \
                                                              (_failure_memorder)),                                    \
                    __builtin_choose_expr(                                                                             \
                        sizeof(*(_ptr)) == 8,                                                                          \
                        __kefir_builtin_atomic_compare_exchange64((void *) (_ptr), (void *) (_expected),               \
                                                                  (__UINT64_TYPE__) * (_desired), (_success_memorder), \
                                                                  (_failure_memorder)),                                \
                        __atomic_compare_exchange(sizeof(*(_ptr)), (void *) (_ptr), (void *) (_expected),              \
                                                  (void *) (_desired), (_success_memorder), (_failure_memorder))))));  \
    })

#define __atomic_compare_exchange_n(_ptr, _expected, _desired, _weak, _success_memorder, _failure_memorder) \
    ({                                                                                                      \
        __typeof__((void) 0, (_desired)) __copy_desired = (_desired);                                       \
        __atomic_compare_exchange((_ptr), (_expected), &(__copy_desired), (_weak), (_success_memorder),     \
                                  (_failure_memorder));                                                     \
    })

#define __atomic_fetch_add(_ptr, _val, _memorder)                                                                   \
    ({                                                                                                              \
        typedef __typeof__((void) 0, *(_ptr)) __result_t;                                                           \
        extern __UINT8_TYPE__ __kefir_builtin_atomic_fetch_add8(void *, __UINT8_TYPE__, int);                       \
        extern __UINT16_TYPE__ __kefir_builtin_atomic_fetch_add16(void *, __UINT16_TYPE__, int);                    \
        extern __UINT32_TYPE__ __kefir_builtin_atomic_fetch_add32(void *, __UINT32_TYPE__, int);                    \
        extern __UINT64_TYPE__ __kefir_builtin_atomic_fetch_add64(void *, __UINT64_TYPE__, int);                    \
        __builtin_choose_expr(                                                                                      \
            sizeof(__result_t) == 1,                                                                                \
            __kefir_builtin_atomic_fetch_add8((void *) (_ptr), (__UINT8_TYPE__) (_val), (_memorder)),               \
            __builtin_choose_expr(                                                                                  \
                sizeof(__result_t) == 2,                                                                            \
                __kefir_builtin_atomic_fetch_add16((void *) (_ptr), (__UINT16_TYPE__) (_val), (_memorder)),         \
                __builtin_choose_expr(                                                                              \
                    sizeof(__result_t) == 4,                                                                        \
                    __kefir_builtin_atomic_fetch_add32((void *) (_ptr), (__UINT32_TYPE__) (_val), (_memorder)),     \
                    __builtin_choose_expr(                                                                          \
                        sizeof(__result_t) == 8,                                                                    \
                        __kefir_builtin_atomic_fetch_add64((void *) (_ptr), (__UINT64_TYPE__) (_val), (_memorder)), \
                        ({ _Static_assert(0, "Atomic fetch operation of specified size is not supported"); })))));  \
    })

#define __atomic_fetch_sub(_ptr, _val, _memorder) __atomic_fetch_add((_ptr), -(_val), (_memorder))

#define __kefir_atomic_fetch_op_impl(_ptr, _val, _memorder, _op)                                                 \
    ({                                                                                                           \
        typedef __typeof_unqual__((void) 0, *(_ptr)) __result_t;                                                 \
        __result_t *const __ptr = (__result_t *) (_ptr);                                                         \
        __result_t const __val = (__result_t) (_val);                                                            \
        const int __memorder = (_memorder);                                                                      \
        __result_t __new_value, __current_value = __atomic_load_n(__ptr, __memorder);                            \
        do {                                                                                                     \
            __new_value = _op(__current_value, __val);                                                           \
        } while (!__atomic_compare_exchange_n(__ptr, &__current_value, __new_value, 0, __memorder, __memorder)); \
        __current_value;                                                                                         \
    })

#define __kefir_atomic_fetch_op_or(_arg1, _arg2) ((_arg1) | (_arg2))
#define __kefir_atomic_fetch_op_xor(_arg1, _arg2) ((_arg1) ^ (_arg2))
#define __kefir_atomic_fetch_op_and(_arg1, _arg2) ((_arg1) & (_arg2))
#define __kefir_atomic_fetch_op_nand(_arg1, _arg2) (!((_arg1) & (_arg2)))
#define __atomic_fetch_or(_ptr, _val, _memorder) \
    __kefir_atomic_fetch_op_impl((_ptr), (_val), (_memorder), __kefir_atomic_fetch_op_or)
#define __atomic_fetch_xor(_ptr, _val, _memorder) \
    __kefir_atomic_fetch_op_impl((_ptr), (_val), (_memorder), __kefir_atomic_fetch_op_xor)
#define __atomic_fetch_and(_ptr, _val, _memorder) \
    __kefir_atomic_fetch_op_impl((_ptr), (_val), (_memorder), __kefir_atomic_fetch_op_and)
#define __atomic_fetch_nand(_ptr, _val, _memorder) \
    __kefir_atomic_fetch_op_impl((_ptr), (_val), (_memorder), __kefir_atomic_fetch_op_nand)

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
#define __atomic_is_lock_free(_size, _ptr)                                                                            \
    ({                                                                                                                \
        extern _Bool __atomic_is_lock_free(__SIZE_TYPE__, volatile void *);                                           \
        const __SIZE_TYPE__ __size = (__SIZE_TYPE__) (_size);                                                         \
        __size == sizeof(__UINT8_TYPE__) || __size == sizeof(__UINT16_TYPE__) || __size == sizeof(__UINT32_TYPE__) || \
            __size == sizeof(__UINT64_TYPE__) || __atomic_is_lock_free(__size, (volatile void *) (_ptr));             \
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

// Builtin functions
extern _Noreturn void __kefir_builtin_trap(void);
extern void *__kefir_builtin_return_address(int);
extern void *__kefir_builtin_frame_address(int);

extern int __kefir_builtin_ffs(int);
extern int __kefir_builtin_clz(unsigned int);
extern int __kefir_builtin_ctz(unsigned int);
extern int __kefir_builtin_clrsb(int);
extern int __kefir_builtin_popcount(unsigned int);
extern int __kefir_builtin_parity(int);

extern unsigned short __kefir_builtin_bswap16(unsigned short);
extern unsigned int __kefir_builtin_bswap32(unsigned int);
extern unsigned long __kefir_builtin_bswap64(unsigned long);

extern int __kefir_builtin_ffsl(long);
extern int __kefir_builtin_clzl(unsigned long);
extern int __kefir_builtin_ctzl(unsigned long);
extern int __kefir_builtin_clrsbl(long);
extern int __kefir_builtin_popcountl(unsigned long);
extern int __kefir_builtin_parityl(long);

extern int __kefir_builtin_ffsll(long);
extern int __kefir_builtin_clzll(unsigned long);
extern int __kefir_builtin_ctzll(unsigned long);
extern int __kefir_builtin_clrsbll(long);
extern int __kefir_builtin_popcountll(unsigned long);
extern int __kefir_builtin_parityll(long);

extern int __kefir_builtin_flt_rounds(void);

// Builtins
#define __builtin_va_start(_vlist, _arg) __builtin_c23_va_start((_vlist), (_arg))
#define __builtin_expect(_exp, _c) ((_c), (_exp))
#define __builtin_expect_with_probability(_expr, _c, _prob) ((_prob), (_c), (_exp))
#define __builtin_prefetch(_addr, ...) \
    do {                               \
        (void) (_addr);                \
    } while (0)
#define __builtin_assume_aligned(_exp, ...) (__VA_ARGS__, (_exp))

#define __builtin_trap() __kefir_builtin_trap()
#define __builtin_unreachable() __kefir_builtin_trap()
#define __builtin_return_address(_level) __kefir_builtin_return_address((_level))
#define __builtin_frame_address(_level) __kefir_builtin_frame_address((_level))
#define __builtin_extract_return_addr(_addr) (_addr)
#define __builtin_frob_return_addr(_addr) (_addr)

#define __builtin_ffs(_x) __kefir_builtin_ffs((_x))
#define __builtin_clz(_x) __kefir_builtin_clz((_x))
#define __builtin_ctz(_x) __kefir_builtin_ctz((_x))
#define __builtin_clrsb(_x) __kefir_builtin_clrsb((_x))
#define __builtin_popcount(_x) __kefir_builtin_popcount((_x))
#define __builtin_parity(_x) __kefir_builtin_parity((_x))

#define __builtin_bswap16(_x) __kefir_builtin_bswap16((_x))
#define __builtin_bswap32(_x) __kefir_builtin_bswap32((_x))
#define __builtin_bswap64(_x) __kefir_builtin_bswap64((_x))

#define __builtin_ffsl(_x) __kefir_builtin_ffsl((_x))
#define __builtin_clzl(_x) __kefir_builtin_clzl((_x))
#define __builtin_ctzl(_x) __kefir_builtin_ctzl((_x))
#define __builtin_clrsbl(_x) __kefir_builtin_clrsbl((_x))
#define __builtin_popcountl(_x) __kefir_builtin_popcountl((_x))
#define __builtin_parityl(_x) __kefir_builtin_parityl((_x))

#define __builtin_ffsll(_x) __kefir_builtin_ffsll((_x))
#define __builtin_clzll(_x) __kefir_builtin_clzll((_x))
#define __builtin_ctzll(_x) __kefir_builtin_ctzll((_x))
#define __builtin_clrsbll(_x) __kefir_builtin_clrsbll((_x))
#define __builtin_popcountll(_x) __kefir_builtin_popcountll((_x))
#define __builtin_parityll(_x) __kefir_builtin_parityll((_x))

#define __builtin_huge_val() __builtin_inf()
#define __builtin_huge_valf() __builtin_inff()
#define __builtin_huge_vall() __builtin_infl()

#define __builtin_flt_rounds() __kefir_builtin_flt_rounds()

#define __builtin_LINE() __LINE__
#define __builtin_FILE() __FILE__
#define __builtin_FUNCTION() __FUNCTION__

#define __builtin_sadd_overflow(_a, _b, _r) (__builtin_add_overflow((int) (_a), (int) (_b), (int *) (_r)))
#define __builtin_saddl_overflow(_a, _b, _r) \
    (__builtin_add_overflow((long int) (_a), (long int) (_b), (long int *) (_r)))
#define __builtin_saddll_overflow(_a, _b, _r) \
    (__builtin_add_overflow((long long int) (_a), (long long int) (_b), (long long int *) (_r)))
#define __builtin_uadd_overflow(_a, _b, _r) \
    (__builtin_add_overflow((unsigned int) (_a), (unsigned int) (_b), (unsigned int *) (_r)))
#define __builtin_uaddl_overflow(_a, _b, _r) \
    (__builtin_add_overflow((unsigned long int) (_a), (unsigned long int) (_b), (unsigned long int *) (_r)))
#define __builtin_uaddll_overflow(_a, _b, _r)                                             \
    (__builtin_add_overflow((unsigned long long int) (_a), (unsigned long long int) (_b), \
                            (unsigned long long int *) (_r)))

#define __builtin_ssub_overflow(_a, _b, _r) (__builtin_sub_overflow((int) (_a), (int) (_b), (int *) (_r)))
#define __builtin_ssubl_overflow(_a, _b, _r) \
    (__builtin_sub_overflow((long int) (_a), (long int) (_b), (long int *) (_r)))
#define __builtin_ssubll_overflow(_a, _b, _r) \
    (__builtin_sub_overflow((long long int) (_a), (long long int) (_b), (long long int *) (_r)))
#define __builtin_usub_overflow(_a, _b, _r) \
    (__builtin_sub_overflow((unsigned int) (_a), (unsigned int) (_b), (unsigned int *) (_r)))
#define __builtin_usubl_overflow(_a, _b, _r) \
    (__builtin_sub_overflow((unsigned long int) (_a), (unsigned long int) (_b), (unsigned long int *) (_r)))
#define __builtin_usubll_overflow(_a, _b, _r)                                             \
    (__builtin_sub_overflow((unsigned long long int) (_a), (unsigned long long int) (_b), \
                            (unsigned long long int *) (_r)))

#define __builtin_smul_overflow(_a, _b, _r) (__builtin_mul_overflow((int) (_a), (int) (_b), (int *) (_r)))
#define __builtin_smull_overflow(_a, _b, _r) \
    (__builtin_mul_overflow((long int) (_a), (long int) (_b), (long int *) (_r)))
#define __builtin_smulll_overflow(_a, _b, _r) \
    (__builtin_mul_overflow((long long int) (_a), (long long int) (_b), (long long int *) (_r)))
#define __builtin_umul_overflow(_a, _b, _r) \
    (__builtin_mul_overflow((unsigned int) (_a), (unsigned int) (_b), (unsigned int *) (_r)))
#define __builtin_umull_overflow(_a, _b, _r) \
    (__builtin_mul_overflow((unsigned long int) (_a), (unsigned long int) (_b), (unsigned long int *) (_r)))
#define __builtin_umulll_overflow(_a, _b, _r)                                             \
    (__builtin_mul_overflow((unsigned long long int) (_a), (unsigned long long int) (_b), \
                            (unsigned long long int *) (_r)))

#undef __KEFIR_PREDEFINED_AREA__
