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
#include "kefir/codegen/target-ir/transform.h"
#include "kefir/codegen/target-ir/tie.h"
#include "kefir/codegen/target-ir/control_flow.h"
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

static kefir_result_t add_produced_resource_outputs(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref) {
    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));

    kefir_uint64_t produced_resources;
    REQUIRE_OK(code->klass->instruction_resources(instr->operation.opcode, &produced_resources, NULL, code->klass->payload));
    for (kefir_size_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        if ((produced_resources >> i) & 1) {
            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, (kefir_codegen_target_ir_value_ref_t) {
                .instr_ref = instr_ref,
                .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_RESOURCE(i)
            }, &(struct kefir_codegen_target_ir_value_type) {
                .kind = KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_RESOURCE,
                .parameters.resource_id = i,
                .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
            }));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t match_int_value(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_value_ref_t value_ref, kefir_uint64_t *int_value_ptr) {
    REQUIRE(value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match target IR integral assign instruction"));

    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, value_ref.instr_ref, &instr));
    REQUIRE(instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(assign) &&
        instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER, KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match target IR integral assign instruction"));
    
    *int_value_ptr = instr->operation.parameters[0].immediate.uint_immediate;
    switch (instr->operation.parameters[0].immediate.variant) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT:
            *int_value_ptr = (kefir_uint8_t) *int_value_ptr;
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT:
            *int_value_ptr = (kefir_uint16_t) *int_value_ptr;
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT:
            *int_value_ptr = (kefir_uint32_t) *int_value_ptr;
            break;

        default:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_add(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));
    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;

    if (classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
            classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            instr->operation.parameters[classification.operands[0].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            !instr->operation.parameters[classification.operands[0].read_index].direct.tied &&
            instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {

        kefir_uint64_t value = 0;
        kefir_result_t res = match_int_value(code, instr->operation.parameters[classification.operands[1].read_index].direct.value_ref, &value);
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
            struct kefir_codegen_target_ir_operation oper = instr->operation;
            oper.parameters[classification.operands[1].read_index] = (struct kefir_codegen_target_ir_operand) {
                .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                .immediate = {
                    .uint_immediate = value,
                    .variant = oper.parameters[classification.operands[1].read_index].direct.variant
                }
            };
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &oper));
            *replaced = true;
            return KEFIR_OK;
        }
    }

    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value_ref;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &used_value_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        REQUIRE(used_value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK);
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    
    if (classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
            classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            instr->operation.parameters[classification.operands[0].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            (instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER ||
            instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UINTEGER) &&
            instr->operation.parameters[classification.operands[1].read_index].immediate.int_immediate == 0) {
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_value(mem, code, instr->operation.parameters[classification.operands[0].read_index].direct.value_ref,
            classification.operands[0].output));
        REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
        *replaced = true;
        return KEFIR_OK;
    }

    REQUIRE(instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
        instr->operation.parameters[0].direct.value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0) &&
        (instr->operation.parameters[0].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
            instr->operation.parameters[0].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT), KEFIR_OK);
    REQUIRE(instr->operation.parameters[1].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER &&
        (instr->operation.parameters[1].immediate.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
        instr->operation.parameters[1].immediate.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT), KEFIR_OK);

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
        REQUIRE_OK(add_produced_resource_outputs(mem, code, replacement_ref));
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

