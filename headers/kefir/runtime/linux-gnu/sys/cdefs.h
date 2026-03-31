/*
    SPDX-License-Identifier: BSD-3-Clause

    Copyright 2020-2026 Jevgenijs Protopopovs

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

#ifndef __KEFIR_RUNTIME_LINUX_GNU_SYS_CDEFS_H_
#define __KEFIR_RUNTIME_LINUX_GNU_SYS_CDEFS_H_

#ifndef __KEFIRCC__
#error "Included sys/cdefs.h is inteded to be used strictly with Kefir C compiler."
#endif

#include_next <sys/cdefs.h>

#ifdef __attribute__
#undef __attribute__
#endif

// Definitions below are provided for compatibility with equivalent definitions
// from glibc <sys/cdefs.h> that are only enabled for compilers declaring GNU C.

#define __KEFIR_LINUX_GNU_SYS_CDEFS_REDIRECT(name, prototype, alias) name prototype __asm__(__ASMNAME(#alias))
#define __KEFIR_LINUX_GNU_SYS_CDEFS_ASMNAME(prefix, name) __STRING(prefix) name

#define __REDIRECT(name, prototype, alias) __KEFIR_LINUX_GNU_SYS_CDEFS_REDIRECT(name, prototype, alias)
#define __REDIRECT_NTH(name, prototype, alias) __KEFIR_LINUX_GNU_SYS_CDEFS_REDIRECT(name, prototype, alias) __THROW
#define __REDIRECT_NTHNL(name, prototype, alias) __KEFIR_LINUX_GNU_SYS_CDEFS_REDIRECT(name, prototype, alias) __THROWNL
#define __ASMNAME(name) __KEFIR_LINUX_GNU_SYS_CDEFS_ASMNAME(__USER_LABEL_PREFIX__, name)
#define __ASMNAME2(prefix, name) __KEFIR_LINUX_GNU_SYS_CDEFS_ASMNAME

#endif
