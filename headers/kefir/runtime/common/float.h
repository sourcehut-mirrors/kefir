/*
    SPDX-License-Identifier: BSD-3-Clause

    Copyright 2020-2024 Jevgenijs Protopopovs

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

#ifndef KEFIR_RUNTIME_COMMON_FLOAT_H_
#define KEFIR_RUNTIME_COMMON_FLOAT_H_

#ifndef __KEFIRCC__
#error \
    "Included float.h is inteded to be used strictly with Kefir C compiler. Use <float.h> from system standard library with other compilers"
#endif

#include_next <float.h>

#ifdef FLT_ROUNDS
#undef FLT_ROUNDS
#endif

#define FLT_ROUNDS                        \
    ({                                    \
        int __kefir_flt_rounds = -1;      \
        switch (__builtin_flt_rounds()) { \
            case 0x0:                     \
                __kefir_flt_rounds = 1;   \
                break;                    \
                                          \
            case 0x400:                   \
                __kefir_flt_rounds = 3;   \
                break;                    \
                                          \
            case 0x800:                   \
                __kefir_flt_rounds = 2;   \
                break;                    \
                                          \
            case 0xc00:                   \
                __kefir_flt_rounds = 0;   \
                break;                    \
        };                                \
        __kefir_flt_rounds;               \
    })

#endif