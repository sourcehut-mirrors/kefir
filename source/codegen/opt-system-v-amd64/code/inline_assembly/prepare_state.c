/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#include "kefir/codegen/opt-system-v-amd64/inline_assembly.h"
#include "kefir/codegen/opt-system-v-amd64/function.h"
#include "kefir/codegen/opt-system-v-amd64/storage_transform.h"
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t preserve_dirty_registers(struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
                                               kefir_size_t *preserved_regs_area_size) {
    *preserved_regs_area_size = 0;
    struct kefir_hashtreeset_iterator iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(&context->dirty_registers, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t, reg, iter.entry);

        kefir_bool_t occupied;
        REQUIRE_OK(
            kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(&context->codegen_func->storage, reg, &occupied));
        if (!occupied) {
            continue;
        }

        if (!kefir_asm_amd64_xasmgen_register_is_floating_point(reg)) {
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_PUSH(&context->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg)));
        } else {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
                &context->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                kefir_asm_amd64_xasmgen_operand_imm(&context->codegen->xasmgen_helpers.operands[0],
                                                    KEFIR_AMD64_ABI_QWORD)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                &context->codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &context->codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0),
                kefir_asm_amd64_xasmgen_operand_reg(reg)));
        }
        *preserved_regs_area_size += KEFIR_AMD64_ABI_QWORD;
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    if (context->dirty_cc) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSHFQ(&context->codegen->xasmgen));
        *preserved_regs_area_size += KEFIR_AMD64_ABI_QWORD;
    }
    return KEFIR_OK;
}

static kefir_result_t preserve_output_addresses(struct kefir_mem *mem,
                                                struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
                                                struct kefir_codegen_opt_amd64_sysv_storage_transform *transform) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&context->ir_inline_assembly->parameter_list);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, ir_asm_param, iter->value);

        struct kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry *entry =
            &context->parameter_allocation[ir_asm_param->parameter_id];
        if (entry->output_address.preserved) {
            struct kefir_opt_inline_assembly_parameter *asm_param = NULL;
            REQUIRE_OK(kefir_opt_code_container_inline_assembly_parameter(
                &context->function->code, context->inline_assembly->node_id, ir_asm_param->parameter_id, &asm_param));

            const struct kefir_codegen_opt_sysv_amd64_register_allocation *load_store_allocation = NULL;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
                &context->codegen_func->register_allocator, asm_param->load_store_ref, &load_store_allocation));

            struct kefir_codegen_opt_amd64_sysv_storage_location source_location;
            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_from_reg_allocation(
                &source_location, &context->codegen_func->stack_frame_map, load_store_allocation));

            struct kefir_codegen_opt_amd64_sysv_storage_location target_location = {
                .type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY,
                .memory = {.base_reg = KEFIR_AMD64_XASMGEN_REGISTER_RSP,
                           .offset = entry->output_address.stack_index * KEFIR_AMD64_ABI_QWORD +
                                     context->stack_map.output_parameter_offset}};
            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_insert(mem, transform, &target_location,
                                                                             &source_location));
        }
    }
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_perform(
        mem, context->codegen, &context->codegen_func->storage, &context->codegen_func->stack_frame_map, transform));
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_reset(mem, transform));
    return KEFIR_OK;
}

