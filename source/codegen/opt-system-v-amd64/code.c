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

#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_CODE_INTERNAL
#include "kefir/codegen/opt-system-v-amd64/code.h"
#include "kefir/codegen/opt-system-v-amd64/runtime.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

const struct kefir_asm_amd64_xasmgen_operand *kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
    struct kefir_asm_amd64_xasmgen_operand *operand,
    struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation) {
    REQUIRE(operand != NULL, NULL);
    REQUIRE(stack_frame_map != NULL, NULL);
    REQUIRE(reg_allocation != NULL, NULL);

    switch (reg_allocation->result.type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
            return NULL;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
            return kefir_asm_amd64_xasmgen_operand_reg(reg_allocation->result.reg);

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
            return kefir_asm_amd64_xasmgen_operand_indirect(
                operand, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                stack_frame_map->offset.spill_area + reg_allocation->result.spill.index * KEFIR_AMD64_SYSV_ABI_QWORD);

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT:
            return kefir_asm_amd64_xasmgen_operand_indirect(
                operand, kefir_asm_amd64_xasmgen_operand_reg(reg_allocation->result.indirect.base_register),
                reg_allocation->result.indirect.offset);
    }
    return NULL;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_load_reg_allocation(
    struct kefir_codegen_opt_amd64 *codegen, struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation,
    kefir_asm_amd64_xasmgen_register_t target_reg) {
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen"));
    REQUIRE(stack_frame_map != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen stack frame map"));
    REQUIRE(reg_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocation"));

    REQUIRE(
        !((reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER ||
           reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER) &&
          reg_allocation->result.reg == target_reg),
        KEFIR_OK);

    switch (reg_allocation->result.type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer codegen register allocation type");

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT:
            if (!kefir_asm_amd64_xasmgen_register_is_floating_point(target_reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_reg),
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_reg),
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation)));
            }
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
            if (kefir_asm_amd64_xasmgen_register_is_floating_point(target_reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVDQU(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_reg),
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation)));
            } else if (kefir_asm_amd64_xasmgen_register_is_wide(target_reg, 64)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_reg),
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation)));
            } else if (kefir_asm_amd64_xasmgen_register_is_wide(target_reg, 32)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVD(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(target_reg),
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation)));
            } else {
                return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                                       "Unable to load floating-point register into 8/16-bit register");
            }
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_store_reg_allocation(
    struct kefir_codegen_opt_amd64 *codegen, struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation,
    kefir_asm_amd64_xasmgen_register_t source_reg) {
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen"));
    REQUIRE(stack_frame_map != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen stack frame map"));
    REQUIRE(reg_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocation"));

    REQUIRE(
        !((reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER ||
           reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER) &&
          reg_allocation->result.reg == source_reg),
        KEFIR_OK);

    switch (reg_allocation->result.type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer codegen register allocation type");

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT:
            if (!kefir_asm_amd64_xasmgen_register_is_floating_point(source_reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation),
                    kefir_asm_amd64_xasmgen_operand_reg(source_reg)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    &codegen->xasmgen,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation),
                    kefir_asm_amd64_xasmgen_operand_reg(source_reg)));
            }
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
            if (kefir_asm_amd64_xasmgen_register_is_floating_point(source_reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVDQU(
                    &codegen->xasmgen,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation),
                    kefir_asm_amd64_xasmgen_operand_reg(source_reg)));
            } else if (kefir_asm_amd64_xasmgen_register_is_wide(source_reg, 64)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    &codegen->xasmgen,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation),
                    kefir_asm_amd64_xasmgen_operand_reg(source_reg)));
            } else if (kefir_asm_amd64_xasmgen_register_is_wide(source_reg, 32)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVD(
                    &codegen->xasmgen,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                        stack_frame_map, reg_allocation),
                    kefir_asm_amd64_xasmgen_operand_reg(source_reg)));
            } else {
                return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                                       "Unable to store 8/16-bit register into floating-point register");
            }
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t collect_dead_registers(struct kefir_mem *mem, const struct kefir_opt_code_analysis *func_analysis,
                                             struct kefir_opt_sysv_amd64_function *codegen_func,
                                             const struct kefir_opt_code_analysis_instruction_properties *instr_props) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&codegen_func->alive_instr); iter != NULL;) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, (kefir_uptr_t) iter->value);

        const struct kefir_opt_instruction_liveness_interval *liveness =
            &func_analysis->liveness.intervals[func_analysis->instructions[instr_ref].linear_position];
        if (liveness->range.end <= instr_props->linear_position + 1) {
            const struct kefir_codegen_opt_sysv_amd64_register_allocation *instr_reg_allocation = NULL;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                           &instr_reg_allocation));

            if (instr_reg_allocation->result.type ==
                    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER ||
                instr_reg_allocation->result.type ==
                    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER) {
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_mark_register_unused(mem, &codegen_func->storage,
                                                                                     instr_reg_allocation->result.reg));
            }

            const struct kefir_list_entry *next_iter = iter->next;
            REQUIRE_OK(kefir_list_pop(mem, &codegen_func->alive_instr, (struct kefir_list_entry *) iter));
            iter = next_iter;
        } else {
            kefir_list_next(&iter);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t update_live_registers(
    struct kefir_mem *mem, struct kefir_opt_sysv_amd64_function *codegen_func,
    const struct kefir_opt_code_analysis_instruction_properties *instr_props,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation) {

    if (reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER ||
        reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER) {
        REQUIRE_OK(kefir_list_insert_after(mem, &codegen_func->alive_instr, kefir_list_tail(&codegen_func->alive_instr),
                                           (void *) (kefir_uptr_t) instr_props->instr_ref));

        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_mark_register_used(mem, &codegen_func->storage,
                                                                           reg_allocation->result.reg));
    }
    if (reg_allocation->result.register_aggregate_allocation != NULL) {
        for (kefir_size_t i = 0;
             i < kefir_vector_length(&reg_allocation->result.register_aggregate_allocation->container.qwords); i++) {
            ASSIGN_DECL_CAST(
                struct kefir_abi_sysv_amd64_qword *, qword,
                kefir_vector_at(&reg_allocation->result.register_aggregate_allocation->container.qwords, i));
            switch (qword->klass) {
                case KEFIR_AMD64_SYSV_PARAM_INTEGER:
                    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_mark_register_used(
                        mem, &codegen_func->storage,
                        KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTERS[qword->location]));
                    break;

                case KEFIR_AMD64_SYSV_PARAM_SSE:
                    // Intentionally left blank
                    break;

                case KEFIR_AMD64_SYSV_PARAM_NO_CLASS:
                    // Intentionally left blank
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                           "Aggregates with non-INTEGER and non-SSE members are not supported yet");
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_instr(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                      struct kefir_opt_module *module, const struct kefir_opt_function *function,
                                      const struct kefir_opt_code_analysis *func_analysis,
                                      struct kefir_opt_sysv_amd64_function *codegen_func,
                                      const struct kefir_opt_code_analysis_instruction_properties *instr_props) {
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator,
                                                                   instr_props->instr_ref, &reg_allocation));

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_props->instr_ref, &instr));

    REQUIRE_OK(collect_dead_registers(mem, func_analysis, codegen_func, instr_props));

