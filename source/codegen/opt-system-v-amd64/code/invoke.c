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

#include "kefir/codegen/opt-system-v-amd64/code_impl.h"
#include "kefir/codegen/opt-system-v-amd64/storage_transform.h"
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t calculate_stack_increment(
    struct kefir_codegen_opt_amd64 *codegen, struct kefir_opt_sysv_amd64_function *codegen_func,
    struct kefir_abi_amd64_sysv_function_decl *abi_func_decl,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation, kefir_size_t *stack_increment) {
    *stack_increment = abi_func_decl->parameters.location.stack_offset;
    for (kefir_size_t i = 0; i < KefirCodegenOptSysvAmd64StackFrameNumOfCallerSavedRegs; i++) {
        kefir_asm_amd64_xasmgen_register_t reg = KefirCodegenOptSysvAmd64StackFrameCallerSavedRegs[i];
        if (result_allocation->result.type ==
                KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER &&
            result_allocation->result.reg == reg) {
            continue;
        }

        kefir_bool_t occupied;
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(&codegen_func->storage, reg, &occupied));
        if (occupied) {
            *stack_increment += KEFIR_AMD64_SYSV_ABI_QWORD;
        }
    }
    *stack_increment = kefir_target_abi_pad_aligned(*stack_increment, 2 * KEFIR_AMD64_SYSV_ABI_QWORD);

    if (*stack_increment > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], *stack_increment)));
    }
    return KEFIR_OK;
}

static kefir_result_t save_registers(struct kefir_codegen_opt_amd64 *codegen,
                                     struct kefir_opt_sysv_amd64_function *codegen_func,
                                     struct kefir_abi_amd64_sysv_function_decl *abi_func_decl,
                                     const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation) {
    kefir_int64_t offset = abi_func_decl->parameters.location.stack_offset;
    for (kefir_size_t i = 0; i < KefirCodegenOptSysvAmd64StackFrameNumOfCallerSavedRegs; i++) {
        kefir_asm_amd64_xasmgen_register_t reg = KefirCodegenOptSysvAmd64StackFrameCallerSavedRegs[i];
        if (result_allocation->result.type ==
                KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER &&
            result_allocation->result.reg == reg) {
            continue;
        }
        if (!kefir_asm_amd64_xasmgen_register_is_floating_point(reg)) {
            kefir_bool_t occupied;
            REQUIRE_OK(
                kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(&codegen_func->storage, reg, &occupied));
            if (occupied) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), -offset),
                    kefir_asm_amd64_xasmgen_operand_reg(reg)));
                offset += KEFIR_AMD64_SYSV_ABI_QWORD;
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t restore_registers(
    struct kefir_codegen_opt_amd64 *codegen, struct kefir_opt_sysv_amd64_function *codegen_func,
    struct kefir_abi_amd64_sysv_function_decl *abi_func_decl,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation) {
    kefir_int64_t offset = abi_func_decl->parameters.location.stack_offset;
    for (kefir_size_t i = 0; i < KefirCodegenOptSysvAmd64StackFrameNumOfCallerSavedRegs; i++) {
        kefir_asm_amd64_xasmgen_register_t reg = KefirCodegenOptSysvAmd64StackFrameCallerSavedRegs[i];
        if (result_allocation->result.type ==
                KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER &&
            result_allocation->result.reg == reg) {
            continue;
        }
        if (!kefir_asm_amd64_xasmgen_register_is_floating_point(reg)) {
            kefir_bool_t occupied;
            REQUIRE_OK(
                kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(&codegen_func->storage, reg, &occupied));
            if (occupied) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), -offset)));
                offset += KEFIR_AMD64_SYSV_ABI_QWORD;
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t visitor_not_supported(const struct kefir_ir_type *type, kefir_size_t index,
                                            const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    UNUSED(payload);
    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Encountered not supported type code while traversing type");
}

