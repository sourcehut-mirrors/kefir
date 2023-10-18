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
#include "kefir/codegen/opt-system-v-amd64/storage.h"
#include "kefir/codegen/opt-system-v-amd64/function.h"
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t update_scratch_reg(
    struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
    struct kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry *entry,
    kefir_asm_amd64_xasmgen_register_t *scratch_reg, kefir_bool_t *pushed_scratch) {
    if (*scratch_reg == entry->allocation.reg) {
        if (*pushed_scratch) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&context->codegen->xasmgen,
                                                     kefir_asm_amd64_xasmgen_operand_reg(*scratch_reg)));
        }

        if (entry->allocation.reg == KEFIR_AMD64_XASMGEN_REGISTER_RAX) {
            if (context->stack_input_parameters.initialized &&
                context->stack_input_parameters.base_register == KEFIR_AMD64_XASMGEN_REGISTER_RCX) {
                *scratch_reg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
            } else {
                *scratch_reg = KEFIR_AMD64_XASMGEN_REGISTER_RCX;
            }
        } else {
            if (context->stack_input_parameters.initialized &&
                context->stack_input_parameters.base_register == KEFIR_AMD64_XASMGEN_REGISTER_RAX) {
                *scratch_reg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
            } else {
                *scratch_reg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            }
        }
    }

    kefir_bool_t scratch_reg_occupied;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(&context->codegen_func->storage, *scratch_reg,
                                                                         &scratch_reg_occupied));
    if (!*pushed_scratch && (scratch_reg_occupied || kefir_hashtreeset_has(&context->dirty_registers,
                                                                           (kefir_hashtreeset_entry_t) *scratch_reg))) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&context->codegen->xasmgen,
                                                  kefir_asm_amd64_xasmgen_operand_reg(*scratch_reg)));
        *pushed_scratch = true;
    }

    return KEFIR_OK;
}