#define INVOKE_TRANSLATOR(_id)                                                                                 \
    (kefir_codegen_opt_sysv_amd64_translate_##_id(mem, codegen, module, function, func_analysis, codegen_func, \
                                                  instr_props->instr_ref))

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_GET_ARGUMENT:
            REQUIRE_OK(INVOKE_TRANSLATOR(get_argument));
            break;

        case KEFIR_OPT_OPCODE_GET_LOCAL:
        case KEFIR_OPT_OPCODE_GET_GLOBAL:
            REQUIRE_OK(INVOKE_TRANSLATOR(data_access));
            break;

        case KEFIR_OPT_OPCODE_INT8_STORE:
        case KEFIR_OPT_OPCODE_INT16_STORE:
        case KEFIR_OPT_OPCODE_INT32_STORE:
        case KEFIR_OPT_OPCODE_INT64_STORE:
            REQUIRE_OK(INVOKE_TRANSLATOR(store));
            break;

        case KEFIR_OPT_OPCODE_INT8_LOAD_UNSIGNED:
        case KEFIR_OPT_OPCODE_INT8_LOAD_SIGNED:
        case KEFIR_OPT_OPCODE_INT16_LOAD_UNSIGNED:
        case KEFIR_OPT_OPCODE_INT16_LOAD_SIGNED:
        case KEFIR_OPT_OPCODE_INT32_LOAD_UNSIGNED:
        case KEFIR_OPT_OPCODE_INT32_LOAD_SIGNED:
        case KEFIR_OPT_OPCODE_INT64_LOAD:
            REQUIRE_OK(INVOKE_TRANSLATOR(load));
            break;

        case KEFIR_OPT_OPCODE_INT_CONST:
        case KEFIR_OPT_OPCODE_UINT_CONST:
        case KEFIR_OPT_OPCODE_FLOAT32_CONST:
        case KEFIR_OPT_OPCODE_FLOAT64_CONST:
        case KEFIR_OPT_OPCODE_STRING_REF:
        case KEFIR_OPT_OPCODE_BLOCK_LABEL:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST:
            REQUIRE_OK(INVOKE_TRANSLATOR(constant));
            break;

        case KEFIR_OPT_OPCODE_INT_ADD:
        case KEFIR_OPT_OPCODE_INT_SUB:
        case KEFIR_OPT_OPCODE_INT_MUL:
        case KEFIR_OPT_OPCODE_INT_AND:
        case KEFIR_OPT_OPCODE_INT_OR:
        case KEFIR_OPT_OPCODE_INT_XOR:
        case KEFIR_OPT_OPCODE_BOOL_AND:
        case KEFIR_OPT_OPCODE_BOOL_OR:
            REQUIRE_OK(INVOKE_TRANSLATOR(binary_op));
            break;

        case KEFIR_OPT_OPCODE_INT_DIV:
        case KEFIR_OPT_OPCODE_INT_MOD:
        case KEFIR_OPT_OPCODE_UINT_DIV:
        case KEFIR_OPT_OPCODE_UINT_MOD:
            REQUIRE_OK(INVOKE_TRANSLATOR(div_mod));
            break;

        case KEFIR_OPT_OPCODE_INT_LSHIFT:
        case KEFIR_OPT_OPCODE_INT_RSHIFT:
        case KEFIR_OPT_OPCODE_INT_ARSHIFT:
            REQUIRE_OK(INVOKE_TRANSLATOR(bitshift));
            break;

        case KEFIR_OPT_OPCODE_INT_NOT:
        case KEFIR_OPT_OPCODE_INT_NEG:
        case KEFIR_OPT_OPCODE_BOOL_NOT:
            REQUIRE_OK(INVOKE_TRANSLATOR(unary_op));
            break;

        case KEFIR_OPT_OPCODE_INT64_TRUNCATE_1BIT:
        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS:
        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS:
        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS:
        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS:
        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS:
        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS:
            REQUIRE_OK(INVOKE_TRANSLATOR(int_conv));
            break;

        case KEFIR_OPT_OPCODE_INT_EQUALS:
        case KEFIR_OPT_OPCODE_INT_GREATER:
        case KEFIR_OPT_OPCODE_INT_GREATER_OR_EQUALS:
        case KEFIR_OPT_OPCODE_INT_LESSER:
        case KEFIR_OPT_OPCODE_INT_LESSER_OR_EQUALS:
        case KEFIR_OPT_OPCODE_INT_ABOVE:
        case KEFIR_OPT_OPCODE_INT_ABOVE_OR_EQUALS:
        case KEFIR_OPT_OPCODE_INT_BELOW:
        case KEFIR_OPT_OPCODE_INT_BELOW_OR_EQUALS:
            REQUIRE_OK(INVOKE_TRANSLATOR(comparison));
            break;

        case KEFIR_OPT_OPCODE_RETURN:
            REQUIRE_OK(INVOKE_TRANSLATOR(return));
            break;

        case KEFIR_OPT_OPCODE_JUMP:
        case KEFIR_OPT_OPCODE_BRANCH:
        case KEFIR_OPT_OPCODE_COMPARE_BRANCH:
        case KEFIR_OPT_OPCODE_IJUMP:
            REQUIRE_OK(INVOKE_TRANSLATOR(jump));
            break;

        case KEFIR_OPT_OPCODE_COPY_MEMORY:
            REQUIRE_OK(INVOKE_TRANSLATOR(copy_memory));
            break;

        case KEFIR_OPT_OPCODE_ZERO_MEMORY:
            REQUIRE_OK(INVOKE_TRANSLATOR(zero_memory));
            break;

        case KEFIR_OPT_OPCODE_INVOKE:
        case KEFIR_OPT_OPCODE_INVOKE_VIRTUAL:
            REQUIRE_OK(INVOKE_TRANSLATOR(invoke));
            break;

        case KEFIR_OPT_OPCODE_GET_THREAD_LOCAL:
            REQUIRE_OK(INVOKE_TRANSLATOR(thread_local_storage));
            break;

        case KEFIR_OPT_OPCODE_BITS_EXTRACT_SIGNED:
        case KEFIR_OPT_OPCODE_BITS_EXTRACT_UNSIGNED:
            REQUIRE_OK(INVOKE_TRANSLATOR(extract_bits));
            break;

        case KEFIR_OPT_OPCODE_BITS_INSERT:
            REQUIRE_OK(INVOKE_TRANSLATOR(insert_bits));
            break;

        case KEFIR_OPT_OPCODE_SCOPE_PUSH:
            REQUIRE_OK(INVOKE_TRANSLATOR(push_scope));
            break;

        case KEFIR_OPT_OPCODE_SCOPE_POP:
            REQUIRE_OK(INVOKE_TRANSLATOR(pop_scope));
            break;

        case KEFIR_OPT_OPCODE_STACK_ALLOC:
            REQUIRE_OK(INVOKE_TRANSLATOR(stack_alloc));
            break;

        case KEFIR_OPT_OPCODE_INT_TO_FLOAT32:
        case KEFIR_OPT_OPCODE_INT_TO_FLOAT64:
        case KEFIR_OPT_OPCODE_UINT_TO_FLOAT32:
        case KEFIR_OPT_OPCODE_UINT_TO_FLOAT64:
        case KEFIR_OPT_OPCODE_FLOAT32_TO_INT:
        case KEFIR_OPT_OPCODE_FLOAT64_TO_INT:
        case KEFIR_OPT_OPCODE_FLOAT32_TO_FLOAT64:
        case KEFIR_OPT_OPCODE_FLOAT64_TO_FLOAT32:
            REQUIRE_OK(INVOKE_TRANSLATOR(float_conv));
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_TO_UINT:
        case KEFIR_OPT_OPCODE_FLOAT64_TO_UINT:
            REQUIRE_OK(INVOKE_TRANSLATOR(float_to_uint_conv));
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_ADD:
        case KEFIR_OPT_OPCODE_FLOAT32_SUB:
        case KEFIR_OPT_OPCODE_FLOAT32_MUL:
        case KEFIR_OPT_OPCODE_FLOAT32_DIV:
        case KEFIR_OPT_OPCODE_FLOAT64_ADD:
        case KEFIR_OPT_OPCODE_FLOAT64_SUB:
        case KEFIR_OPT_OPCODE_FLOAT64_MUL:
        case KEFIR_OPT_OPCODE_FLOAT64_DIV:
            REQUIRE_OK(INVOKE_TRANSLATOR(float_binary_op));
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_NEG:
        case KEFIR_OPT_OPCODE_FLOAT64_NEG:
            REQUIRE_OK(INVOKE_TRANSLATOR(float_unary_op));
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_EQUALS:
        case KEFIR_OPT_OPCODE_FLOAT64_EQUALS:
        case KEFIR_OPT_OPCODE_FLOAT32_GREATER:
        case KEFIR_OPT_OPCODE_FLOAT32_GREATER_OR_EQUALS:
        case KEFIR_OPT_OPCODE_FLOAT64_GREATER:
        case KEFIR_OPT_OPCODE_FLOAT64_GREATER_OR_EQUALS:
        case KEFIR_OPT_OPCODE_FLOAT32_LESSER:
        case KEFIR_OPT_OPCODE_FLOAT32_LESSER_OR_EQUALS:
        case KEFIR_OPT_OPCODE_FLOAT64_LESSER:
        case KEFIR_OPT_OPCODE_FLOAT64_LESSER_OR_EQUALS:
            REQUIRE_OK(INVOKE_TRANSLATOR(float_comparison));
            break;

        case KEFIR_OPT_OPCODE_VARARG_START:
            REQUIRE_OK(INVOKE_TRANSLATOR(vararg_start));
            break;

        case KEFIR_OPT_OPCODE_VARARG_COPY:
            REQUIRE_OK(INVOKE_TRANSLATOR(vararg_copy));
            break;

        case KEFIR_OPT_OPCODE_VARARG_END:
            // Intentionally left blank
            break;

        case KEFIR_OPT_OPCODE_VARARG_GET:
            REQUIRE_OK(INVOKE_TRANSLATOR(vararg_get));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_ADD:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_SUB:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_MUL:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_DIV:
            REQUIRE_OK(INVOKE_TRANSLATOR(long_double_binary_op));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_NEG:
            REQUIRE_OK(INVOKE_TRANSLATOR(long_double_unary_op));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
            REQUIRE_OK(INVOKE_TRANSLATOR(long_double_store));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_EQUALS:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_GREATER:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_LESSER:
            REQUIRE_OK(INVOKE_TRANSLATOR(long_double_comparison));
            break;

        case KEFIR_OPT_OPCODE_INT_TO_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_UINT_TO_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_FLOAT32_TO_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_FLOAT64_TO_LONG_DOUBLE:
            REQUIRE_OK(INVOKE_TRANSLATOR(long_double_conversion_to));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_INT:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_UINT:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_FLOAT32:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_FLOAT64:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TRUNCATE_1BIT:
            REQUIRE_OK(INVOKE_TRANSLATOR(long_double_conversion_from));
            break;

        case KEFIR_OPT_OPCODE_PHI:
            // Intentionally left blank
            break;

        case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY:
            REQUIRE_OK(INVOKE_TRANSLATOR(inline_assembly));
            break;
    }
