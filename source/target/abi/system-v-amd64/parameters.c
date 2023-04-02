/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#include "kefir/target/abi/system-v-amd64/parameters.h"
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/mem.h"
#include "kefir/ir/builtins.h"

kefir_asm_amd64_xasmgen_register_t KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTERS[] = {
    KEFIR_AMD64_XASMGEN_REGISTER_RDI, KEFIR_AMD64_XASMGEN_REGISTER_RSI, KEFIR_AMD64_XASMGEN_REGISTER_RDX,
    KEFIR_AMD64_XASMGEN_REGISTER_RCX, KEFIR_AMD64_XASMGEN_REGISTER_R8,  KEFIR_AMD64_XASMGEN_REGISTER_R9};

const kefir_size_t KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTER_COUNT =
    sizeof(KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTERS) /
    sizeof(KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTERS[0]);

kefir_asm_amd64_xasmgen_register_t KEFIR_ABI_SYSV_AMD64_PARAMETER_SSE_REGISTERS[] = {
    KEFIR_AMD64_XASMGEN_REGISTER_XMM0, KEFIR_AMD64_XASMGEN_REGISTER_XMM1, KEFIR_AMD64_XASMGEN_REGISTER_XMM2,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM3, KEFIR_AMD64_XASMGEN_REGISTER_XMM4, KEFIR_AMD64_XASMGEN_REGISTER_XMM5,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM6, KEFIR_AMD64_XASMGEN_REGISTER_XMM7};

const kefir_size_t KEFIR_ABI_SYSV_AMD64_PARAMETER_SSE_REGISTER_COUNT =
    sizeof(KEFIR_ABI_SYSV_AMD64_PARAMETER_SSE_REGISTERS) / sizeof(KEFIR_ABI_SYSV_AMD64_PARAMETER_SSE_REGISTERS[0]);

#define ABI_INTEGER_REGS KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTER_COUNT
#define ABI_SSE_REGS KEFIR_ABI_SYSV_AMD64_PARAMETER_SSE_REGISTER_COUNT

static kefir_result_t visitor_not_supported(const struct kefir_ir_type *type, kefir_size_t index,
                                            const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    UNUSED(payload);
    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Encountered not supported type code while traversing type");
}

struct input_allocation {
    struct kefir_mem *mem;
    const struct kefir_abi_sysv_amd64_type_layout *layout;
    struct kefir_vector *allocation;
    kefir_size_t slot;
};

static kefir_result_t assign_immediate_integer(const struct kefir_ir_type *type, kefir_size_t index,
                                               const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(typeentry);
    struct input_allocation *info = (struct input_allocation *) payload;
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(info->allocation, info->slot++));
    allocation->type = KEFIR_AMD64_SYSV_INPUT_PARAM_IMMEDIATE;
    allocation->klass = KEFIR_AMD64_SYSV_PARAM_INTEGER;
    allocation->index = index;
    allocation->requirements.integer = 1;
    return KEFIR_OK;
}

static kefir_result_t assign_immediate_sse(const struct kefir_ir_type *type, kefir_size_t index,
                                           const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(typeentry);
    struct input_allocation *info = (struct input_allocation *) payload;
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(info->allocation, info->slot++));
    allocation->type = KEFIR_AMD64_SYSV_INPUT_PARAM_IMMEDIATE;
    allocation->klass = KEFIR_AMD64_SYSV_PARAM_SSE;
    allocation->index = index;
    allocation->requirements.sse = 1;
    return KEFIR_OK;
}

static kefir_result_t assign_immediate_long_double(const struct kefir_ir_type *type, kefir_size_t index,
                                                   const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(typeentry);
    struct input_allocation *info = (struct input_allocation *) payload;
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(info->allocation, info->slot++));
    allocation->type = KEFIR_AMD64_SYSV_INPUT_PARAM_IMMEDIATE;
    allocation->klass = KEFIR_AMD64_SYSV_PARAM_X87;
    allocation->index = index;
    allocation->requirements.x87 = 1;
    allocation->requirements.memory.size = 16;
    allocation->requirements.memory.alignment = 16;
    return KEFIR_OK;
}

struct recursive_aggregate_allocation {
    const struct kefir_ir_type *type;
    const struct kefir_abi_sysv_amd64_type_layout *layout;
    struct kefir_vector *allocation;
    struct kefir_abi_sysv_amd64_parameter_allocation *top_allocation;
    struct kefir_ir_type_visitor *visitor;
    kefir_size_t *slot;
    kefir_size_t nested;
    struct kefir_abi_sysv_amd64_qword_position init_position;
    struct kefir_abi_sysv_amd64_qword_position max_position;
};