struct invoke_arg {
    struct kefir_mem *mem;
    struct kefir_codegen_opt_amd64 *codegen;
    const struct kefir_opt_function *function;
    struct kefir_opt_sysv_amd64_function *codegen_func;
    struct kefir_abi_amd64_sysv_function_decl *abi_func_decl;
    const struct kefir_opt_call_node *call_node;
    struct kefir_codegen_opt_amd64_sysv_storage_transform *transform;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation;
    kefir_size_t argument_index;
};

static kefir_result_t scalar_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                      const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct invoke_arg *, arg, payload);
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                         "Expected valid optimizer codegen invoke parameter preparation argument"));

    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(&arg->abi_func_decl->parameters.allocation, slot));

    kefir_opt_instruction_ref_t argument_ref;
    REQUIRE_OK(kefir_opt_code_container_call_get_argument(&arg->function->code, arg->call_node->node_id,
                                                          arg->argument_index, &argument_ref));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *argument_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&arg->codegen_func->register_allocator, argument_ref,
                                                                   &argument_allocation));

    struct kefir_codegen_opt_amd64_sysv_storage_transform_location argument_location;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_location_from_reg_allocation(
        &argument_location, &arg->codegen_func->stack_frame_map, argument_allocation));

    switch (allocation->klass) {
        case KEFIR_AMD64_SYSV_PARAM_INTEGER:
            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_insert(
                arg->mem, arg->transform,
                &(struct kefir_codegen_opt_amd64_sysv_storage_transform_location){
                    .type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_REGISTER,
                    .reg = KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTERS[allocation->location.integer_register]},
                &argument_location));
            break;

        case KEFIR_AMD64_SYSV_PARAM_MEMORY:
            REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_insert(
                arg->mem, arg->transform,
                &(struct kefir_codegen_opt_amd64_sysv_storage_transform_location){
                    .type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_MEMORY,
                    .memory.base_reg = KEFIR_AMD64_XASMGEN_REGISTER_RSP,
                    .memory.offset = allocation->location.stack_offset},
                &argument_location));
            break;

            // case KEFIR_AMD64_SYSV_PARAM_SSE:
            //     REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PINSRQ(
            //         &info->codegen->xasmgen,
            //         kefir_asm_amd64_xasmgen_operand_reg(
            //             KEFIR_ABI_SYSV_AMD64_PARAMETER_SSE_REGISTERS[allocation->location.sse_register]),
            //         kefir_asm_amd64_xasmgen_operand_indirect(
            //             &info->codegen->xasmgen_helpers.operands[0],
            //             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
            //             (info->total_arguments - info->argument - 1) * KEFIR_AMD64_SYSV_ABI_QWORD),
            //         kefir_asm_amd64_xasmgen_operand_imm(&info->codegen->xasmgen_helpers.operands[1], 0)));
            //     break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR,
                                   "Integer function argument cannot have non-INTEGER and non-MEMORY class");
    }
    arg->argument_index++;
    return KEFIR_OK;
}

static kefir_result_t register_aggregate_argument(struct invoke_arg *arg,
                                                  struct kefir_abi_sysv_amd64_parameter_allocation *allocation) {
    kefir_opt_instruction_ref_t argument_ref;
    REQUIRE_OK(kefir_opt_code_container_call_get_argument(&arg->function->code, arg->call_node->node_id,
                                                          arg->argument_index, &argument_ref));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *argument_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&arg->codegen_func->register_allocator, argument_ref,
                                                                   &argument_allocation));

    struct kefir_codegen_opt_amd64_sysv_storage_transform_location argument_location;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_location_from_reg_allocation(
        &argument_location, &arg->codegen_func->stack_frame_map, argument_allocation));

    for (kefir_size_t i = 0; i < kefir_vector_length(&allocation->container.qwords); i++) {
        ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_qword *, qword, kefir_vector_at(&allocation->container.qwords, i));
        switch (qword->klass) {
            case KEFIR_AMD64_SYSV_PARAM_INTEGER:
                REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_insert(
                    arg->mem, arg->transform,
                    &(struct kefir_codegen_opt_amd64_sysv_storage_transform_location){
                        .type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_REGISTER,
                        .reg = KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTERS[qword->location]},
                    &argument_location));
                break;

                // case KEFIR_AMD64_SYSV_PARAM_SSE:
                //     REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PINSRQ(
                //         &info->codegen->xasmgen,
                //         kefir_asm_amd64_xasmgen_operand_reg(KEFIR_ABI_SYSV_AMD64_PARAMETER_SSE_REGISTERS[qword->location]),
                //         kefir_asm_amd64_xasmgen_operand_indirect(
                //             &info->codegen->xasmgen_helpers.operands[0],
                //             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG),
                //             i * KEFIR_AMD64_SYSV_ABI_QWORD),
                //         kefir_asm_amd64_xasmgen_operand_imm(&info->codegen->xasmgen_helpers.operands[1], 0)));
                //     break;

            default:
                return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Non-INTEGER & non-SSE arguments are not supported");
        }
    }
    return KEFIR_OK;
}

