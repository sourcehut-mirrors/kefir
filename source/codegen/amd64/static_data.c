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

#include <string.h>
#include "kefir/codegen/amd64/static_data.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/target/abi/util.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/vector.h"
#include "kefir/ir/builtins.h"

struct static_data_param {
    struct kefir_codegen_amd64 *codegen;
    const struct kefir_ir_module *module;
    const struct kefir_ir_data *data;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_ir_type_visitor *visitor;
    kefir_size_t slot;
    kefir_size_t offset;
    struct kefir_ir_data_map_iterator data_map_iter;
};

static kefir_result_t visitor_not_supported(const struct kefir_ir_type *type, kefir_size_t index,
                                            const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    UNUSED(payload);
    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Encountered not supported type code while traversing IR type");
}

static kefir_result_t align_offset(const struct kefir_abi_amd64_typeentry_layout *layout,
                                   struct static_data_param *param) {

    kefir_size_t new_offset = kefir_target_abi_pad_aligned(param->offset, layout->alignment);
    if (new_offset > param->offset) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ZERODATA(&param->codegen->xasmgen, new_offset - param->offset));
        param->offset = new_offset;
    }
    return KEFIR_OK;
}

static kefir_result_t trailing_padding(kefir_size_t start_offset, const struct kefir_abi_amd64_typeentry_layout *layout,
                                       struct static_data_param *param) {
    kefir_size_t end_offset = start_offset + layout->size;
    if (end_offset > param->offset) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ZERODATA(&param->codegen->xasmgen, end_offset - param->offset));
        param->offset = end_offset;
    } else {
        REQUIRE(end_offset == param->offset,
                KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Actual object size exceeds expected"));
    }
    return KEFIR_OK;
}

