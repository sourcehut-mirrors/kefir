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

#include "kefir/codegen/target-ir/amd64/late_transform.h"
#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t verify_epilogue(const struct kefir_codegen_target_ir_code *code,
                                      kefir_codegen_target_ir_instruction_ref_t head_ref) {
    for (kefir_codegen_target_ir_instruction_ref_t iter_ref = head_ref; iter_ref != KEFIR_ID_NONE;
         iter_ref = kefir_codegen_target_ir_code_control_next(code, iter_ref)) {
        const struct kefir_codegen_target_ir_instruction *instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, iter_ref, &instr));

        REQUIRE(instr->operation.opcode == code->klass->function_epilogue_opcode ||
                    instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(ret),
                KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to materialize epilogue block"));
    }
    return KEFIR_OK;
}

static kefir_result_t materialize_epilogue(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
                                           kefir_codegen_target_ir_instruction_ref_t head_ref,
                                           kefir_codegen_target_ir_block_ref_t *epilogue_block_ref) {
    REQUIRE_OK(verify_epilogue(code, head_ref));

    REQUIRE_OK(kefir_codegen_target_ir_code_new_block(mem, code, epilogue_block_ref));
    for (kefir_codegen_target_ir_instruction_ref_t iter_ref = head_ref; iter_ref != KEFIR_ID_NONE;
         iter_ref = kefir_codegen_target_ir_code_control_next(code, iter_ref)) {
        REQUIRE_OK(kefir_codegen_target_ir_code_copy_instruction(
            mem, code, *epilogue_block_ref, kefir_codegen_target_ir_code_block_control_tail(code, *epilogue_block_ref),
            iter_ref, NULL));
    }
    return KEFIR_OK;
}

static kefir_result_t do_late_split_epilogue(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
                                             kefir_codegen_target_ir_block_ref_t block_ref,
                                             kefir_codegen_target_ir_block_ref_t *epilogue_block_ref) {
    for (kefir_codegen_target_ir_instruction_ref_t iter_ref =
             kefir_codegen_target_ir_code_block_control_head(code, block_ref);
         iter_ref != KEFIR_ID_NONE; iter_ref = kefir_codegen_target_ir_code_control_next(code, iter_ref)) {
        const struct kefir_codegen_target_ir_instruction *instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, iter_ref, &instr));

        if (instr->operation.opcode != code->klass->function_epilogue_opcode) {
            continue;
        }
        if (iter_ref == kefir_codegen_target_ir_code_block_control_head(code, block_ref)) {
            if (*epilogue_block_ref == KEFIR_ID_NONE) {
                REQUIRE_OK(verify_epilogue(code, iter_ref));
                *epilogue_block_ref = block_ref;
            }
            return KEFIR_OK;
        }

        if (*epilogue_block_ref == KEFIR_ID_NONE) {
            REQUIRE_OK(materialize_epilogue(mem, code, iter_ref, epilogue_block_ref));
        }

        for (; iter_ref != KEFIR_ID_NONE;) {
            kefir_codegen_target_ir_instruction_ref_t next_iter_ref =
                kefir_codegen_target_ir_code_control_next(code, iter_ref);
            REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, iter_ref));
            iter_ref = next_iter_ref;
        }
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(
            mem, code, block_ref, kefir_codegen_target_ir_code_block_control_tail(code, block_ref),
            &(struct kefir_codegen_target_ir_operation) {
                .opcode = KEFIR_TARGET_IR_AMD64_OPCODE(jmp),
                .parameters[0] = {.type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF,
                                  .block_ref = *epilogue_block_ref}},
            NULL, NULL));
        break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_transform_late_split_epilogue(struct kefir_mem *mem,
                                                                           struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    kefir_codegen_target_ir_block_ref_t epilogue_block_ref = KEFIR_ID_NONE;
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        if (block_ref == epilogue_block_ref) {
            continue;
        }
        kefir_result_t res = do_late_split_epilogue(mem, code, block_ref, &epilogue_block_ref);
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}