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
    code->use_entries = NULL;
    code->use_entries_length = 0;
    code->use_entries_capacity = 0;
    code->entry_block = KEFIR_ID_NONE;
    code->indirect_jump_gate_block = KEFIR_ID_NONE;
    code->klass = klass;

    REQUIRE_OK(kefir_string_pool_init(&code->strings));
    REQUIRE_OK(kefir_hashtable_init(&code->attributes, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&code->attributes, free_attributes, NULL));
    REQUIRE_OK(kefir_hashset_init(&code->gate_blocks, &kefir_hashtable_uint_ops));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    for (kefir_size_t i = 0; i < code->code_length; i++) {
        if (code->code[i].operation.opcode == code->klass->phi_opcode) {
            KEFIR_FREE(mem, code->code[i].operation.phi_node.links);
        } else if (code->code[i].operation.opcode == code->klass->inline_asm_opcode) {   
            REQUIRE_OK(kefir_list_free(mem, &code->code[i].operation.inline_asm_node.fragments));
        }
        REQUIRE_OK(kefir_hashtable_free(mem, &code->code[i].aspects.all));
    }
    for (kefir_size_t i = 0; i < code->blocks_length; i++) {
        REQUIRE_OK(kefir_hashtreeset_free(mem, &code->blocks[i].public_labels));
        REQUIRE_OK(kefir_hashset_free(mem, &code->blocks[i].phi_refs));
    }
    REQUIRE_OK(kefir_hashset_free(mem, &code->gate_blocks));
    REQUIRE_OK(kefir_hashtable_free(mem, &code->attributes));
    REQUIRE_OK(kefir_string_pool_free(mem, &code->strings));
    KEFIR_FREE(mem, code->use_entries);
    KEFIR_FREE(mem, code->value_types);
    KEFIR_FREE(mem, code->blocks);
    KEFIR_FREE(mem, code->code);
    memset(code, 0, sizeof(struct kefir_codegen_target_ir_code));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_reset(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    for (kefir_size_t i = 0; i < code->code_length; i++) {
        if (code->code[i].operation.opcode == code->klass->phi_opcode) {
            KEFIR_FREE(mem, code->code[i].operation.phi_node.links);
        } else if (code->code[i].operation.opcode == code->klass->inline_asm_opcode) {   
            REQUIRE_OK(kefir_list_free(mem, &code->code[i].operation.inline_asm_node.fragments));
        }
        REQUIRE_OK(kefir_hashtable_free(mem, &code->code[i].aspects.all));
    }
    for (kefir_size_t i = 0; i < code->blocks_length; i++) {
        REQUIRE_OK(kefir_hashtreeset_free(mem, &code->blocks[i].public_labels));
        REQUIRE_OK(kefir_hashset_free(mem, &code->blocks[i].phi_refs));
    }
    REQUIRE_OK(kefir_hashset_free(mem, &code->gate_blocks));
    REQUIRE_OK(kefir_hashtable_free(mem, &code->attributes));
    REQUIRE_OK(kefir_string_pool_free(mem, &code->strings));
    KEFIR_FREE(mem, code->use_entries);
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
        kefir_size_t new_capacity = MAX(code->blocks_capacity * 9 / 8, 32);
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
    REQUIRE_OK(kefir_hashset_init(&block->phi_refs, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&block->public_labels, &kefir_hashtree_str_ops));
    code->blocks_length++;

    if (code->entry_block == KEFIR_ID_NONE) {
        code->entry_block = block->block_ref;
    }

    *block_ref_ptr = block->block_ref;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_indirect_jump_gate_block(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_block_ref_t *block_ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    if (code->indirect_jump_gate_block == KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_codegen_target_ir_code_new_block(mem, code, &code->indirect_jump_gate_block));
    }
    ASSIGN_PTR(block_ref_ptr, code->indirect_jump_gate_block);
    return KEFIR_OK;
}

