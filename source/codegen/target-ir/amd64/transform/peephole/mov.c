/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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
#include "kefir/codegen/target-ir/amd64/util.h"
#include "kefir/codegen/target-ir/tie.h"
#include "kefir/codegen/target-ir/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_codegen_target_ir_amd64_peephole_movx(struct kefir_mem *mem,
                                                           struct kefir_codegen_target_ir_code *code,
                                                           const struct kefir_codegen_target_ir_instruction *instr,
                                                           kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    REQUIRE(classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE, KEFIR_OK);

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;

    kefir_codegen_target_ir_value_ref_t output_value_ref = classification.operands[0].output;
    REQUIRE(output_value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK);

    const struct kefir_codegen_target_ir_value_type *output_value_type;
    kefir_result_t res = kefir_codegen_target_ir_code_value_props(code, output_value_ref, &output_value_type);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);
    REQUIRE(output_value_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT, KEFIR_OK);

    if (instr->operation.parameters[classification.operands[1].read_index].type ==
            KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
        instr->operation.parameters[classification.operands[1].read_index].direct.value_ref.aspect ==
            KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)) {
        kefir_int64_t value = 0;
        res = kefir_codegen_target_ir_amd64_match_immediate(
            code, instr->operation.parameters[classification.operands[1].read_index].direct.value_ref, true, &value);
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
#define CONST_FOLD                                                                                                \
    if (instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(movsx) &&                                         \
        (output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||                         \
         output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT ||                           \
         output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) &&                          \
        instr->operation.parameters[classification.operands[1].read_index].direct.variant ==                      \
            KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT) {                                                       \
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(                                                \
            mem, code, instr_ref,                                                                                 \
            &(struct kefir_codegen_target_ir_operation) {                                                         \
                .opcode = code->klass->assign_opcode,                                                             \
                .parameters[0] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,                           \
                                  .immediate = {.int_immediate = (kefir_int8_t) value,                            \
                                                .variant = output_value_type->variant}}},                         \
            NULL));                                                                                               \
        *replaced = true;                                                                                         \
        return KEFIR_OK;                                                                                          \
    } else if (instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(movsx) &&                                  \
               (output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||                  \
                output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT ||                    \
                output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) &&                   \
               instr->operation.parameters[classification.operands[1].read_index].direct.variant ==               \
                   KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT) {                                               \
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(                                                \
            mem, code, instr_ref,                                                                                 \
            &(struct kefir_codegen_target_ir_operation) {                                                         \
                .opcode = code->klass->assign_opcode,                                                             \
                .parameters[0] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,                           \
                                  .immediate = {.int_immediate = (kefir_int16_t) value,                           \
                                                .variant = output_value_type->variant}}},                         \
            NULL));                                                                                               \
        *replaced = true;                                                                                         \
        return KEFIR_OK;                                                                                          \
    } else if (instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(movsx) &&                                  \
               (output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||                  \
                output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT ||                    \
                output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) &&                   \
               instr->operation.parameters[classification.operands[1].read_index].direct.variant ==               \
                   KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) {                                               \
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(                                                \
            mem, code, instr_ref,                                                                                 \
            &(struct kefir_codegen_target_ir_operation) {                                                         \
                .opcode = code->klass->assign_opcode,                                                             \
                .parameters[0] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,                           \
                                  .immediate = {.int_immediate = (kefir_int32_t) value,                           \
                                                .variant = output_value_type->variant}}},                         \
            NULL));                                                                                               \
        *replaced = true;                                                                                         \
        return KEFIR_OK;                                                                                          \
    } else if (instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(movzx) &&                                  \
               (output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||                  \
                output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT ||                    \
                output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) &&                   \
               instr->operation.parameters[classification.operands[1].read_index].direct.variant ==               \
                   KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT) {                                                \
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(                                                \
            mem, code, instr_ref,                                                                                 \
            &(struct kefir_codegen_target_ir_operation) {                                                         \
                .opcode = code->klass->assign_opcode,                                                             \
                .parameters[0] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,                           \
                                  .immediate = {.int_immediate = (kefir_uint8_t) value,                           \
                                                .variant = output_value_type->variant}}},                         \
            NULL));                                                                                               \
        *replaced = true;                                                                                         \
        return KEFIR_OK;                                                                                          \
    } else if (instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(movzx) &&                                  \
               (output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||                  \
                output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT ||                    \
                output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) &&                   \
               instr->operation.parameters[classification.operands[1].read_index].direct.variant ==               \
                   KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT) {                                               \
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(                                                \
            mem, code, instr_ref,                                                                                 \
            &(struct kefir_codegen_target_ir_operation) {                                                         \
                .opcode = code->klass->assign_opcode,                                                             \
                .parameters[0] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,                           \
                                  .immediate = {.int_immediate = (kefir_uint16_t) value,                          \
                                                .variant = output_value_type->variant}}},                         \
            NULL));                                                                                               \
        *replaced = true;                                                                                         \
        return KEFIR_OK;                                                                                          \
    } else if (instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(mov) &&                                    \
               output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT &&                     \
               instr->operation.parameters[classification.operands[1].read_index].direct.variant ==               \
                   KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) {                                               \
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(                                                \
            mem, code, instr_ref,                                                                                 \
            &(struct kefir_codegen_target_ir_operation) {                                                         \
                .opcode = code->klass->assign_opcode,                                                             \
                .parameters[0] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,                           \
                                  .immediate = {.int_immediate = (kefir_uint32_t) value,                          \
                                                .variant = output_value_type->variant}}},                         \
            NULL));                                                                                               \
        *replaced = true;                                                                                         \
        return KEFIR_OK;                                                                                          \
    } else if (instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(mov) &&                                    \
               (output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT ||                    \
                output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT) &&                 \
               (instr->operation.parameters[classification.operands[1].read_index].direct.variant ==              \
                    KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT ||                                              \
                instr->operation.parameters[classification.operands[1].read_index].direct.variant ==              \
                    KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT)) {                                           \
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(                                                \
            mem, code, instr_ref,                                                                                 \
            &(struct kefir_codegen_target_ir_operation) {                                                         \
                .opcode = code->klass->assign_opcode,                                                             \
                .parameters[0] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,                           \
                                  .immediate = {.int_immediate = value, .variant = output_value_type->variant}}}, \
            NULL));                                                                                               \
        *replaced = true;                                                                                         \
        return KEFIR_OK;                                                                                          \
    }
        }
    } else if (instr->operation.parameters[classification.operands[1].read_index].type ==
               KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER) {
        kefir_uint64_t value =
            instr->operation.parameters[classification.operands[1].read_index].immediate.uint_immediate;
        switch (instr->operation.parameters[classification.operands[1].read_index].immediate.variant) {
            case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT:
                value = (kefir_uint8_t) value;
                break;

            case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT:
                value = (kefir_uint16_t) value;
                break;

            case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT:
                value = (kefir_uint32_t) value;
                break;

            default:
                // Intentionally left blank
                break;
        }

        CONST_FOLD
#undef CONST_FOLD
    }

    REQUIRE(classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                instr->operation.parameters[classification.operands[1].read_index].type ==
                    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                instr->operation.parameters[classification.operands[1].read_index].direct.value_ref.aspect ==
                    KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0),
            KEFIR_OK);

    const struct kefir_codegen_target_ir_operand input_param =
        instr->operation.parameters[classification.operands[1].read_index];

    kefir_bool_t ext_uses = false;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value_ref;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &used_value_ref);
         res == KEFIR_OK && !ext_uses;
         res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        if (used_value_ref.aspect != output_value_ref.aspect) {
            continue;
        }

        const struct kefir_codegen_target_ir_instruction *user_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &user_instr));
        if (user_instr->operation.opcode == code->klass->phi_opcode ||
            user_instr->operation.opcode == code->klass->inline_asm_opcode || user_instr->block_ref == KEFIR_ID_NONE) {
            ext_uses = true;
            continue;
        }

        struct kefir_codegen_target_ir_operation oper = user_instr->operation;
        for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS && !ext_uses; i++) {
            if ((oper.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                 oper.parameters[i].direct.value_ref.instr_ref == output_value_ref.instr_ref &&
                 oper.parameters[i].direct.value_ref.aspect == output_value_ref.aspect &&
                 (oper.parameters[i].direct.tied ||
                  !(oper.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT ||
                    (oper.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT &&
                     input_param.direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT) ||
                    (oper.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT &&
                     input_param.direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) ||
                    (oper.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT &&
                     input_param.direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT))))) {
                ext_uses = true;
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    REQUIRE(!ext_uses, KEFIR_OK);
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &used_value_ref);
         res == KEFIR_OK; res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        if (used_value_ref.aspect != output_value_ref.aspect) {
            continue;
        }

        const struct kefir_codegen_target_ir_instruction *user_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &user_instr));
        if (user_instr->operation.opcode == code->klass->phi_opcode ||
            user_instr->operation.opcode == code->klass->inline_asm_opcode || user_instr->block_ref == KEFIR_ID_NONE) {
            continue;
        }

        kefir_bool_t replace = false;
        struct kefir_codegen_target_ir_operation oper = user_instr->operation;
        for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
            if (oper.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                oper.parameters[i].direct.value_ref.instr_ref == output_value_ref.instr_ref &&
                oper.parameters[i].direct.value_ref.aspect == output_value_ref.aspect &&
                (oper.parameters[i].direct.tied ||
                 (oper.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT ||
                  (oper.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT &&
                   input_param.direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT) ||
                  (oper.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT &&
                   input_param.direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) ||
                  (oper.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT &&
                   input_param.direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT)))) {
                oper.parameters[i].direct.value_ref = input_param.direct.value_ref;
                replace = true;
            }
        }

        if (replace) {
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, use_instr_ref, &oper, NULL));
            *replaced = true;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_peephole_mov(struct kefir_mem *mem,
                                                          struct kefir_codegen_target_ir_code *code,
                                                          const struct kefir_codegen_target_ir_instruction *instr,
                                                          kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    REQUIRE(classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE, KEFIR_OK);

    kefir_codegen_target_ir_value_ref_t output_value_ref = classification.operands[0].output;
    REQUIRE(output_value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK);

    const struct kefir_codegen_target_ir_value_type *output_value_type;
    kefir_result_t res = kefir_codegen_target_ir_code_value_props(code, output_value_ref, &output_value_type);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    const struct kefir_codegen_target_ir_operand *input_param =
        &instr->operation.parameters[classification.operands[1].read_index];

    REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_rmw_mem(mem, code, instr, replaced));
    REQUIRE(!*replaced, KEFIR_OK);

    REQUIRE(input_param->type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                input_param->direct.value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0),
            KEFIR_OK);

    if ((input_param->direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
         input_param->direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT) &&
        output_value_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT &&
        (output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
         output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT)) {
        REQUIRE_OK(
            kefir_codegen_target_ir_code_replace_value(mem, code, input_param->direct.value_ref, output_value_ref));
        REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, output_value_ref.instr_ref));
        *replaced = true;
    } else {
        REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_movx(mem, code, instr, replaced));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_peephole_movabs(struct kefir_mem *mem,
                                                             struct kefir_codegen_target_ir_code *code,
                                                             const struct kefir_codegen_target_ir_instruction *instr,
                                                             kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    if (classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE &&
        classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
        instr->operation.parameters[classification.operands[1].read_index].type ==
            KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER) {
        const struct kefir_codegen_target_ir_operand *operand =
            &instr->operation.parameters[classification.operands[1].read_index];
        kefir_int64_t value =
            kefir_codegen_target_ir_sign_extend(operand->immediate.int_immediate, operand->immediate.variant);
        if (value >= KEFIR_INT32_MIN && value <= KEFIR_INT32_MAX) {
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(
                mem, code, instr->instr_ref,
                &(struct kefir_codegen_target_ir_operation) {
                    .opcode = code->klass->assign_opcode,
                    .parameters[0] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                                      .immediate = {.int_immediate = value, .variant = operand->immediate.variant}}},
                NULL));
            *replaced = true;
        }
    }
    return KEFIR_OK;
}
