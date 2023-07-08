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

#ifndef KEFIR_RUNTIME_COMMON_TYPECLASS_KEFIR_IMPL_H_
#define KEFIR_RUNTIME_COMMON_TYPECLASS_KEFIR_IMPL_H_

#ifndef __KEFIR_IMPL_TYPECLASS_HEADER__
#error "typeclass_kefir_impl.h shall not be included directly"
#endif

#define __KEFIR_IMPL_TYPECLASS_NO_TYPE_CLASS -1
#define __KEFIR_IMPL_TYPECLASS_VOID_TYPE_CLASS 0
#define __KEFIR_IMPL_TYPECLASS_INTEGER_TYPE_CLASS 1
#define __KEFIR_IMPL_TYPECLASS_CHAR_TYPE_CLASS 2
#define __KEFIR_IMPL_TYPECLASS_ENUMERAL_TYPE_CLASS 3
#define __KEFIR_IMPL_TYPECLASS_BOOLEAN_TYPE_CLASS 4
#define __KEFIR_IMPL_TYPECLASS_POINTER_TYPE_CLASS 5
#define __KEFIR_IMPL_TYPECLASS_REAL_TYPE_CLASS 8
#define __KEFIR_IMPL_TYPECLASS_FUNCTION_TYPE_CLASS 10
#define __KEFIR_IMPL_TYPECLASS_RECORD_TYPE_CLASS 12
#define __KEFIR_IMPL_TYPECLASS_UNION_TYPE_CLASS 13
#define __KEFIR_IMPL_TYPECLASS_ARRAY_TYPE_CLASS 14
#define __KEFIR_IMPL_TYPECLASS_LANG_TYPE_CLASS 16

#endif
