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

#define KEFIR_CODEGEN_TARGET_IR_AMD64_PEEPHOLE_INTERNAL
#include "kefir/codegen/target-ir/amd64/transform.h"
#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/codegen/target-ir/tie.h"
#include "kefir/codegen/target-ir/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_target_ir_amd64_peephole_setcc(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    struct kefir_codegen_target_ir_operation oper = instr->operation;
    REQUIRE(classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
            classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            oper.parameters[classification.operands[0].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            !oper.parameters[classification.operands[0].read_index].direct.tied, KEFIR_OK);

    kefir_codegen_target_ir_value_ref_t input_value_ref = oper.parameters[classification.operands[0].read_index].direct.value_ref;
    const struct kefir_codegen_target_ir_instruction *input_instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, input_value_ref.instr_ref, &input_instr));
    REQUIRE(input_instr->operation.opcode != code->klass->placeholder_opcode, KEFIR_OK);

    const struct kefir_codegen_target_ir_value_type *input_value_type;
    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, input_value_ref, &input_value_type));

    kefir_codegen_target_ir_value_ref_t placeholder_value_ref = {
        .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
    };
    struct kefir_codegen_target_ir_value_type placeholder_input_value_type = {
        .kind = input_value_type->kind,
        .variant = input_value_type->variant
    };
    REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, instr->block_ref,
        kefir_codegen_target_ir_code_control_prev(code, instr->instr_ref),
        &(struct kefir_codegen_target_ir_operation) {
            .opcode = code->klass->placeholder_opcode
        }, NULL, &placeholder_value_ref.instr_ref));
    REQUIRE_OK(kefir_codegen_target_ir_add_produced_resource_aspects(mem, code, placeholder_value_ref.instr_ref));
    REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, placeholder_value_ref, &placeholder_input_value_type));

    oper.parameters[classification.operands[0].read_index].direct.value_ref = placeholder_value_ref;

    REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &oper));
    *replaced = true;

    return KEFIR_OK;
}

static kefir_result_t peephole_setcc_preamble(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, const kefir_codegen_target_ir_resource_id_t *resources, kefir_size_t resources_len, struct kefir_codegen_target_ir_operation *oper, kefir_codegen_target_ir_instruction_ref_t *replace_instr_ref) {
    UNUSED(mem);
    *replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    for (kefir_size_t i = 0; i < resources_len; i++) {
        kefir_bool_t found_operand = false;
        for (kefir_size_t j = 0; !found_operand && j < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; j++) {
            if (instr->operation.parameters[j].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {
                found_operand = instr->operation.parameters[j].direct.value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_RESOURCE(resources[i]);
            }
        }
        REQUIRE(found_operand, KEFIR_OK);
    }

    const struct kefir_codegen_target_ir_value_type *output_type;
    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, classification.operands[0].output, &output_type));
    REQUIRE(output_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT, KEFIR_OK);

    kefir_codegen_target_ir_instruction_ref_t user_instr_ref = KEFIR_ID_NONE;
    const struct kefir_codegen_target_ir_instruction *user_instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_get_single_user(code, classification.operands[0].output, &user_instr_ref));
    REQUIRE(user_instr_ref != KEFIR_ID_NONE, KEFIR_OK);
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, user_instr_ref, &user_instr));

    REQUIRE(user_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(test), KEFIR_OK);
    REQUIRE(user_instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF, KEFIR_OK);
    REQUIRE(user_instr->operation.parameters[0].direct.value_ref.instr_ref == classification.operands[0].output.instr_ref, KEFIR_OK);
    REQUIRE(user_instr->operation.parameters[0].direct.value_ref.aspect == classification.operands[0].output.aspect, KEFIR_OK);
    REQUIRE(user_instr->operation.parameters[1].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF, KEFIR_OK);
    REQUIRE(user_instr->operation.parameters[1].direct.value_ref.instr_ref == classification.operands[0].output.instr_ref, KEFIR_OK);
    REQUIRE(user_instr->operation.parameters[1].direct.value_ref.aspect == classification.operands[0].output.aspect, KEFIR_OK);

    kefir_codegen_target_ir_instruction_ref_t test_user_instr_ref = KEFIR_ID_NONE;
    const struct kefir_codegen_target_ir_instruction *test_user_instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_get_single_user(code, (kefir_codegen_target_ir_value_ref_t) {
        .instr_ref = user_instr_ref,
        .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_RESOURCE(KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF)
    }, &test_user_instr_ref));
    REQUIRE(test_user_instr_ref != KEFIR_ID_NONE, KEFIR_OK);
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, test_user_instr_ref, &test_user_instr));
    REQUIRE(test_user_instr->operation.opcode != code->klass->phi_opcode, KEFIR_OK);
    REQUIRE(test_user_instr->operation.opcode != code->klass->inline_asm_opcode, KEFIR_OK);

    REQUIRE(instr->block_ref == user_instr->block_ref, KEFIR_OK);
    REQUIRE(instr->block_ref == test_user_instr->block_ref, KEFIR_OK);

    kefir_bool_t found_user = false;
    for (kefir_codegen_target_ir_instruction_ref_t iter_ref = instr->instr_ref; iter_ref != KEFIR_ID_NONE;
        iter_ref = kefir_codegen_target_ir_code_control_next(code, iter_ref)) {
        if (iter_ref == test_user_instr_ref) {
            found_user = true;
            break;
        } else if (iter_ref == user_instr_ref ||
            iter_ref == instr->instr_ref) {
            continue;
        }
        
        const struct kefir_codegen_target_ir_instruction *iter_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, iter_ref, &iter_instr));
        for (kefir_size_t i = 0; i < resources_len; i++) {
            const struct kefir_codegen_target_ir_value_type *output_type;
            kefir_result_t res = kefir_codegen_target_ir_code_value_props(code, (kefir_codegen_target_ir_value_ref_t) {
                .instr_ref = iter_ref,
                .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_RESOURCE(resources[i])
            }, &output_type);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                return KEFIR_OK;
            }
        }
    }
    REQUIRE(found_user, KEFIR_OK);

    *oper = test_user_instr->operation;
    for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
        if (oper->parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            oper->parameters[i].direct.value_ref.instr_ref == user_instr_ref &&
            oper->parameters[i].direct.value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_RESOURCE(KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF)) {
            oper->parameters[i].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE;
        }
    }

    kefir_size_t resource_idx = 0;
    for (kefir_size_t i = 0; resource_idx < resources_len && i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
        if (oper->parameters[i].type != KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE) {
            continue;
        }

        for (kefir_size_t j = 0; j < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; j++) {
            if (instr->operation.parameters[j].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                instr->operation.parameters[j].direct.value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_RESOURCE(resources[resource_idx])) {
                oper->parameters[i] = instr->operation.parameters[j];
                resource_idx++;
                break;
            }
        }
        REQUIRE(oper->parameters[i].type != KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE, KEFIR_OK);
    }
    REQUIRE(resource_idx == resources_len, KEFIR_OK);

    *replace_instr_ref = test_user_instr_ref;
    return KEFIR_OK;
}

