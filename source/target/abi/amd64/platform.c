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

#include "kefir/target/abi/amd64/platform.h"
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/target/abi/amd64/system-v/qwords.h"
#include "kefir/target/abi/amd64/system-v/type_layout.h"
#include "kefir/target/abi/amd64/bitfields.h"
#include "kefir/ir/platform.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

typedef struct kefir_abi_sysv_amd64_type {
    const struct kefir_ir_type *ir_type;
    struct kefir_abi_amd64_type_layout layout;
} kefir_abi_sysv_amd64_type_t;

static kefir_result_t amd64_sysv_get_type(struct kefir_mem *mem, const struct kefir_ir_target_platform *platform,
                                          const struct kefir_ir_type *ir_type,
                                          kefir_ir_target_platform_type_handle_t *type_ptr) {
    UNUSED(platform);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    REQUIRE(type_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR target platform type pointer"));

    struct kefir_abi_sysv_amd64_type *type = KEFIR_MALLOC(mem, sizeof(struct kefir_abi_sysv_amd64_type));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AMD64 SysV platform IR type"));
    type->ir_type = ir_type;
    kefir_result_t res = kefir_abi_amd64_type_layout(mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, ir_type, &type->layout);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, type);
        return res;
    });

    *type_ptr = type;
    return KEFIR_OK;
}

static kefir_result_t amd64_sysv_free_type(struct kefir_mem *mem,
                                           kefir_ir_target_platform_type_handle_t platform_type) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(platform_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR platform type"));

    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_type *, type, platform_type);
    REQUIRE_OK(kefir_abi_amd64_type_layout_free(mem, &type->layout));
    type->ir_type = NULL;
    KEFIR_FREE(mem, type);
    return KEFIR_OK;
}

static kefir_result_t amd64_sysv_typeentry_info(struct kefir_mem *mem,
                                                kefir_ir_target_platform_type_handle_t platform_type,
                                                kefir_size_t index,
                                                struct kefir_ir_target_platform_typeentry_info *type_info) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(platform_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR platform type"));
    REQUIRE(type_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target type info"));

    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_type *, type, platform_type);
    REQUIRE(index < kefir_ir_type_length(type->ir_type),
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Specified index is out of bounds of IR type"));
    const struct kefir_abi_amd64_typeentry_layout *data_layout = NULL;
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(&type->layout, index, &data_layout));
    type_info->size = data_layout->size;
    type_info->alignment = data_layout->alignment;
    type_info->aligned = data_layout->aligned;
    type_info->max_alignment = KEFIR_ABI_AMD64_SYSV_MAX_ALIGNMENT;
    type_info->relative_offset = data_layout->relative_offset;
    return KEFIR_OK;
}