#undef INVOKE_TRANSLATOR

    if (instr->operation.opcode != KEFIR_OPT_OPCODE_GET_ARGUMENT) {
        REQUIRE_OK(update_live_registers(mem, codegen_func, instr_props, reg_allocation));
    }

    kefir_bool_t has_borrowed_regs;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_has_borrowed_registers(&codegen_func->storage, &has_borrowed_regs));
    REQUIRE(!has_borrowed_regs,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected set of borrowed general purpose registers to be empty"));
    return KEFIR_OK;
}

static kefir_result_t update_frame_temporaries(struct kefir_opt_sysv_amd64_function *codegen_func,
                                               struct kefir_abi_amd64_sysv_function_decl *decl) {
    if (kefir_ir_type_children(decl->decl->result) == 0) {
        return KEFIR_OK;
    }

    struct kefir_ir_typeentry *typeentry = kefir_ir_type_at(decl->decl->result, 0);
    if (typeentry->typecode == KEFIR_IR_TYPE_STRUCT || typeentry->typecode == KEFIR_IR_TYPE_UNION ||
        typeentry->typecode == KEFIR_IR_TYPE_ARRAY || typeentry->typecode == KEFIR_IR_TYPE_BUILTIN ||
        typeentry->typecode == KEFIR_IR_TYPE_LONG_DOUBLE) {
        const struct kefir_abi_sysv_amd64_typeentry_layout *layout = NULL;
        REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_at(&decl->returns.layout, 0, &layout));
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_stack_frame_ensure_temporary(&codegen_func->stack_frame, layout->size,
                                                                             layout->alignment));
    }
    return KEFIR_OK;
}