static kefir_result_t assign_nested_scalar(const struct kefir_ir_type *type, kefir_size_t index,
                                           const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    struct recursive_aggregate_allocation *info = (struct recursive_aggregate_allocation *) payload;
    const struct kefir_abi_sysv_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_at(info->layout, index, &layout));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(info->allocation, (*info->slot)++));
    allocation->type = KEFIR_AMD64_SYSV_INPUT_PARAM_NESTED;
    allocation->klass = KEFIR_AMD64_SYSV_PARAM_NO_CLASS;
    allocation->index = index;
    kefir_abi_sysv_amd64_data_class_t dataclass =
        (typeentry->typecode == KEFIR_IR_TYPE_FLOAT32 || typeentry->typecode == KEFIR_IR_TYPE_FLOAT64)
            ? KEFIR_AMD64_SYSV_PARAM_SSE
            : KEFIR_AMD64_SYSV_PARAM_INTEGER;
    if (typeentry->typecode == KEFIR_IR_TYPE_BITS) {
        REQUIRE_OK(kefir_abi_sysv_amd64_qwords_next_bitfield(&info->top_allocation->container, dataclass,
                                                             typeentry->param, &allocation->container_reference));
    } else {
        REQUIRE_OK(kefir_abi_sysv_amd64_qwords_next(&info->top_allocation->container, dataclass, layout->size,
                                                    layout->alignment, &allocation->container_reference));
    }
    return KEFIR_OK;
}

static kefir_result_t assign_nested_long_double(const struct kefir_ir_type *type, kefir_size_t index,
                                                const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);

    struct recursive_aggregate_allocation *info = (struct recursive_aggregate_allocation *) payload;
    const struct kefir_abi_sysv_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_at(info->layout, index, &layout));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(info->allocation, (*info->slot)++));
    allocation->type = KEFIR_AMD64_SYSV_INPUT_PARAM_NESTED;
    allocation->klass = KEFIR_AMD64_SYSV_PARAM_NO_CLASS;
    allocation->index = index;
    REQUIRE_OK(kefir_abi_sysv_amd64_qwords_next(&info->top_allocation->container, KEFIR_AMD64_SYSV_PARAM_X87,
                                                KEFIR_AMD64_SYSV_ABI_QWORD, layout->alignment,
                                                &allocation->container_reference));
    REQUIRE_OK(kefir_abi_sysv_amd64_qwords_next(&info->top_allocation->container, KEFIR_AMD64_SYSV_PARAM_X87UP,
                                                KEFIR_AMD64_SYSV_ABI_QWORD, layout->alignment,
                                                &allocation->container_reference));
    return KEFIR_OK;
}

static kefir_result_t assign_nested_struct(const struct kefir_ir_type *type, kefir_size_t index,
                                           const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    struct recursive_aggregate_allocation *info = (struct recursive_aggregate_allocation *) payload;
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(info->allocation, (*info->slot)++));
    allocation->type = KEFIR_AMD64_SYSV_INPUT_PARAM_CONTAINER;
    allocation->klass = KEFIR_AMD64_SYSV_PARAM_NO_CLASS;
    allocation->index = index;
    info->nested++;
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, info->visitor, (void *) info, index + 1, typeentry->param));
    info->nested--;
    return KEFIR_OK;
}

static kefir_result_t nested_visitor_init(struct kefir_ir_type_visitor *);

static kefir_result_t union_reset_hook(const struct kefir_ir_type *type, kefir_size_t index,
                                       const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    UNUSED(payload);
    struct recursive_aggregate_allocation *info = (struct recursive_aggregate_allocation *) payload;
    if (info->nested == 0) {
        struct kefir_abi_sysv_amd64_qword_position current_position;
        REQUIRE_OK(kefir_abi_sysv_amd64_qwords_save_position(&info->top_allocation->container, &current_position));
        REQUIRE_OK(
            kefir_abi_sysv_amd64_qwords_max_position(&info->max_position, &current_position, &info->max_position));
        return kefir_abi_sysv_amd64_qwords_restore_position(&info->top_allocation->container, &info->init_position);
    } else {
        return KEFIR_OK;
    }
}

static kefir_result_t assign_nested_union(const struct kefir_ir_type *type, kefir_size_t index,
                                          const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    struct recursive_aggregate_allocation *info = (struct recursive_aggregate_allocation *) payload;
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(info->allocation, (*info->slot)++));
    allocation->type = KEFIR_AMD64_SYSV_INPUT_PARAM_CONTAINER;
    allocation->klass = KEFIR_AMD64_SYSV_PARAM_NO_CLASS;
    allocation->index = index;
    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(nested_visitor_init(&visitor));
    visitor.posthook = union_reset_hook;
    struct recursive_aggregate_allocation nested_info = {.type = type,
                                                         .layout = info->layout,
                                                         .allocation = info->allocation,
                                                         .top_allocation = info->top_allocation,
                                                         .visitor = &visitor,
                                                         .slot = info->slot,
                                                         .nested = 0};
    REQUIRE_OK(kefir_abi_sysv_amd64_qwords_save_position(&info->top_allocation->container, &nested_info.init_position));
    REQUIRE_OK(kefir_abi_sysv_amd64_qwords_save_position(&info->top_allocation->container, &nested_info.max_position));
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, &visitor, (void *) &nested_info, index + 1, typeentry->param));
    REQUIRE_OK(
        kefir_abi_sysv_amd64_qwords_restore_position(&info->top_allocation->container, &nested_info.max_position));
    return KEFIR_OK;
}