static kefir_result_t memory_aggregate_argument(struct invoke_arg *arg, kefir_size_t index,
                                                struct kefir_abi_sysv_amd64_parameter_allocation *allocation) {
    const struct kefir_abi_sysv_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_at(&arg->abi_func_decl->parameters.layout, index, &layout));
    if (layout->size > 0) {
        kefir_opt_instruction_ref_t argument_ref;
        REQUIRE_OK(kefir_opt_code_container_call_get_argument(&arg->function->code, arg->call_node->node_id,
                                                              arg->argument_index, &argument_ref));

        const struct kefir_codegen_opt_sysv_amd64_register_allocation *argument_allocation = NULL;
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&arg->codegen_func->register_allocator,
                                                                       argument_ref, &argument_allocation));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(
            &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(
            &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSI)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(
            &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RCX)));

        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(
            arg->codegen, &arg->codegen_func->stack_frame_map, argument_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RSI));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RCX),
            kefir_asm_amd64_xasmgen_operand_immu(&arg->codegen->xasmgen_helpers.operands[0], layout->size)));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
            &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI),
            kefir_asm_amd64_xasmgen_operand_indirect(
                &arg->codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                3 * KEFIR_AMD64_SYSV_ABI_QWORD + allocation->location.stack_offset)));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CLD(&arg->codegen->xasmgen));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVSB(&arg->codegen->xasmgen, true));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(
            &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RCX)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(
            &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSI)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(
            &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI)));
    }
    return KEFIR_OK;
}

static kefir_result_t aggregate_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                         const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct invoke_arg *, arg, payload);
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                         "Expected valid optimizer codegen invoke parameter preparation argument"));

    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(&arg->abi_func_decl->parameters.allocation, slot));
    if (allocation->klass == KEFIR_AMD64_SYSV_PARAM_MEMORY) {
        REQUIRE_OK(memory_aggregate_argument(arg, index, allocation));
    } else {
        REQUIRE_OK(register_aggregate_argument(arg, allocation));
    }
    arg->argument_index++;
    return KEFIR_OK;
}

