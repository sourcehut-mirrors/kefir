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

#include <stdbool.h>
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/target/abi/util.h"
#include "kefir/target/abi/amd64/system-v/type_layout.h"

kefir_result_t kefir_abi_amd64_type_layout(struct kefir_mem *mem, kefir_abi_amd64_variant_t variant,
                                           const struct kefir_ir_type *type,
                                           struct kefir_abi_amd64_type_layout *layout) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to ABI type layout"));

    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            REQUIRE_OK(kefir_abi_amd64_sysv_calculate_type_layout(mem, type, 0, kefir_ir_type_children(type),
                                                                  &layout->layout));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_type_layout_free(struct kefir_mem *mem, struct kefir_abi_amd64_type_layout *layout) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid ABI type layout vector"));

    REQUIRE_OK(kefir_vector_free(mem, &layout->layout));
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_type_layout_at(const struct kefir_abi_amd64_type_layout *layout, kefir_size_t index,
                                              const struct kefir_abi_amd64_typeentry_layout **entry_layout_ptr) {
    REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid ABI data layout vector"));
    REQUIRE(index < kefir_vector_length(&layout->layout),
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid ABI type layout vector index"));
    REQUIRE(entry_layout_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to ABI type entry layout"));

    *entry_layout_ptr = kefir_vector_at(&layout->layout, index);
    REQUIRE(*entry_layout_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unable to fetch ABI type entry layout"));
    return KEFIR_OK;
}

struct type_properties {
    kefir_size_t size;
    kefir_size_t alignment;
    const struct kefir_vector *layout;
};

static kefir_result_t calculate_type_properties_visitor(const struct kefir_ir_type *type, kefir_size_t index,
                                                        const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(typeentry);

    ASSIGN_DECL_CAST(struct type_properties *, props, payload);
    ASSIGN_DECL_CAST(struct kefir_abi_amd64_typeentry_layout *, layout, kefir_vector_at(props->layout, index));
    props->size = kefir_target_abi_pad_aligned(props->size, layout->alignment);
    props->size += layout->size;
    props->alignment = MAX(props->alignment, layout->alignment);
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_calculate_type_properties(const struct kefir_ir_type *type,
                                                         const struct kefir_abi_amd64_type_layout *type_layout,
                                                         kefir_size_t *size, kefir_size_t *alignment) {
    struct type_properties props = {.size = 0, .alignment = 0, .layout = &type_layout->layout};
    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, calculate_type_properties_visitor));
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, &visitor, (void *) &props, 0, kefir_ir_type_children(type)));
    *size = props.size;
    *alignment = props.alignment;
    return KEFIR_OK;
}

kefir_size_t kefir_abi_amd64_complex_float_qword_size(kefir_abi_amd64_variant_t variant) {
    UNUSED(variant);
    return 1;
}

kefir_size_t kefir_abi_amd64_complex_float_qword_alignment(kefir_abi_amd64_variant_t variant) {
    UNUSED(variant);
    return 1;
}

kefir_size_t kefir_abi_amd64_complex_double_qword_size(kefir_abi_amd64_variant_t variant) {
    UNUSED(variant);
    return 2;
}

kefir_size_t kefir_abi_amd64_complex_double_qword_alignment(kefir_abi_amd64_variant_t variant) {
    UNUSED(variant);
    return 1;
}

kefir_size_t kefir_abi_amd64_complex_long_double_qword_size(kefir_abi_amd64_variant_t variant) {
    UNUSED(variant);
    return 4;
}

kefir_size_t kefir_abi_amd64_complex_long_double_qword_alignment(kefir_abi_amd64_variant_t variant) {
    UNUSED(variant);
    return 2;
}

kefir_size_t kefir_abi_amd64_float_qword_size(kefir_abi_amd64_variant_t variant) {
    UNUSED(variant);
    return 1;
}

kefir_size_t kefir_abi_amd64_float_qword_alignment(kefir_abi_amd64_variant_t variant) {
    UNUSED(variant);
    return 1;
}

kefir_size_t kefir_abi_amd64_long_double_qword_size(kefir_abi_amd64_variant_t variant) {
    UNUSED(variant);
    return 2;
}

kefir_size_t kefir_abi_amd64_long_double_qword_alignment(kefir_abi_amd64_variant_t variant) {
    UNUSED(variant);
    return 2;
}