static kefir_result_t integral_static_data(const struct kefir_ir_type *type, kefir_size_t index,
                                           const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct static_data_param *, param, payload);
    const struct kefir_ir_data_value *entry;
    REQUIRE_OK(kefir_ir_data_value_at(param->data, param->slot++, &entry));

    kefir_int64_t value = 0;
    switch (entry->type) {
        case KEFIR_IR_DATA_VALUE_UNDEFINED:
            break;

        case KEFIR_IR_DATA_VALUE_INTEGER:
            value = entry->value.integer;
            break;

        case KEFIR_IR_DATA_VALUE_POINTER: {
            REQUIRE(typeentry->typecode == KEFIR_IR_TYPE_LONG || typeentry->typecode == KEFIR_IR_TYPE_INT64,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to store pointer in requested location"));

            const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
            REQUIRE_OK(kefir_abi_amd64_type_layout_at(&param->layout, index, &layout));
            REQUIRE_OK(align_offset(layout, param));

            const struct kefir_ir_identifier *ir_identifier;
            REQUIRE_OK(kefir_ir_module_get_identifier(param->module, entry->value.pointer.reference, &ir_identifier));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_offset(
                    &param->codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(&param->codegen->xasmgen_helpers.operands[1],
                                                          KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE, ir_identifier->symbol),
                    entry->value.pointer.offset)));

            param->offset += layout->size;
            return KEFIR_OK;
        }

        case KEFIR_IR_DATA_VALUE_STRING_POINTER: {
            REQUIRE(typeentry->typecode == KEFIR_IR_TYPE_LONG || typeentry->typecode == KEFIR_IR_TYPE_INT64,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to store pointer in requested location"));

            const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
            REQUIRE_OK(kefir_abi_amd64_type_layout_at(&param->layout, index, &layout));
            REQUIRE_OK(align_offset(layout, param));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_offset(
                    &param->codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(
                        &param->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                        kefir_asm_amd64_xasmgen_helpers_format(&param->codegen->xasmgen_helpers,
                                                               KEFIR_AMD64_STRING_LITERAL, entry->value.string_ptr.id)),
                    entry->value.string_ptr.offset)));

            param->offset += layout->size;
            return KEFIR_OK;
        }

        case KEFIR_IR_DATA_VALUE_BITS:
            // Intentionally left blank
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected value of integral field");
    }

    const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(&param->layout, index, &layout));
    if (typeentry->typecode != KEFIR_IR_TYPE_BITS) {
        REQUIRE_OK(align_offset(layout, param));
    }
    switch (typeentry->typecode) {
        case KEFIR_IR_TYPE_BOOL:
        case KEFIR_IR_TYPE_CHAR:
        case KEFIR_IR_TYPE_INT8:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_DATA(&param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_BYTE, 1,
                                         kefir_asm_amd64_xasmgen_operand_immu(
                                             &param->codegen->xasmgen_helpers.operands[0], (kefir_uint8_t) value)));
            break;

        case KEFIR_IR_TYPE_SHORT:
        case KEFIR_IR_TYPE_INT16:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_DATA(&param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_WORD, 1,
                                         kefir_asm_amd64_xasmgen_operand_immu(
                                             &param->codegen->xasmgen_helpers.operands[0], (kefir_uint16_t) value)));
            break;

        case KEFIR_IR_TYPE_INT:
        case KEFIR_IR_TYPE_INT32:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_DATA(&param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                                         kefir_asm_amd64_xasmgen_operand_immu(
                                             &param->codegen->xasmgen_helpers.operands[0], (kefir_uint32_t) value)));
            break;

        case KEFIR_IR_TYPE_LONG:
        case KEFIR_IR_TYPE_INT64:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_DATA(&param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                                         kefir_asm_amd64_xasmgen_operand_immu(
                                             &param->codegen->xasmgen_helpers.operands[0], (kefir_uint64_t) value)));
            break;

        case KEFIR_IR_TYPE_BITS: {
            kefir_size_t bits = typeentry->param;
            kefir_size_t bytes = bits / 8;
            if (bits % 8 != 0) {
                bytes += 1;
            }
            for (kefir_size_t i = 0; i < bytes; i++) {
                const kefir_size_t qword_container_idx = i / sizeof(kefir_uint64_t);
                const kefir_size_t qword_offset = i % sizeof(kefir_uint64_t);
                kefir_uint64_t qword_container =
                    qword_container_idx < entry->value.bits.length ? entry->value.bits.bits[qword_container_idx] : 0;
                REQUIRE_OK(
                    KEFIR_AMD64_XASMGEN_DATA(&param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_BYTE, 1,
                                             kefir_asm_amd64_xasmgen_operand_immu(
                                                 &param->codegen->xasmgen_helpers.operands[0],
                                                 (kefir_uint8_t) ((qword_container >> (qword_offset << 3)) & 0xff))));
            }
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpectedly encountered non-integral type");
    }
    param->offset += layout->size;
    return KEFIR_OK;
}

static kefir_result_t word_static_data(const struct kefir_ir_type *type, kefir_size_t index,
                                       const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct static_data_param *, param, payload);
    const struct kefir_ir_data_value *entry;
    REQUIRE_OK(kefir_ir_data_value_at(param->data, param->slot++, &entry));

    const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(&param->layout, index, &layout));
    REQUIRE_OK(align_offset(layout, param));
    switch (entry->type) {
        case KEFIR_IR_DATA_VALUE_UNDEFINED:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_imm(&param->codegen->xasmgen_helpers.operands[0], 0)));
            break;

        case KEFIR_IR_DATA_VALUE_INTEGER:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_DATA(&param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                                         kefir_asm_amd64_xasmgen_operand_imm(
                                             &param->codegen->xasmgen_helpers.operands[0], entry->value.integer)));
            break;

        case KEFIR_IR_DATA_VALUE_POINTER: {
            const struct kefir_ir_identifier *ir_identifier;
            REQUIRE_OK(kefir_ir_module_get_identifier(param->module, entry->value.pointer.reference, &ir_identifier));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_offset(
                    &param->codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(&param->codegen->xasmgen_helpers.operands[1],
                                                          KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE, ir_identifier->symbol),
                    entry->value.pointer.offset)));
        } break;

        case KEFIR_IR_DATA_VALUE_STRING_POINTER:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_offset(
                    &param->codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(
                        &param->codegen->xasmgen_helpers.operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                        kefir_asm_amd64_xasmgen_helpers_format(&param->codegen->xasmgen_helpers,
                                                               KEFIR_AMD64_STRING_LITERAL, entry->value.string_ptr.id)),
                    entry->value.pointer.offset)));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected value of word type field");
    }

    param->offset += layout->size;
    return KEFIR_OK;
}

