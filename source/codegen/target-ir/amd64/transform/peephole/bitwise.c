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
#include "kefir/codegen/target-ir/amd64/util.h"
#include "kefir/codegen/target-ir/tie.h"
#include "kefir/codegen/target-ir/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define OP_AND(_lhs, _rhs) ((_lhs) & (_rhs))
#define OP_OR(_lhs, _rhs) ((_lhs) | (_rhs))
#define OP_XOR(_lhs, _rhs) ((_lhs) ^ (_rhs))
#define OP_SHL(_lhs, _rhs) ((_lhs) << (_rhs))
#define OP_SHR(_lhs, _rhs) (((kefir_uint64_t) (_lhs)) >> (_rhs))
#define OP_SAR(_lhs, _rhs) (((kefir_int64_t) (_lhs)) >> (_rhs))
#define COND_TRUE(_lhs, _rhs) true
#define COND_SHIFT(_lhs, _rhs) ((_rhs) >= 0 && (_rhs) < (kefir_int64_t) (sizeof(kefir_int64_t) * CHAR_BIT))
#define CONST_EVAL_BASE(_op, _cond, _ext) \
    do { \
        kefir_result_t res; \
        struct kefir_codegen_target_ir_use_iterator use_iter; \
        kefir_codegen_target_ir_instruction_ref_t use_instr_ref; \
        kefir_codegen_target_ir_value_ref_t used_value_ref; \
        for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &used_value_ref); \
            res == KEFIR_OK; \
            res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) { \
            REQUIRE(used_value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK); \
        } \
        if (res != KEFIR_ITERATOR_END) { \
            REQUIRE_OK(res); \
        } \
 \
        if (classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE && \
            classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE && \
            classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE) { \
            kefir_int64_t lhs, rhs, result; \
            kefir_result_t res = kefir_codegen_target_ir_amd64_match_immediate_operand(code, &instr->operation.parameters[classification.operands[0].read_index], (_ext), &lhs); \
 \
            if (res != KEFIR_NO_MATCH) { \
                REQUIRE_OK(res); \
                const struct kefir_codegen_target_ir_value_type *value_type; \
                REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, classification.operands[0].output, &value_type)); \
                if (value_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) { \
                    res = kefir_codegen_target_ir_amd64_match_immediate_operand(code, &instr->operation.parameters[classification.operands[1].read_index], true, &rhs); \
 \
                    if (res != KEFIR_NO_MATCH) { \
                        REQUIRE_OK(res); \
                        result = _op(lhs, rhs); \
                        result = kefir_codegen_target_ir_sign_extend(result, value_type->variant); \
 \
                        if (_cond(lhs, rhs)) { \
                            REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, \
                                &(struct kefir_codegen_target_ir_operation) { \
                                    .opcode = code->klass->assign_opcode, \
                                    .parameters[0] = { \
                                        .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER, \
                                        .immediate.int_immediate = result, \
                                        .immediate.variant = value_type->variant \
                                    } \
                                }, NULL)); \
                            *replaced = true; \
                            return KEFIR_OK; \
                        } \
                    } \
                } \
            } \
        } \
    } while (0)

#define CONST_EVAL(_op, _cond) CONST_EVAL_BASE(_op, _cond, true)
#define CONST_EVAL_ZX(_op, _cond) CONST_EVAL_BASE(_op, _cond, false)