static kefir_result_t assign_nested_array(const struct kefir_ir_type *type, kefir_size_t index,
                                          const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    struct recursive_aggregate_allocation *info = (struct recursive_aggregate_allocation *) payload;
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(info->allocation, (*info->slot)++));
    allocation->type = KEFIR_AMD64_SYSV_INPUT_PARAM_CONTAINER;
    allocation->klass = KEFIR_AMD64_SYSV_PARAM_NO_CLASS;
    allocation->index = index;
    info->nested++;
    for (kefir_size_t i = 0; i < (kefir_size_t) typeentry->param; i++) {
        REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, info->visitor, (void *) info, index + 1, 1));
    }
    info->nested--;
    return KEFIR_OK;
}

static kefir_result_t assign_nested_builtin(const struct kefir_ir_type *type, kefir_size_t index,
                                            const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    struct recursive_aggregate_allocation *info = (struct recursive_aggregate_allocation *) payload;
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(info->allocation, (*info->slot)++));
    kefir_ir_builtin_type_t builtin = (kefir_ir_builtin_type_t) typeentry->param;
    allocation->type = KEFIR_AMD64_SYSV_INPUT_PARAM_NESTED;
    allocation->klass = KEFIR_AMD64_SYSV_PARAM_NO_CLASS;
    allocation->index = index;
    switch (builtin) {
        case KEFIR_IR_TYPE_BUILTIN_VARARG:
            REQUIRE_OK(kefir_abi_sysv_amd64_qwords_next(&info->top_allocation->container,
                                                        KEFIR_AMD64_SYSV_PARAM_INTEGER, KEFIR_AMD64_SYSV_ABI_QWORD,
                                                        KEFIR_AMD64_SYSV_ABI_QWORD, &allocation->container_reference));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unknown built-in type");
    }
    return KEFIR_OK;
}

static kefir_result_t aggregate_disown(struct kefir_mem *mem, struct kefir_abi_sysv_amd64_parameter_allocation *alloc) {
    REQUIRE_OK(kefir_abi_sysv_amd64_qwords_free(&alloc->container, mem));
    alloc->type = KEFIR_AMD64_SYSV_INPUT_PARAM_CONTAINER;
    alloc->klass = KEFIR_AMD64_SYSV_PARAM_MEMORY;
    return KEFIR_OK;
}

static kefir_result_t aggregate_postmerger(struct kefir_mem *mem,
                                           struct kefir_abi_sysv_amd64_parameter_allocation *alloc) {
    const kefir_size_t length = kefir_vector_length(&alloc->container.qwords);
    kefir_size_t previous_x87 = KEFIR_SIZE_MAX;
    kefir_size_t ssecount = 0;
    bool had_nonsseup = false;
    for (kefir_size_t i = 0; i < length; i++) {
        ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_qword *, qword, kefir_vector_at(&alloc->container.qwords, i));
        had_nonsseup = had_nonsseup || (i > 0 && qword->klass != KEFIR_AMD64_SYSV_PARAM_SSEUP);
        if (qword->klass == KEFIR_AMD64_SYSV_PARAM_MEMORY) {
            return aggregate_disown(mem, alloc);
        } else if (qword->klass == KEFIR_AMD64_SYSV_PARAM_X87) {
            previous_x87 = i;
        } else if (qword->klass == KEFIR_AMD64_SYSV_PARAM_X87UP) {
            if (i == 0 || i - 1 != previous_x87) {
                return aggregate_disown(mem, alloc);
            }
        } else if (qword->klass == KEFIR_AMD64_SYSV_PARAM_SSE) {
            ssecount++;
        } else if (qword->klass == KEFIR_AMD64_SYSV_PARAM_SSEUP) {
            if (ssecount == 0) {
                qword->klass = KEFIR_AMD64_SYSV_PARAM_SSE;
            } else {
                ssecount--;
            }
        }
    }
    if (length > 2 && (((struct kefir_abi_sysv_amd64_qword *) kefir_vector_at(&alloc->container.qwords, 0))->klass !=
                           KEFIR_AMD64_SYSV_PARAM_SSE ||
                       had_nonsseup)) {
        return aggregate_disown(mem, alloc);
    }
    return KEFIR_OK;
}

static kefir_result_t nested_visitor_init(struct kefir_ir_type_visitor *visitor) {
    kefir_ir_type_visitor_init(visitor, visitor_not_supported);
    KEFIR_IR_TYPE_VISITOR_INIT_SCALARS(visitor, assign_nested_scalar);
    visitor->visit[KEFIR_IR_TYPE_LONG_DOUBLE] = assign_nested_long_double;
    visitor->visit[KEFIR_IR_TYPE_STRUCT] = assign_nested_struct;
    visitor->visit[KEFIR_IR_TYPE_ARRAY] = assign_nested_array;
    visitor->visit[KEFIR_IR_TYPE_UNION] = assign_nested_union;
    visitor->visit[KEFIR_IR_TYPE_BUILTIN] = assign_nested_builtin;
    return KEFIR_OK;
}