static kefir_result_t float32_static_data(const struct kefir_ir_type *type, kefir_size_t index,
                                          const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct static_data_param *, param, payload);
    const struct kefir_ir_data_value *entry;
    REQUIRE_OK(kefir_ir_data_value_at(param->data, param->slot++, &entry));

    union {
        kefir_float32_t fp32;
        kefir_uint32_t uint32;
    } value = {.fp32 = 0.0f};
    switch (entry->type) {
        case KEFIR_IR_DATA_VALUE_UNDEFINED:
            break;

        case KEFIR_IR_DATA_VALUE_FLOAT32:
            value.fp32 = entry->value.float32;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected value of floating-point type field");
    }

    const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(&param->layout, index, &layout));
    REQUIRE_OK(align_offset(layout, param));
    switch (typeentry->typecode) {
        case KEFIR_IR_TYPE_FLOAT32:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                kefir_asm_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[0], value.uint32)));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpectedly encountered non-float32 type");
    }
    param->offset += layout->size;
    return KEFIR_OK;
}

static kefir_result_t float64_static_data(const struct kefir_ir_type *type, kefir_size_t index,
                                          const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct static_data_param *, param, payload);
    const struct kefir_ir_data_value *entry;
    REQUIRE_OK(kefir_ir_data_value_at(param->data, param->slot++, &entry));

    union {
        kefir_float64_t fp64;
        kefir_uint64_t uint64;
    } value = {.fp64 = 0.0};
    switch (entry->type) {
        case KEFIR_IR_DATA_VALUE_UNDEFINED:
            break;

        case KEFIR_IR_DATA_VALUE_FLOAT64:
            value.fp64 = entry->value.float64;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected value of floating-point type field");
    }

    const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(&param->layout, index, &layout));
    REQUIRE_OK(align_offset(layout, param));
    switch (typeentry->typecode) {
        case KEFIR_IR_TYPE_FLOAT64:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[0], value.uint64)));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpectedly encountered non-float type");
    }
    param->offset += layout->size;
    return KEFIR_OK;
}

static kefir_result_t long_double_static_data(const struct kefir_ir_type *type, kefir_size_t index,
                                              const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct static_data_param *, param, payload);
    const struct kefir_ir_data_value *entry;
    REQUIRE_OK(kefir_ir_data_value_at(param->data, param->slot++, &entry));

    union {
        kefir_long_double_t long_double;
        kefir_uint64_t uint64[2];
    } value = {.uint64 = {0, 0}};
    switch (entry->type) {
        case KEFIR_IR_DATA_VALUE_UNDEFINED:
            break;

        case KEFIR_IR_DATA_VALUE_LONG_DOUBLE:
            value.long_double = entry->value.long_double;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected value of long double type field");
    }

    const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(&param->layout, index, &layout));
    REQUIRE_OK(align_offset(layout, param));
    switch (typeentry->typecode) {
        case KEFIR_IR_TYPE_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[0], value.uint64[0]),
                kefir_asm_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[1], value.uint64[1])));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpectedly encountered non-long double type");
    }
    param->offset += layout->size;
    return KEFIR_OK;
}