kefir_size_t kefir_codegen_target_ir_code_block_count(const struct kefir_codegen_target_ir_code *code) {
    REQUIRE(code != NULL, 0);
    return code->blocks_length;
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

    public_label = kefir_string_pool_insert(mem, &code->strings, public_label, NULL);
    REQUIRE(public_label != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert public label into string pool"));
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

kefir_bool_t kefir_codegen_target_ir_code_is_gate_block(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_block_ref_t block_ref) {
    REQUIRE(code != NULL, false);

    return code->indirect_jump_gate_block == block_ref || kefir_hashset_has(&code->gate_blocks, (kefir_hashset_key_t) block_ref);
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

static kefir_result_t track_use_instr(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t user_instr_ref, kefir_codegen_target_ir_instruction_ref_t instr_ref, kefir_size_t used_aspect, kefir_bool_t add_use) {
    struct kefir_codegen_target_ir_instruction *instr = &code->code[instr_ref];
    if (add_use) {
        if (code->use_entries_length >= code->use_entries_capacity) {
            kefir_size_t new_capacity = MAX(code->use_entries_capacity * 9 / 8, 128);
            struct kefir_codegen_target_ir_use_entry *new_use_entries = KEFIR_REALLOC(mem, code->use_entries, sizeof(struct kefir_codegen_target_ir_use_entry) * new_capacity);
            REQUIRE(new_use_entries != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR use entries"));
            code->use_entries = new_use_entries;
            code->use_entries_capacity = new_capacity;
        }

        code->use_entries[code->use_entries_length].next_entry = instr->use_entry_top;
        code->use_entries[code->use_entries_length].user_instr_ref = user_instr_ref;
        code->use_entries[code->use_entries_length].used_aspect = used_aspect;
        instr->use_entry_top = code->use_entries_length++;
    } else {
        kefir_size_t prev_use_entry_idx = (kefir_size_t) ~0ull;
        for (kefir_size_t use_entry_idx = instr->use_entry_top; use_entry_idx != (kefir_size_t) ~0ull;) {
            struct kefir_codegen_target_ir_use_entry *use_entry = &code->use_entries[use_entry_idx];
            kefir_size_t next_use_entry = use_entry->next_entry;
            if (use_entry->user_instr_ref != user_instr_ref || (use_entry->used_aspect != used_aspect && used_aspect != ~0ull)) {
                prev_use_entry_idx = use_entry_idx;
                use_entry_idx = next_use_entry;
            } else if (prev_use_entry_idx != (kefir_size_t) ~0ull) {
                struct kefir_codegen_target_ir_use_entry *prev_use_entry = &code->use_entries[prev_use_entry_idx];
                prev_use_entry->next_entry = next_use_entry;
                use_entry_idx = next_use_entry;
            } else {
                instr->use_entry_top = use_entry->next_entry;
                use_entry_idx = next_use_entry;
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t operand_record_uses(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t user_instr_ref, const struct kefir_codegen_target_ir_operand *operand, kefir_bool_t add_use) {
    switch (operand->type) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_PHYSICAL_REGISTER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_NATIVE:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_EXTERNAL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NATIVE_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_EXTERNAL_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87:
            // Intentionally left blank
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF:
            REQUIRE_OK(track_use_instr(mem, code, user_instr_ref, operand->direct.value_ref.instr_ref, operand->direct.value_ref.aspect, add_use));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT:
            switch (operand->indirect.type) {
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS:
                    REQUIRE_OK(track_use_instr(mem, code, user_instr_ref, operand->indirect.base.value_ref.instr_ref, operand->indirect.base.value_ref.aspect, add_use));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_PHYSICAL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_IMMEDIATE_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_NATIVE_LABEL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_EXTERNAL_LABEL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_LOCAL_AREA_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_SPILL_AREA_BASIS:
                    // Intentionally left blank
                    break;
            }
            switch (operand->indirect.index_type) {
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_VALUE_REF:
                    REQUIRE_OK(track_use_instr(mem, code, user_instr_ref, operand->indirect.index.value_ref.instr_ref, operand->indirect.index.value_ref.aspect, add_use));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_NONE:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_PHYSICAL:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UPSILON: {
            const struct kefir_codegen_target_ir_instruction *phi_instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, operand->upsilon_ref.instr_ref, &phi_instr));
            REQUIRE(phi_instr->operation.opcode == code->klass->phi_opcode, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected upsilon operand to point at phi instruction"));
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(code, operand->upsilon_ref, NULL));
            REQUIRE_OK(track_use_instr(mem, code, user_instr_ref, operand->upsilon_ref.instr_ref, operand->upsilon_ref.aspect, add_use));
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t record_uses(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t user_instr_ref, kefir_bool_t add_use) {
    const struct kefir_codegen_target_ir_instruction *user_instr = &code->code[user_instr_ref];
    if (user_instr->operation.opcode == code->klass->phi_opcode) {
        kefir_result_t res;
        struct kefir_codegen_target_ir_value_phi_link_iterator iter;
        struct kefir_codegen_target_ir_value_ref link_value_ref;
        for (res = kefir_codegen_target_ir_code_phi_link_iter(code, &iter, user_instr_ref, NULL, &link_value_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_phi_link_next(&iter, NULL, &link_value_ref)) {
            REQUIRE_OK(track_use_instr(mem, code, user_instr_ref, link_value_ref.instr_ref, link_value_ref.aspect, add_use));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    } else if (user_instr->operation.opcode == code->klass->inline_asm_opcode) {
        for (const struct kefir_list_entry *iter = kefir_list_head(&user_instr->operation.inline_asm_node.fragments);
            iter != NULL;
            kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(const struct kefir_codegen_target_ir_inline_assembly_fragment *, fragment,
                iter->value);
            switch (fragment->type) {
                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_TEXT:
                    // Intetionally left blank
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_OPERAND:
                    REQUIRE_OK(operand_record_uses(mem, code, user_instr_ref, &fragment->operand, add_use));
                    break;
            }
        }
    } else {
        for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
            REQUIRE_OK(operand_record_uses(mem, code, user_instr_ref, &user_instr->operation.parameters[i], add_use));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t store_strings_in_operand(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
                                                struct kefir_codegen_target_ir_operand *operand) {
    switch (operand->type) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_EXTERNAL_LABEL:
            operand->external_label.symbolic = kefir_string_pool_insert(mem, &code->strings, operand->external_label.symbolic, NULL);
            REQUIRE(operand->external_label.symbolic != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert external label into string pool"));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_EXTERNAL:
            operand->rip_indirection.external = kefir_string_pool_insert(mem, &code->strings, operand->rip_indirection.external, NULL);
            REQUIRE(operand->rip_indirection.external != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert external label into string pool"));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT:
            switch (operand->indirect.type) {
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_EXTERNAL_LABEL_BASIS:
                    operand->indirect.base.external_label = kefir_string_pool_insert(mem, &code->strings, operand->indirect.base.external_label, NULL);
                    REQUIRE(operand->indirect.base.external_label != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert external label into string pool"));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_PHYSICAL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_IMMEDIATE_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_NATIVE_LABEL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_LOCAL_AREA_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_SPILL_AREA_BASIS:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_PHYSICAL_REGISTER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_NATIVE:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NATIVE_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UPSILON:
            // Intentionally left blank
            break;
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_new_instruction(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
    kefir_codegen_target_ir_block_ref_t block_ref, kefir_codegen_target_ir_instruction_ref_t after_instr_ref,
    const struct kefir_codegen_target_ir_operation *operation, const struct kefir_codegen_target_ir_instruction_metadata *metadata, kefir_codegen_target_ir_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(block_ref != KEFIR_ID_NONE && block_ref < code->blocks_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR block reference"));
    REQUIRE(operation != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR operation"));
    
    struct kefir_codegen_target_ir_block *block = &code->blocks[block_ref];
    struct kefir_codegen_target_ir_instruction *after_instr = NULL, *instr = NULL;

    if (code->code_length + 1 >= code->code_capacity) {
        kefir_size_t new_capacity = MAX(code->code_capacity * 9 / 8, 128);
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
    if (metadata != NULL) {
        instr->metadata = *metadata;
    } else {
        instr->metadata.source_location.source = NULL;
        instr->metadata.source_location.line = 0;
        instr->metadata.source_location.column = 0;
        instr->metadata.code_ref = KEFIR_CODEGEN_TARGET_IR_METADATA_CODE_REF_NONE;
    }
    if (instr->metadata.source_location.source != NULL) {
        instr->metadata.source_location.source = kefir_string_pool_insert(mem, &code->strings, instr->metadata.source_location.source, NULL);
        REQUIRE(instr->metadata.source_location.source != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Unable to insert source location into string pool"));
    }

    if (instr->operation.opcode == code->klass->phi_opcode) {
        instr->operation.phi_node.links = NULL;
        instr->operation.phi_node.links_length = 0;
        instr->operation.phi_node.links_capacity = 0;
        REQUIRE_OK(kefir_hashset_add(mem, &block->phi_refs, (kefir_hashset_key_t) instr->instr_ref));
    } else if (instr->operation.opcode == code->klass->inline_asm_opcode) {
        REQUIRE_OK(kefir_list_init(&instr->operation.inline_asm_node.fragments));
        REQUIRE_OK(kefir_list_on_remove(&instr->operation.inline_asm_node.fragments, free_inline_asm_node, NULL));
        if (instr->operation.inline_asm_node.target_block_ref != KEFIR_ID_NONE) {
            REQUIRE_OK(kefir_codegen_target_ir_code_new_block(mem, code, &instr->operation.inline_asm_node.gate_block_ref));
            REQUIRE_OK(kefir_hashset_add(mem, &code->gate_blocks, (kefir_hashset_key_t) instr->operation.inline_asm_node.gate_block_ref));
            block = &code->blocks[block_ref];
        } else {
            instr->operation.inline_asm_node.gate_block_ref = KEFIR_ID_NONE;
        }
    } else {
        if (instr->operation.opcode == code->klass->upsilon_opcode) {
            REQUIRE(instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UPSILON,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected upsilon instruction to have upsilon type operand"));
        }
        for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
            REQUIRE(instr->operation.parameters[i].type != KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UPSILON ||
                instr->operation.opcode == code->klass->upsilon_opcode,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Upsilon type parameters may only be used in upsilon instructions"));
            REQUIRE_OK(store_strings_in_operand(mem, code, &instr->operation.parameters[i]));
        }
    }
    REQUIRE_OK(kefir_hashtable_init(&instr->aspects.all, &kefir_hashtable_uint_ops));
    for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_DIRECT_OUTPUT_ASPECT_CACHE; i++) {
        instr->aspects.direct_output[i] = ~0ull;
    }
    for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_INDIRECT_OUTPUT_ASPECT_CACHE; i++) {
        instr->aspects.indirect_output[i] = ~0ull;
    }
    instr->use_entry_top = (kefir_size_t) ~0ull;

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
    REQUIRE_OK(record_uses(mem, code, instr->instr_ref, true));
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

static kefir_result_t drop_instruction(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
                               kefir_codegen_target_ir_instruction_ref_t instr_ref, kefir_bool_t ignore_uses) {

    struct kefir_codegen_target_ir_instruction *instr = &code->code[instr_ref];
    REQUIRE(instr->block_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Requested target IR instruction was previously dropped"));
    REQUIRE(ignore_uses || instr->use_entry_top == (kefir_size_t) ~0ull, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to drop target IR instruction with active uses"));

    REQUIRE_OK(record_uses(mem, code, instr->instr_ref, false));

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

    if (instr->operation.opcode == code->klass->phi_opcode) {
        REQUIRE_OK(kefir_hashset_delete(&block->phi_refs, (kefir_hashset_key_t) instr_ref));
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_drop_block(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_block_ref_t block_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(block_ref < code->blocks_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR instruction reference"));

    struct kefir_codegen_target_ir_block *block = &code->blocks[block_ref];
    for (; block->control_flow.tail != KEFIR_ID_NONE;) {
        REQUIRE_OK(drop_instruction(mem, code, block->control_flow.tail, true));
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_drop_instruction(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
                               kefir_codegen_target_ir_instruction_ref_t instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR instruction reference"));

    REQUIRE_OK(drop_instruction(mem, code, instr_ref, false));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_copy_instruction(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
    kefir_codegen_target_ir_block_ref_t block_ref,
    kefir_codegen_target_ir_instruction_ref_t after_instr_ref,
    kefir_codegen_target_ir_instruction_ref_t source_instr_ref,
    kefir_codegen_target_ir_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(source_instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR instruction reference"));
    REQUIRE(block_ref < code->blocks_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR block reference"));
    
    struct kefir_codegen_target_ir_instruction *source_instr = &code->code[source_instr_ref];

    kefir_result_t res;
    kefir_codegen_target_ir_instruction_ref_t instr_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_instruction_metadata metadata = source_instr->metadata;
    if (source_instr->operation.opcode == code->klass->phi_opcode) {
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref, after_instr_ref, &(struct kefir_codegen_target_ir_operation) {
            .opcode = code->klass->phi_opcode
        }, &metadata, &instr_ref));

        for (kefir_size_t i = 0; i < source_instr->operation.phi_node.links_length; i++) {
            REQUIRE_OK(kefir_codegen_target_ir_code_phi_attach(mem, code, instr_ref, source_instr->operation.phi_node.links[i].link_block_ref,
                source_instr->operation.phi_node.links[i].link_value_ref));
        }
    } else if (source_instr->operation.opcode == code->klass->inline_asm_opcode) {
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref, after_instr_ref, &(struct kefir_codegen_target_ir_operation) {
            .opcode = code->klass->inline_asm_opcode,
            .inline_asm_node.target_block_ref = source_instr->operation.inline_asm_node.target_block_ref
        }, &metadata, &instr_ref));

        struct kefir_codegen_target_ir_code_inline_assembly_fragment_iterator iter;
        const struct kefir_codegen_target_ir_inline_assembly_fragment *fragment;
        for (res = kefir_codegen_target_ir_code_inline_assembly_fragment_iter(code, &iter, source_instr_ref, &fragment);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_inline_assembly_fragment_next(&iter, &fragment)) {
            switch (fragment->type) {
                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_TEXT:
                    REQUIRE_OK(kefir_codegen_target_ir_code_inline_assembly_text_fragment(mem, code, instr_ref, fragment->text));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_OPERAND: {
                    struct kefir_codegen_target_ir_operand operand = fragment->operand;
                    REQUIRE_OK(kefir_codegen_target_ir_code_inline_assembly_operand_fragment(mem, code, instr_ref, &operand));
                } break;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

    } else {
        struct kefir_codegen_target_ir_operation oper = source_instr->operation;
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, block_ref, after_instr_ref, &oper, &metadata, &instr_ref));
    }


    struct kefir_codegen_target_ir_value_iterator source_value_iter;
    struct kefir_codegen_target_ir_value_ref source_value_ref;
    const struct kefir_codegen_target_ir_value_type *source_value_type;
    for (res = kefir_codegen_target_ir_code_value_iter(code, &source_value_iter, source_instr_ref, &source_value_ref, &source_value_type);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_value_next(&source_value_iter, &source_value_ref, &source_value_type)) {
        const struct kefir_codegen_target_ir_value_type value_type_copy = *source_value_type;
        REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, (struct kefir_codegen_target_ir_value_ref) {
            .instr_ref = instr_ref,
            .aspect = source_value_ref.aspect
        }, &value_type_copy));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    kefir_codegen_target_ir_native_id_t attribute;
    struct kefir_codegen_target_ir_code_attribute_iterator attr_iter;
    for (res = kefir_codegen_target_ir_code_instruction_attribute_iter(code, &attr_iter, source_instr_ref, &attribute);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_instruction_attribute_next(&attr_iter, &attribute)) {
        REQUIRE_OK(kefir_codegen_target_ir_code_add_instruction_attribute(mem, code, instr_ref, attribute));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    ASSIGN_PTR(instr_ref_ptr, instr_ref);
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

    for (kefir_size_t link_idx = 0; link_idx < instr->operation.phi_node.links_length; link_idx++) {
        if (instr->operation.phi_node.links[link_idx].link_block_ref == block_ref) {
            REQUIRE(instr->operation.phi_node.links[link_idx].link_value_ref.instr_ref == linked_value_ref.instr_ref &&
                instr->operation.phi_node.links[link_idx].link_value_ref.aspect == linked_value_ref.aspect,
                KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Target IR link for provided block reference already exists"));
            return KEFIR_OK;
        }
    }

    if (instr->operation.phi_node.links_length >= instr->operation.phi_node.links_capacity) {
        kefir_size_t new_capacity = MAX(instr->operation.phi_node.links_capacity * 2, 4);
        struct kefir_codegen_target_ir_phi_link *new_links = KEFIR_REALLOC(mem, instr->operation.phi_node.links, sizeof(struct kefir_codegen_target_ir_phi_link) * new_capacity);
        REQUIRE(new_links != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR phi links"));
        instr->operation.phi_node.links = new_links;
        instr->operation.phi_node.links_capacity = new_capacity;
    }

    instr->operation.phi_node.links[instr->operation.phi_node.links_length].link_block_ref = block_ref;
    instr->operation.phi_node.links[instr->operation.phi_node.links_length].link_value_ref = linked_value_ref;
    instr->operation.phi_node.links_length++;
    REQUIRE_OK(track_use_instr(mem, code, instr_ref, linked_value_ref.instr_ref, linked_value_ref.aspect, true));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_phi_drop(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
    kefir_codegen_target_ir_instruction_ref_t instr_ref,
    kefir_codegen_target_ir_block_ref_t block_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR instruction reference"));
    REQUIRE(block_ref < code->blocks_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR block reference"));
    
    struct kefir_codegen_target_ir_instruction *instr = &code->code[instr_ref];
    REQUIRE(instr->operation.opcode == code->klass->phi_opcode, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to attach a link to non-phi target IR instruction"));

    for (kefir_size_t i = 0; i < instr->operation.phi_node.links_length; i++) {
        if (instr->operation.phi_node.links[i].link_block_ref == block_ref) {
            kefir_bool_t has_other_uses = false;
            for (kefir_size_t j = 0; !has_other_uses && j < instr->operation.phi_node.links_length; j++) {
                if (i == j) {
                    continue;
                }

                if (instr->operation.phi_node.links[i].link_value_ref.instr_ref == instr->operation.phi_node.links[j].link_value_ref.instr_ref &&
                    instr->operation.phi_node.links[i].link_value_ref.aspect == instr->operation.phi_node.links[j].link_value_ref.aspect) {
                    has_other_uses = true;
                }
            }
            if (!has_other_uses) {
                REQUIRE_OK(track_use_instr(mem, code, instr_ref, instr->operation.phi_node.links[i].link_value_ref.instr_ref, instr->operation.phi_node.links[i].link_value_ref.aspect, false));
            }
            memmove(&instr->operation.phi_node.links[i], &instr->operation.phi_node.links[i + 1], (instr->operation.phi_node.links_length - (i + 1)) * sizeof(struct kefir_codegen_target_ir_phi_link));
            instr->operation.phi_node.links_length--;
            break;
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_phi_link_for(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t phi_instr_ref,
    kefir_codegen_target_ir_block_ref_t link_block_ref, kefir_codegen_target_ir_value_ref_t *link_value_ref_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(phi_instr_ref != KEFIR_ID_NONE && phi_instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR instruction reference"));
   
    struct kefir_codegen_target_ir_instruction *phi_instr = &code->code[phi_instr_ref];
    REQUIRE(phi_instr->operation.opcode == code->klass->phi_opcode, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected target IR phi instruction"));

    for (kefir_size_t link_idx = 0; link_idx < phi_instr->operation.phi_node.links_length; link_idx++) {
        if (phi_instr->operation.phi_node.links[link_idx].link_block_ref == link_block_ref) {
            ASSIGN_PTR(link_value_ref_ptr, phi_instr->operation.phi_node.links[link_idx].link_value_ref);
            return KEFIR_OK;
        }
    }

    return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find link for block in phi node");
}

kefir_result_t kefir_codegen_target_ir_code_phi_link_iter(const struct kefir_codegen_target_ir_code *code,
    struct kefir_codegen_target_ir_value_phi_link_iterator *iter,
    kefir_codegen_target_ir_instruction_ref_t instr_ref,
    kefir_codegen_target_ir_block_ref_t *block_ref_ptr,
    struct kefir_codegen_target_ir_value_ref *value_ref_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR code phi link iterator"));
    REQUIRE(instr_ref != KEFIR_ID_NONE && instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR code instruction reference"));

    struct kefir_codegen_target_ir_instruction *instr = &code->code[instr_ref];
    REQUIRE(instr->operation.opcode == code->klass->phi_opcode, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected target IR phi node"));

    iter->phi_node = &instr->operation.phi_node;
    iter->link_index = 0;
    REQUIRE(iter->link_index < iter->phi_node->links_length, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR phi link iterator"));

    ASSIGN_PTR(block_ref_ptr, iter->phi_node->links[iter->link_index].link_block_ref);
    ASSIGN_PTR(value_ref_ptr, iter->phi_node->links[iter->link_index].link_value_ref);
    return KEFIR_OK;    
}

kefir_result_t kefir_codegen_target_ir_code_phi_link_next(struct kefir_codegen_target_ir_value_phi_link_iterator *iter,
    kefir_codegen_target_ir_block_ref_t *block_ref_ptr,
    struct kefir_codegen_target_ir_value_ref *value_ref_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code phi link iterator"));

    iter->link_index++;
    REQUIRE(iter->link_index < iter->phi_node->links_length, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR phi link iterator"));

    ASSIGN_PTR(block_ref_ptr, iter->phi_node->links[iter->link_index].link_block_ref);
    ASSIGN_PTR(value_ref_ptr, iter->phi_node->links[iter->link_index].link_value_ref);
    return KEFIR_OK;    
}

kefir_result_t kefir_codegen_target_ir_code_phi_node_iter(const struct kefir_codegen_target_ir_code *code, struct kefir_codegen_target_ir_value_phi_node_iterator *iter, kefir_codegen_target_ir_block_ref_t block_ref, kefir_codegen_target_ir_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR code block phi link iterator"));
    REQUIRE(block_ref != KEFIR_ID_NONE && block_ref < code->blocks_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR code block reference"));

    struct kefir_codegen_target_ir_block *block = &code->blocks[block_ref];

    kefir_hashset_key_t key;
    kefir_result_t res = kefir_hashset_iter(&block->phi_refs, &iter->iter, &key);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR code block phi node iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(instr_ref_ptr, (kefir_codegen_target_ir_instruction_ref_t) key);
    return KEFIR_OK; 
}

kefir_result_t kefir_codegen_target_ir_code_phi_node_next(struct kefir_codegen_target_ir_value_phi_node_iterator *iter, kefir_codegen_target_ir_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR code block phi link iterator"));

    kefir_hashset_key_t key;
    kefir_result_t res = kefir_hashset_next(&iter->iter, &key);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR code block phi node iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(instr_ref_ptr, (kefir_codegen_target_ir_instruction_ref_t) key);
    return KEFIR_OK; 
}

kefir_size_t kefir_codegen_target_ir_code_num_of_uses(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_value_ref_t value_ref) {
    REQUIRE(code != NULL, 0);

    kefir_result_t res;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    kefir_codegen_target_ir_value_ref_t use_value_ref;
    kefir_size_t count = 0;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, value_ref.instr_ref, &use_instr_ref, &use_value_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &use_value_ref)) {
        if (use_value_ref.aspect == value_ref.aspect) {
            count++;
        }
    }
    
    return count;
}

kefir_result_t kefir_codegen_target_ir_code_use_iter(const struct kefir_codegen_target_ir_code *code,
    struct kefir_codegen_target_ir_use_iterator *iter,
    kefir_codegen_target_ir_instruction_ref_t instr_ref, kefir_codegen_target_ir_instruction_ref_t *user_instr_ref_ptr,
    kefir_codegen_target_ir_value_ref_t *used_value_ref_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR code use instruction reference iterator"));
    REQUIRE(instr_ref != KEFIR_ID_NONE && instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR instruction reference"));

    struct kefir_codegen_target_ir_instruction *instr = &code->code[instr_ref];

    iter->code = code;
    iter->use_entry_index = instr->use_entry_top;
    iter->instr_ref = instr_ref;
    REQUIRE(iter->use_entry_index != (kefir_size_t) ~0ull, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR code use instruction reference iterator"));
    
    const struct kefir_codegen_target_ir_use_entry *use_entry = &code->use_entries[iter->use_entry_index];
    ASSIGN_PTR(user_instr_ref_ptr, use_entry->user_instr_ref);
    if (used_value_ref_ptr != NULL) {
        used_value_ref_ptr->instr_ref = iter->instr_ref;
        used_value_ref_ptr->aspect = use_entry->used_aspect;
    }
    return KEFIR_OK; 
}

kefir_result_t kefir_codegen_target_ir_code_use_next(struct kefir_codegen_target_ir_use_iterator *iter, kefir_codegen_target_ir_instruction_ref_t *user_instr_ref_ptr,
    kefir_codegen_target_ir_value_ref_t *used_value_ref_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code use instruction reference iterator"));

    const struct kefir_codegen_target_ir_use_entry *use_entry = &iter->code->use_entries[iter->use_entry_index];
    iter->use_entry_index = use_entry->next_entry;
    REQUIRE(iter->use_entry_index != (kefir_size_t) ~0ull, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR code use instruction reference iterator"));
    
    use_entry = &iter->code->use_entries[iter->use_entry_index];
    ASSIGN_PTR(user_instr_ref_ptr, use_entry->user_instr_ref);
    if (used_value_ref_ptr != NULL) {
        used_value_ref_ptr->instr_ref = iter->instr_ref;
        used_value_ref_ptr->aspect = use_entry->used_aspect;
    }
    return KEFIR_OK; 
}

kefir_result_t kefir_codegen_target_ir_code_add_aspect(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_value_ref_t value_ref, const struct kefir_codegen_target_ir_value_type *value_type) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(value_ref.instr_ref != KEFIR_ID_NONE && value_ref.instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR code value reference"));
    REQUIRE(value_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR value type"));

    if (code->value_types_length >= code->value_types_capacity) {
        kefir_size_t new_capacity = MAX(code->value_types_capacity * 9 / 8, 128);
        struct kefir_codegen_target_ir_value_type *new_types = KEFIR_REALLOC(mem, code->value_types, sizeof(struct kefir_codegen_target_ir_value_type) * new_capacity);
        REQUIRE(new_types != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR value type"));
        code->value_types = new_types;
        code->value_types_capacity = new_capacity;
    }

    code->value_types[code->value_types_length] = *value_type;

    if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(value_ref.aspect) &&
        KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(value_ref.aspect) < KEFIR_CODEGEN_TARGET_IR_OPERATION_DIRECT_OUTPUT_ASPECT_CACHE) {
        kefir_size_t index = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(value_ref.aspect);
        REQUIRE(code->code[value_ref.instr_ref].aspects.direct_output[index] == ~0ull, KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Target IR value aspect already exists"));
        code->code[value_ref.instr_ref].aspects.direct_output[index] = code->value_types_length;
    } else if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_INDIRECT_OUTPUT(value_ref.aspect) &&
        KEFIR_CODEGEN_TARGET_IR_VALUE_INDIRECT_OUTPUT(value_ref.aspect) < KEFIR_CODEGEN_TARGET_IR_OPERATION_INDIRECT_OUTPUT_ASPECT_CACHE) {
        kefir_size_t index = KEFIR_CODEGEN_TARGET_IR_VALUE_INDIRECT_OUTPUT(value_ref.aspect);
        REQUIRE(code->code[value_ref.instr_ref].aspects.indirect_output[index] == ~0ull, KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Target IR value aspect already exists"));
        code->code[value_ref.instr_ref].aspects.indirect_output[index] = code->value_types_length;
    }

    kefir_result_t res = kefir_hashtable_insert(mem, &code->code[value_ref.instr_ref].aspects.all, (kefir_hashtable_key_t) value_ref.aspect, (kefir_hashtable_value_t) code->value_types_length);
    if (res == KEFIR_ALREADY_EXISTS) {
        res = KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Target IR value aspect already exists");
    }
    REQUIRE_OK(res);

    code->value_types_length++;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_replace_aspect(struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_value_ref_t value_ref, const struct kefir_codegen_target_ir_value_type *value_type) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(value_ref.instr_ref != KEFIR_ID_NONE && value_ref.instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR code value reference"));
    REQUIRE(value_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR value type"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&code->code[value_ref.instr_ref].aspects.all, (kefir_hashtable_key_t) value_ref.aspect, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested target IR instruction aspect");
    }
    REQUIRE_OK(res);

    code->value_types[table_value] = *value_type;
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
    kefir_result_t res = kefir_codegen_target_ir_code_value_props(code, output_value_ref, value_type_ptr);
    if (res == KEFIR_NOT_FOUND) {
        output_value_ref.aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_INDIRECT_OUTPUT(output_index);
        res = kefir_codegen_target_ir_code_value_props(code, output_value_ref, value_type_ptr);
        if (res == KEFIR_NOT_FOUND) {
            res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find specified target IR instruction output");
        }
    }

    REQUIRE_OK(res);
    ASSIGN_PTR(value_ref_ptr, output_value_ref);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_value_props(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_value_ref_t value_ref, const struct kefir_codegen_target_ir_value_type **value_type_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(value_ref.instr_ref != KEFIR_ID_NONE && value_ref.instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR value reference"));

    if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(value_ref.aspect) &&
        KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(value_ref.aspect) < KEFIR_CODEGEN_TARGET_IR_OPERATION_DIRECT_OUTPUT_ASPECT_CACHE) {
        kefir_size_t index = code->code[value_ref.instr_ref].aspects.direct_output[KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(value_ref.aspect)];
        REQUIRE(index != ~0ull, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find target IR value reference"));
        ASSIGN_PTR(value_type_ptr, &code->value_types[index]);
    } else if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_INDIRECT_OUTPUT(value_ref.aspect) &&
        KEFIR_CODEGEN_TARGET_IR_VALUE_INDIRECT_OUTPUT(value_ref.aspect) < KEFIR_CODEGEN_TARGET_IR_OPERATION_INDIRECT_OUTPUT_ASPECT_CACHE) {
        kefir_size_t index = code->code[value_ref.instr_ref].aspects.indirect_output[KEFIR_CODEGEN_TARGET_IR_VALUE_INDIRECT_OUTPUT(value_ref.aspect)];
        REQUIRE(index != ~0ull, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find target IR value reference"));
        ASSIGN_PTR(value_type_ptr, &code->value_types[index]);
    } else {
        kefir_hashtable_value_t table_value;
        kefir_result_t res = kefir_hashtable_at(&code->code[value_ref.instr_ref].aspects.all, (kefir_hashtable_key_t) value_ref.aspect, &table_value);
        if (res == KEFIR_NOT_FOUND) {
            res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find target IR value reference");
        }
        REQUIRE_OK(res);
        ASSIGN_PTR(value_type_ptr, &code->value_types[(kefir_size_t) table_value]);
    }
    return KEFIR_OK;
}

static kefir_result_t value_ref_repace_use(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t user_instr_ref, struct kefir_codegen_target_ir_value_ref *value_ref, kefir_codegen_target_ir_instruction_ref_t to_instr_ref, kefir_codegen_target_ir_instruction_ref_t from_instr_ref) {
    if (value_ref->instr_ref == from_instr_ref) {
        kefir_result_t res = kefir_codegen_target_ir_code_value_props(code, (kefir_codegen_target_ir_value_ref_t) {
            .instr_ref = to_instr_ref,
            .aspect = value_ref->aspect
        }, NULL);
        if (res == KEFIR_NOT_FOUND) {
            res = KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to replace target IR instruction use with missing value aspect");
        }
        REQUIRE_OK(res);
        value_ref->instr_ref = to_instr_ref;
        REQUIRE_OK(track_use_instr(mem, code, user_instr_ref, to_instr_ref, value_ref->aspect, true));
    }
    return KEFIR_OK;
}

static kefir_result_t operand_replace_uses(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t user_instr_ref, struct kefir_codegen_target_ir_operand *operand, kefir_codegen_target_ir_instruction_ref_t to_instr_ref, kefir_codegen_target_ir_instruction_ref_t from_instr_ref) {
    switch (operand->type) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_PHYSICAL_REGISTER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_NATIVE:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_EXTERNAL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NATIVE_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_EXTERNAL_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87:
            // Intentionally left blank
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF:
            REQUIRE_OK(value_ref_repace_use(mem, code, user_instr_ref, &operand->direct.value_ref, to_instr_ref, from_instr_ref));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UPSILON:
            REQUIRE_OK(value_ref_repace_use(mem, code, user_instr_ref, &operand->upsilon_ref, to_instr_ref, from_instr_ref));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT:
            switch (operand->indirect.type) {
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS:
                    REQUIRE_OK(value_ref_repace_use(mem, code, user_instr_ref, &operand->indirect.base.value_ref, to_instr_ref, from_instr_ref));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_PHYSICAL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_IMMEDIATE_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_NATIVE_LABEL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_EXTERNAL_LABEL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_LOCAL_AREA_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_SPILL_AREA_BASIS:
                    // Intentionally left blank
                    break;
            }
            switch (operand->indirect.index_type) {
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_VALUE_REF:
                    REQUIRE_OK(value_ref_repace_use(mem, code, user_instr_ref, &operand->indirect.index.value_ref, to_instr_ref, from_instr_ref));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_NONE:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_PHYSICAL:
                    // Intentionally left blank
                    break;
            }
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t replace_uses(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t user_instr_ref, kefir_codegen_target_ir_instruction_ref_t to_instr_ref, kefir_codegen_target_ir_instruction_ref_t from_instr_ref) {
    struct kefir_codegen_target_ir_instruction *user_instr = &code->code[user_instr_ref];
    if (user_instr->operation.opcode == code->klass->phi_opcode) {
        for (kefir_size_t link_idx = 0; link_idx < user_instr->operation.phi_node.links_length; link_idx++) {
            struct kefir_codegen_target_ir_value_ref value_ref = user_instr->operation.phi_node.links[link_idx].link_value_ref;
            if (value_ref.instr_ref == from_instr_ref) {
                kefir_result_t res;
                res = kefir_codegen_target_ir_code_value_props(code, (kefir_codegen_target_ir_value_ref_t) {
                    .instr_ref = to_instr_ref,
                    .aspect = value_ref.aspect
                }, NULL);
                if (res == KEFIR_NOT_FOUND) {
                    res = KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to replace target IR instruction use with missing value aspect");
                }
                REQUIRE_OK(res);

                user_instr->operation.phi_node.links[link_idx].link_value_ref.instr_ref = to_instr_ref;
                REQUIRE_OK(track_use_instr(mem, code, user_instr_ref, to_instr_ref, value_ref.aspect, true));
            }
        }
    } else if (user_instr->operation.opcode == code->klass->inline_asm_opcode) {
        for (const struct kefir_list_entry *iter = kefir_list_head(&user_instr->operation.inline_asm_node.fragments);
            iter != NULL;
            kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_inline_assembly_fragment *, fragment,
                iter->value);
            switch (fragment->type) {
                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_TEXT:
                    // Intetionally left blank
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_OPERAND:
                    REQUIRE_OK(operand_replace_uses(mem, code, user_instr_ref, &fragment->operand, to_instr_ref, from_instr_ref));
            }
        }
    } else {
        for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
            REQUIRE_OK(operand_replace_uses(mem, code, user_instr_ref, &user_instr->operation.parameters[i], to_instr_ref, from_instr_ref));
        }
    }
    REQUIRE_OK(track_use_instr(mem, code, user_instr_ref, from_instr_ref, ~0ull, false));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_replace_operation(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref, const struct kefir_codegen_target_ir_operation *operation) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(operation != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR operation"));

    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));

    kefir_codegen_target_ir_instruction_ref_t inserted_ref;
    struct kefir_codegen_target_ir_instruction_metadata metadata = instr->metadata;
    REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, instr->block_ref,
        instr_ref, operation, &metadata, &inserted_ref));

    struct kefir_codegen_target_ir_value_iterator value_iter;
    struct kefir_codegen_target_ir_value_ref value_ref;
    const struct kefir_codegen_target_ir_value_type *value_type;
    kefir_result_t res;
    for (res = kefir_codegen_target_ir_code_value_iter(code, &value_iter, instr_ref, &value_ref, &value_type);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_value_next(&value_iter, &value_ref, &value_type)) {
        struct kefir_codegen_target_ir_value_type value_type_copy = *value_type;
        REQUIRE_OK(kefir_codegen_target_ir_code_add_aspect(mem, code, (kefir_codegen_target_ir_value_ref_t) {
            .instr_ref = inserted_ref,
            .aspect = value_ref.aspect
        }, &value_type_copy));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    kefir_codegen_target_ir_native_id_t attribute;
    struct kefir_codegen_target_ir_code_attribute_iterator attr_iter;
    for (res = kefir_codegen_target_ir_code_instruction_attribute_iter(code, &attr_iter, instr_ref, &attribute);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_instruction_attribute_next(&attr_iter, &attribute)) {
        REQUIRE_OK(kefir_codegen_target_ir_code_add_instruction_attribute(mem, code, inserted_ref, attribute));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    REQUIRE_OK(kefir_codegen_target_ir_code_replace_instruction(mem, code, inserted_ref, instr_ref));
    REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_replace_instruction(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t to_instr_ref, kefir_codegen_target_ir_instruction_ref_t from_instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(from_instr_ref != KEFIR_ID_NONE && from_instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR instruction reference"));
    REQUIRE(to_instr_ref != KEFIR_ID_NONE && to_instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR instruction reference"));

    struct kefir_codegen_target_ir_instruction *from_instr = &code->code[from_instr_ref];
    for (kefir_size_t use_entry_index = from_instr->use_entry_top; use_entry_index != (kefir_size_t) ~0ull; use_entry_index = code->use_entries[use_entry_index].next_entry) {
        REQUIRE_OK(replace_uses(mem, code, code->use_entries[use_entry_index].user_instr_ref, to_instr_ref, from_instr_ref));
    }
    from_instr->use_entry_top = (kefir_size_t) ~0ull;
    return KEFIR_OK;
}

static kefir_result_t track_use_value(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t user_instr_ref, kefir_codegen_target_ir_value_ref_t value_ref, kefir_bool_t add_use) {
    struct kefir_codegen_target_ir_instruction *instr = &code->code[value_ref.instr_ref];
    if (add_use) {
        if (code->use_entries_length >= code->use_entries_capacity) {
            kefir_size_t new_capacity = MAX(code->use_entries_capacity * 9 / 8, 128);
            struct kefir_codegen_target_ir_use_entry *new_use_entries = KEFIR_REALLOC(mem, code->use_entries, sizeof(struct kefir_codegen_target_ir_use_entry) * new_capacity);
            REQUIRE(new_use_entries != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR use entries"));
            code->use_entries = new_use_entries;
            code->use_entries_capacity = new_capacity;
        }

        code->use_entries[code->use_entries_length].next_entry = instr->use_entry_top;
        code->use_entries[code->use_entries_length].user_instr_ref = user_instr_ref;
        code->use_entries[code->use_entries_length].used_aspect = value_ref.aspect;
        instr->use_entry_top = code->use_entries_length++;
    } else {
        kefir_size_t prev_use_entry_idx = (kefir_size_t) ~0ull;
        for (kefir_size_t use_entry_idx = instr->use_entry_top; use_entry_idx != (kefir_size_t) ~0ull;) {
            struct kefir_codegen_target_ir_use_entry *use_entry = &code->use_entries[use_entry_idx];
            kefir_size_t next_use_entry = use_entry->next_entry;
            if (use_entry->user_instr_ref != user_instr_ref || use_entry->used_aspect != value_ref.aspect) {
                prev_use_entry_idx = use_entry_idx;
                use_entry_idx = next_use_entry;
            } else if (prev_use_entry_idx != (kefir_size_t) ~0ull) {
                struct kefir_codegen_target_ir_use_entry *prev_use_entry = &code->use_entries[prev_use_entry_idx];
                prev_use_entry->next_entry = next_use_entry;
                use_entry_idx = next_use_entry;
            } else {
                instr->use_entry_top = use_entry->next_entry;
                use_entry_idx = next_use_entry;
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t value_ref_replace(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t user_instr_ref, struct kefir_codegen_target_ir_value_ref *value_ref, kefir_codegen_target_ir_value_ref_t to_value_ref, kefir_codegen_target_ir_value_ref_t from_value_ref) {
    if (value_ref->instr_ref == from_value_ref.instr_ref && value_ref->aspect == from_value_ref.aspect) {
        *value_ref = to_value_ref;
        REQUIRE_OK(track_use_value(mem, code, user_instr_ref, to_value_ref, true));
    }
    return KEFIR_OK;
}

static kefir_result_t operand_replace_value_uses(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t user_instr_ref, struct kefir_codegen_target_ir_operand *operand, kefir_codegen_target_ir_value_ref_t to_value_ref, kefir_codegen_target_ir_value_ref_t from_value_ref) {
    switch (operand->type) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_PHYSICAL_REGISTER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_NATIVE:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_EXTERNAL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NATIVE_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_EXTERNAL_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87:
            // Intentionally left blank
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF:
            REQUIRE_OK(value_ref_replace(mem, code, user_instr_ref, &operand->direct.value_ref, to_value_ref, from_value_ref));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UPSILON:
            REQUIRE_OK(value_ref_replace(mem, code, user_instr_ref, &operand->upsilon_ref, to_value_ref, from_value_ref));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT:
            switch (operand->indirect.type) {
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS:
                    REQUIRE_OK(value_ref_replace(mem, code, user_instr_ref, &operand->indirect.base.value_ref, to_value_ref, from_value_ref));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_PHYSICAL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_IMMEDIATE_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_NATIVE_LABEL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_EXTERNAL_LABEL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_LOCAL_AREA_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_SPILL_AREA_BASIS:
                    // Intentionally left blank
                    break;
            }
            switch (operand->indirect.index_type) {
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_VALUE_REF:
                    REQUIRE_OK(value_ref_replace(mem, code, user_instr_ref, &operand->indirect.index.value_ref, to_value_ref, from_value_ref));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_NONE:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_INDEX_PHYSICAL:
                    // Intentionally left blank
                    break;
            }
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t replace_value_uses(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t user_instr_ref, kefir_codegen_target_ir_value_ref_t to_value_ref, kefir_codegen_target_ir_value_ref_t from_value_ref) {
    struct kefir_codegen_target_ir_instruction *user_instr = &code->code[user_instr_ref];
    if (user_instr->operation.opcode == code->klass->phi_opcode) {
        for (kefir_size_t link_idx = 0; link_idx < user_instr->operation.phi_node.links_length; link_idx++) {
            struct kefir_codegen_target_ir_value_ref value_ref = user_instr->operation.phi_node.links[link_idx].link_value_ref;
            if (value_ref.instr_ref == from_value_ref.instr_ref && value_ref.aspect == from_value_ref.aspect) {
                user_instr->operation.phi_node.links[link_idx].link_value_ref = to_value_ref;
                REQUIRE_OK(track_use_value(mem, code, user_instr_ref, to_value_ref, true));
            }
        }
    } else if (user_instr->operation.opcode == code->klass->inline_asm_opcode) {
        for (const struct kefir_list_entry *iter = kefir_list_head(&user_instr->operation.inline_asm_node.fragments);
            iter != NULL;
            kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_inline_assembly_fragment *, fragment,
                iter->value);
            switch (fragment->type) {
                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_TEXT:
                    // Intetionally left blank
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_OPERAND:
                    REQUIRE_OK(operand_replace_value_uses(mem, code, user_instr_ref, &fragment->operand, to_value_ref, from_value_ref));
            }
        }
    } else {
        for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
            REQUIRE_OK(operand_replace_value_uses(mem, code, user_instr_ref, &user_instr->operation.parameters[i], to_value_ref, from_value_ref));
        }
    }
    REQUIRE_OK(track_use_value(mem, code, user_instr_ref, from_value_ref, false));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_replace_value(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_value_ref_t to_value_ref, kefir_codegen_target_ir_value_ref_t from_value_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(from_value_ref.instr_ref != KEFIR_ID_NONE && from_value_ref.instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR value reference"));
    REQUIRE(to_value_ref.instr_ref != KEFIR_ID_NONE && to_value_ref.instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR value reference"));
    REQUIRE(from_value_ref.instr_ref != to_value_ref.instr_ref || from_value_ref.aspect != to_value_ref.aspect, KEFIR_OK);

    struct kefir_codegen_target_ir_instruction *from_instr = &code->code[from_value_ref.instr_ref];
    for (kefir_size_t use_entry_index = from_instr->use_entry_top; use_entry_index != (kefir_size_t) ~0ull; use_entry_index = code->use_entries[use_entry_index].next_entry) {
        REQUIRE_OK(replace_value_uses(mem, code, code->use_entries[use_entry_index].user_instr_ref, to_value_ref, from_value_ref));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_replace_value_in(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t user_instr_ref,
    kefir_codegen_target_ir_value_ref_t to_value_ref, kefir_codegen_target_ir_value_ref_t from_value_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(user_instr_ref != KEFIR_ID_NONE && user_instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR value reference"));
    REQUIRE(from_value_ref.instr_ref != KEFIR_ID_NONE && from_value_ref.instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR value reference"));
    REQUIRE(to_value_ref.instr_ref != KEFIR_ID_NONE && to_value_ref.instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR value reference"));
    REQUIRE(from_value_ref.instr_ref != to_value_ref.instr_ref || from_value_ref.aspect != to_value_ref.aspect, KEFIR_OK);

    REQUIRE_OK(replace_value_uses(mem, code, user_instr_ref, to_value_ref, from_value_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_move_after(struct kefir_codegen_target_ir_code *code,kefir_codegen_target_ir_block_ref_t block_ref, kefir_codegen_target_ir_instruction_ref_t move_after_ref, kefir_codegen_target_ir_instruction_ref_t instr_ref) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(block_ref != KEFIR_ID_NONE || block_ref < code->blocks_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR block reference"));
    REQUIRE(move_after_ref == KEFIR_ID_NONE || move_after_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction reference"));
    REQUIRE(instr_ref != KEFIR_ID_NONE || instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction reference"));
    REQUIRE(instr_ref != move_after_ref, KEFIR_OK);

    struct kefir_codegen_target_ir_instruction *instr = &code->code[instr_ref];
    REQUIRE(instr->block_ref != KEFIR_ID_NONE, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Target IR instruction has already been dropped"));

    struct kefir_codegen_target_ir_instruction *after_instr = NULL;
    if (move_after_ref != KEFIR_ID_NONE) {
        after_instr = &code->code[move_after_ref];
        REQUIRE(after_instr->block_ref == block_ref, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Mismatch with target IR instructon block reference"));
    }

    struct kefir_codegen_target_ir_block *source_block = &code->blocks[instr->block_ref];
    struct kefir_codegen_target_ir_block *target_block = &code->blocks[block_ref];
    if (source_block->control_flow.head == instr->instr_ref) {
        source_block->control_flow.head = instr->control_flow.next;
    }

    if (source_block->control_flow.tail == instr->instr_ref) {
        source_block->control_flow.tail = instr->control_flow.prev;
    }

    if (instr->control_flow.prev != KEFIR_ID_NONE) {
        struct kefir_codegen_target_ir_instruction *prev_instr = &code->code[instr->control_flow.prev];
        prev_instr->control_flow.next = instr->control_flow.next;
    }

    if (instr->control_flow.next != KEFIR_ID_NONE) {
        struct kefir_codegen_target_ir_instruction *next_instr = &code->code[instr->control_flow.next];
        next_instr->control_flow.prev = instr->control_flow.prev;
    }
    
    if (after_instr == NULL) {
        if (target_block->control_flow.head != KEFIR_ID_NONE) {
            struct kefir_codegen_target_ir_instruction *head_instr = &code->code[target_block->control_flow.head];
            head_instr->control_flow.prev = instr->instr_ref;
        }
        instr->control_flow.prev = KEFIR_ID_NONE;
        instr->control_flow.next = target_block->control_flow.head;
        target_block->control_flow.head = instr->instr_ref;
        if (target_block->control_flow.tail == KEFIR_ID_NONE) {
            target_block->control_flow.tail = instr->instr_ref;
        }
    } else {
        if (after_instr->control_flow.next != KEFIR_ID_NONE) {
            struct kefir_codegen_target_ir_instruction *next_instr = &code->code[after_instr->control_flow.next];
            next_instr->control_flow.prev = instr->instr_ref;
        }

        instr->control_flow.prev = after_instr->instr_ref;
        instr->control_flow.next = after_instr->control_flow.next;
        after_instr->control_flow.next = instr->instr_ref;

        if (target_block->control_flow.tail == after_instr->instr_ref) {
            target_block->control_flow.tail = instr->instr_ref;
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_inline_assembly_text_fragment(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref,
    const char *text) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid target IR instruction reference"));
    REQUIRE(text != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR inline assembly text"));
    
    text = kefir_string_pool_insert(mem, &code->strings, text, NULL);
    REQUIRE(text != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert inline assembly fragment into string pool"));

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

    REQUIRE_OK(store_strings_in_operand(mem, code, &fragment->operand));
    REQUIRE_OK(operand_record_uses(mem, code, instr_ref, &fragment->operand, true));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_value_iter(const struct kefir_codegen_target_ir_code *code, struct kefir_codegen_target_ir_value_iterator *iter, kefir_codegen_target_ir_instruction_ref_t instr_ref, kefir_codegen_target_ir_value_ref_t *value_ref_ptr, const struct kefir_codegen_target_ir_value_type **value_type_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR value iterator"));
    REQUIRE(instr_ref != KEFIR_ID_NONE && instr_ref < code->code_length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction reference"));

    kefir_hashtable_key_t table_key;
    kefir_hashtable_value_t table_value;
    REQUIRE_OK(kefir_hashtable_iter(&code->code[instr_ref].aspects.all, &iter->iter, &table_key, &table_value));
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