static kefir_result_t peephole_imul(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    REQUIRE(classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE &&
            classification.classification.operands[1].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
            classification.classification.operands[2].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
            classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            classification.operands[2].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            !instr->operation.parameters[classification.operands[1].read_index].direct.tied &&
            instr->operation.parameters[classification.operands[2].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER, KEFIR_OK);

    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value_ref;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &used_value_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        REQUIRE(used_value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK);
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    switch (instr->operation.parameters[classification.operands[2].read_index].immediate.int_immediate) {
        case 0: {
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
            REQUIRE_OK(add_produced_resource_outputs(mem, code, zero_ref.instr_ref));

            REQUIRE_OK(kefir_codegen_target_ir_code_replace_value(mem, code, zero_ref,
                classification.operands[0].output));
            REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
            *replaced = true;
        } break;

        case 1:
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_value(mem, code, instr->operation.parameters[classification.operands[1].read_index].direct.value_ref,
                classification.operands[0].output));
            REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
            *replaced = true;
            break;

        default:
            for (kefir_uint32_t i = 1; i < sizeof(kefir_uint32_t) * CHAR_BIT - 1; i++) {
                if (instr->operation.parameters[classification.operands[2].read_index].immediate.int_immediate == (1 << i)) {
                    kefir_codegen_target_ir_value_ref_t shift_ref = {
                        .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
                    };

                    struct kefir_codegen_target_ir_instruction_metadata metadata = instr->metadata;
                    REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, instr->block_ref, instr_ref, 
                        &(struct kefir_codegen_target_ir_operation) {
                            .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(shl),
                            .parameters[0] = instr->operation.parameters[classification.operands[1].read_index],
                            .parameters[1] = {
                                .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                                .immediate.int_immediate = i,
                                .immediate.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                            }
                        }, &metadata, &shift_ref.instr_ref));

                    const struct kefir_codegen_target_ir_value_type *output_value_type;
                    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, classification.operands[0].output,
                        &output_value_type));
                    struct kefir_codegen_target_ir_value_type shift_value_type = *output_value_type;
                    REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, shift_ref, &shift_value_type));

                    REQUIRE_OK(add_produced_resource_outputs(mem, code, shift_ref.instr_ref));

                    REQUIRE_OK(kefir_codegen_target_ir_code_replace_value(mem, code, shift_ref,
                        classification.operands[0].output));
                    REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
                    *replaced = true;
                    break;
                }
            }
            break;
    }

    return KEFIR_OK;
}