static kefir_result_t complex_float32_static_data(const struct kefir_ir_type *type, kefir_size_t index,
                                                  const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct static_data_param *, param, payload);
    const struct kefir_ir_data_value *entry;
    REQUIRE_OK(kefir_ir_data_value_at(param->data, param->slot++, &entry));

    union {
        kefir_float32_t fp32[2];
        kefir_uint32_t uint32[2];
    } value = {.fp32 = {0.0f}};
    switch (entry->type) {
        case KEFIR_IR_DATA_VALUE_UNDEFINED:
            break;

        case KEFIR_IR_DATA_VALUE_COMPLEX_FLOAT32:
            value.fp32[0] = entry->value.complex_float32.real;
            value.fp32[1] = entry->value.complex_float32.imaginary;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected value of complex floating-point type field");
    }

    const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(&param->layout, index, &layout));
    REQUIRE_OK(align_offset(layout, param));
    switch (typeentry->typecode) {
        case KEFIR_IR_TYPE_COMPLEX_FLOAT32:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 2,
                kefir_asm_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[0], value.uint32[0]),
                kefir_asm_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[1], value.uint32[1])));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpectedly encountered non-float32 type");
    }
    param->offset += layout->size;
    return KEFIR_OK;
}

static kefir_result_t complex_float64_static_data(const struct kefir_ir_type *type, kefir_size_t index,
                                                  const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct static_data_param *, param, payload);
    const struct kefir_ir_data_value *entry;
    REQUIRE_OK(kefir_ir_data_value_at(param->data, param->slot++, &entry));

    union {
        kefir_float64_t fp64[2];
        kefir_uint64_t uint64[2];
    } value = {.fp64 = {0.0}};
    switch (entry->type) {
        case KEFIR_IR_DATA_VALUE_UNDEFINED:
            break;

        case KEFIR_IR_DATA_VALUE_COMPLEX_FLOAT64:
            value.fp64[0] = entry->value.complex_float64.real;
            value.fp64[1] = entry->value.complex_float64.imaginary;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected value of complex floating-point type field");
    }

    const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(&param->layout, index, &layout));
    REQUIRE_OK(align_offset(layout, param));
    switch (typeentry->typecode) {
        case KEFIR_IR_TYPE_COMPLEX_FLOAT64:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[0], value.uint64[0]),
                kefir_asm_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[1], value.uint64[1])));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpectedly encountered non-float32 type");
    }
    param->offset += layout->size;
    return KEFIR_OK;
}

static kefir_result_t complex_long_double_static_data(const struct kefir_ir_type *type, kefir_size_t index,
                                                      const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct static_data_param *, param, payload);
    const struct kefir_ir_data_value *entry;
    REQUIRE_OK(kefir_ir_data_value_at(param->data, param->slot++, &entry));

    union {
        kefir_long_double_t ldouble[2];
        kefir_uint64_t uint64[4];
    } value = {.ldouble = {0.0L}};
    switch (entry->type) {
        case KEFIR_IR_DATA_VALUE_UNDEFINED:
            break;

        case KEFIR_IR_DATA_VALUE_COMPLEX_LONG_DOUBLE:
            value.ldouble[0] = entry->value.complex_long_double.real;
            value.ldouble[1] = entry->value.complex_long_double.imaginary;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected value of complex floating-point type field");
    }

    const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(&param->layout, index, &layout));
    REQUIRE_OK(align_offset(layout, param));
    switch (typeentry->typecode) {
        case KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[0], value.uint64[0]),
                kefir_asm_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[1], value.uint64[1])));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_asm_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[0], value.uint64[2]),
                kefir_asm_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[1], value.uint64[3])));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpectedly encountered non-float32 type");
    }
    param->offset += layout->size;
    return KEFIR_OK;
}

static kefir_result_t struct_static_data(const struct kefir_ir_type *type, kefir_size_t index,
                                         const struct kefir_ir_typeentry *typeentry, void *payload) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type"));
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct static_data_param *, param, payload);
    const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(&param->layout, index, &layout));
    param->slot++;

    REQUIRE_OK(align_offset(layout, param));
    kefir_size_t start_offset = param->offset;
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, param->visitor, payload, index + 1, typeentry->param));

    REQUIRE_OK(trailing_padding(start_offset, layout, param));
    return KEFIR_OK;
}