static kefir_result_t immediate_struct_unwrap(struct kefir_mem *mem, const struct kefir_ir_type *type,
                                              kefir_size_t index, const struct kefir_ir_typeentry *typeentry,
                                              const struct kefir_abi_sysv_amd64_typeentry_layout *top_layout,
                                              const struct kefir_abi_sysv_amd64_type_layout *layout,
                                              struct kefir_vector *allocation, kefir_size_t *slot,
                                              struct kefir_abi_sysv_amd64_parameter_allocation *top_allocation) {
    kefir_size_t qword_count = top_layout->size / KEFIR_AMD64_SYSV_ABI_QWORD;
    if (top_layout->size % KEFIR_AMD64_SYSV_ABI_QWORD != 0) {
        qword_count++;
    }
    top_allocation->type = KEFIR_AMD64_SYSV_INPUT_PARAM_OWNING_CONTAINER;
    top_allocation->klass = KEFIR_AMD64_SYSV_PARAM_NO_CLASS;
    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(nested_visitor_init(&visitor));
    REQUIRE_OK(kefir_abi_sysv_amd64_qwords_alloc(&top_allocation->container, mem, qword_count));
    struct recursive_aggregate_allocation info = {.type = type,
                                                  .layout = layout,
                                                  .allocation = allocation,
                                                  .top_allocation = top_allocation,
                                                  .visitor = &visitor,
                                                  .slot = slot,
                                                  .nested = 0};
    kefir_result_t res = kefir_ir_type_visitor_list_nodes(type, &visitor, (void *) &info, index + 1, typeentry->param);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_sysv_amd64_qwords_free(&top_allocation->container, mem);
        return res;
    });
    return aggregate_postmerger(mem, top_allocation);
}

static kefir_result_t calculate_qword_requirements(struct kefir_abi_sysv_amd64_parameter_allocation *allocation,
                                                   const struct kefir_abi_sysv_amd64_typeentry_layout *layout) {
    allocation->requirements = (const struct kefir_abi_sysv_amd64_parameter_location_requirements){0};
    if (allocation->klass == KEFIR_AMD64_SYSV_PARAM_MEMORY) {
        allocation->requirements.memory.size = layout->size;
        allocation->requirements.memory.alignment = layout->alignment;
    } else {
        for (kefir_size_t i = 0; i < kefir_vector_length(&allocation->container.qwords); i++) {
            ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_qword *, qword,
                             kefir_vector_at(&allocation->container.qwords, i));
            switch (qword->klass) {
                case KEFIR_AMD64_SYSV_PARAM_INTEGER:
                    allocation->requirements.integer++;
                    break;

                case KEFIR_AMD64_SYSV_PARAM_SSE:
                    allocation->requirements.sse++;
                    break;

                case KEFIR_AMD64_SYSV_PARAM_SSEUP:
                    allocation->requirements.sseup++;
                    break;

                case KEFIR_AMD64_SYSV_PARAM_NO_CLASS:
                    break;

                case KEFIR_AMD64_SYSV_PARAM_X87:
                    REQUIRE(i + 1 < kefir_vector_length(&allocation->container.qwords),
                            KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                            "Expected X87 qword to be directly followed by X87UP qword"));
                    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_qword *, next_qword,
                                     kefir_vector_at(&allocation->container.qwords, ++i));
                    REQUIRE(next_qword->klass == KEFIR_AMD64_SYSV_PARAM_X87UP,
                            KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                            "Expected X87 qword to be directly followed by X87UP qword"));
                    allocation->requirements.memory.size += 2 * KEFIR_AMD64_SYSV_ABI_QWORD;
                    allocation->requirements.memory.alignment =
                        MAX(allocation->requirements.memory.alignment, 2 * KEFIR_AMD64_SYSV_ABI_QWORD);
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter class in QWord vector");
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t assign_immediate_struct(const struct kefir_ir_type *type, kefir_size_t index,
                                              const struct kefir_ir_typeentry *typeentry, void *payload) {
    struct input_allocation *info = (struct input_allocation *) payload;
    const struct kefir_abi_sysv_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_at(info->layout, index, &layout));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(info->allocation, info->slot++));
    allocation->type = KEFIR_AMD64_SYSV_INPUT_PARAM_CONTAINER;
    allocation->index = index;

    if (layout->size > 8 * KEFIR_AMD64_SYSV_ABI_QWORD || !layout->aligned) {
        allocation->klass = KEFIR_AMD64_SYSV_PARAM_MEMORY;
        info->slot += kefir_ir_type_node_slots(type, index) - 1;
    } else {
        REQUIRE_OK(immediate_struct_unwrap(info->mem, type, index, typeentry, layout, info->layout, info->allocation,
                                           &info->slot, allocation));
    }
    return calculate_qword_requirements(allocation, layout);
}

