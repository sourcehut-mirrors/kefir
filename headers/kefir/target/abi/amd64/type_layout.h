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

#ifndef KEFIR_TARGET_ABI_AMD64_TYPE_LAYOUT_H_
#define KEFIR_TARGET_ABI_AMD64_TYPE_LAYOUT_H_

#include <stdbool.h>
#include "kefir/core/basic-types.h"
#include "kefir/core/vector.h"
#include "kefir/core/mem.h"
#include "kefir/target/abi/amd64/base.h"
#include "kefir/ir/type.h"

typedef struct kefir_abi_amd64_typeentry_layout {
    kefir_size_t size;
    kefir_size_t alignment;
    bool aligned;
    kefir_size_t relative_offset;
} kefir_abi_amd64_typeentry_layout_t;

typedef struct kefir_abi_amd64_type_layout {
    struct kefir_vector layout;
} kefir_abi_amd64_type_layout_t;

typedef enum kefir_abi_amd64_type_layout_context {
    KEFIR_ABI_AMD64_TYPE_LAYOUT_CONTEXT_STACK,
    KEFIR_ABI_AMD64_TYPE_LAYOUT_CONTEXT_GLOBAL,
    KEFIR_ABI_AMD64_TYPE_LAYOUT_CONTEXT_GENERIC
} kefir_abi_amd64_type_layout_context_t;

kefir_result_t kefir_abi_amd64_type_layout(struct kefir_mem *, kefir_abi_amd64_variant_t,
                                           kefir_abi_amd64_type_layout_context_t, const struct kefir_ir_type *,
                                           struct kefir_abi_amd64_type_layout *);

kefir_result_t kefir_abi_amd64_type_layout_free(struct kefir_mem *, struct kefir_abi_amd64_type_layout *);

kefir_result_t kefir_abi_amd64_type_layout_at(const struct kefir_abi_amd64_type_layout *, kefir_size_t,
                                              const struct kefir_abi_amd64_typeentry_layout **);

kefir_result_t kefir_abi_amd64_calculate_type_properties(const struct kefir_ir_type *,
                                                         const struct kefir_abi_amd64_type_layout *, kefir_size_t *,
                                                         kefir_size_t *);

kefir_size_t kefir_abi_amd64_complex_float_qword_size(kefir_abi_amd64_variant_t);
kefir_size_t kefir_abi_amd64_complex_float_qword_alignment(kefir_abi_amd64_variant_t);
kefir_size_t kefir_abi_amd64_complex_double_qword_size(kefir_abi_amd64_variant_t);
kefir_size_t kefir_abi_amd64_complex_double_qword_alignment(kefir_abi_amd64_variant_t);
kefir_size_t kefir_abi_amd64_complex_long_double_qword_size(kefir_abi_amd64_variant_t);
kefir_size_t kefir_abi_amd64_complex_long_double_qword_alignment(kefir_abi_amd64_variant_t);
kefir_size_t kefir_abi_amd64_float_qword_size(kefir_abi_amd64_variant_t);
kefir_size_t kefir_abi_amd64_float_qword_alignment(kefir_abi_amd64_variant_t);
kefir_size_t kefir_abi_amd64_double_qword_size(kefir_abi_amd64_variant_t);
kefir_size_t kefir_abi_amd64_double_qword_alignment(kefir_abi_amd64_variant_t);
kefir_size_t kefir_abi_amd64_long_double_qword_size(kefir_abi_amd64_variant_t);
kefir_size_t kefir_abi_amd64_long_double_qword_alignment(kefir_abi_amd64_variant_t);
kefir_size_t kefir_abi_amd64_long_double_actual_size(kefir_abi_amd64_variant_t);
kefir_size_t kefir_abi_amd64_fenv_qword_size(kefir_abi_amd64_variant_t);
kefir_size_t kefir_abi_amd64_fenv_qword_alignment(kefir_abi_amd64_variant_t);

#endif
