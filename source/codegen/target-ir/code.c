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
#include "kefir/core/hashset.h"
#include <string.h>

struct instruction_attributes {
    struct kefir_hashset attributes;
};

static kefir_result_t free_constraint(struct kefir_mem *mem, struct kefir_hashtable *table,
                                                          kefir_hashtable_key_t key, kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_allocation_constraint *, constraint,
        value);
    REQUIRE(constraint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR allocation constraint"));

    KEFIR_FREE(mem, constraint);
    return KEFIR_OK;
}

static kefir_result_t free_attributes(struct kefir_mem *mem, struct kefir_hashtable *table,
                                                          kefir_hashtable_key_t key, kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct instruction_attributes *, attributes,
        value);
    REQUIRE(attributes != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR attributes"));

    REQUIRE_OK(kefir_hashset_free(mem, &attributes->attributes));
    KEFIR_FREE(mem, attributes);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_init(struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_code_class *klass) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR code"));
    REQUIRE(klass != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR code class"));

    code->code = NULL;
    code->code_length = 0;
    code->code_capacity = 0;
    code->blocks = NULL;
    code->blocks_length = 0;
    code->blocks_capacity = 0;
    code->value_types = NULL;
    code->value_types_length = 0;
    code->value_types_capacity = 0;
    code->entry_block = KEFIR_ID_NONE;
    code->klass = klass;

    REQUIRE_OK(kefir_hashtable_init(&code->constraints, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&code->constraints, free_constraint, NULL));
    REQUIRE_OK(kefir_hashtable_init(&code->attributes, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&code->attributes, free_attributes, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    for (kefir_size_t i = 0; i < code->code_length; i++) {
        if (code->code[i].operation.opcode == code->klass->phi_opcode) {
            REQUIRE_OK(kefir_hashtable_free(mem, &code->code[i].operation.phi_node.links));
        } else if (code->code[i].operation.opcode == code->klass->inline_asm_opcode) {   
            REQUIRE_OK(kefir_list_free(mem, &code->code[i].operation.inline_asm_node.fragments));
        }
        REQUIRE_OK(kefir_hashtable_free(mem, &code->code[i].aspects));
    }
    for (kefir_size_t i = 0; i < code->blocks_length; i++) {
        REQUIRE_OK(kefir_hashtreeset_free(mem, &code->blocks[i].public_labels));
    }
    REQUIRE_OK(kefir_hashtable_free(mem, &code->attributes));
    REQUIRE_OK(kefir_hashtable_free(mem, &code->constraints));
    KEFIR_FREE(mem, code->value_types);
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
    block->externally_visible = false;
    REQUIRE_OK(kefir_hashtreeset_init(&block->public_labels, &kefir_hashtree_str_ops));
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

kefir_codegen_target_ir_block_ref_t kefir_codegen_target_ir_code_block_by_index(const struct kefir_codegen_target_ir_code *code, kefir_size_t index) {
    REQUIRE(code != NULL, KEFIR_ID_NONE);
    REQUIRE(index < code->blocks_length, KEFIR_ID_NONE);

    return code->blocks[index].block_ref;
}

const struct kefir_codegen_target_ir_block *kefir_codegen_target_ir_code_block_at(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_block_ref_t block_ref) {
    REQUIRE(code != NULL, NULL);
    REQUIRE(block_ref != KEFIR_ID_NONE && block_ref < code->blocks_length, NULL);

    return &code->blocks[block_ref];
}

kefir_result_t kefir_codegen_target_ir_code_block_mark_externally_visible(struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_block_ref_t block_ref) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(block_ref != KEFIR_ID_NONE && block_ref < code->blocks_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR block reference"));

    struct kefir_codegen_target_ir_block *block = &code->blocks[block_ref];
    block->externally_visible = true;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_block_add_public_label(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_block_ref_t block_ref, const char *public_label) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(block_ref != KEFIR_ID_NONE && block_ref < code->blocks_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR block reference"));
    REQUIRE(public_label != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR block public label"));

    struct kefir_codegen_target_ir_block *block = &code->blocks[block_ref];
    REQUIRE_OK(kefir_hashtreeset_add(mem, &block->public_labels, (kefir_hashtreeset_entry_t) public_label));
    return KEFIR_OK;
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

static kefir_result_t free_inline_asm_node(struct kefir_mem *mem, struct kefir_list *list, struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL && entry->value, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR inline assembly fragment"));

    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_inline_assembly_fragment *, fragment,
        entry->value);
    KEFIR_FREE(mem, fragment);
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
    } else if (instr->operation.opcode == code->klass->inline_asm_opcode) {
        REQUIRE_OK(kefir_list_init(&instr->operation.inline_asm_node.fragments));
        REQUIRE_OK(kefir_list_on_remove(&instr->operation.inline_asm_node.fragments, free_inline_asm_node, NULL));
    }
    REQUIRE_OK(kefir_hashtable_init(&instr->aspects, &kefir_hashtable_uint_ops));

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

kefir_result_t kefir_codegen_target_ir_code_phi_attach(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref,
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

kefir_result_t kefir_codegen_target_ir_code_phi_link_iter(const struct kefir_codegen_target_ir_code *code,
    struct kefir_codegen_target_ir_value_phi_node_iterator *iter,
    kefir_codegen_target_ir_instruction_ref_t instr_ref,
    kefir_codegen_target_ir_block_ref_t *block_ref_ptr,
    struct kefir_codegen_target_ir_value_ref *value_ref_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR code phi link iterator"));
    REQUIRE(instr_ref != KEFIR_ID_NONE && instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR code instruction reference"));

    struct kefir_codegen_target_ir_instruction *instr = &code->code[instr_ref];
    REQUIRE(instr->operation.opcode == code->klass->phi_opcode, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected target IR phi node"));

    kefir_hashtable_key_t key;
    kefir_hashtable_value_t value;
    kefir_result_t res = kefir_hashtable_iter(&instr->operation.phi_node.links, &iter->iter, &key, &value);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR phi link iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(block_ref_ptr, (kefir_codegen_target_ir_block_ref_t) key);
    ASSIGN_PTR(value_ref_ptr, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(value));
    return KEFIR_OK;    
}

kefir_result_t kefir_codegen_target_ir_code_phi_link_next(struct kefir_codegen_target_ir_value_phi_node_iterator *iter,
    kefir_codegen_target_ir_block_ref_t *block_ref_ptr,
    struct kefir_codegen_target_ir_value_ref *value_ref_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code phi link iterator"));

    kefir_hashtable_key_t key;
    kefir_hashtable_value_t value;
    kefir_result_t res = kefir_hashtable_next(&iter->iter, &key, &value);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR phi link iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(block_ref_ptr, (kefir_codegen_target_ir_block_ref_t) key);
    ASSIGN_PTR(value_ref_ptr, KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(value));
    return KEFIR_OK;    
}

kefir_result_t kefir_codegen_target_ir_code_add_aspect(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_value_ref_t value_ref, const struct kefir_codegen_target_ir_value_type *value_type) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(value_ref.instr_ref != KEFIR_ID_NONE && value_ref.instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR code value reference"));
    REQUIRE(value_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR value type"));

    if (code->value_types_length >= code->value_types_capacity) {
        kefir_size_t new_capacity = MAX(code->value_types_capacity * 2, 128);
        struct kefir_codegen_target_ir_value_type *new_types = KEFIR_REALLOC(mem, code->value_types, sizeof(struct kefir_codegen_target_ir_value_type) * new_capacity);
        REQUIRE(new_types != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR value type"));
        code->value_types = new_types;
        code->value_types_capacity = new_capacity;
    }

    code->value_types[code->value_types_length] = *value_type;
    kefir_result_t res = kefir_hashtable_insert(mem, &code->code[value_ref.instr_ref].aspects, (kefir_hashtable_key_t) value_ref.aspect, (kefir_hashtable_value_t) code->value_types_length);
    if (res == KEFIR_ALREADY_EXISTS) {
        res = KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Target IR value aspect already exists");
    }
    REQUIRE_OK(res);

    code->value_types_length++;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_add_constraint(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_value_ref_t value_ref, const struct kefir_codegen_target_ir_allocation_constraint *constraint) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(constraint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR allocation constraint"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&code->constraints, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_allocation_constraint *, current_constraint,
            table_value);

        switch (current_constraint->type) {
            case KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT:
                REQUIRE(constraint->type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT || current_constraint->physical_register == constraint->physical_register,
                    KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Conflicting target IR allocation constraints specified"));
                break;

            case KEFIR_CODEGEN_TARGET_IR_ALLOCATION_HINT:
                if (constraint->type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_SAME_AS) {
                    *current_constraint = *constraint;
                }
                break;

            case KEFIR_CODEGEN_TARGET_IR_ALLOCATION_SAME_AS:
                if (constraint->type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_SAME_AS) {
                    *current_constraint = *constraint;
                }
                break;
        }
        return KEFIR_OK;
    }

    struct kefir_codegen_target_ir_allocation_constraint *constraint_copy = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_target_ir_allocation_constraint));
    REQUIRE(constraint_copy != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR allocation constraint"));
    memcpy(constraint_copy, constraint, sizeof(struct kefir_codegen_target_ir_allocation_constraint));

    res = kefir_hashtable_insert(mem, &code->constraints, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), (kefir_hashtable_value_t) constraint_copy);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constraint_copy);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_add_instruction_attribute(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref, kefir_codegen_target_ir_native_id_t attribute) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr_ref != KEFIR_ID_NONE && instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction reference"));

    struct instruction_attributes *attrs = NULL;
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&code->attributes, (kefir_hashtable_key_t) instr_ref, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        attrs = (struct instruction_attributes *) table_value;
    } else {
        attrs = KEFIR_MALLOC(mem, sizeof(struct instruction_attributes));
        REQUIRE(attrs != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR instruction attriubutes"));
        res = kefir_hashset_init(&attrs->attributes, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashtable_insert(mem, &code->attributes, (kefir_hashtable_key_t) instr_ref, (kefir_hashtable_value_t) attrs));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, attrs);
            return res;
        });
    }

    REQUIRE_OK(kefir_hashset_add(mem, &attrs->attributes, (kefir_hashset_key_t) attribute));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_instruction_output(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref,
    kefir_size_t output_index, kefir_codegen_target_ir_value_ref_t *value_ref_ptr, const struct kefir_codegen_target_ir_value_type **value_type_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr_ref != KEFIR_ID_NONE && instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR value reference"));
    
    kefir_codegen_target_ir_value_ref_t output_value_ref = {
        .instr_ref = instr_ref,
        .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(output_index)
    };
    kefir_result_t res = kefir_codegen_target_ir_code_value_props(code, output_value_ref, value_type_ptr, NULL);
    if (res == KEFIR_NOT_FOUND) {
        output_value_ref.aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_INDIRECT_OUTPUT(output_index);
        res = kefir_codegen_target_ir_code_value_props(code, output_value_ref, value_type_ptr, NULL);
        if (res == KEFIR_NOT_FOUND) {
            res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find specified target IR instruction output");
        }
    }

    REQUIRE_OK(res);
    ASSIGN_PTR(value_ref_ptr, output_value_ref);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_value_props(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_value_ref_t value_ref, const struct kefir_codegen_target_ir_value_type **value_type_ptr, const struct kefir_codegen_target_ir_allocation_constraint **constraint_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(value_ref.instr_ref != KEFIR_ID_NONE && value_ref.instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR value reference"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&code->code[value_ref.instr_ref].aspects, (kefir_hashtable_key_t) value_ref.aspect, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find target IR value reference");
    }
    REQUIRE_OK(res);
    ASSIGN_PTR(value_type_ptr, &code->value_types[(kefir_size_t) table_value]);

    res = kefir_hashtable_at(&code->constraints, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_PTR(constraint_ptr, (const struct kefir_codegen_target_ir_allocation_constraint *) table_value);
    } else {
        ASSIGN_PTR(constraint_ptr, NULL);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_inline_assembly_text_fragment(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref,
    const char *text) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR instruction reference"));
    REQUIRE(text != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR inline assembly text"));
    
    struct kefir_codegen_target_ir_instruction *instr = &code->code[instr_ref];
    REQUIRE(instr->operation.opcode == code->klass->inline_asm_opcode, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to attach a link to non-inline assembly target IR instruction"));

    struct kefir_codegen_target_ir_inline_assembly_fragment *fragment = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_target_ir_inline_assembly_fragment));
    REQUIRE(fragment != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR inline assembly fragment"));

    fragment->type = KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_TEXT;
    fragment->text = text;
    kefir_result_t res = kefir_list_insert_after(mem, &instr->operation.inline_asm_node.fragments, kefir_list_tail(&instr->operation.inline_asm_node.fragments), fragment);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, fragment);
        return res;
    });

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_inline_assembly_operand_fragment(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref,
    const struct kefir_codegen_target_ir_operand *operand) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR instruction reference"));
    REQUIRE(operand != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR operand"));
    
    struct kefir_codegen_target_ir_instruction *instr = &code->code[instr_ref];
    REQUIRE(instr->operation.opcode == code->klass->inline_asm_opcode, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to attach a link to non-inline assembly target IR instruction"));

    struct kefir_codegen_target_ir_inline_assembly_fragment *fragment = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_target_ir_inline_assembly_fragment));
    REQUIRE(fragment != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR inline assembly fragment"));

    fragment->type = KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_OPERAND;
    fragment->operand = *operand;
    kefir_result_t res = kefir_list_insert_after(mem, &instr->operation.inline_asm_node.fragments, kefir_list_tail(&instr->operation.inline_asm_node.fragments), fragment);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, fragment);
        return res;
    });

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_value_iter(const struct kefir_codegen_target_ir_code *code, struct kefir_codegen_target_ir_value_iterator *iter, kefir_codegen_target_ir_instruction_ref_t instr_ref, kefir_codegen_target_ir_value_ref_t *value_ref_ptr, const struct kefir_codegen_target_ir_value_type **value_type_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR value iterator"));
    REQUIRE(instr_ref != KEFIR_ID_NONE && instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction reference"));

    kefir_hashtable_key_t table_key;
    kefir_hashtable_value_t table_value;
    REQUIRE_OK(kefir_hashtable_iter(&code->code[instr_ref].aspects, &iter->iter, &table_key, &table_value));
    iter->code = code;
    iter->instr_ref = instr_ref;

    struct kefir_codegen_target_ir_value_ref value_ref = {
        .instr_ref = instr_ref,
        .aspect = table_key
    };
    ASSIGN_PTR(value_ref_ptr, value_ref);
    ASSIGN_PTR(value_type_ptr, &code->value_types[(kefir_size_t) table_value]);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_value_next(struct kefir_codegen_target_ir_value_iterator *iter, struct kefir_codegen_target_ir_value_ref *value_ref_ptr, const struct kefir_codegen_target_ir_value_type **value_type_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR value iterator"));

    kefir_hashtable_key_t table_key;
    kefir_hashtable_value_t table_value;
    REQUIRE_OK(kefir_hashtable_next(&iter->iter, &table_key, &table_value));

    struct kefir_codegen_target_ir_value_ref value_ref = {
        .instr_ref = iter->instr_ref,
        .aspect = table_key
    };
    ASSIGN_PTR(value_ref_ptr, value_ref);
    ASSIGN_PTR(value_type_ptr, &iter->code->value_types[(kefir_size_t) table_value]);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_instruction_attribute_iter(const struct kefir_codegen_target_ir_code *code,
    struct kefir_codegen_target_ir_code_attribute_iterator *iter,
    kefir_codegen_target_ir_instruction_ref_t instr_ref,
    kefir_codegen_target_ir_native_id_t *attribute_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR instruction attribute iterator"));
    REQUIRE(instr_ref != KEFIR_ID_NONE && instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction reference"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&code->attributes, (kefir_hashtable_key_t) instr_ref, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR instruction iterator");
    }
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(const struct instruction_attributes *, attributes,
        table_value);

    kefir_hashset_key_t table_key;
    res = kefir_hashset_iter(&attributes->attributes, &iter->iter, &table_key);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR instruction iterator");
    }
    REQUIRE_OK(res);
    
    ASSIGN_PTR(attribute_ptr, table_key);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_instruction_attribute_next(struct kefir_codegen_target_ir_code_attribute_iterator *iter,
    kefir_codegen_target_ir_native_id_t *attribute_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction attribute iterator"));

    kefir_hashset_key_t table_key;
    kefir_result_t res = kefir_hashset_next(&iter->iter, &table_key);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR instruction iterator");
    }
    REQUIRE_OK(res);
    
    ASSIGN_PTR(attribute_ptr, table_key);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_inline_assembly_fragment_iter(const struct kefir_codegen_target_ir_code *code,
    struct kefir_codegen_target_ir_code_inline_assembly_fragment_iterator *iter,
    kefir_codegen_target_ir_instruction_ref_t instr_ref,
    const struct kefir_codegen_target_ir_inline_assembly_fragment **fragment_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR inline assembly fragment iterator"));
    REQUIRE(instr_ref != KEFIR_ID_NONE && instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction reference"));
    
    struct kefir_codegen_target_ir_instruction *instr = &code->code[instr_ref];
    REQUIRE(instr->operation.opcode == code->klass->inline_asm_opcode, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected target IR inline assembly node"));

    iter->iter = kefir_list_head(&instr->operation.inline_asm_node.fragments);
    REQUIRE(iter->iter != NULL, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR inline assembly fragment iterator"));

    ASSIGN_PTR(fragment_ptr, iter->iter->value);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_inline_assembly_fragment_next(struct kefir_codegen_target_ir_code_inline_assembly_fragment_iterator *iter, const struct kefir_codegen_target_ir_inline_assembly_fragment **fragment_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR inline assembly fragment iterator"));

    kefir_list_next(&iter->iter);
    REQUIRE(iter->iter != NULL, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR inline assembly fragment iterator"));

    ASSIGN_PTR(fragment_ptr, iter->iter->value);
    return KEFIR_OK;
}