static kefir_result_t union_static_data(const struct kefir_ir_type *type, kefir_size_t index,
                                        const struct kefir_ir_typeentry *typeentry, void *payload) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type"));
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct static_data_param *, param, payload);
    const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(&param->layout, index, &layout));
    param->slot++;

    REQUIRE_OK(align_offset(layout, param));
    kefir_size_t start_offset = param->offset;

    kefir_size_t subindex = index + 1;
    kefir_bool_t defined = false;
    for (kefir_size_t i = 0; i < (kefir_size_t) typeentry->param; i++) {
        const struct kefir_ir_data_value *subentry;
        REQUIRE_OK(kefir_ir_data_value_at(param->data, param->slot, &subentry));
        if (!defined && subentry->defined) {
            REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, param->visitor, payload, subindex, 1));
            defined = true;
        } else {
            param->slot += kefir_ir_type_slots_of(type, subindex);
        }
        subindex += kefir_ir_type_length_of(type, subindex);
    }

    REQUIRE_OK(trailing_padding(start_offset, layout, param));
    return KEFIR_OK;
}

static kefir_result_t dump_binary(struct static_data_param *param, const char *raw, kefir_size_t length) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_BINDATA(&param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_BYTE, raw, length));
    param->offset += length;
    return KEFIR_OK;
}

static kefir_result_t array_static_data(const struct kefir_ir_type *type, kefir_size_t index,
                                        const struct kefir_ir_typeentry *typeentry, void *payload) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type"));
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct static_data_param *, param, payload);
    const struct kefir_ir_data_value *entry;
    REQUIRE_OK(kefir_ir_data_value_at(param->data, param->slot++, &entry));
    const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(&param->layout, index, &layout));

    REQUIRE_OK(align_offset(layout, param));
    kefir_size_t start_offset = param->offset;

    kefir_size_t array_element_slots = kefir_ir_type_slots_of(type, index + 1);
    kefir_size_t array_content_slots = kefir_ir_type_slots_of(type, index) - 1;

    switch (entry->type) {
        case KEFIR_IR_DATA_VALUE_UNDEFINED:
        case KEFIR_IR_DATA_VALUE_AGGREGATE: {
            kefir_size_t array_end_slot = param->slot + array_content_slots;
            const struct kefir_abi_amd64_typeentry_layout *array_element_layout = NULL;
            REQUIRE_OK(kefir_abi_amd64_type_layout_at(&param->layout, index + 1, &array_element_layout));
            for (kefir_size_t i = 0; i < (kefir_size_t) typeentry->param; i++) {
                REQUIRE_OK(kefir_ir_data_map_skip_to(param->data, &param->data_map_iter, param->slot));
                if (!param->data_map_iter.has_mapped_values) {
                    param->slot = array_end_slot;
                    break;
                } else if (param->slot < param->data_map_iter.next_mapped_slot) {
                    const kefir_size_t missing_slots =
                        MIN(param->data_map_iter.next_mapped_slot, array_end_slot) - param->slot;
                    const kefir_size_t missing_array_elements = missing_slots / array_element_slots;
                    if (missing_array_elements > 0) {
                        i += missing_array_elements - 1;
                        param->slot += missing_array_elements * array_element_slots;
                        kefir_size_t zero_count = missing_array_elements * array_element_layout->size;
                        param->offset += zero_count;
                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ZERODATA(&param->codegen->xasmgen, zero_count));
                        continue;
                    }
                }

                REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, param->visitor, payload, index + 1, 1));
            }
        } break;

        case KEFIR_IR_DATA_VALUE_STRING:
            REQUIRE_OK(dump_binary(param, entry->value.raw.data, entry->value.raw.length));
            param->slot += array_content_slots;
            break;

        case KEFIR_IR_DATA_VALUE_RAW: {
            ASSIGN_DECL_CAST(const char *, raw, entry->value.raw.data);
            REQUIRE_OK(dump_binary(param, raw, entry->value.raw.length));
            param->slot += array_content_slots;
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected array data type");
    }

    REQUIRE_OK(trailing_padding(start_offset, layout, param));
    return KEFIR_OK;
}