kefir_result_t kefir_codegen_target_ir_amd64_peephole_xor(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;

    CONST_EVAL(OP_XOR, COND_TRUE);
    REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_const_operand(mem, code, instr, true, replaced));
    REQUIRE(!*replaced, KEFIR_OK);

    REQUIRE(classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
            classification.classification.operands[1].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
            classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            instr->operation.parameters[classification.operands[0].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            instr->operation.parameters[classification.operands[0].read_index].direct.variant ==
                instr->operation.parameters[classification.operands[1].read_index].direct.variant &&
            (instr->operation.parameters[classification.operands[0].read_index].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT ||
            instr->operation.parameters[classification.operands[0].read_index].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT ||
            instr->operation.parameters[classification.operands[0].read_index].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT) &&
            instr->operation.parameters[classification.operands[0].read_index].direct.value_ref.instr_ref ==
                instr->operation.parameters[classification.operands[1].read_index].direct.value_ref.instr_ref &&
            instr->operation.parameters[classification.operands[0].read_index].direct.value_ref.aspect ==
                instr->operation.parameters[classification.operands[1].read_index].direct.value_ref.aspect, KEFIR_OK);

    kefir_codegen_target_ir_value_ref_t zero_ref = {
        .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
    };

    struct kefir_codegen_target_ir_instruction_metadata metadata = instr->metadata;
    REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, instr->block_ref, instr_ref, 
        &(struct kefir_codegen_target_ir_operation) {
            .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(mov),
            .parameters[0] = {
                .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                .immediate.int_immediate = 0,
                .immediate.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
            }
        }, &metadata, &zero_ref.instr_ref));

    const struct kefir_codegen_target_ir_value_type *output_value_type;
    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, classification.operands[0].output,
        &output_value_type));
    struct kefir_codegen_target_ir_value_type zero_value_type = *output_value_type;
    REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, zero_ref, &zero_value_type));

    REQUIRE_OK(kefir_codegen_target_ir_code_replace_value(mem, code, zero_ref,
        classification.operands[0].output));
    REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
    *replaced = true;

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_peephole_and(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;

    kefir_result_t res;
    CONST_EVAL(OP_AND, COND_TRUE);
    REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_const_operand(mem, code, instr, true, replaced));
    REQUIRE(!*replaced, KEFIR_OK);

    REQUIRE(classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
            classification.classification.operands[1].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
            classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            instr->operation.parameters[classification.operands[0].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            (instr->operation.parameters[classification.operands[0].read_index].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT ||
            instr->operation.parameters[classification.operands[0].read_index].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT ||
            instr->operation.parameters[classification.operands[0].read_index].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT) &&
            instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER, KEFIR_OK);

    if (instr->operation.parameters[classification.operands[1].read_index].immediate.uint_immediate == 0) {
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &(struct kefir_codegen_target_ir_operation) {
            .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(mov),
            .parameters[0] = {
                .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                .immediate.int_immediate = 0
            }
        }, NULL));
        *replaced = true;
    } else if (instr->operation.parameters[classification.operands[1].read_index].immediate.uint_immediate < (1 << 7)) {
        struct kefir_codegen_target_ir_use_iterator use_iter;
        kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
        kefir_codegen_target_ir_value_ref_t used_value_ref;
        for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &used_value_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
            if (used_value_ref.aspect != KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)) {
                continue;
            }

            const struct kefir_codegen_target_ir_instruction *user_instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &user_instr));
            if (user_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(movsx) ||
                user_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(movzx)) {
                struct kefir_codegen_target_ir_tie_classification user_classification;
                REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, use_instr_ref, &user_classification));
                if (user_classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                    user_instr->operation.parameters[user_classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                    user_instr->operation.parameters[user_classification.operands[1].read_index].direct.value_ref.instr_ref == instr_ref &&
                    user_instr->operation.parameters[user_classification.operands[1].read_index].direct.value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)) {
                    REQUIRE_OK(kefir_codegen_target_ir_code_replace_instruction(mem, code, instr_ref, use_instr_ref));
                    REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, use_instr_ref));
                    *replaced = true;
                }
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_peephole_or(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr_ref, &classification));

    CONST_EVAL(OP_OR, COND_TRUE);
    REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_const_operand(mem, code, instr, true, replaced));
    return KEFIR_OK;
}

kefir_result_t peephole_const_operand_nonneg(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr_ref, &classification));
    if (classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
        classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
        classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
        instr->operation.parameters[classification.operands[0].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
        !instr->operation.parameters[classification.operands[0].read_index].direct.tied &&
        instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {
        kefir_int64_t rhs_value = 0;
        kefir_result_t res = kefir_codegen_target_ir_amd64_match_immediate(code, instr->operation.parameters[classification.operands[1].read_index].direct.value_ref, true, &rhs_value);
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
            if (rhs_value >= 0) {
                struct kefir_codegen_target_ir_operation oper = instr->operation;
                oper.parameters[classification.operands[1].read_index] = (struct kefir_codegen_target_ir_operand) {
                    .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                    .immediate = {
                        .uint_immediate = rhs_value,
                        .variant = oper.parameters[classification.operands[1].read_index].direct.variant
                    }
                };
                REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &oper, NULL));
                *replaced = true;
                return KEFIR_OK;
            }
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_peephole_shl(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;

    CONST_EVAL(OP_SHL, COND_SHIFT);
    REQUIRE_OK(peephole_const_operand_nonneg(mem, code, instr, replaced));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_peephole_shr(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    
    CONST_EVAL_ZX(OP_SHR, COND_SHIFT);
    REQUIRE_OK(peephole_const_operand_nonneg(mem, code, instr, replaced));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_peephole_sar(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    
    CONST_EVAL(OP_SAR, COND_SHIFT);
    REQUIRE_OK(peephole_const_operand_nonneg(mem, code, instr, replaced));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_peephole_rol(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    REQUIRE_OK(peephole_const_operand_nonneg(mem, code, instr, replaced));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_peephole_btc(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    REQUIRE_OK(peephole_const_operand_nonneg(mem, code, instr, replaced));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_peephole_shxd(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr_ref, &classification));
    if (classification.operands[2].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
        instr->operation.parameters[classification.operands[2].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {
        kefir_int64_t rhs_value = 0;
        kefir_result_t res = kefir_codegen_target_ir_amd64_match_immediate(code, instr->operation.parameters[classification.operands[2].read_index].direct.value_ref, true, &rhs_value);
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
            if (rhs_value >= 0) {
                struct kefir_codegen_target_ir_operation oper = instr->operation;
                oper.parameters[classification.operands[2].read_index] = (struct kefir_codegen_target_ir_operand) {
                    .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                    .immediate = {
                        .uint_immediate = rhs_value,
                        .variant = oper.parameters[classification.operands[2].read_index].direct.variant
                    }
                };
                REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &oper, NULL));
                *replaced = true;
                return KEFIR_OK;
            }
        }
    }
    return KEFIR_OK;
}
