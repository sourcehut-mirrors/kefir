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

#include "kefir/codegen//target-ir/code.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_codegen_target_ir_code_init(struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_code_class *klass) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR code"));
    REQUIRE(klass != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR code class"));

    code->code = NULL;
    code->code_length = 0;
    code->code_capacity = 0;
    code->blocks = NULL;
    code->blocks_length = 0;
    code->blocks_capacity = 0;
    code->entry_block = KEFIR_ID_NONE;
    code->klass = klass;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    for (kefir_size_t i = 0; i < code->code_length; i++) {
        if (code->code[i].operation.opcode == code->klass->phi_opcode) {
            REQUIRE_OK(kefir_hashtable_free(mem, &code->code[i].operation.phi_node.links));
        }
    }
    KEFIR_FREE(mem, code->blocks);
    KEFIR_FREE(mem, code->code);
    memset(code, 0, sizeof(struct kefir_codegen_target_ir_code));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_new_block(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_block_ref_t *block_ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(block_ref_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR block reference"));

    if (code->blocks_length + 1 >= code->blocks_capacity) {
        kefir_size_t new_capacity = MAX(code->blocks_capacity * 2, 32);
        struct kefir_codegen_target_ir_block *new_blocks = KEFIR_REALLOC(mem, code->blocks, sizeof(struct kefir_codegen_target_ir_block) * new_capacity);
        REQUIRE(new_blocks != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR code blocks"));
        code->blocks_capacity = new_capacity;
        code->blocks = new_blocks;
    }

    struct kefir_codegen_target_ir_block *block = &code->blocks[code->blocks_length];
    block->block_ref = code->blocks_length;
    block->control_flow.head = KEFIR_ID_NONE;
    block->control_flow.tail = KEFIR_ID_NONE;
    code->blocks_length++;

    if (code->entry_block == KEFIR_ID_NONE) {
        code->entry_block = block->block_ref;
    }

    *block_ref_ptr = block->block_ref;
    return KEFIR_OK;
}

kefir_size_t kefir_codegen_target_ir_code_block_count(const struct kefir_codegen_target_ir_code *code) {
    REQUIRE(code != NULL, 0);
    return code->blocks_length;;
}

kefir_codegen_target_ir_block_ref_t kefir_codegen_target_ir_code_block_at(const struct kefir_codegen_target_ir_code *code, kefir_size_t index) {
    REQUIRE(code != NULL, KEFIR_ID_NONE);
    REQUIRE(index < code->blocks_length, KEFIR_ID_NONE);

    return code->blocks[index].block_ref;
}

kefir_codegen_target_ir_instruction_ref_t kefir_codegen_target_ir_code_block_control_head(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_block_ref_t block_ref) {
    REQUIRE(code != NULL, KEFIR_ID_NONE);
    REQUIRE(block_ref < code->blocks_length, KEFIR_ID_NONE);

    struct kefir_codegen_target_ir_block *block = &code->blocks[block_ref];
    return block->control_flow.head;
}

kefir_codegen_target_ir_instruction_ref_t kefir_codegen_target_ir_code_block_control_tail(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_block_ref_t block_ref) {
    REQUIRE(code != NULL, KEFIR_ID_NONE);
    REQUIRE(block_ref < code->blocks_length, KEFIR_ID_NONE);

    struct kefir_codegen_target_ir_block *block = &code->blocks[block_ref];
    return block->control_flow.tail;
}

kefir_result_t kefir_codegen_target_ir_code_instruction(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref, const struct kefir_codegen_target_ir_instruction **instr_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr_ref != KEFIR_ID_NONE && instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested instruction"));

    ASSIGN_PTR(instr_ptr, &code->code[instr_ref]);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_new_instruction(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
    kefir_codegen_target_ir_block_ref_t block_ref, kefir_codegen_target_ir_instruction_ref_t after_instr_ref,
    const struct kefir_codegen_target_ir_operation *operation, kefir_codegen_target_ir_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(block_ref != KEFIR_ID_NONE && block_ref < code->blocks_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR block reference"));
    REQUIRE(operation != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR operation"));
    
    struct kefir_codegen_target_ir_block *block = &code->blocks[block_ref];
    struct kefir_codegen_target_ir_instruction *after_instr = NULL, *instr = NULL;

    if (code->code_length + 1 >= code->code_capacity) {
        kefir_size_t new_capacity = MAX(code->code_capacity * 2, 128);
        struct kefir_codegen_target_ir_instruction *new_code = KEFIR_REALLOC(mem, code->code, sizeof(struct kefir_codegen_target_ir_instruction) * new_capacity);
        REQUIRE(new_code != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR code"));
        code->code_capacity = new_capacity;
        code->code = new_code;
    }

    if (after_instr_ref != KEFIR_ID_NONE) {
        REQUIRE(after_instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target codeblock reference"));
        after_instr = &code->code[after_instr_ref];
        REQUIRE(after_instr->block_ref == block_ref, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected target IR predecessor instruction to belong to the same block"));
    }
    instr = &code->code[code->code_length];

    instr->instr_ref = code->code_length;
    instr->block_ref = block_ref;
    instr->operation = *operation;

    if (instr->operation.opcode == code->klass->phi_opcode) {
        REQUIRE_OK(kefir_hashtable_init(&instr->operation.phi_node.links, &kefir_hashtable_uint_ops));
    }

    if (after_instr == NULL) {
        if (block->control_flow.head != KEFIR_ID_NONE) {
            struct kefir_codegen_target_ir_instruction *head_instr = &code->code[block->control_flow.head];
            head_instr->control_flow.prev = instr->instr_ref;
        }
        instr->control_flow.prev = KEFIR_ID_NONE;
        instr->control_flow.next = block->control_flow.head;
        block->control_flow.head = instr->instr_ref;
        if (block->control_flow.tail == KEFIR_ID_NONE) {
            block->control_flow.tail = instr->instr_ref;
        }
    } else {
        if (after_instr->control_flow.next != KEFIR_ID_NONE) {
            struct kefir_codegen_target_ir_instruction *next_instr = &code->code[after_instr->control_flow.next];
            next_instr->control_flow.prev = instr->instr_ref;
        }

        instr->control_flow.prev = after_instr->instr_ref;
        instr->control_flow.next = after_instr->control_flow.next;
        after_instr->control_flow.next = instr->instr_ref;

        if (block->control_flow.tail == after_instr->instr_ref) {
            block->control_flow.tail = instr->instr_ref;
        }
    }

    code->code_length++;
    ASSIGN_PTR(instr_ref_ptr, instr->instr_ref);
    return KEFIR_OK;
}

kefir_codegen_target_ir_instruction_ref_t kefir_codegen_target_ir_code_control_next(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref) {
    REQUIRE(code != NULL, KEFIR_ID_NONE);
    REQUIRE(instr_ref < code->code_length, KEFIR_ID_NONE);

    return code->code[instr_ref].control_flow.next;
}

kefir_codegen_target_ir_instruction_ref_t kefir_codegen_target_ir_code_control_prev(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref) {
    REQUIRE(code != NULL, KEFIR_ID_NONE);
    REQUIRE(instr_ref < code->code_length, KEFIR_ID_NONE);

    return code->code[instr_ref].control_flow.prev;
}

kefir_result_t kefir_codegen_target_ir_code_drop_instruction(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
                               kefir_codegen_target_ir_instruction_ref_t instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR instruction reference"));

    struct kefir_codegen_target_ir_instruction *instr = &code->code[instr_ref];
    REQUIRE(instr->block_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Requested target IR instruction was previously dropped"));

    struct kefir_codegen_target_ir_block *block = &code->blocks[instr->block_ref];
    if (block->control_flow.head == instr->instr_ref) {
        block->control_flow.head = instr->control_flow.next;
    }

    if (block->control_flow.tail == instr->instr_ref) {
        block->control_flow.tail = instr->control_flow.prev;
    }

    if (instr->control_flow.prev != KEFIR_ID_NONE) {
        struct kefir_codegen_target_ir_instruction *prev_instr = &code->code[instr->control_flow.prev];
        prev_instr->control_flow.next = instr->control_flow.next;
    }

    if (instr->control_flow.next != KEFIR_ID_NONE) {
        struct kefir_codegen_target_ir_instruction *next_instr = &code->code[instr->control_flow.next];
        next_instr->control_flow.prev = instr->control_flow.prev;
    }

    instr->control_flow.prev = KEFIR_ID_NONE;
    instr->control_flow.next = KEFIR_ID_NONE;
    instr->block_ref = KEFIR_ID_NONE;

    return KEFIR_OK;
}

kefir_codegen_target_ir_instruction_ref_t kefir_codegen_target_ir_code_phi_attach(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref,
    kefir_codegen_target_ir_block_ref_t block_ref, struct kefir_codegen_target_ir_value_ref linked_value_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR instruction reference"));
    REQUIRE(block_ref < code->blocks_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR block reference"));
    REQUIRE(linked_value_ref.instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR instruction reference"));
    
    struct kefir_codegen_target_ir_instruction *instr = &code->code[instr_ref];
    REQUIRE(instr->operation.opcode == code->klass->phi_opcode, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to attach a link to non-phi target IR instruction"));

    kefir_hashtable_value_t value_ref_encoded = (((kefir_uint64_t) linked_value_ref.instr_ref) << 32) | (kefir_uint32_t) linked_value_ref.aspect;

    kefir_hashtable_value_t *current_value;
    kefir_result_t res = kefir_hashtable_at_mut(&instr->operation.phi_node.links, (kefir_hashtable_key_t) block_ref, &current_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        REQUIRE(value_ref_encoded == *current_value,
            KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Target IR link for provided block reference already exists"));
        return KEFIR_OK;
    }

    REQUIRE_OK(kefir_hashtable_insert(mem, &instr->operation.phi_node.links, (kefir_hashtable_key_t) block_ref, (kefir_hashtable_value_t) value_ref_encoded));
    return KEFIR_OK;
}