static kefir_result_t load_register_aggregate_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                                       const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct invoke_arg *, arg, payload);
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                         "Expected valid optimizer codegen invoke parameter preparation argument"));

    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(&arg->abi_func_decl->parameters.allocation, slot));
    if (allocation->klass != KEFIR_AMD64_SYSV_PARAM_MEMORY) {
        for (kefir_size_t i = 0; i < kefir_vector_length(&allocation->container.qwords); i++) {
            ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_qword *, qword,
                             kefir_vector_at(&allocation->container.qwords, i));
            switch (qword->klass) {
                case KEFIR_AMD64_SYSV_PARAM_INTEGER:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                        &arg->codegen->xasmgen,
                        kefir_asm_amd64_xasmgen_operand_reg(
                            KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTERS[qword->location]),
                        kefir_asm_amd64_xasmgen_operand_indirect(
                            &arg->codegen->xasmgen_helpers.operands[0],
                            kefir_asm_amd64_xasmgen_operand_reg(
                                KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTERS[qword->location]),
                            i * KEFIR_AMD64_SYSV_ABI_QWORD)));
                    break;

                    // case KEFIR_AMD64_SYSV_PARAM_SSE:
                    //     REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PINSRQ(
                    //         &info->codegen->xasmgen,
                    //         kefir_asm_amd64_xasmgen_operand_reg(KEFIR_ABI_SYSV_AMD64_PARAMETER_SSE_REGISTERS[qword->location]),
                    //         kefir_asm_amd64_xasmgen_operand_indirect(
                    //             &info->codegen->xasmgen_helpers.operands[0],
                    //             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG),
                    //             i * KEFIR_AMD64_SYSV_ABI_QWORD),
                    //         kefir_asm_amd64_xasmgen_operand_imm(&info->codegen->xasmgen_helpers.operands[1], 0)));
                    //     break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Non-INTEGER & non-SSE arguments are not supported");
            }
        }
    }
    arg->argument_index++;
    return KEFIR_OK;
}

static kefir_result_t prepare_parameters(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                         const struct kefir_opt_function *function,
                                         struct kefir_opt_sysv_amd64_function *codegen_func,
                                         const struct kefir_ir_function_decl *ir_func_decl,
                                         struct kefir_abi_amd64_sysv_function_decl *abi_func_decl,
                                         const struct kefir_opt_call_node *call_node) {
    struct kefir_ir_type_visitor visitor;
    struct invoke_arg arg = {.mem = mem,
                             .codegen = codegen,
                             .function = function,
                             .codegen_func = codegen_func,
                             .abi_func_decl = abi_func_decl,
                             .call_node = call_node,
                             .transform = NULL,
                             .result_allocation = NULL,
                             .argument_index = 0};

    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, visitor_not_supported));
    KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS(&visitor, scalar_argument);
    KEFIR_IR_TYPE_VISITOR_INIT_FIXED_FP(&visitor, scalar_argument);
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = aggregate_argument;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = aggregate_argument;
    visitor.visit[KEFIR_IR_TYPE_UNION] = aggregate_argument;

    struct kefir_codegen_opt_amd64_sysv_storage_transform transform;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_init(&transform));
    arg.transform = &transform;

    kefir_result_t res =
        kefir_ir_type_visitor_list_nodes(ir_func_decl->params, &visitor, (void *) &arg, 0, call_node->argument_count);
    REQUIRE_CHAIN(
        &res, kefir_codegen_opt_amd64_sysv_storage_transform_perform(mem, codegen, &codegen_func->storage, &transform));

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_amd64_sysv_storage_transform_free(mem, &transform);
        return res;
    });
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_free(mem, &transform));

    arg.transform = NULL;
    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, NULL));
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = load_register_aggregate_argument;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = load_register_aggregate_argument;
    visitor.visit[KEFIR_IR_TYPE_UNION] = load_register_aggregate_argument;
    REQUIRE_OK(
        kefir_ir_type_visitor_list_nodes(ir_func_decl->params, &visitor, (void *) &arg, 0, call_node->argument_count));

    if (abi_func_decl->returns.implicit_parameter) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTERS[0]),
            kefir_asm_amd64_xasmgen_operand_indirect(
                &codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                codegen_func->stack_frame_map.offset.temporary_area)));
    }
    if (ir_func_decl->vararg) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                                 abi_func_decl->parameters.location.sse_register)));
    }
    return KEFIR_OK;
}

static kefir_result_t integer_return(const struct kefir_ir_type *type, kefir_size_t index,
                                     const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct invoke_arg *, arg, payload);
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                         "Expected valid optimizer codegen invoke parameter preparation argument"));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(
        arg->codegen, &arg->codegen_func->stack_frame_map, arg->result_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RAX));
    return KEFIR_OK;
}