static kefir_result_t store_register_aggregate_outputs(
    struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
    kefir_asm_amd64_xasmgen_register_t *scratch_reg, kefir_bool_t *pushed_scratch) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&context->ir_inline_assembly->parameter_list);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, ir_asm_param, iter->value);

        struct kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry *entry =
            &context->parameter_allocation[ir_asm_param->parameter_id];

        if (!entry->register_aggregate || ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ ||
            ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD ||
            ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE) {
            continue;
        }

        REQUIRE_OK(update_scratch_reg(context, entry, scratch_reg, pushed_scratch));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &context->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(*scratch_reg),
            kefir_asm_amd64_xasmgen_operand_indirect(
                &context->codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                context->stack_map.output_parameter_offset + KEFIR_AMD64_ABI_QWORD * entry->output_address.stack_index +
                    (*pushed_scratch ? KEFIR_AMD64_ABI_QWORD : 0))));

        kefir_asm_amd64_xasmgen_register_t reg;
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_inline_assembly_match_register_to_size(
            entry->allocation.reg, entry->parameter_props.size, &reg));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &context->codegen->xasmgen,
            kefir_asm_amd64_xasmgen_operand_indirect(&context->codegen->xasmgen_helpers.operands[0],
                                                     kefir_asm_amd64_xasmgen_operand_reg(*scratch_reg), 0),
            kefir_asm_amd64_xasmgen_operand_reg(reg)));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_inline_assembly_store_outputs(
    struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context) {
    REQUIRE(context != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen inline assembly context"));

    kefir_asm_amd64_xasmgen_register_t scratch_reg;
    kefir_bool_t pushed_scratch = false;
    if (kefir_list_length(&context->available_registers)) {
        scratch_reg =
            (kefir_asm_amd64_xasmgen_register_t) ((kefir_uptr_t) kefir_list_head(&context->available_registers)->value);
    } else if (context->stack_input_parameters.initialized) {
        scratch_reg = context->stack_input_parameters.base_register == KEFIR_AMD64_XASMGEN_REGISTER_RAX
                          ? KEFIR_AMD64_XASMGEN_REGISTER_RDX
                          : KEFIR_AMD64_XASMGEN_REGISTER_RAX;
    } else {
        scratch_reg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&context->ir_inline_assembly->parameter_list);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, ir_asm_param, iter->value);

        if (ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ ||
            ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD ||
            ir_asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE) {
            continue;
        }

        struct kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry *entry =
            &context->parameter_allocation[ir_asm_param->parameter_id];

        const struct kefir_ir_typeentry *param_type =
            kefir_ir_type_at(ir_asm_param->type.type, ir_asm_param->type.index);
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
                    case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER: {
                        REQUIRE_OK(update_scratch_reg(context, entry, &scratch_reg, &pushed_scratch));

                        kefir_asm_amd64_xasmgen_register_t reg;
                        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_inline_assembly_match_register_to_size(
                            entry->allocation.reg, entry->parameter_props.size, &reg));

                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &context->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(scratch_reg),
                            kefir_asm_amd64_xasmgen_operand_indirect(
                                &context->codegen->xasmgen_helpers.operands[0],
                                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                context->stack_map.output_parameter_offset +
                                    KEFIR_AMD64_ABI_QWORD * entry->output_address.stack_index +
                                    (pushed_scratch ? KEFIR_AMD64_ABI_QWORD : 0))));

                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &context->codegen->xasmgen,
                            kefir_asm_amd64_xasmgen_operand_indirect(&context->codegen->xasmgen_helpers.operands[0],
                                                                     kefir_asm_amd64_xasmgen_operand_reg(scratch_reg),
                                                                     0),
                            kefir_asm_amd64_xasmgen_operand_reg(reg)));
                    } break;

                    case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT:
                        REQUIRE(
                            !entry->direct_value,
                            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter properties"));
                        break;

                    case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_STACK:
                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &context->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(scratch_reg),
                            kefir_asm_amd64_xasmgen_operand_indirect(
                                &context->codegen->xasmgen_helpers.operands[0],
                                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                context->stack_map.output_parameter_offset +
                                    KEFIR_AMD64_ABI_QWORD * entry->output_address.stack_index +
                                    (pushed_scratch ? KEFIR_AMD64_ABI_QWORD : 0))));

                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &context->codegen->xasmgen,
                            kefir_asm_amd64_xasmgen_operand_reg(context->stack_input_parameters.base_register),
                            kefir_asm_amd64_xasmgen_operand_indirect(
                                &context->codegen->xasmgen_helpers.operands[0],
                                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                context->stack_map.input_parameter_offset +
                                    KEFIR_AMD64_ABI_QWORD * entry->allocation.stack.index +
                                    (pushed_scratch ? KEFIR_AMD64_ABI_QWORD : 0))));

                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &context->codegen->xasmgen,
                            kefir_asm_amd64_xasmgen_operand_indirect(&context->codegen->xasmgen_helpers.operands[0],
                                                                     kefir_asm_amd64_xasmgen_operand_reg(scratch_reg),
                                                                     0),
                            kefir_asm_amd64_xasmgen_operand_reg(context->stack_input_parameters.base_register)));
                        break;
                }
                break;

            case KEFIR_IR_TYPE_LONG_DOUBLE:
            case KEFIR_IR_TYPE_STRUCT:
            case KEFIR_IR_TYPE_ARRAY:
            case KEFIR_IR_TYPE_UNION:
                switch (entry->allocation_type) {
                    case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER:
                    case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT:
                        // Intentionally left blank
                        break;

                    case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_STACK:
                        return KEFIR_SET_ERROR(
                            KEFIR_INVALID_STATE,
                            "On-stack aggregate parameters of IR inline assembly are not supported yet");
                }
                break;

            case KEFIR_IR_TYPE_BITS:
            case KEFIR_IR_TYPE_BUILTIN:
            case KEFIR_IR_TYPE_COUNT:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter type");
        }
    }

    REQUIRE_OK(store_register_aggregate_outputs(context, &scratch_reg, &pushed_scratch));
    if (pushed_scratch) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&context->codegen->xasmgen,
                                                 kefir_asm_amd64_xasmgen_operand_reg(scratch_reg)));
    }
    return KEFIR_OK;
}
