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

#include "kefir/codegen/target-ir/amd64/transform.h"
#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/codegen/target-ir/tie.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t peephole_lea(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT, KEFIR_OK);
    struct kefir_codegen_target_ir_operation base_oper = instr->operation;
    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;

    if (instr->operation.parameters[0].indirect.type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS) {
        const struct kefir_codegen_target_ir_value_type *value_type = NULL;
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, base_oper.parameters[0].indirect.base.value_ref, &value_type));
        REQUIRE(value_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT, KEFIR_OK);
    }

    const struct kefir_codegen_target_ir_value_type *output_type;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction_output(code, instr->instr_ref, 0, NULL, &output_type));
    REQUIRE(output_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT, KEFIR_OK);

    kefir_result_t res;
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
        if (user_instr->operation.opcode == code->klass->phi_opcode ||
            user_instr->operation.opcode == code->klass->inline_asm_opcode ||
            user_instr->block_ref == KEFIR_ID_NONE) {
            continue;
        }

        kefir_bool_t replace = false;
        struct kefir_codegen_target_ir_operation oper = user_instr->operation;
        for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
            if (oper.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT &&
                oper.parameters[i].indirect.type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS &&
                oper.parameters[i].indirect.base.value_ref.instr_ref == instr_ref &&
                oper.parameters[i].indirect.base.value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)) {
                replace = true;

                kefir_int64_t offset = oper.parameters[i].indirect.offset;
                kefir_codegen_target_ir_operand_variant_t variant = oper.parameters[i].indirect.variant;
                oper.parameters[i] = base_oper.parameters[0];
                oper.parameters[i].indirect.offset += offset;
                oper.parameters[i].indirect.variant = variant;
            }
        }

        if (replace) {
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, use_instr_ref, &oper));
            *replaced = true;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_add(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
        instr->operation.parameters[0].direct.value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0) &&
        (instr->operation.parameters[0].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
            instr->operation.parameters[0].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT), KEFIR_OK);
    REQUIRE(instr->operation.parameters[1].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER &&
        (instr->operation.parameters[1].immediate.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
        instr->operation.parameters[1].immediate.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT), KEFIR_OK);

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    struct kefir_codegen_target_ir_operation base_oper = instr->operation;

    const struct kefir_codegen_target_ir_value_type *value_type = NULL;
    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, base_oper.parameters[0].direct.value_ref, &value_type));
    REQUIRE(value_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT, KEFIR_OK);

    const struct kefir_codegen_target_ir_value_type *output_type;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction_output(code, instr->instr_ref, 0, NULL, &output_type));
    REQUIRE((output_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
            output_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT) &&
            output_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT, KEFIR_OK);

    const struct kefir_codegen_target_ir_instruction *base_instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr->operation.parameters[0].direct.value_ref.instr_ref, &base_instr));
    if (base_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(lea) &&
        base_instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT) {
        struct kefir_codegen_target_ir_operation replacement_oper = {
            .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(lea),
            .parameters[0] = base_instr->operation.parameters[0]
        };
        if (base_oper.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(add)) {
            replacement_oper.parameters[0].indirect.offset += instr->operation.parameters[1].immediate.int_immediate;
        } else {
            replacement_oper.parameters[0].indirect.offset -= instr->operation.parameters[1].immediate.int_immediate;
        }

        struct kefir_codegen_target_ir_instruction_metadata replacement_metadata = instr->metadata;
        struct kefir_codegen_target_ir_value_type replacement_type = *output_type;

        kefir_codegen_target_ir_instruction_ref_t replacement_ref;
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, instr->block_ref, instr->instr_ref, &replacement_oper, &replacement_metadata, &replacement_ref));
        REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, (kefir_codegen_target_ir_value_ref_t) {
            .instr_ref = replacement_ref,
            .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
        }, &replacement_type));
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_instruction(mem, code, replacement_ref, instr_ref));
        REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));

        *replaced = true;
    } else {
        kefir_result_t res;
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
            if (user_instr->operation.opcode == code->klass->phi_opcode ||
                user_instr->operation.opcode == code->klass->inline_asm_opcode ||
                user_instr->block_ref == KEFIR_ID_NONE) {
                continue;
            }

            kefir_bool_t replace = false;
            struct kefir_codegen_target_ir_operation oper = user_instr->operation;
            for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
                if (oper.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT &&
                    oper.parameters[i].indirect.type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS &&
                    oper.parameters[i].indirect.base.value_ref.instr_ref == instr_ref &&
                    oper.parameters[i].indirect.base.value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)) {
                    replace = true;

                    oper.parameters[i].indirect.base.value_ref = base_oper.parameters[0].direct.value_ref;
                    if (base_oper.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(add)) {
                        oper.parameters[i].indirect.offset += base_oper.parameters[1].immediate.int_immediate;
                    } else {
                        oper.parameters[i].indirect.offset -= base_oper.parameters[1].immediate.int_immediate;
                    }
                }
            }

            if (replace) {
                REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, use_instr_ref, &oper));
                *replaced = true;
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_movx(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    REQUIRE(classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
        instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
        instr->operation.parameters[classification.operands[1].read_index].direct.value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK);

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;

    kefir_codegen_target_ir_value_ref_t output_value_ref = classification.operands[0].output;
    REQUIRE(output_value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK);

    const struct kefir_codegen_target_ir_value_type *output_value_type;
    kefir_result_t res = kefir_codegen_target_ir_code_value_props(code, output_value_ref, &output_value_type);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);
    REQUIRE(output_value_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT, KEFIR_OK);

    const struct kefir_codegen_target_ir_operand input_param = instr->operation.parameters[classification.operands[1].read_index];

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
            user_instr->operation.opcode == code->klass->inline_asm_opcode ||
            user_instr->block_ref == KEFIR_ID_NONE) {
            ext_uses = true;
            continue;
        }

        struct kefir_codegen_target_ir_operation oper = user_instr->operation;
        for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS && !ext_uses; i++) {
            if ((oper.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                oper.parameters[i].direct.value_ref.instr_ref == output_value_ref.instr_ref &&
                oper.parameters[i].direct.value_ref.aspect == output_value_ref.aspect &&
                !(oper.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT ||
                (oper.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT &&
                input_param.direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT) ||
                (oper.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT &&
                input_param.direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) ||
                (oper.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT &&
                input_param.direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT)))) {
                ext_uses = true;
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    REQUIRE(!ext_uses, KEFIR_OK);
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &used_value_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        if (used_value_ref.aspect != output_value_ref.aspect) {
            continue;
        }

        const struct kefir_codegen_target_ir_instruction *user_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &user_instr));
        if (user_instr->operation.opcode == code->klass->phi_opcode ||
            user_instr->operation.opcode == code->klass->inline_asm_opcode ||
            user_instr->block_ref == KEFIR_ID_NONE) {
            continue;
        }
        
        kefir_bool_t replace = false;
        struct kefir_codegen_target_ir_operation oper = user_instr->operation;
        for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS && !ext_uses; i++) {
            if (oper.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                oper.parameters[i].direct.value_ref.instr_ref == output_value_ref.instr_ref &&
                oper.parameters[i].direct.value_ref.aspect == output_value_ref.aspect &&
                (oper.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT ||
                (oper.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT &&
                input_param.direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT) ||
                (oper.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT &&
                input_param.direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) ||
                (oper.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT &&
                input_param.direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT))) {
                oper.parameters[i].direct.value_ref = input_param.direct.value_ref;
                replace = true;
            }
        }

        if (replace) {
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, use_instr_ref, &oper));
            *replaced = true;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t peephole_mov(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    REQUIRE(classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
        instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
        instr->operation.parameters[classification.operands[1].read_index].direct.value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK);

    kefir_codegen_target_ir_value_ref_t output_value_ref = classification.operands[0].output;
    REQUIRE(output_value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK);

    const struct kefir_codegen_target_ir_value_type *output_value_type;
    kefir_result_t res = kefir_codegen_target_ir_code_value_props(code, output_value_ref, &output_value_type);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    const struct kefir_codegen_target_ir_operand *input_param = &instr->operation.parameters[classification.operands[1].read_index];
    if ((input_param->direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
            input_param->direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT) &&
            output_value_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT &&
            (output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
            output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT)) {
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_value(mem, code, input_param->direct.value_ref, output_value_ref));
        REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, output_value_ref.instr_ref));
        *replaced = true;
    } else {
        REQUIRE_OK(peephole_movx(mem, code, instr, replaced));
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_untie(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_bool_t replace = false;
    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    struct kefir_codegen_target_ir_operation oper = instr->operation;
    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        if (!classification.classification.operands[i].tied_rw ||
            classification.operands[i].read_index == KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE) {
            continue;
        }

        if (oper.parameters[classification.operands[i].read_index].type != KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {
            continue;
        }

        kefir_codegen_target_ir_value_ref_t input_value_ref = oper.parameters[classification.operands[i].read_index].direct.value_ref, output_value_ref = classification.operands[i].output;
        if (!KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(output_value_ref.aspect)) {
            continue;
        }

        const struct kefir_codegen_target_ir_instruction *input_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, input_value_ref.instr_ref, &input_instr));
        if (input_instr->operation.opcode == code->klass->placeholder_opcode) {
            continue;
        }

        const struct kefir_codegen_target_ir_value_type *input_value_type, *output_value_type;
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, input_value_ref, &input_value_type));
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, output_value_ref, &output_value_type));
        if (output_value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
            continue;
        }

        kefir_result_t res;
        struct kefir_codegen_target_ir_use_iterator use_iter;
        kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
        kefir_codegen_target_ir_value_ref_t used_value_ref;
        kefir_bool_t ext_uses = false;
        for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, output_value_ref.instr_ref, &use_instr_ref, &used_value_ref);
            res == KEFIR_OK && !ext_uses;
            res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
            if (used_value_ref.aspect != output_value_ref.aspect) {
                continue;
            }

            const struct kefir_codegen_target_ir_instruction *user_instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &user_instr));
            if (user_instr->operation.opcode == code->klass->phi_opcode ||
                user_instr->operation.opcode == code->klass->inline_asm_opcode ||
                user_instr->block_ref == KEFIR_ID_NONE) {
                ext_uses = true;
                continue;
            }
            
            for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS && !ext_uses; i++) {
                if ((user_instr->operation.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                    user_instr->operation.parameters[i].direct.value_ref.instr_ref == output_value_ref.instr_ref &&
                    user_instr->operation.parameters[i].direct.value_ref.aspect == output_value_ref.aspect &&
                    !(user_instr->operation.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT ||
                    (user_instr->operation.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT &&
                    output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT) ||
                    (user_instr->operation.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT &&
                    output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) ||
                    (user_instr->operation.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT &&
                    output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT)))) {
                    ext_uses = true;
                }
            }
        }

        if (ext_uses) {
            continue;
        }

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
        REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, placeholder_value_ref, &placeholder_input_value_type));

        oper.parameters[classification.operands[i].read_index].direct.value_ref = placeholder_value_ref;
        replace = true;
    }

    if (replace) {
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &oper));
        *replaced = true;
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_transform_peephole(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    kefir_bool_t reached_fixpoint = false;

    for (; !reached_fixpoint;) {
        kefir_bool_t replaced = false;
        for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
            kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);

            for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(code, block_ref);
                instr_ref != KEFIR_ID_NONE;
                instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref)) {
                const struct kefir_codegen_target_ir_instruction *instr;
                REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));

                kefir_bool_t instr_replaced = false;
                switch (instr->operation.opcode) {
                    case KEFIR_TARGET_IR_AMD64_OPCODE(lea):
                        REQUIRE_OK(peephole_lea(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(add):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(sub):
                        REQUIRE_OK(peephole_add(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(movzx):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(movsx):
                        REQUIRE_OK(peephole_movx(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(mov):
                        REQUIRE_OK(peephole_mov(mem, code, instr, &instr_replaced));
                        break;

                    default:
                        // Intentionally left blank
                        break;
                }

                if (!instr_replaced &&
                    instr->operation.opcode != code->klass->placeholder_opcode &&
                    instr->operation.opcode != code->klass->phi_opcode &&
                    instr->operation.opcode != code->klass->upsilon_opcode &&
                    instr->operation.opcode != code->klass->inline_asm_opcode &&
                    instr->operation.opcode != code->klass->assign_opcode) {
                    REQUIRE_OK(peephole_untie(mem, code, instr, &instr_replaced));
                }

                replaced = replaced || instr_replaced;
            }
        }

        reached_fixpoint = !replaced;
    }
    return KEFIR_OK;
}