static kefir_result_t update_frame_temporaries_type(struct kefir_mem *mem,
                                                    struct kefir_opt_sysv_amd64_function *codegen_func,
                                                    struct kefir_ir_type *type, kefir_size_t index) {
    struct kefir_abi_sysv_amd64_type_layout layout;
    struct kefir_vector allocation;
    struct kefir_ir_typeentry *typeentry = kefir_ir_type_at(type, index);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Unable to fetch IR type entry at index"));

    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout(type, mem, &layout));
    kefir_result_t res = kefir_abi_sysv_amd64_parameter_classify(mem, type, &layout, &allocation);
    REQUIRE_ELSE(res == KEFIR_OK, {
        REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_free(mem, &layout));
        return res;
    });

    const struct kefir_abi_sysv_amd64_typeentry_layout *arg_layout = NULL;
    const struct kefir_abi_sysv_amd64_parameter_allocation *arg_alloc = NULL;
    REQUIRE_CHAIN(&res, kefir_abi_sysv_amd64_type_layout_at(&layout, index, &arg_layout));
    REQUIRE_ELSE(res == KEFIR_OK, {
        REQUIRE_OK(kefir_abi_sysv_amd64_parameter_free(mem, &allocation));
        REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_free(mem, &layout));
        return res;
    });
    arg_alloc = kefir_vector_at(&allocation, index);
    REQUIRE_ELSE(arg_alloc != NULL, {
        REQUIRE_OK(kefir_abi_sysv_amd64_parameter_free(mem, &allocation));
        REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_free(mem, &layout));
        return KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Unable to fetch argument layout and classification");
    });

    kefir_size_t size = 0;
    kefir_size_t alignment = 0;
    if ((typeentry->typecode == KEFIR_IR_TYPE_STRUCT || typeentry->typecode == KEFIR_IR_TYPE_UNION ||
         typeentry->typecode == KEFIR_IR_TYPE_ARRAY || typeentry->typecode == KEFIR_IR_TYPE_BUILTIN) &&
        arg_alloc->klass != KEFIR_AMD64_SYSV_PARAM_MEMORY) {
        size = arg_layout->size;
        alignment = arg_layout->alignment;
    }

    REQUIRE_OK(kefir_abi_sysv_amd64_parameter_free(mem, &allocation));
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_free(mem, &layout));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_stack_frame_ensure_temporary(&codegen_func->stack_frame, size, alignment));
    return KEFIR_OK;
}