static kefir_result_t immediate_array_unwrap(struct kefir_mem *mem, const struct kefir_ir_type *type,
                                             kefir_size_t index, const struct kefir_ir_typeentry *typeentry,
                                             const struct kefir_abi_sysv_amd64_typeentry_layout *top_layout,
                                             const struct kefir_abi_sysv_amd64_type_layout *layout,
                                             struct kefir_vector *allocation, kefir_size_t *slot,
                                             struct kefir_abi_sysv_amd64_parameter_allocation *top_allocation) {
    UNUSED(typeentry);
    kefir_size_t qword_count = top_layout->size / KEFIR_AMD64_SYSV_ABI_QWORD;
    if (top_layout->size % KEFIR_AMD64_SYSV_ABI_QWORD != 0) {
        qword_count++;
    }
    top_allocation->type = KEFIR_AMD64_SYSV_INPUT_PARAM_OWNING_CONTAINER;
    top_allocation->klass = KEFIR_AMD64_SYSV_PARAM_NO_CLASS;
    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(nested_visitor_init(&visitor));
    REQUIRE_OK(kefir_abi_sysv_amd64_qwords_alloc(&top_allocation->container, mem, qword_count));
    struct recursive_aggregate_allocation info = {.type = type,
                                                  .layout = layout,
                                                  .allocation = allocation,
                                                  .top_allocation = top_allocation,
                                                  .visitor = &visitor,
                                                  .slot = slot,
                                                  .nested = 0};
    for (kefir_size_t i = 0; i < (kefir_size_t) typeentry->param; i++) {
        kefir_result_t res = kefir_ir_type_visitor_list_nodes(type, &visitor, (void *) &info, index + 1, 1);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_abi_sysv_amd64_qwords_free(&top_allocation->container, mem);
            return res;
        });
    }
    return aggregate_postmerger(mem, top_allocation);
}

static kefir_result_t assign_immediate_array(const struct kefir_ir_type *type, kefir_size_t index,
                                             const struct kefir_ir_typeentry *typeentry, void *payload) {
    struct input_allocation *info = (struct input_allocation *) payload;
    const struct kefir_abi_sysv_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_at(info->layout, index, &layout));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(info->allocation, info->slot++));
    allocation->type = KEFIR_AMD64_SYSV_INPUT_PARAM_CONTAINER;
    allocation->index = index;

    if (layout->size > 8 * KEFIR_AMD64_SYSV_ABI_QWORD || !layout->aligned) {
        allocation->klass = KEFIR_AMD64_SYSV_PARAM_MEMORY;
        info->slot += kefir_ir_type_node_slots(type, index) - 1;
    } else {
        immediate_array_unwrap(info->mem, type, index, typeentry, layout, info->layout, info->allocation, &info->slot,
                               allocation);
    }
    return calculate_qword_requirements(allocation, layout);
}

static kefir_result_t immediate_union_unwrap(struct kefir_mem *mem, const struct kefir_ir_type *type,
                                             kefir_size_t index, const struct kefir_ir_typeentry *typeentry,
                                             const struct kefir_abi_sysv_amd64_typeentry_layout *top_layout,
                                             const struct kefir_abi_sysv_amd64_type_layout *layout,
                                             struct kefir_vector *allocation, kefir_size_t *slot,
                                             struct kefir_abi_sysv_amd64_parameter_allocation *top_allocation) {
    UNUSED(typeentry);
    kefir_size_t qword_count = top_layout->size / KEFIR_AMD64_SYSV_ABI_QWORD;
    if (top_layout->size % KEFIR_AMD64_SYSV_ABI_QWORD != 0) {
        qword_count++;
    }
    top_allocation->type = KEFIR_AMD64_SYSV_INPUT_PARAM_OWNING_CONTAINER;
    top_allocation->klass = KEFIR_AMD64_SYSV_PARAM_NO_CLASS;
    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(nested_visitor_init(&visitor));
    visitor.posthook = union_reset_hook;
    REQUIRE_OK(kefir_abi_sysv_amd64_qwords_alloc(&top_allocation->container, mem, qword_count));
    struct recursive_aggregate_allocation info = {.type = type,
                                                  .layout = layout,
                                                  .allocation = allocation,
                                                  .top_allocation = top_allocation,
                                                  .visitor = &visitor,
                                                  .slot = slot,
                                                  .nested = 0};
    REQUIRE_OK(kefir_abi_sysv_amd64_qwords_save_position(&top_allocation->container, &info.init_position));
    REQUIRE_OK(kefir_abi_sysv_amd64_qwords_save_position(&top_allocation->container, &info.max_position));
    kefir_result_t res = kefir_ir_type_visitor_list_nodes(type, &visitor, (void *) &info, index + 1, typeentry->param);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_sysv_amd64_qwords_free(&top_allocation->container, mem);
        return res;
    });
    REQUIRE_OK(kefir_abi_sysv_amd64_qwords_restore_position(&top_allocation->container, &info.max_position));
    return aggregate_postmerger(mem, top_allocation);
}