static kefir_result_t peephole_xor(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
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

    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value_ref;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &used_value_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        REQUIRE(used_value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK);
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

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

static kefir_result_t peephole_test(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    REQUIRE(classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
            classification.classification.operands[1].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
            classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            instr->operation.parameters[classification.operands[0].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF, KEFIR_OK);

    if (kefir_codegen_target_ir_code_control_prev(code, instr_ref) != KEFIR_ID_NONE &&
        kefir_codegen_target_ir_code_control_prev(code, kefir_codegen_target_ir_code_control_prev(code, instr_ref)) != KEFIR_ID_NONE) {
        const struct kefir_codegen_target_ir_instruction *prev_instr, *prev2_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, kefir_codegen_target_ir_code_control_prev(code, instr_ref), &prev_instr));
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, kefir_codegen_target_ir_code_control_prev(code, kefir_codegen_target_ir_code_control_prev(code, instr_ref)), &prev2_instr));

        if (prev2_instr->operation.opcode == instr->operation.opcode) {
            struct kefir_codegen_target_ir_tie_classification prev2_classification;
            REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, prev2_instr->instr_ref, &prev2_classification));
            if (prev2_classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
                prev2_classification.classification.operands[1].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
                prev2_classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                prev2_classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                prev2_instr->operation.parameters[prev2_classification.operands[0].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                prev2_instr->operation.parameters[prev2_classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&instr->operation.parameters[classification.operands[0].read_index].direct.value_ref) ==
                    KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&prev2_instr->operation.parameters[prev2_classification.operands[0].read_index].direct.value_ref) &&
                KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&instr->operation.parameters[classification.operands[1].read_index].direct.value_ref) ==
                    KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&prev2_instr->operation.parameters[prev2_classification.operands[1].read_index].direct.value_ref) &&
                instr->operation.parameters[classification.operands[0].read_index].direct.variant ==
                    prev2_instr->operation.parameters[prev2_classification.operands[0].read_index].direct.variant &&
                instr->operation.parameters[classification.operands[1].read_index].direct.variant ==
                    prev2_instr->operation.parameters[prev2_classification.operands[1].read_index].direct.variant) {
                switch (prev_instr->operation.opcode) {
                    case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setnp):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setp):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setns):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(sets):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setb):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setnb):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setae):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setg):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setge):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setl):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setle):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(seta):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setbe):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(seto):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setno):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setc):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setnc):
                        REQUIRE_OK(kefir_codegen_target_ir_code_replace_instruction(mem, code, prev2_instr->instr_ref, instr_ref));
                        REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
                        *replaced = true;
                        return KEFIR_OK;

                    default:
                        // Intentionally left blank
                        break;
                }
            }
        }
    }

    REQUIRE(instr->operation.parameters[classification.operands[0].read_index].direct.variant ==
                instr->operation.parameters[classification.operands[1].read_index].direct.variant &&
            instr->operation.parameters[classification.operands[0].read_index].direct.variant !=
                KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT &&
            instr->operation.parameters[classification.operands[0].read_index].direct.value_ref.instr_ref ==
                instr->operation.parameters[classification.operands[1].read_index].direct.value_ref.instr_ref &&
            instr->operation.parameters[classification.operands[0].read_index].direct.value_ref.aspect ==
                instr->operation.parameters[classification.operands[1].read_index].direct.value_ref.aspect, KEFIR_OK);

    const struct kefir_codegen_target_ir_instruction *producer_instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr->operation.parameters[classification.operands[0].read_index].direct.value_ref.instr_ref, &producer_instr));

    switch (producer_instr->operation.opcode) {
        case KEFIR_TARGET_IR_AMD64_OPCODE(movsx):
        case KEFIR_TARGET_IR_AMD64_OPCODE(movzx): {
            struct kefir_codegen_target_ir_tie_classification producer_classification;
            REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, producer_instr->instr_ref, &producer_classification));
            REQUIRE(producer_classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                producer_instr->operation.parameters[producer_classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                producer_instr->operation.parameters[producer_classification.operands[1].read_index].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT, KEFIR_OK);

            const struct kefir_codegen_target_ir_instruction *base_producer_instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, producer_instr->operation.parameters[producer_classification.operands[1].read_index].direct.value_ref.instr_ref, &base_producer_instr));
            switch (base_producer_instr->operation.opcode) {
                case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setnp):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setp):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setns):
                case KEFIR_TARGET_IR_AMD64_OPCODE(sets):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setb):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setnb):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setae):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setg):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setge):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setl):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setle):
                case KEFIR_TARGET_IR_AMD64_OPCODE(seta):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setbe):
                case KEFIR_TARGET_IR_AMD64_OPCODE(seto):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setno):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setc):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setnc): {
                    struct kefir_codegen_target_ir_operation oper = instr->operation;
                    oper.parameters[classification.operands[0].read_index].direct = producer_instr->operation.parameters[producer_classification.operands[1].read_index].direct;
                    oper.parameters[classification.operands[1].read_index].direct = producer_instr->operation.parameters[producer_classification.operands[1].read_index].direct;

                    REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref,
                        &oper));
                    *replaced = true;
                } break;

                default:
                    // Intentionally left blank
                    break;
            }
        } break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setnp):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setp):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setns):
        case KEFIR_TARGET_IR_AMD64_OPCODE(sets):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setb):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setnb):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setae):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setg):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setge):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setl):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setle):
        case KEFIR_TARGET_IR_AMD64_OPCODE(seta):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setbe):
        case KEFIR_TARGET_IR_AMD64_OPCODE(seto):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setno):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setc):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setnc): {
            struct kefir_codegen_target_ir_tie_classification producer_classification;
            REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, producer_instr->instr_ref, &producer_classification));
            REQUIRE(producer_classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
                producer_classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                producer_instr->operation.parameters[producer_classification.operands[0].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF, KEFIR_OK);

            const struct kefir_codegen_target_ir_instruction *base_producer_instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, producer_instr->operation.parameters[producer_classification.operands[0].read_index].direct.value_ref.instr_ref, &base_producer_instr));

            REQUIRE(base_producer_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(mov), KEFIR_OK);
            const struct kefir_codegen_target_ir_value_type *base_producer_value_type;
            kefir_result_t res = kefir_codegen_target_ir_code_instruction_output(code, base_producer_instr->instr_ref, 0, NULL, &base_producer_value_type);
            REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
            REQUIRE_OK(res);
            REQUIRE(base_producer_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT ||
                base_producer_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT ||
                base_producer_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT, KEFIR_OK);
            REQUIRE(base_producer_instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER, KEFIR_OK);
            REQUIRE(base_producer_instr->operation.parameters[0].immediate.int_immediate == 0, KEFIR_OK);

            struct kefir_codegen_target_ir_operation oper = instr->operation;
            oper.parameters[classification.operands[0].read_index].direct.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT;
            oper.parameters[classification.operands[1].read_index].direct.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT;

            REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref,
                &oper));
            *replaced = true;
        } break;

        default:
            // Intentionally left blank
            break;
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

static kefir_result_t peephole_setcc(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
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
    REQUIRE_OK(add_produced_resource_outputs(mem, code, placeholder_value_ref.instr_ref));
    REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, placeholder_value_ref, &placeholder_input_value_type));

    oper.parameters[classification.operands[0].read_index].direct.value_ref = placeholder_value_ref;

    REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &oper));
    *replaced = true;

    return KEFIR_OK;
}