static kefir_result_t aggregate_return(const struct kefir_ir_type *type, kefir_size_t index,
                                       const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct invoke_arg *, arg, payload);
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                         "Expected valid optimizer codegen invoke parameter preparation argument"));

    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, allocation,
                     kefir_vector_at(&arg->abi_func_decl->returns.allocation, slot));
    if (allocation->klass == KEFIR_AMD64_SYSV_PARAM_MEMORY) {
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(arg->codegen, &arg->codegen_func->stack_frame_map,
                                                                     arg->result_allocation,
                                                                     KEFIR_AMD64_XASMGEN_REGISTER_RAX));
    } else {
        // REQUIRE_OK(register_aggregate_return(info, allocation));
    }
    return KEFIR_OK;
}

static kefir_result_t save_return_value(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen, const struct kefir_opt_function *function,
    struct kefir_opt_sysv_amd64_function *codegen_func, const struct kefir_ir_function_decl *ir_func_decl,
    struct kefir_abi_amd64_sysv_function_decl *abi_func_decl, const struct kefir_opt_call_node *call_node,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation) {
    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, visitor_not_supported));
    KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS(&visitor, integer_return);
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = aggregate_return;
    visitor.visit[KEFIR_IR_TYPE_UNION] = aggregate_return;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = aggregate_return;

    struct invoke_arg arg = {.mem = mem,
                             .codegen = codegen,
                             .function = function,
                             .codegen_func = codegen_func,
                             .abi_func_decl = abi_func_decl,
                             .call_node = call_node,
                             .transform = NULL,
                             .result_allocation = result_allocation};

    REQUIRE(kefir_ir_type_children(ir_func_decl->result) <= 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Function cannot return more than one value"));
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(ir_func_decl->result, &visitor, (void *) &arg, 0, 1));
    return KEFIR_OK;
}

static kefir_result_t invoke_impl(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                  const struct kefir_opt_function *function,
                                  struct kefir_opt_sysv_amd64_function *codegen_func,
                                  const struct kefir_ir_function_decl *ir_func_decl,
                                  struct kefir_abi_amd64_sysv_function_decl *abi_func_decl,
                                  const struct kefir_opt_call_node *call_node,
                                  const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation) {
    kefir_size_t stack_increment;

    // Prologue
    REQUIRE_OK(calculate_stack_increment(codegen, codegen_func, abi_func_decl, result_allocation, &stack_increment));
    REQUIRE_OK(save_registers(codegen, codegen_func, abi_func_decl, result_allocation));
    REQUIRE_OK(prepare_parameters(mem, codegen, function, codegen_func, ir_func_decl, abi_func_decl, call_node));

    // Call the function
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], ir_func_decl->name)));

    // Epilogue
    REQUIRE_OK(save_return_value(mem, codegen, function, codegen_func, ir_func_decl, abi_func_decl, call_node,
                                 result_allocation));
    REQUIRE_OK(restore_registers(codegen, codegen_func, abi_func_decl, result_allocation));
    if (stack_increment > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], stack_increment)));
    }
    return KEFIR_OK;
}

DEFINE_TRANSLATOR(invoke) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));

    struct kefir_opt_call_node *call_node = NULL;
    REQUIRE_OK(
        kefir_opt_code_container_call(&function->code, instr->operation.parameters.function_call.call_ref, &call_node));

    const struct kefir_ir_function_decl *ir_func_decl =
        kefir_ir_module_get_declaration(module->ir_module, call_node->function_declaration_id);
    REQUIRE(ir_func_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR function declaration"));

    struct kefir_abi_amd64_sysv_function_decl abi_func_decl;
    REQUIRE_OK(kefir_abi_amd64_sysv_function_decl_alloc(mem, ir_func_decl, &abi_func_decl));

    kefir_result_t res =
        invoke_impl(mem, codegen, function, codegen_func, ir_func_decl, &abi_func_decl, call_node, result_allocation);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_sysv_function_decl_free(mem, &abi_func_decl);
        return res;
    });
    REQUIRE_OK(kefir_abi_amd64_sysv_function_decl_free(mem, &abi_func_decl));
    return KEFIR_OK;
}
