/*
    SPDX-License-Identifier: BSD-3-Clause

    Copyright 2020-2025 Jevgenijs Protopopovs

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its contributors
    may be used to endorse or promote products derived from this software without
    specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
    OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
    OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef KEFIR_RUNTIME_LINUX_GNU_BITS_FLOATN_COMMON_H_
#define KEFIR_RUNTIME_LINUX_GNU_BITS_FLOATN_COMMON_H_

#ifndef __KEFIRCC__
#error \
    "Included bits/floatn-common.h is inteded to be used strictly with Kefir C compiler."
#endif

#if __has_include(<features.h>)
#include <features.h>
#endif

#if __has_include(<bits/long-double.h>)
#include <bits/long-double.h>
#endif

#define __HAVE_FLOAT16 0
#define __HAVE_FLOAT32 1
#define __HAVE_FLOAT64 1
#define __HAVE_FLOAT32X 1
#define __HAVE_FLOAT64X 1
#define __HAVE_FLOAT128X 0

#define __HAVE_DISTINCT_FLOAT16 0
#define __HAVE_DISTINCT_FLOAT32 0
#define __HAVE_DISTINCT_FLOAT64 0
#define __HAVE_DISTINCT_FLOAT32X 0
#define __HAVE_DISTINCT_FLOAT64X 0
#define __HAVE_DISTINCT_FLOAT128X 0
#define __HAVE_FLOAT128_UNLIKE_LDBL 0
#define __HAVE_FLOATN_NOT_TYPEDEF 0

#ifndef __ASSEMBLER__

#if __HAVE_FLOAT32
#define __f32(x) x##f
#define __CFLOAT32 _Complex float

#define __builtin_huge_valf32() (__builtin_huge_valf())
#define __builtin_inff32() (__builtin_inff())
#define __builtin_nanf32(x) (__builtin_nanf(x))
#define __builtin_nansf32(x) (__builtin_nansf(x))
#endif

#if __HAVE_FLOAT64
#define __f64(x) x
#define __CFLOAT64 _Complex float

#define __builtin_huge_valf64() (__builtin_huge_val())
#define __builtin_inff64() (__builtin_inf())
#define __builtin_nanf64(x) (__builtin_nan(x))
#define __builtin_nansf64(x) (__builtin_nans(x))
#endif

#if __HAVE_FLOAT32X
#define __f32x(x) x
#define __CFLOAT32X _Complex double

#define __builtin_huge_valf32x() (__builtin_huge_val())
#define __builtin_inff32x() (__builtin_inf())
#define __builtin_nanf32x(x) (__builtin_nan(x))
#define __builtin_nansf32x(x) (__builtin_nans(x))
#endif

#if __HAVE_FLOAT64X
#define __f64x(x) x##l
#define __CFLOAT64X _Complex long double

#define __builtin_huge_valf64x() (__builtin_huge_vall())
#define __builtin_inff64x() (__builtin_infl())
#define __builtin_nanf64x(x) (__builtin_nanl(x))
#define __builtin_nansf64x(x) (__builtin_nansl(x))
#endif

#endif

#endif