static kefir_result_t get_single_user(const struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_instruction_ref_t *user_instr_ref) {
    UNUSED(control_flow);
    *user_instr_ref = KEFIR_ID_NONE;
    
    kefir_codegen_target_ir_instruction_ref_t single_user_instr_ref = KEFIR_ID_NONE;
    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value_ref;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, value_ref.instr_ref, &use_instr_ref, &used_value_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        const struct kefir_codegen_target_ir_instruction *user_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &user_instr));
        if (user_instr->block_ref == KEFIR_ID_NONE) {
            continue;
        }

        if (use_instr_ref != single_user_instr_ref) {
            REQUIRE(used_value_ref.aspect == value_ref.aspect, KEFIR_OK);
            REQUIRE(single_user_instr_ref == KEFIR_ID_NONE, KEFIR_OK);
        }

        single_user_instr_ref = use_instr_ref;
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    
    *user_instr_ref = single_user_instr_ref;
    return KEFIR_OK;
}

static kefir_result_t peephole_setcc_preamble(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, const kefir_codegen_target_ir_resource_id_t *resources, kefir_size_t resources_len, struct kefir_codegen_target_ir_operation *oper, kefir_codegen_target_ir_instruction_ref_t *replace_instr_ref) {
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
    REQUIRE_OK(get_single_user(code, control_flow, classification.operands[0].output, &user_instr_ref));
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
    REQUIRE_OK(get_single_user(code, control_flow, (kefir_codegen_target_ir_value_ref_t) {
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

static kefir_result_t peephole_sete(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr, (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF}, 1, &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jnz);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jz);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jne);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(je);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setne);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(sete);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_setne(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr, (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF}, 1, &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_setnp(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr, (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF}, 1, &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jp);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jnp);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setp);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setnp);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_setp(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr, (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF}, 1, &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jnp);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jp);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setnp);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setp);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_setnc(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr, (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF}, 1, &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jc);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jnc);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setc);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setnc);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_setc(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr, (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF}, 1, &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jnc);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jc);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setnc);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setc);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_setno(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr, (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF}, 1, &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jo);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jno);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(seto);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setno);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_seto(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr, (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF}, 1, &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jno);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jo);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setno);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(seto);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_setns(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr, (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF}, 1, &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(js);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jns);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(sets);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setns);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_sets(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr, (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF}, 1, &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jns);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(js);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setns);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(sets);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_setnb(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr, (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF}, 1, &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jb);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jnb);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setb);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setnb);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_setb(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr, (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF}, 1, &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jnb);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jb);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setnb);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setb);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_setge(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr,
        (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF}, 2,
        &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jl);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jge);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setl);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setge);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_setl(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr,
        (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF}, 2,
        &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jge);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jl);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setge);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setl);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_setle(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr,
        (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF}, 3,
        &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jg);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jle);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setg);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setle);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_setg(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr,
        (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF}, 3,
        &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jle);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jg);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setle);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setg);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_setbe(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr,
        (kefir_codegen_target_ir_resource_id_t[]){KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF}, 2,
        &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(ja);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jbe);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(seta);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setbe);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_seta(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    kefir_codegen_target_ir_instruction_ref_t replace_instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_operation oper;
    REQUIRE_OK(peephole_setcc_preamble(mem, code, control_flow, instr,
        (kefir_codegen_target_ir_resource_id_t[]){ KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF, KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF}, 2,
        &oper, &replace_instr_ref));
    if (replace_instr_ref != KEFIR_ID_NONE) {
        switch (oper.opcode) {
            case KEFIR_TARGET_IR_AMD64_OPCODE(jz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(je):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jbe);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(jnz):
            case KEFIR_TARGET_IR_AMD64_OPCODE(jne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(ja);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(setbe);
                break;

            case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                oper.opcode = KEFIR_TARGET_IR_AMD64_OPCODE(seta);
                break;

            default:
                return KEFIR_OK;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, replace_instr_ref, &oper));

        *replaced = true;
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_and(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
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
        }));
        *replaced = true;
    } else if (instr->operation.parameters[classification.operands[1].read_index].immediate.uint_immediate < (1 << 7)) {
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
            if (user_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(movsx) ||
                user_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(movzx)) {
                REQUIRE_OK(kefir_codegen_target_ir_code_replace_instruction(mem, code, instr_ref, use_instr_ref));
                REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, use_instr_ref));
                *replaced = true;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_div(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    REQUIRE(classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
            classification.classification.operands[1].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
            classification.classification.operands[2].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
            classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            classification.operands[2].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            instr->operation.parameters[classification.operands[0].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER &&
            instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            instr->operation.parameters[classification.operands[2].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF, KEFIR_OK);

    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value_ref;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &used_value_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        REQUIRE(used_value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK);
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_codegen_target_ir_operation oper = instr->operation;
    for (kefir_uint32_t i = 0; i < sizeof(kefir_uint64_t) * CHAR_BIT; i++) {
        if (oper.parameters[classification.operands[0].read_index].immediate.uint_immediate == (1ull << i)) {
            const struct kefir_codegen_target_ir_value_type *output_value_type, *arg1_value_type, *arg2_value_type;
            kefir_codegen_target_ir_value_ref_t output_value_ref = {
                .instr_ref = instr_ref,
                .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
            };
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, output_value_ref, &output_value_type));
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, oper.parameters[classification.operands[1].read_index].direct.value_ref, &arg1_value_type));
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, oper.parameters[classification.operands[2].read_index].direct.value_ref, &arg2_value_type));
            struct kefir_codegen_target_ir_value_type arg1_value_type_copy = *arg1_value_type;
            struct kefir_codegen_target_ir_value_type arg2_value_type_copy = *arg2_value_type;

            kefir_codegen_target_ir_instruction_ref_t single_user_ref;
            REQUIRE_OK(get_single_user(code, NULL, oper.parameters[classification.operands[1].read_index].direct.value_ref, &single_user_ref));
            if (single_user_ref == instr_ref) {
                arg1_value_type_copy.constraint.type = KEFIR_CODEGEN_TARGET_IR_ALLOCATION_NO_CONSTRAINT;
            }
            REQUIRE_OK(get_single_user(code, NULL, oper.parameters[classification.operands[2].read_index].direct.value_ref, &single_user_ref));
            if (single_user_ref == instr_ref) {
                arg2_value_type_copy.constraint.type = KEFIR_CODEGEN_TARGET_IR_ALLOCATION_NO_CONSTRAINT;
            }

            kefir_codegen_target_ir_value_ref_t shr_value_ref = {
                .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
            };
            struct kefir_codegen_target_ir_instruction_metadata metadata = instr->metadata;
            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, instr->block_ref, instr_ref,
                &(struct kefir_codegen_target_ir_operation) {
                .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(shr),
                .parameters[0] = oper.parameters[classification.operands[1].read_index],
                .parameters[1] = {
                    .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                    .immediate = {
                        .int_immediate = i,
                        .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                    }
                }
            }, &metadata, &shr_value_ref.instr_ref));

            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, shr_value_ref, &(struct kefir_codegen_target_ir_value_type) {
                .kind = output_value_type->kind,
                .metadata = output_value_type->metadata,
                .variant = output_value_type->variant != KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                    ? output_value_type->variant
                    : KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT
            }));
            REQUIRE_OK(add_produced_resource_outputs(mem, code, shr_value_ref.instr_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_value(mem, code, shr_value_ref, output_value_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
            *replaced = true;

            REQUIRE_OK(kefir_codegen_target_ir_code_replace_aspect(code, oper.parameters[classification.operands[1].read_index].direct.value_ref, &arg1_value_type_copy));
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_aspect(code, oper.parameters[classification.operands[2].read_index].direct.value_ref, &arg2_value_type_copy));
            break;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t peephole_idiv(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    REQUIRE(classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
            classification.classification.operands[1].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
            classification.classification.operands[2].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
            classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            classification.operands[2].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            instr->operation.parameters[classification.operands[0].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER &&
            instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            instr->operation.parameters[classification.operands[2].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF, KEFIR_OK);

    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t used_value_ref;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &used_value_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
        REQUIRE(used_value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_OK);
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_codegen_target_ir_operation oper = instr->operation;
    for (kefir_uint32_t i = 0; i < sizeof(kefir_uint32_t) * CHAR_BIT - 1; i++) {
        kefir_int64_t imm = oper.parameters[classification.operands[0].read_index].immediate.int_immediate;
        kefir_bool_t neg = false;
        if (imm < 0) {
            imm *= -1;
            neg = true;
        }
        if (imm == (1ll << i)) {
            const struct kefir_codegen_target_ir_value_type *output_value_type, *arg1_value_type, *arg2_value_type;
            kefir_codegen_target_ir_value_ref_t output_value_ref = {
                .instr_ref = instr_ref,
                .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
            };
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, output_value_ref, &output_value_type));
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, oper.parameters[classification.operands[1].read_index].direct.value_ref, &arg1_value_type));
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, oper.parameters[classification.operands[2].read_index].direct.value_ref, &arg2_value_type));
            struct kefir_codegen_target_ir_value_type arg1_value_type_copy = *arg1_value_type;
            struct kefir_codegen_target_ir_value_type arg2_value_type_copy = *arg2_value_type;
            struct kefir_codegen_target_ir_value_type output_value_type_copy = *output_value_type;

            kefir_codegen_target_ir_operand_variant_t variant = oper.parameters[classification.operands[1].read_index].direct.variant != KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                ? oper.parameters[classification.operands[1].read_index].direct.variant
                : KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT;


            struct kefir_codegen_target_ir_use_iterator use_iter;
            kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
            kefir_codegen_target_ir_value_ref_t used_value_ref;
            for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, oper.parameters[classification.operands[1].read_index].direct.value_ref.instr_ref, &use_instr_ref, &used_value_ref);
                res == KEFIR_OK;
                res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
                const struct kefir_codegen_target_ir_instruction *user_instr;
                REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &user_instr));
                if (user_instr->block_ref == KEFIR_ID_NONE) {
                    continue;
                }

                REQUIRE(use_instr_ref == instr_ref ||
                    use_instr_ref == oper.parameters[classification.operands[2].read_index].direct.value_ref.instr_ref, KEFIR_OK);
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
            arg1_value_type_copy.constraint.type = KEFIR_CODEGEN_TARGET_IR_ALLOCATION_NO_CONSTRAINT;

            kefir_codegen_target_ir_instruction_ref_t single_user_ref;
            REQUIRE_OK(get_single_user(code, NULL, oper.parameters[classification.operands[2].read_index].direct.value_ref, &single_user_ref));
            if (single_user_ref == instr_ref) {
                arg2_value_type_copy.constraint.type = KEFIR_CODEGEN_TARGET_IR_ALLOCATION_NO_CONSTRAINT;
            }

            kefir_codegen_target_ir_block_ref_t block_ref = instr->block_ref;
            struct kefir_codegen_target_ir_instruction_metadata metadata = instr->metadata;

            kefir_codegen_target_ir_instruction_ref_t test_instr_ref;
            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref, instr_ref,
                &(struct kefir_codegen_target_ir_operation) {
                .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(test),
                .parameters[0] = oper.parameters[classification.operands[1].read_index],
                .parameters[1] = oper.parameters[classification.operands[1].read_index]
            }, &metadata, &test_instr_ref));

            REQUIRE_OK(add_produced_resource_outputs(mem, code, test_instr_ref));

            kefir_codegen_target_ir_value_ref_t lea_value_ref = {
                .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
            };
            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref, test_instr_ref,
                &(struct kefir_codegen_target_ir_operation) {
                .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(lea),
                .parameters[0] = {
                    .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT,
                    .indirect = {
                        .type = KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS,
                        .base = {
                            .value_ref = oper.parameters[classification.operands[1].read_index].direct.value_ref,
                        },
                        .offset = (1ll << i) - 1,
                        .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                    }
                }
            }, &metadata, &lea_value_ref.instr_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, lea_value_ref, &(struct kefir_codegen_target_ir_value_type) {
                .kind = KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE,
                .metadata.value_ref = KEFIR_CODEGEN_TARGET_IR_METADATA_VALUE_REF_NONE,
                .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
            }));
            REQUIRE_OK(add_produced_resource_outputs(mem, code, lea_value_ref.instr_ref));

            kefir_codegen_target_ir_value_ref_t cmovns_value_ref = {
                .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
            };
            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref, lea_value_ref.instr_ref,
                &(struct kefir_codegen_target_ir_operation) {
                .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(cmovns),
                .parameters[0] = {
                    .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                    .direct = {
                        .value_ref = lea_value_ref,
                        .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                    }
                },
                .parameters[1] = {
                    .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                    .direct = {
                        .value_ref = oper.parameters[classification.operands[1].read_index].direct.value_ref,
                        .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                    }
                },
                .parameters[2] = {
                    .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                    .direct = {
                        .value_ref = {
                            .instr_ref = test_instr_ref,
                            .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_RESOURCE(KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF)
                        },
                        .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
                    }
                }
            }, &metadata, &cmovns_value_ref.instr_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, cmovns_value_ref, &(struct kefir_codegen_target_ir_value_type) {
                .kind = KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE,
                .metadata.value_ref = KEFIR_CODEGEN_TARGET_IR_METADATA_VALUE_REF_NONE,
                .variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT
            }));
            REQUIRE_OK(add_produced_resource_outputs(mem, code, cmovns_value_ref.instr_ref));

            kefir_codegen_target_ir_value_ref_t sar_value_ref = {
                .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
            };
            REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref, cmovns_value_ref.instr_ref,
                &(struct kefir_codegen_target_ir_operation) {
                .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(sar),
                .parameters[0] = {
                    .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                    .direct = {
                        .value_ref = cmovns_value_ref,
                        .variant = variant
                    }
                },
                .parameters[1] = {
                    .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                    .immediate = {
                        .int_immediate = i,
                        .variant = variant
                    }
                }
            }, &metadata, &sar_value_ref.instr_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, sar_value_ref, &(struct kefir_codegen_target_ir_value_type) {
                .kind = output_value_type_copy.kind,
                .metadata.value_ref = neg
                    ? KEFIR_CODEGEN_TARGET_IR_METADATA_VALUE_REF_NONE
                    : output_value_type_copy.metadata.value_ref,
                .variant = variant
            }));
            REQUIRE_OK(add_produced_resource_outputs(mem, code, sar_value_ref.instr_ref));

            kefir_codegen_target_ir_value_ref_t result_ref = sar_value_ref;
            if (neg) {
                kefir_codegen_target_ir_value_ref_t neg_value_ref = {
                    .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
                };
                REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref, sar_value_ref.instr_ref,
                    &(struct kefir_codegen_target_ir_operation) {
                    .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(neg),
                    .parameters[0] = {
                        .type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                        .direct = {
                            .value_ref = sar_value_ref,
                            .variant = variant
                        }
                    }
                }, &metadata, &neg_value_ref.instr_ref));
                REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, neg_value_ref, &(struct kefir_codegen_target_ir_value_type) {
                    .kind = output_value_type_copy.kind,
                    .metadata.value_ref = output_value_type_copy.metadata.value_ref,
                    .variant = variant
                }));
                REQUIRE_OK(add_produced_resource_outputs(mem, code, neg_value_ref.instr_ref));
                result_ref = neg_value_ref;
            }

            REQUIRE_OK(kefir_codegen_target_ir_code_replace_value(mem, code, result_ref, output_value_ref));
            REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
            *replaced = true;

            REQUIRE_OK(kefir_codegen_target_ir_code_replace_aspect(code, oper.parameters[classification.operands[1].read_index].direct.value_ref, &arg1_value_type_copy));
            REQUIRE_OK(kefir_codegen_target_ir_code_replace_aspect(code, oper.parameters[classification.operands[2].read_index].direct.value_ref, &arg2_value_type_copy));
            break;
        }
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
        if (classification.classification.operands[i].class != KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE ||
            classification.operands[i].read_index == KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE ||
            oper.parameters[classification.operands[i].read_index].type != KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF ||
            !oper.parameters[classification.operands[i].read_index].direct.tied) {
            continue;
        }

        kefir_codegen_target_ir_value_ref_t output_value_ref = classification.operands[i].output;
        if (!KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(output_value_ref.aspect)) {
            continue;
        }

        const struct kefir_codegen_target_ir_value_type *output_value_type;
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
                if (user_instr->operation.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                    user_instr->operation.parameters[i].direct.value_ref.instr_ref == output_value_ref.instr_ref &&
                    user_instr->operation.parameters[i].direct.value_ref.aspect == output_value_ref.aspect &&
                    (user_instr->operation.parameters[i].direct.tied ||
                    !(user_instr->operation.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT ||
                     user_instr->operation.parameters[i].direct.variant == output_value_type->variant ||
                    (user_instr->operation.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT &&
                    output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) ||
                    (user_instr->operation.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT &&
                    output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT) ||
                    (user_instr->operation.parameters[i].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT &&
                    output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT)))) {
                    ext_uses = true;
                } else if (user_instr->operation.parameters[i].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT &&
                    user_instr->operation.parameters[i].indirect.type == KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS &&
                    user_instr->operation.parameters[i].indirect.base.value_ref.instr_ref == output_value_ref.instr_ref &&
                    user_instr->operation.parameters[i].indirect.base.value_ref.aspect == output_value_ref.aspect) {
                    ext_uses = true;
                }
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        if (!ext_uses) {
            oper.parameters[classification.operands[i].read_index].direct.tied = false;
            replace = true;
        }
    }

    if (replace) {
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &oper));
        *replaced = true;
    }

    return KEFIR_OK;
}