static kefir_result_t parameter_allocation_to_location(
    struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
    const struct kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry *entry,
    struct kefir_codegen_opt_amd64_sysv_storage_location *location) {
    switch (entry->allocation_type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER:
            location->type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER;
            location->reg = entry->allocation.reg;
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT:
            location->type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER;
            location->reg = entry->allocation.indirect.reg;
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_STACK:
            location->type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY;
            location->memory.base_reg = KEFIR_AMD64_XASMGEN_REGISTER_RSP;
            location->memory.offset =
                context->stack_map.input_parameter_offset + entry->allocation.stack.index * KEFIR_AMD64_ABI_QWORD;
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t load_inputs(struct kefir_mem *mem,
                                  struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
                                  struct kefir_codegen_opt_amd64_sysv_storage_transform *transform) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&context->ir_inline_assembly->parameter_list);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, ir_asm_param, iter->value);
        struct kefir_opt_inline_assembly_parameter *asm_param = NULL;
        REQUIRE_OK(kefir_opt_code_container_inline_assembly_parameter(
            &context->function->code, context->inline_assembly->node_id, ir_asm_param->parameter_id, &asm_param));
        struct kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry *entry =
            &context->parameter_allocation[ir_asm_param->parameter_id];

        if (ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE ||
            (ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE &&
             entry->allocation_type !=
                 KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT)) {
            continue;
        }

        struct kefir_codegen_opt_amd64_sysv_storage_location source_location;
        struct kefir_codegen_opt_amd64_sysv_storage_location target_location;
        switch (ir_asm_param->klass) {
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE: {
                const struct kefir_codegen_opt_sysv_amd64_register_allocation *load_store_allocation = NULL;
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
                    &context->codegen_func->register_allocator, asm_param->load_store_ref, &load_store_allocation));
                REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_from_reg_allocation(
                    &source_location, &context->codegen_func->stack_frame_map, load_store_allocation));
            } break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE: {
                const struct kefir_codegen_opt_sysv_amd64_register_allocation *read_allocation = NULL;
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
                    &context->codegen_func->register_allocator, asm_param->read_ref, &read_allocation));
                REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_from_reg_allocation(
                    &source_location, &context->codegen_func->stack_frame_map, read_allocation));
            } break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE:
                // Intentionally left blank
                break;
        }
        REQUIRE_OK(parameter_allocation_to_location(context, entry, &target_location));
        REQUIRE_OK(
            kefir_codegen_opt_amd64_sysv_storage_transform_insert(mem, transform, &target_location, &source_location));
    }
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_perform(
        mem, context->codegen, &context->codegen_func->storage, &context->codegen_func->stack_frame_map, transform));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_inline_assembly_match_register_to_size(
    kefir_asm_amd64_xasmgen_register_t in, kefir_size_t sz, kefir_asm_amd64_xasmgen_register_t *out) {
    if (sz <= 1) {
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(in, out));
    } else if (sz <= 2) {
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register16(in, out));
    } else if (sz <= 4) {
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(in, out));
    } else if (sz <= 8) {
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register64(in, out));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to match register variant wider than 8 bytes");
    }
    return KEFIR_OK;
}