#define SETCC_IMPL(_setcc, _setncc, _jz, _jnz, ...) \
    kefir_result_t kefir_codegen_target_ir_amd64_peephole_##_setcc(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) { \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator")); \
        REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code")); \
        REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction")); \
        REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag")); \
        \
        kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE; \
        struct kefir_codegen_target_ir_operation oper; \
        kefir_codegen_target_ir_resource_id_t resources[] = { __VA_ARGS__ }; \
        REQUIRE_OK(peephole_setcc_preamble(mem, code, instr, resources, sizeof(resources) / sizeof(resources[0]), &oper, &replace_instr_ref)); \
        if (replace_instr_ref != KEFIR_ID_NONE) { \
            switch (oper.opcode) { \
                case KEFIR_TARGET_IR_AMD64_OPCODE(jz): \
                case KEFIR_TARGET_IR_AMD64_OPCODE(je): \
                    oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(_jz); \
                    break; \
 \
                case KEFIR_TARGET_IR_AMD64_OPCODE(jnz): \
                case KEFIR_TARGET_IR_AMD64_OPCODE(jne): \
                    oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(_jnz); \
                    break; \
 \
                case KEFIR_TARGET_IR_AMD64_OPCODE(sete): \
                    oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(_setncc); \
                    break; \
 \
                case KEFIR_TARGET_IR_AMD64_OPCODE(setne): \
                    oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(_setcc); \
                    break; \
 \
                default: \
                    return KEFIR_OK; \
            } \
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper)); \
 \
            *replaced = true; \
        } \
        return KEFIR_OK; \
    }

SETCC_IMPL(sete, setne, jnz, jz, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF)
SETCC_IMPL(setne, sete, jz, jnz, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF)
SETCC_IMPL(setnp, setp, jp, jnp, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF)
SETCC_IMPL(setp, setnp, jnp, jp, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF)
SETCC_IMPL(setnc, setc, jc, jnc, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF)
SETCC_IMPL(setc, setnc, jnc, jc, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF)
SETCC_IMPL(setno, seto, jo, jno, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF)
SETCC_IMPL(seto, setno, jno, jo, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF)
SETCC_IMPL(setns, sets, js, jns, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF)
SETCC_IMPL(sets, setns, jns, js, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF)
SETCC_IMPL(setnb, setb, jb, jnb, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF)
SETCC_IMPL(setb, setnb, jnb, jb, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF)
SETCC_IMPL(setge, setl, jl, jge, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF)
SETCC_IMPL(setl, setge, jge, jl, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF)
SETCC_IMPL(setg, setle, jle, jg, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF)
SETCC_IMPL(setle, setg, jg, jle, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF)
SETCC_IMPL(setbe, seta, ja, jbe, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF)
SETCC_IMPL(seta, setbe, jbe, ja, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF)

#undef SETCC_IMPL
