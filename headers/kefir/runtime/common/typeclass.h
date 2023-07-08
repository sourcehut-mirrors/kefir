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

#ifndef KEFIR_RUNTIME_COMMON_TYPECLASS_H_
#define KEFIR_RUNTIME_COMMON_TYPECLASS_H_

#define __KEFIR_IMPL_TYPECLASS_HEADER__
#include "typeclass_kefir_impl.h"
#undef __KEFIR_IMPL_TYPECLASS_HEADER__

enum type_class {
    no_type_class = __KEFIR_IMPL_TYPECLASS_NO_TYPE_CLASS,
    void_type_class = __KEFIR_IMPL_TYPECLASS_VOID_TYPE_CLASS,
    integer_type_class = __KEFIR_IMPL_TYPECLASS_INTEGER_TYPE_CLASS,
    char_type_class = __KEFIR_IMPL_TYPECLASS_CHAR_TYPE_CLASS,
    enumeral_type_class = __KEFIR_IMPL_TYPECLASS_ENUMERAL_TYPE_CLASS,
    boolean_type_class = __KEFIR_IMPL_TYPECLASS_BOOLEAN_TYPE_CLASS,
    pointer_type_class = __KEFIR_IMPL_TYPECLASS_POINTER_TYPE_CLASS,
    real_type_class = __KEFIR_IMPL_TYPECLASS_REAL_TYPE_CLASS,
    function_type_class = __KEFIR_IMPL_TYPECLASS_FUNCTION_TYPE_CLASS,
    record_type_class = __KEFIR_IMPL_TYPECLASS_RECORD_TYPE_CLASS,
    union_type_class = __KEFIR_IMPL_TYPECLASS_UNION_TYPE_CLASS,
    array_type_class = __KEFIR_IMPL_TYPECLASS_ARRAY_TYPE_CLASS,
    lang_type_class = __KEFIR_IMPL_TYPECLASS_LANG_TYPE_CLASS
};

#endif
