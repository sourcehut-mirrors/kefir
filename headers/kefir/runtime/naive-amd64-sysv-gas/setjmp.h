/*
    SPDX-License-Identifier: BSD-3-Clause

    Copyright 2020-2023 Jevgenijs Protopopovs

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

#ifndef KEFIR_RUNTIME_NAIVE_AMD64_SYSV_GAS_SETJMP_H_
#define KEFIR_RUNTIME_NAIVE_AMD64_SYSV_GAS_SETJMP_H_

#ifndef __KEFIRCC__
#error \
    "Included setjmp.h is inteded to be used strictly with Kefir C compiler. Use <setjmp.h> from system standard library with other compilers"
#endif

#ifdef __x86_64__
typedef unsigned long __jmp_buf[8 + 32];
#else
#error "setjmp.h unable to detect target processor architecture"
#endif

typedef struct __jmp_buf_tag {
    __jmp_buf __jb;
} jmp_buf[1];

int __kefirrt_setjmp(jmp_buf);
_Noreturn void __kefirrt_longjmp(jmp_buf, int);

#define _setjmp __kefirrt_setjmp
#define setjmp __kefirrt_setjmp

#define _longjmp __kefirrt_longjmp
#define longjmp __kefirrt_longjmp

#endif