static kefir_result_t do_peephole(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct kefir_codegen_target_ir_control_flow *control_flow) {
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_build(mem, control_flow));
    kefir_bool_t reached_fixpoint = false, post_cleanup = false;

    for (; !reached_fixpoint;) {
        kefir_bool_t replaced = false;
        for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
            kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);

            for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(code, block_ref);
                instr_ref != KEFIR_ID_NONE;
                instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref)) {
                const struct kefir_codegen_target_ir_instruction *instr;
                REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));

                if (instr->operation.opcode != code->klass->upsilon_opcode &&
                    instr->operation.opcode != code->klass->phi_opcode &&
                    instr->operation.opcode != code->klass->inline_asm_opcode &&
                    instr->operation.opcode != code->klass->placeholder_opcode) {
                    kefir_bool_t untied = false;
                    REQUIRE_OK(peephole_untie(mem, code, instr, &untied));
                    if (untied) {
                        replaced = true;
                        continue;
                    }
                }

                kefir_bool_t instr_replaced = false;
                switch (instr->operation.opcode) {
                    case KEFIR_TARGET_IR_AMD64_OPCODE(lea):
                        REQUIRE_OK(peephole_lea(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(add):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(sub):
                        REQUIRE_OK(peephole_add(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(imul3):
                        REQUIRE_OK(peephole_imul(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(xor):
                        REQUIRE_OK(peephole_xor(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(movzx):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(movsx):
                        REQUIRE_OK(peephole_movx(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(mov):
                        REQUIRE_OK(peephole_mov(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                        REQUIRE_OK(peephole_sete(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                        REQUIRE_OK(peephole_setne(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(setnp):
                        REQUIRE_OK(peephole_setnp(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(setnc):
                        REQUIRE_OK(peephole_setnc(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(setc):
                        REQUIRE_OK(peephole_setc(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(setno):
                        REQUIRE_OK(peephole_setno(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(seto):
                        REQUIRE_OK(peephole_seto(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(setp):
                        REQUIRE_OK(peephole_setp(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(setns):
                        REQUIRE_OK(peephole_setns(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(sets):
                        REQUIRE_OK(peephole_sets(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(setb):
                        REQUIRE_OK(peephole_setb(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(setnb):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setae):
                        REQUIRE_OK(peephole_setnb(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(setl):
                        REQUIRE_OK(peephole_setl(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(setge):
                        REQUIRE_OK(peephole_setge(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(setg):
                        REQUIRE_OK(peephole_setg(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(setle):
                        REQUIRE_OK(peephole_setle(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(seta):
                        REQUIRE_OK(peephole_seta(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(setbe):
                        REQUIRE_OK(peephole_setbe(mem, code, control_flow, instr, &instr_replaced));
                        if (!instr_replaced) {
                            REQUIRE_OK(peephole_setcc(mem, code, instr, &instr_replaced));
                        }
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(test):
                        REQUIRE_OK(peephole_test(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(and):
                        REQUIRE_OK(peephole_and(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(div):
                        REQUIRE_OK(peephole_div(mem, code, instr, &instr_replaced));
                        break;

                    case KEFIR_TARGET_IR_AMD64_OPCODE(idiv):
                        REQUIRE_OK(peephole_idiv(mem, code, instr, &instr_replaced));
                        break;

                    default:
                        // Intentionally left blank
                        break;
                }

                if (instr_replaced) {
                    replaced = true;
                }
            }
        }

        if (!replaced && !post_cleanup) {
            REQUIRE_OK(kefir_codegen_target_ir_transform_copy_elision(mem, code));
            REQUIRE_OK(kefir_codegen_target_ir_amd64_transform_dead_code_elimination(mem, code));
            REQUIRE_OK(kefir_codegen_target_ir_transform_phi_removal(mem, code, true));
            post_cleanup = true;
            replaced = true;
        } else {
            post_cleanup = false;
        }

        reached_fixpoint = !replaced;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_transform_peephole(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    struct kefir_codegen_target_ir_control_flow control_flow;
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_init(&control_flow, code));
    kefir_result_t res = do_peephole(mem, code, &control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_control_flow_free(mem, &control_flow);
        return res;
    });
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_free(mem, &control_flow));
    return KEFIR_OK;
}