static kefir_result_t read_inputs(struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&context->ir_inline_assembly->parameter_list);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, ir_asm_param, iter->value);
        struct kefir_opt_inline_assembly_parameter *asm_param = NULL;
        REQUIRE_OK(kefir_opt_code_container_inline_assembly_parameter(
            &context->function->code, context->inline_assembly->node_id, ir_asm_param->parameter_id, &asm_param));
        struct kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry *entry =
            &context->parameter_allocation[ir_asm_param->parameter_id];

        if (ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE ||
            ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE) {
            continue;
        }

        const struct kefir_ir_typeentry *param_type = NULL;
        switch (ir_asm_param->klass) {
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE:
                param_type = kefir_ir_type_at(ir_asm_param->read_type.type, ir_asm_param->read_type.index);
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE:
                param_type = kefir_ir_type_at(ir_asm_param->type.type, ir_asm_param->type.index);
                break;
        }
        REQUIRE(param_type != NULL,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain IR inline assembly parameter type"));

        switch (param_type->typecode) {
            case KEFIR_IR_TYPE_INT8:
            case KEFIR_IR_TYPE_INT16:
            case KEFIR_IR_TYPE_INT32:
            case KEFIR_IR_TYPE_INT64:
            case KEFIR_IR_TYPE_BOOL:
            case KEFIR_IR_TYPE_CHAR:
            case KEFIR_IR_TYPE_SHORT:
            case KEFIR_IR_TYPE_INT:
            case KEFIR_IR_TYPE_LONG:
            case KEFIR_IR_TYPE_WORD:
            case KEFIR_IR_TYPE_FLOAT32:
            case KEFIR_IR_TYPE_FLOAT64:
                switch (entry->allocation_type) {
                    case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER:
                        if (!entry->direct_value) {
                            kefir_asm_amd64_xasmgen_register_t reg;
                            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_inline_assembly_match_register_to_size(
                                entry->allocation.reg, entry->parameter_props.size, &reg));
                            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                                &context->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                                kefir_asm_amd64_xasmgen_operand_indirect(
                                    &context->codegen->xasmgen_helpers.operands[0],
                                    kefir_asm_amd64_xasmgen_operand_reg(entry->allocation.reg), 0)));
                        }
                        break;

                    case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT:
                        REQUIRE(
                            !entry->direct_value,
                            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter properties"));
                        break;

                    case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_STACK: {
                        if (!entry->direct_value) {
                            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                                &context->codegen->xasmgen,
                                kefir_asm_amd64_xasmgen_operand_reg(context->stack_input_parameters.base_register),
                                kefir_asm_amd64_xasmgen_operand_indirect(
                                    &context->codegen->xasmgen_helpers.operands[0],
                                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                    context->stack_map.input_parameter_offset +
                                        entry->allocation.stack.index * KEFIR_AMD64_ABI_QWORD)));

                            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                                &context->codegen->xasmgen,
                                kefir_asm_amd64_xasmgen_operand_reg(context->stack_input_parameters.base_register),
                                kefir_asm_amd64_xasmgen_operand_indirect(
                                    &context->codegen->xasmgen_helpers.operands[0],
                                    kefir_asm_amd64_xasmgen_operand_reg(context->stack_input_parameters.base_register),
                                    0)));

                            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                                &context->codegen->xasmgen,
                                kefir_asm_amd64_xasmgen_operand_indirect(
                                    &context->codegen->xasmgen_helpers.operands[0],
                                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                    context->stack_map.input_parameter_offset +
                                        entry->allocation.stack.index * KEFIR_AMD64_ABI_QWORD),
                                kefir_asm_amd64_xasmgen_operand_reg(context->stack_input_parameters.base_register)));
                        }
                    } break;
                }
                break;

            case KEFIR_IR_TYPE_LONG_DOUBLE:
            case KEFIR_IR_TYPE_STRUCT:
            case KEFIR_IR_TYPE_ARRAY:
            case KEFIR_IR_TYPE_UNION:
                switch (entry->allocation_type) {
                    case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER: {
                        kefir_asm_amd64_xasmgen_register_t reg;
                        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_inline_assembly_match_register_to_size(
                            entry->allocation.reg,
                            entry->direct_value ? MIN(entry->parameter_read_props.size, KEFIR_AMD64_ABI_QWORD)
                                                : entry->parameter_props.size,
                            &reg));

                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &context->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                            kefir_asm_amd64_xasmgen_operand_indirect(
                                &context->codegen->xasmgen_helpers.operands[0],
                                kefir_asm_amd64_xasmgen_operand_reg(entry->allocation.reg), 0)));

                    } break;

                    case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT:
                        // Intentionally left blank
                        break;

                    case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_STACK:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                               "On-stack aggregate parameters of IR inline assembly are not supported");
                }
                break;

            case KEFIR_IR_TYPE_BITS:
            case KEFIR_IR_TYPE_BUILTIN:
            case KEFIR_IR_TYPE_COUNT:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter type");
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_inline_assembly_prepare_state(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen inline assembly context"));

    kefir_size_t preserved_regs_area_size;
    kefir_size_t output_parameters_area_size = context->stack_output_parameters.count * KEFIR_AMD64_ABI_QWORD;
    kefir_size_t input_parameters_area_size = context->stack_input_parameters.count * KEFIR_AMD64_ABI_QWORD;

    REQUIRE_OK(preserve_dirty_registers(context, &preserved_regs_area_size));

    context->stack_map.total_size = kefir_target_abi_pad_aligned(
        preserved_regs_area_size + output_parameters_area_size + input_parameters_area_size, 2 * KEFIR_AMD64_ABI_QWORD);
    context->stack_map.input_parameter_offset = 0;
    context->stack_map.output_parameter_offset = input_parameters_area_size;
    context->stack_map.preserved_reg_offset = context->stack_map.total_size - preserved_regs_area_size;
    context->stack_map.preserved_reg_size = preserved_regs_area_size;

    if (context->stack_map.preserved_reg_offset > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
            &context->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_imm(&context->codegen->xasmgen_helpers.operands[0],
                                                context->stack_map.preserved_reg_offset)));
    }

    struct kefir_codegen_opt_amd64_sysv_storage_transform transform;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_init(&transform));
    kefir_result_t res = preserve_output_addresses(mem, context, &transform);
    REQUIRE_CHAIN(&res, load_inputs(mem, context, &transform));
    REQUIRE_ELSE(res == KEFIR_OK, { kefir_codegen_opt_amd64_sysv_storage_transform_free(mem, &transform); });
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_free(mem, &transform));

    REQUIRE_OK(read_inputs(context));

    if (context->stack_input_parameters.initialized) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &context->codegen->xasmgen,
            kefir_asm_amd64_xasmgen_operand_reg(context->stack_input_parameters.base_register),
            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP)));
    }
    return KEFIR_OK;
}