static kefir_result_t assign_immediate_union(const struct kefir_ir_type *type, kefir_size_t index,
                                             const struct kefir_ir_typeentry *typeentry, void *payload) {
    struct input_allocation *info = (struct input_allocation *) payload;
    const struct kefir_abi_sysv_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_at(info->layout, index, &layout));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(info->allocation, info->slot++));
    allocation->type = KEFIR_AMD64_SYSV_INPUT_PARAM_CONTAINER;
    allocation->index = index;

    if (layout->size > 8 * KEFIR_AMD64_SYSV_ABI_QWORD || !layout->aligned) {
        allocation->klass = KEFIR_AMD64_SYSV_PARAM_MEMORY;
        info->slot += kefir_ir_type_node_slots(type, index) - 1;
    } else {
        immediate_union_unwrap(info->mem, type, index, typeentry, layout, info->layout, info->allocation, &info->slot,
                               allocation);
    }
    return calculate_qword_requirements(allocation, layout);
}

static kefir_result_t assign_immediate_builtin(const struct kefir_ir_type *type, kefir_size_t index,
                                               const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(typeentry);
    struct input_allocation *info = (struct input_allocation *) payload;
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(info->allocation, info->slot++));
    kefir_ir_builtin_type_t builtin = (kefir_ir_builtin_type_t) typeentry->param;
    allocation->index = index;
    switch (builtin) {
        case KEFIR_IR_TYPE_BUILTIN_VARARG:
            allocation->type = KEFIR_AMD64_SYSV_INPUT_PARAM_IMMEDIATE;
            allocation->klass = KEFIR_AMD64_SYSV_PARAM_INTEGER;
            allocation->requirements.integer = 1;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unknown built-in type");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_abi_sysv_amd64_parameter_classify(struct kefir_mem *mem, const struct kefir_ir_type *type,
                                                       const struct kefir_abi_sysv_amd64_type_layout *layout,
                                                       struct kefir_vector *allocation) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type definition"));
    REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid System-V AMD64 ABI type layout"));
    REQUIRE(allocation != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid allocation vector"));
    kefir_size_t slots = kefir_ir_type_total_slots(type);
    REQUIRE_OK(kefir_vector_alloc(mem, sizeof(struct kefir_abi_sysv_amd64_parameter_allocation), slots, allocation));
    kefir_result_t res = kefir_vector_extend(allocation, slots);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_vector_free(mem, allocation);
        return res;
    });
    for (kefir_size_t i = 0; i < slots; i++) {
        ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, entry, kefir_vector_at(allocation, i));
        entry->type = KEFIR_AMD64_SYSV_INPUT_PARAM_SKIP;
        entry->klass = KEFIR_AMD64_SYSV_PARAM_NO_CLASS;
        entry->index = 0;
        entry->requirements = (const struct kefir_abi_sysv_amd64_parameter_location_requirements){0};
        entry->location.integer_register = KEFIR_AMD64_SYSV_PARAMETER_LOCATION_NONE;
        entry->location.sse_register = KEFIR_AMD64_SYSV_PARAMETER_LOCATION_NONE;
        entry->location.stack_offset = KEFIR_AMD64_SYSV_PARAMETER_LOCATION_NONE;
    }
    struct kefir_ir_type_visitor visitor;
    kefir_ir_type_visitor_init(&visitor, visitor_not_supported);
    KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS(&visitor, assign_immediate_integer);
    KEFIR_IR_TYPE_VISITOR_INIT_FIXED_FP(&visitor, assign_immediate_sse);
    visitor.visit[KEFIR_IR_TYPE_LONG_DOUBLE] = assign_immediate_long_double;
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = assign_immediate_struct;
    visitor.visit[KEFIR_IR_TYPE_UNION] = assign_immediate_union;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = assign_immediate_array;
    visitor.visit[KEFIR_IR_TYPE_BUILTIN] = assign_immediate_builtin;
    struct input_allocation info = {.mem = mem, .layout = layout, .allocation = allocation};
    res = kefir_ir_type_visitor_list_nodes(type, &visitor, (void *) &info, 0, kefir_ir_type_nodes(type));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_vector_free(mem, allocation);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_abi_sysv_amd64_parameter_free(struct kefir_mem *mem, struct kefir_vector *allocation) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(allocation != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid allocation vector"));
    for (kefir_size_t i = 0; i < kefir_vector_length(allocation); i++) {
        ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, alloc, kefir_vector_at(allocation, i));
        if (alloc->type == KEFIR_AMD64_SYSV_INPUT_PARAM_OWNING_CONTAINER) {
            REQUIRE_OK(kefir_abi_sysv_amd64_qwords_free(&alloc->container, mem));
        }
    }
    REQUIRE_OK(kefir_vector_free(mem, allocation));
    return KEFIR_OK;
}

