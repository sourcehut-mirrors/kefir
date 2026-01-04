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
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_target_ir_amd64_transform_dead_code_elimination(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    static kefir_bool_t DEAD_CODE_CANDIDATE_OPCODES[KEFIR_TARGET_IR_AMD64_OPCODE(num_of_opcodes) + 1] = {
        [KEFIR_TARGET_IR_AMD64_OPCODE(xchg)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(mov)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(movabs)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(movzx)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(movsx)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cmovl)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cmovle)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cmovg)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cmovge)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cmove)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cmovne)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cmovb)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cmovbe)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cmova)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cmovae)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cmovs)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cmovns)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cmovz)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cmovnz)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(lea)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(sete)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(setne)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(seta)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(setae)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(setb)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(setbe)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(setg)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(setge)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(setl)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(setle)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(setp)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(setnp)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(seto)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(sets)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(setc)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(add)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(adc)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(xadd)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(sub)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(sbb)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(mul)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(imul)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(imul1)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(imul3)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(div)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(idiv)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(shl)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(shld)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(shr)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(shrd)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(sar)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cwd)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cdq)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(cqo)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(and)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(or)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(xor)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(not)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(neg)] = true,
        [KEFIR_TARGET_IR_AMD64_OPCODE(dec)] = true
    };

    kefir_bool_t reached_fixpoint = false;

    for (; !reached_fixpoint;) {
        reached_fixpoint = true;
        for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
            kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);

            for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(code, block_ref);
                instr_ref != KEFIR_ID_NONE;) {
                const struct kefir_codegen_target_ir_instruction *instr;
                REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));
                if (!DEAD_CODE_CANDIDATE_OPCODES[instr->operation.opcode]) {
                    instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref);
                    continue;
                }

                struct kefir_codegen_target_ir_use_iterator use_iter;
                kefir_bool_t has_uses = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, NULL, NULL) != KEFIR_ITERATOR_END;
                
                struct kefir_codegen_target_ir_value_iterator value_iter;
                struct kefir_codegen_target_ir_value_ref value_ref;
                const struct kefir_codegen_target_ir_value_type *value_type;
                kefir_result_t res;
                for (res = kefir_codegen_target_ir_code_value_iter(code, &value_iter, instr_ref, &value_ref, &value_type);
                    res == KEFIR_OK && !has_uses;
                    res = kefir_codegen_target_ir_code_value_next(&value_iter, &value_ref, &value_type)) {
                    if (value_type->kind == KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT ||
                        value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
                        has_uses = true;
                    }
                }
                if (res != KEFIR_ITERATOR_END) {
                    REQUIRE_OK(res);
                }
                
                if (!has_uses) {
                    kefir_codegen_target_ir_instruction_ref_t next_instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref);
                    REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
                    instr_ref = next_instr_ref;
                    reached_fixpoint = false;
                } else {
                    instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref);
                }
            }
        }
    }
    return KEFIR_OK;
}