static kefir_result_t amd64_sysv_bitfield_allocator(struct kefir_mem *mem,
                                                    const struct kefir_ir_target_platform *platform,
                                                    struct kefir_ir_type *type,
                                                    struct kefir_ir_bitfield_allocator *allocator) {
    UNUSED(platform);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    REQUIRE(allocator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR bitfield allocator"));

    REQUIRE_OK(kefir_abi_amd64_bitfield_allocator(mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, type, allocator));
    return KEFIR_OK;
}

static kefir_result_t amd64_sysv_decode_inline_assembly_constraints(
    const struct kefir_ir_target_platform *platform, const char *constraints,
    struct kefir_ir_inline_assembly_parameter_constraints *decoded_constraints,
    const struct kefir_source_location *source_location) {
    UNUSED(platform);
    REQUIRE(constraints != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 inline assembly constraints"));
    REQUIRE(
        decoded_constraints != NULL,
        KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR inline assembly decoded constraints"));

    for (const char *iter = constraints; *iter != '\0'; ++iter) {
        switch (*iter) {
            case 'i':
                decoded_constraints->immediate = true;
                decoded_constraints->strict_immediate = false;
                break;

            case 'n':
            case 'N':
                decoded_constraints->immediate = true;
                decoded_constraints->strict_immediate = true;
                break;

            case 'r':
                decoded_constraints->general_purpose_register = true;
                break;

            case 'x':
                decoded_constraints->floating_point_register = true;
                break;

            case 'f':
                decoded_constraints->x87_stack = true;
                break;

            case 't':
                decoded_constraints->x87_stack = true;
                decoded_constraints->explicit_register = "st0";
                break;

            case 'u':
                decoded_constraints->x87_stack = true;
                decoded_constraints->explicit_register = "st1";
                break;

            case 'm':
                decoded_constraints->memory_location = true;
                break;

            case 'a':
                decoded_constraints->general_purpose_register = true;
                decoded_constraints->explicit_register = "rax";
                break;

            case 'b':
                decoded_constraints->general_purpose_register = true;
                decoded_constraints->explicit_register = "rbx";
                break;

            case 'c':
                decoded_constraints->general_purpose_register = true;
                decoded_constraints->explicit_register = "rcx";
                break;

            case 'd':
                decoded_constraints->general_purpose_register = true;
                decoded_constraints->explicit_register = "rdx";
                break;

            case 'D':
                decoded_constraints->general_purpose_register = true;
                decoded_constraints->explicit_register = "rdi";
                break;

            case 'S':
                decoded_constraints->general_purpose_register = true;
                decoded_constraints->explicit_register = "rsi";
                break;

            case 'X':
                decoded_constraints->general_purpose_register = true;
                decoded_constraints->floating_point_register = true;
                decoded_constraints->memory_location = true;
                break;

            default:
                return KEFIR_SET_SOURCE_ERRORF(KEFIR_ANALYSIS_ERROR, source_location,
                                               "Unsupported inline assembly constraint '%s'", constraints);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_sysv_free(struct kefir_mem *mem, struct kefir_ir_target_platform *platform) {
    UNUSED(mem);
    REQUIRE(platform != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target platform"));
    platform->typeentry_info = NULL;
    platform->free = NULL;
    platform->payload = NULL;
    return KEFIR_OK;
}

static kefir_result_t builtin_va_list_debug_info(struct kefir_mem *mem, const struct kefir_ir_target_platform *platform,
                                                 struct kefir_ir_debug_entries *entries,
                                                 kefir_ir_debug_entry_id_t *entry_id_ptr) {
    UNUSED(platform);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entries != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR debug entries"));
    REQUIRE(entry_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer IR debug entry identifier"));

    kefir_ir_debug_entry_id_t index_type_id, offset_type_id, void_type_id, ptr_type_id, va_list_tag_id,
        va_list_tag_arr_id, va_list_tag_subrange_id, va_list_tag_member;

    REQUIRE_OK(kefir_ir_debug_entry_new(mem, entries, KEFIR_IR_DEBUG_ENTRY_TYPE_UNSIGNED_INT, &index_type_id));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, index_type_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("unsigned long")));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, index_type_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(KEFIR_AMD64_ABI_QWORD)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, index_type_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_ALIGNMENT(KEFIR_AMD64_ABI_QWORD)));

    REQUIRE_OK(kefir_ir_debug_entry_new(mem, entries, KEFIR_IR_DEBUG_ENTRY_TYPE_UNSIGNED_INT, &offset_type_id));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, offset_type_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("unsigned int")));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, offset_type_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(KEFIR_AMD64_ABI_QWORD / 2)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, offset_type_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_ALIGNMENT(KEFIR_AMD64_ABI_QWORD / 2)));

    REQUIRE_OK(kefir_ir_debug_entry_new(mem, entries, KEFIR_IR_DEBUG_ENTRY_TYPE_VOID, &void_type_id));
    REQUIRE_OK(
        kefir_ir_debug_entry_add_attribute(mem, entries, NULL, void_type_id, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("void")));

    REQUIRE_OK(kefir_ir_debug_entry_new(mem, entries, KEFIR_IR_DEBUG_ENTRY_TYPE_POINTER, &ptr_type_id));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, ptr_type_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_TYPE(void_type_id)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, ptr_type_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(KEFIR_AMD64_ABI_QWORD)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, ptr_type_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_ALIGNMENT(KEFIR_AMD64_ABI_QWORD)));

    REQUIRE_OK(kefir_ir_debug_entry_new(mem, entries, KEFIR_IR_DEBUG_ENTRY_TYPE_STRUCTURE, &va_list_tag_id));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("__va_list_tag")));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(3 * KEFIR_AMD64_ABI_QWORD)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_ALIGNMENT(KEFIR_AMD64_ABI_QWORD)));

    REQUIRE_OK(kefir_ir_debug_entry_new_child(mem, entries, va_list_tag_id, KEFIR_IR_DEBUG_ENTRY_STRUCTURE_MEMBER,
                                              &va_list_tag_member));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("gp_offset")));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_TYPE(offset_type_id)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(KEFIR_AMD64_ABI_QWORD / 2)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_ALIGNMENT(KEFIR_AMD64_ABI_QWORD / 2)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_OFFSET(0)));
    REQUIRE_OK(kefir_ir_debug_entry_new_child(mem, entries, va_list_tag_id, KEFIR_IR_DEBUG_ENTRY_STRUCTURE_MEMBER,
                                              &va_list_tag_member));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("fp_offset")));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_TYPE(offset_type_id)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(KEFIR_AMD64_ABI_QWORD / 2)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_ALIGNMENT(KEFIR_AMD64_ABI_QWORD / 2)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_OFFSET(4)));
    REQUIRE_OK(kefir_ir_debug_entry_new_child(mem, entries, va_list_tag_id, KEFIR_IR_DEBUG_ENTRY_STRUCTURE_MEMBER,
                                              &va_list_tag_member));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("overflow_arg_area")));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_TYPE(ptr_type_id)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(KEFIR_AMD64_ABI_QWORD)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_ALIGNMENT(KEFIR_AMD64_ABI_QWORD)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_OFFSET(8)));
    REQUIRE_OK(kefir_ir_debug_entry_new_child(mem, entries, va_list_tag_id, KEFIR_IR_DEBUG_ENTRY_STRUCTURE_MEMBER,
                                              &va_list_tag_member));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("reg_save_area")));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_TYPE(ptr_type_id)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(KEFIR_AMD64_ABI_QWORD)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_ALIGNMENT(KEFIR_AMD64_ABI_QWORD)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_member,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_OFFSET(16)));

    REQUIRE_OK(kefir_ir_debug_entry_new(mem, entries, KEFIR_IR_DEBUG_ENTRY_TYPE_ARRAY, &va_list_tag_arr_id));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_arr_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_TYPE(va_list_tag_id)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_arr_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(3 * KEFIR_AMD64_ABI_QWORD)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_arr_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_ALIGNMENT(KEFIR_AMD64_ABI_QWORD)));
    REQUIRE_OK(kefir_ir_debug_entry_new_child(mem, entries, va_list_tag_arr_id, KEFIR_IR_DEBUG_ENTRY_ARRAY_SUBRANGE,
                                              &va_list_tag_subrange_id));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_subrange_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_TYPE(index_type_id)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, va_list_tag_subrange_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_LENGTH(1)));

    REQUIRE_OK(kefir_ir_debug_entry_new(mem, entries, KEFIR_IR_DEBUG_ENTRY_TYPEDEF, entry_id_ptr));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, *entry_id_ptr,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("__builtin_va_list")));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, NULL, *entry_id_ptr,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_TYPE(va_list_tag_arr_id)));
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_target_platform(kefir_abi_amd64_variant_t variant,
                                               struct kefir_ir_target_platform *platform) {
    REQUIRE(platform != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation platform pointer"));

    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            platform->get_type = amd64_sysv_get_type;
            platform->free_type = amd64_sysv_free_type;
            platform->typeentry_info = amd64_sysv_typeentry_info;
            platform->bitfield_allocator = amd64_sysv_bitfield_allocator;
            platform->decode_inline_assembly_constraints = amd64_sysv_decode_inline_assembly_constraints;
            platform->builtin_va_list_debug_info = builtin_va_list_debug_info;
            platform->free = amd64_sysv_free;
            platform->payload = NULL;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}