struct allocation_state {
    struct kefir_mem *mem;
    struct kefir_abi_sysv_amd64_parameter_location *current;
    const struct kefir_abi_sysv_amd64_type_layout *layout;
    struct kefir_vector *allocation;
};

static kefir_result_t integer_allocate(const struct kefir_ir_type *type, kefir_size_t index,
                                       const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(index);
    UNUSED(typeentry);
    struct allocation_state *state = (struct allocation_state *) payload;
    struct kefir_ir_type_iterator iter;
    REQUIRE_OK(kefir_ir_type_iterator_init(type, &iter));
    REQUIRE_OK(kefir_ir_type_iterator_goto(&iter, index));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, alloc,
                     kefir_vector_at(state->allocation, iter.slot));
    REQUIRE(alloc->requirements.integer == 1 && alloc->requirements.sse == 0 && alloc->requirements.sseup == 0 &&
                alloc->requirements.memory.size == 0,
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected INTEGER to require exactly 1 int eightbyte"));
    if (state->current->integer_register + 1 <= ABI_INTEGER_REGS) {
        alloc->location.integer_register = state->current->integer_register++;
    } else {
        const kefir_size_t alignment = MAX(alloc->requirements.memory.alignment, KEFIR_AMD64_SYSV_ABI_QWORD);
        state->current->stack_offset = kefir_target_abi_pad_aligned(state->current->stack_offset, alignment);
        alloc->klass = KEFIR_AMD64_SYSV_PARAM_MEMORY;
        alloc->location.stack_offset = state->current->stack_offset;
        state->current->stack_offset += KEFIR_AMD64_SYSV_ABI_QWORD;
    }
    return KEFIR_OK;
}

static kefir_result_t sse_allocate(const struct kefir_ir_type *type, kefir_size_t index,
                                   const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(index);
    UNUSED(typeentry);
    struct allocation_state *state = (struct allocation_state *) payload;
    struct kefir_ir_type_iterator iter;
    REQUIRE_OK(kefir_ir_type_iterator_init(type, &iter));
    REQUIRE_OK(kefir_ir_type_iterator_goto(&iter, index));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, alloc,
                     kefir_vector_at(state->allocation, iter.slot));
    REQUIRE(alloc->requirements.integer == 0 && alloc->requirements.sse == 1 && alloc->requirements.sseup == 0 &&
                alloc->requirements.memory.size == 0,
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected SSE to require exactly 1 sse eightbyte"));
    if (state->current->sse_register + 1 <= ABI_SSE_REGS) {
        alloc->location.sse_register = state->current->sse_register++;
    } else {
        const kefir_size_t alignment = MAX(alloc->requirements.memory.alignment, KEFIR_AMD64_SYSV_ABI_QWORD);
        state->current->stack_offset = kefir_target_abi_pad_aligned(state->current->stack_offset, alignment);
        alloc->klass = KEFIR_AMD64_SYSV_PARAM_MEMORY;
        alloc->location.stack_offset = state->current->stack_offset;
        state->current->stack_offset += KEFIR_AMD64_SYSV_ABI_QWORD;
    }
    return KEFIR_OK;
}

static kefir_result_t long_double_allocate(const struct kefir_ir_type *type, kefir_size_t index,
                                           const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(index);
    UNUSED(typeentry);
    struct allocation_state *state = (struct allocation_state *) payload;
    struct kefir_ir_type_iterator iter;
    REQUIRE_OK(kefir_ir_type_iterator_init(type, &iter));
    REQUIRE_OK(kefir_ir_type_iterator_goto(&iter, index));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, alloc,
                     kefir_vector_at(state->allocation, iter.slot));

    const kefir_size_t alignment = MAX(alloc->requirements.memory.alignment, KEFIR_AMD64_SYSV_ABI_QWORD * 2);
    state->current->stack_offset = kefir_target_abi_pad_aligned(state->current->stack_offset, alignment);
    alloc->klass = KEFIR_AMD64_SYSV_PARAM_MEMORY;
    alloc->location.stack_offset = state->current->stack_offset;
    state->current->stack_offset += KEFIR_AMD64_SYSV_ABI_QWORD * 2;
    return KEFIR_OK;
}

static bool aggregate_register_allocate(const struct kefir_abi_sysv_amd64_parameter_allocation *alloc,
                                        const struct kefir_abi_sysv_amd64_parameter_location *location) {
    return alloc->type == KEFIR_AMD64_SYSV_INPUT_PARAM_OWNING_CONTAINER &&
           alloc->klass == KEFIR_AMD64_SYSV_PARAM_NO_CLASS &&
           location->integer_register + alloc->requirements.integer <= ABI_INTEGER_REGS &&
           location->sse_register + alloc->requirements.sse <= ABI_SSE_REGS && alloc->requirements.memory.size == 0;
}