static kefir_result_t builtin_static_data(const struct kefir_ir_type *type, kefir_size_t index,
                                          const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(typeentry);
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct static_data_param *, param, payload);
    const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(&param->layout, index, &layout));
    param->slot++;

    switch ((kefir_ir_builtin_type_t) typeentry->param) {
        case KEFIR_IR_TYPE_BUILTIN_VARARG:
            REQUIRE_OK(align_offset(layout, param));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_ZERODATA(&param->codegen->xasmgen, layout->size));
            param->offset += layout->size;
            break;

        case KEFIR_IR_TYPE_BUILTIN_COUNT:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected built-in type");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_static_data(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                               const struct kefir_ir_module *module, const struct kefir_ir_data *data,
                                               const char *identifier) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 code generator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data"));
    REQUIRE(data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected finalized IR data"));

    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, visitor_not_supported));
    KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS(&visitor, integral_static_data);
    visitor.visit[KEFIR_IR_TYPE_WORD] = word_static_data;
    visitor.visit[KEFIR_IR_TYPE_FLOAT32] = float32_static_data;
    visitor.visit[KEFIR_IR_TYPE_FLOAT64] = float64_static_data;
    visitor.visit[KEFIR_IR_TYPE_LONG_DOUBLE] = long_double_static_data;
    visitor.visit[KEFIR_IR_TYPE_COMPLEX_FLOAT32] = complex_float32_static_data;
    visitor.visit[KEFIR_IR_TYPE_COMPLEX_FLOAT64] = complex_float64_static_data;
    visitor.visit[KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE] = complex_long_double_static_data;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = array_static_data;
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = struct_static_data;
    visitor.visit[KEFIR_IR_TYPE_UNION] = union_static_data;
    visitor.visit[KEFIR_IR_TYPE_BUILTIN] = builtin_static_data;

    struct static_data_param param = {
        .codegen = codegen, .module = module, .data = data, .visitor = &visitor, .slot = 0, .offset = 0};
    REQUIRE_OK(kefir_ir_data_map_iter(data, &param.data_map_iter));
    REQUIRE_OK(kefir_abi_amd64_type_layout(mem, codegen->abi_variant, KEFIR_ABI_AMD64_TYPE_LAYOUT_CONTEXT_GLOBAL,
                                           data->type, &param.layout));

    kefir_size_t total_size, total_alignment;
    kefir_result_t res =
        kefir_abi_amd64_calculate_type_properties(data->type, &param.layout, &total_size, &total_alignment);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_type_layout_free(mem, &param.layout);
        return res;
    });
    if (identifier != NULL) {
        if (total_alignment > 1) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, total_alignment));
        }
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", identifier));
    }

    res =
        kefir_ir_type_visitor_list_nodes(data->type, &visitor, (void *) &param, 0, kefir_ir_type_children(data->type));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_type_layout_free(mem, &param.layout);
        return res;
    });

    if (param.offset < total_size) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ZERODATA(&codegen->xasmgen, total_size - param.offset));
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen->xasmgen, 1));
    REQUIRE_OK(kefir_abi_amd64_type_layout_free(mem, &param.layout));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_static_data_uninit(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                                      const struct kefir_ir_data *data, const char *identifier) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 code generator"));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR data"));
    REQUIRE(data->finalized, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected finalized IR data"));

    struct kefir_abi_amd64_type_layout layout;
    REQUIRE_OK(kefir_abi_amd64_type_layout(mem, codegen->abi_variant, KEFIR_ABI_AMD64_TYPE_LAYOUT_CONTEXT_GLOBAL,
                                           data->type, &layout));

    kefir_size_t total_size, total_alignment;
    kefir_result_t res = kefir_abi_amd64_calculate_type_properties(data->type, &layout, &total_size, &total_alignment);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_type_layout_free(mem, &layout);
        return res;
    });
    if (identifier != NULL) {
        if (total_alignment > 1) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, total_alignment));
        }
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", identifier));
    }

    if (total_size > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_UNINITDATA(&codegen->xasmgen, total_size));
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen->xasmgen, 1));
    REQUIRE_OK(kefir_abi_amd64_type_layout_free(mem, &layout));
    return KEFIR_OK;
}