static kefir_result_t calculate_frame_temporaries(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                                  const struct kefir_opt_function *function,
                                                  const struct kefir_opt_code_analysis *func_analysis,
                                                  struct kefir_opt_sysv_amd64_function *codegen_func) {

    for (kefir_size_t instr_idx = 0; instr_idx < func_analysis->linearization_length; instr_idx++) {
        const struct kefir_opt_code_analysis_instruction_properties *instr_props =
            func_analysis->linearization[instr_idx];

        struct kefir_opt_instruction *instr = NULL;
        REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_props->instr_ref, &instr));

        if (instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE ||
            instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE_VIRTUAL) {
            struct kefir_opt_call_node *call_node = NULL;
            REQUIRE_OK(kefir_opt_code_container_call(&function->code,
                                                     instr->operation.parameters.function_call.call_ref, &call_node));

            const struct kefir_ir_function_decl *ir_func_decl =
                kefir_ir_module_get_declaration(module->ir_module, call_node->function_declaration_id);
            REQUIRE(ir_func_decl != NULL,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR function declaration"));

            struct kefir_abi_amd64_sysv_function_decl abi_func_decl;
            REQUIRE_OK(kefir_abi_amd64_sysv_function_decl_alloc(mem, ir_func_decl, &abi_func_decl));

            kefir_result_t res = update_frame_temporaries(codegen_func, &abi_func_decl);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_abi_amd64_sysv_function_decl_free(mem, &abi_func_decl);
                return res;
            });

            REQUIRE_OK(kefir_abi_amd64_sysv_function_decl_free(mem, &abi_func_decl));
        } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_VARARG_GET) {
            struct kefir_ir_type *type =
                kefir_ir_module_get_named_type(module->ir_module, instr->operation.parameters.typed_refs.type_id);
            REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unable to find named type"));
            REQUIRE_OK(update_frame_temporaries_type(mem, codegen_func, type,
                                                     instr->operation.parameters.typed_refs.type_index));
        } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_SCOPE_PUSH ||
                   instr->operation.opcode == KEFIR_OPT_OPCODE_SCOPE_POP ||
                   instr->operation.opcode == KEFIR_OPT_OPCODE_STACK_ALLOC) {
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_stack_frame_enable_dynamic_scope(&codegen_func->stack_frame));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t init_translator(struct kefir_mem *mem, struct kefir_opt_sysv_amd64_function *codegen_func) {
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_mark_register_used(mem, &codegen_func->storage,
                                                                       KEFIR_AMD64_XASMGEN_REGISTER_RBP));
    for (kefir_size_t i = 0; i < KefirCodegenOptSysvAmd64StackFrameNumOfPreservedRegs; i++) {
        kefir_bool_t preserved;
        REQUIRE_OK(kefir_bitset_get(&codegen_func->stack_frame.preserve.regs, i, &preserved));
        if (!preserved) {
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_mark_register_used(
                mem, &codegen_func->storage, KefirCodegenOptSysvAmd64StackFramePreservedRegs[i]));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t generate_constants(struct kefir_codegen_opt_amd64 *codegen,
                                         const struct kefir_opt_function *function,
                                         const struct kefir_opt_code_analysis *func_analysis) {
    kefir_bool_t f32neg = false;
    kefir_bool_t f64neg = false;
    kefir_bool_t uint_to_ld = false;

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".rodata"));

    for (kefir_size_t instr_idx = 0; instr_idx < func_analysis->linearization_length; instr_idx++) {
        const struct kefir_opt_code_analysis_instruction_properties *instr_props =
            func_analysis->linearization[instr_idx];
        struct kefir_opt_instruction *instr = NULL;
        REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_props->instr_ref, &instr));

        switch (instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_FLOAT32_CONST: {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 4));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen,
                                                     KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_CONSTANT_LABEL,
                                                     function->ir_func->name, instr_props->instr_ref));

                union {
                    kefir_uint32_t u32;
                    kefir_float32_t f32;
                } value = {.f32 = instr->operation.parameters.imm.float32};

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                    kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], value.u32)));
            } break;

            case KEFIR_OPT_OPCODE_FLOAT64_CONST: {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 8));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen,
                                                     KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_CONSTANT_LABEL,
                                                     function->ir_func->name, instr_props->instr_ref));

                union {
                    kefir_uint64_t u64;
                    kefir_float64_t f64;
                } value = {.f64 = instr->operation.parameters.imm.float64};

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                    kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], value.u64)));
            } break;

            case KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST: {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 16));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen,
                                                     KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_CONSTANT_LABEL,
                                                     function->ir_func->name, instr_props->instr_ref));

                volatile union {
                    kefir_uint64_t u64[2];
                    kefir_long_double_t ld;
                } value = {.u64 = {0, 0}};
                value.ld = instr->operation.parameters.imm.long_double.value;

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                    kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], value.u64[0])));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                    kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], value.u64[1])));

            } break;

            case KEFIR_OPT_OPCODE_FLOAT32_NEG:
                f32neg = true;
                break;

            case KEFIR_OPT_OPCODE_FLOAT64_NEG:
                f64neg = true;
                break;

            case KEFIR_OPT_OPCODE_UINT_TO_LONG_DOUBLE:
                uint_to_ld = true;
                break;

            default:
                // Intentionally left blank
                break;
        }
    }

    if (f32neg) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_CONSTANT_FLOAT32_NEG,
                                             function->ir_func->name));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 0x80000000ull)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 0x80000000ull)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 0x80000000ull)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 0x80000000ull)));
    }

    if (f64neg) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 16));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_CONSTANT_FLOAT64_NEG,
                                             function->ir_func->name));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 0x8000000000000000ull)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 0x8000000000000000ull)));
    }

    if (uint_to_ld) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 4));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_CONSTANT_UINT_TO_LD,
                                             function->ir_func->name));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 0x5F800000)));
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".text"));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_translate_code(struct kefir_mem *mem,
                                                           struct kefir_codegen_opt_amd64 *codegen,
                                                           struct kefir_opt_module *module,
                                                           const struct kefir_opt_function *function,
                                                           const struct kefir_opt_code_analysis *func_analysis,
                                                           struct kefir_opt_sysv_amd64_function *codegen_func) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));
    REQUIRE(func_analysis != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function analysis"));
    REQUIRE(codegen_func != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen function"));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", function->ir_func->name));
    if (function->ir_func->locals != NULL) {
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_stack_frame_allocate_set_locals(
            &codegen_func->stack_frame, function->ir_func->locals, &codegen_func->locals_layout));
    }
    if (codegen_func->declaration.returns.implicit_parameter) {
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_stack_frame_preserve_implicit_parameter(&codegen_func->stack_frame));
    }
    if (function->ir_func->declaration->vararg) {
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_stack_frame_save_registers(&codegen_func->stack_frame));
    }
    REQUIRE_OK(calculate_frame_temporaries(mem, module, function, func_analysis, codegen_func));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_stack_frame_prologue(&codegen_func->stack_frame, &codegen->xasmgen,
                                                                 codegen->config->position_independent_code));

    REQUIRE_OK(
        kefir_codegen_opt_sysv_amd64_stack_frame_compute(&codegen_func->stack_frame, &codegen_func->stack_frame_map));

    REQUIRE_OK(init_translator(mem, codegen_func));

    kefir_bool_t scan_arguments = true;
    for (kefir_size_t block_idx = 0; scan_arguments && block_idx < func_analysis->block_linearization_length;
         block_idx++) {
        const struct kefir_opt_code_analysis_block_properties *block_props =
            func_analysis->block_linearization[block_idx];

        for (kefir_size_t instr_idx = block_props->linear_range.begin_index;
             scan_arguments && instr_idx < block_props->linear_range.end_index; instr_idx++) {

            const struct kefir_opt_code_analysis_instruction_properties *instr_props =
                func_analysis->linearization[instr_idx];
            struct kefir_opt_instruction *instr = NULL;
            REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_props->instr_ref, &instr));
            if (instr->operation.opcode == KEFIR_OPT_OPCODE_GET_ARGUMENT) {
                const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
                    &codegen_func->register_allocator, instr_props->instr_ref, &reg_allocation));
                REQUIRE_OK(update_live_registers(mem, codegen_func, instr_props, reg_allocation));
            } else {
                scan_arguments = false;
            }
        }
    }

    for (kefir_size_t block_idx = 0; block_idx < func_analysis->block_linearization_length; block_idx++) {
        const struct kefir_opt_code_analysis_block_properties *block_props =
            func_analysis->block_linearization[block_idx];
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK,
                                             function->ir_func->name, block_props->block_id));

        for (kefir_size_t instr_idx = block_props->linear_range.begin_index;
             instr_idx < block_props->linear_range.end_index; instr_idx++) {
            REQUIRE_OK(translate_instr(mem, codegen, module, function, func_analysis, codegen_func,
                                       func_analysis->linearization[instr_idx]));
        }
    }

    REQUIRE_OK(generate_constants(codegen, function, func_analysis));
    return KEFIR_OK;
}