static kefir_result_t aggregate_allocate(const struct kefir_ir_type *type, kefir_size_t index,
                                         const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(index);
    UNUSED(typeentry);
    struct allocation_state *state = (struct allocation_state *) payload;
    struct kefir_ir_type_iterator iter;
    REQUIRE_OK(kefir_ir_type_iterator_init(type, &iter));
    REQUIRE_OK(kefir_ir_type_iterator_goto(&iter, index));
    const struct kefir_abi_sysv_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_at(state->layout, index, &layout));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, alloc,
                     kefir_vector_at(state->allocation, iter.slot));
    if (aggregate_register_allocate(alloc, state->current)) {
        if (alloc->requirements.integer > 0) {
            alloc->location.integer_register = state->current->integer_register;
            state->current->integer_register += alloc->requirements.integer;
        }
        if (alloc->requirements.sse > 0) {
            alloc->location.sse_register = state->current->sse_register;
            state->current->sse_register += alloc->requirements.sse;
        }

        kefir_size_t integer_location = alloc->location.integer_register;
        kefir_size_t sse_location = alloc->location.sse_register;
        for (kefir_size_t i = 0; i < kefir_vector_length(&alloc->container.qwords); i++) {
            ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_qword *, qword, kefir_vector_at(&alloc->container.qwords, i));
            switch (qword->klass) {
                case KEFIR_AMD64_SYSV_PARAM_INTEGER:
                    qword->location = integer_location++;
                    break;

                case KEFIR_AMD64_SYSV_PARAM_SSE:
                    qword->location = sse_location++;
                    break;

                case KEFIR_AMD64_SYSV_PARAM_SSEUP:
                    qword->location = sse_location;
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected QWord class");
            }
        }
    } else {
        if (alloc->type == KEFIR_AMD64_SYSV_INPUT_PARAM_OWNING_CONTAINER) {
            REQUIRE_OK(aggregate_disown(state->mem, alloc));
        }
        const kefir_size_t alignment = MAX(layout->alignment, KEFIR_AMD64_SYSV_ABI_QWORD);
        state->current->stack_offset = kefir_target_abi_pad_aligned(state->current->stack_offset, alignment);
        alloc->location.stack_offset = state->current->stack_offset;
        state->current->stack_offset += layout->size;
    }
    return KEFIR_OK;
}

static kefir_result_t builtin_allocate(const struct kefir_ir_type *type, kefir_size_t index,
                                       const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    struct allocation_state *state = (struct allocation_state *) payload;
    struct kefir_ir_type_iterator iter;
    REQUIRE_OK(kefir_ir_type_iterator_init(type, &iter));
    REQUIRE_OK(kefir_ir_type_iterator_goto(&iter, index));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, alloc,
                     kefir_vector_at(state->allocation, iter.slot));
    kefir_ir_builtin_type_t builtin = (kefir_ir_builtin_type_t) typeentry->param;

    switch (builtin) {
        case KEFIR_IR_TYPE_BUILTIN_VARARG:
            if (state->current->integer_register + 1 <= ABI_INTEGER_REGS) {
                alloc->location.integer_register = state->current->integer_register++;
            } else {
                const kefir_size_t alignment = MAX(alloc->requirements.memory.alignment, KEFIR_AMD64_SYSV_ABI_QWORD);
                state->current->stack_offset = kefir_target_abi_pad_aligned(state->current->stack_offset, alignment);
                alloc->klass = KEFIR_AMD64_SYSV_PARAM_MEMORY;
                alloc->location.stack_offset = state->current->stack_offset;
                state->current->stack_offset += KEFIR_AMD64_SYSV_ABI_QWORD;
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unknown built-in type");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_abi_sysv_amd64_parameter_allocate(struct kefir_mem *mem, const struct kefir_ir_type *type,
                                                       const struct kefir_abi_sysv_amd64_type_layout *layout,
                                                       struct kefir_vector *allocation,
                                                       struct kefir_abi_sysv_amd64_parameter_location *location) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type layout"));
    REQUIRE(allocation != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type allocation"));
    REQUIRE(location != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parameter location pointer"));

    struct kefir_ir_type_visitor visitor;
    kefir_ir_type_visitor_init(&visitor, visitor_not_supported);
    KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS(&visitor, integer_allocate);
    KEFIR_IR_TYPE_VISITOR_INIT_FIXED_FP(&visitor, sse_allocate);
    visitor.visit[KEFIR_IR_TYPE_LONG_DOUBLE] = long_double_allocate;
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = aggregate_allocate;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = aggregate_allocate;
    visitor.visit[KEFIR_IR_TYPE_UNION] = aggregate_allocate;
    visitor.visit[KEFIR_IR_TYPE_BUILTIN] = builtin_allocate;
    struct allocation_state state = {.mem = mem, .current = location, .layout = layout, .allocation = allocation};
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, &visitor, (void *) &state, 0, kefir_ir_type_nodes(type)));
    return KEFIR_OK;
}
