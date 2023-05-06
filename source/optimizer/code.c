/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#include "kefir/optimizer/code.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t free_block(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                 kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_block *, block, value);
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));

    memset(block, 0, sizeof(struct kefir_opt_code_block));
    KEFIR_FREE(mem, block);
    return KEFIR_OK;
}

static kefir_result_t free_phi_node(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                    kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_phi_node *, phi_node, value);
    REQUIRE(phi_node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer phi node"));

    REQUIRE_OK(kefir_hashtree_free(mem, &phi_node->links));
    memset(phi_node, 0, sizeof(struct kefir_opt_phi_node));
    KEFIR_FREE(mem, phi_node);
    return KEFIR_OK;
}

static kefir_result_t free_call_node(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                     kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_call_node *, call_node, value);
    REQUIRE(call_node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer call node"));

    KEFIR_FREE(mem, call_node->arguments);
    memset(call_node, 0, sizeof(struct kefir_opt_call_node));
    KEFIR_FREE(mem, call_node);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_init(struct kefir_opt_code_container *code) {
    REQUIRE(code != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code container"));

    code->code = NULL;
    code->capacity = 0;
    code->length = 0;
    code->next_block_id = 0;
    code->next_phi_node_id = 0;
    code->next_call_node_id = 0;
    code->entry_point = KEFIR_ID_NONE;
    REQUIRE_OK(kefir_hashtree_init(&code->blocks, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&code->blocks, free_block, NULL));
    REQUIRE_OK(kefir_hashtree_init(&code->phi_nodes, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&code->phi_nodes, free_phi_node, NULL));
    REQUIRE_OK(kefir_hashtree_init(&code->call_nodes, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&code->call_nodes, free_call_node, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_free(struct kefir_mem *mem, struct kefir_opt_code_container *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_hashtree_free(mem, &code->blocks));
    REQUIRE_OK(kefir_hashtree_free(mem, &code->phi_nodes));
    REQUIRE_OK(kefir_hashtree_free(mem, &code->call_nodes));
    KEFIR_FREE(mem, code->code);
    memset(code, 0, sizeof(struct kefir_opt_code_container));
    return KEFIR_OK;
}

kefir_bool_t kefir_opt_code_container_is_empty(const struct kefir_opt_code_container *code) {
    REQUIRE(code != NULL, true);
    return kefir_hashtree_empty(&code->blocks) && code->length == 0 && code->entry_point == KEFIR_ID_NONE;
}

kefir_size_t kefir_opt_code_container_length(const struct kefir_opt_code_container *code) {
    REQUIRE(code != NULL, 0);
    return code->length;
}

kefir_result_t kefir_opt_code_container_new_block(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                  kefir_bool_t entry_point, kefir_opt_block_id_t *block_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(!entry_point || code->entry_point == KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Entry point to optimizer code container already exists"));
    REQUIRE(block_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code block identifier"));

    struct kefir_opt_code_block *block = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_block));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer code block"));
    block->id = code->next_block_id;
    block->content.head = KEFIR_ID_NONE;
    block->content.tail = KEFIR_ID_NONE;
    block->control_flow.head = KEFIR_ID_NONE;
    block->control_flow.tail = KEFIR_ID_NONE;
    block->phi_nodes.head = KEFIR_ID_NONE;
    block->phi_nodes.tail = KEFIR_ID_NONE;
    block->call_nodes.head = KEFIR_ID_NONE;
    block->call_nodes.tail = KEFIR_ID_NONE;

    kefir_result_t res =
        kefir_hashtree_insert(mem, &code->blocks, (kefir_hashtree_key_t) block->id, (kefir_hashtree_value_t) block);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, block);
        return res;
    });

    code->next_block_id++;
    if (entry_point) {
        code->entry_point = block->id;
    }

    *block_id_ptr = block->id;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_block(const struct kefir_opt_code_container *code,
                                              kefir_opt_block_id_t block_id, struct kefir_opt_code_block **block_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code block"));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&code->blocks, (kefir_hashtree_key_t) block_id, &node);
    if (res == KEFIR_NOT_FOUND) {
        return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Cannot find optimizer code block with specified identifier");
    } else {
        REQUIRE_OK(res);
    }

    *block_ptr = (struct kefir_opt_code_block *) node->value;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_block_count(const struct kefir_opt_code_container *code,
                                                    kefir_size_t *length_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(length_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code container length"));

    *length_ptr = code->next_block_id;
    return KEFIR_OK;
}

static kefir_result_t ensure_code_container_capacity(struct kefir_mem *mem, struct kefir_opt_code_container *code) {
    if (code->length == code->capacity) {
        kefir_size_t new_capacity = code->capacity + 64;
        struct kefir_opt_instruction *new_code = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_instruction) * new_capacity);
        REQUIRE(new_code != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer code container"));

        if (code->code != NULL) {
            memcpy(new_code, code->code, sizeof(struct kefir_opt_instruction) * code->capacity);
            KEFIR_FREE(mem, code->code);
        }
        code->code = new_code;
        code->capacity = new_capacity;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_instr(const struct kefir_opt_code_container *code,
                                              kefir_opt_instruction_ref_t instr_id,
                                              struct kefir_opt_instruction **instr_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(instr_id != KEFIR_ID_NONE && instr_id < code->length,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS,
                            "Requested optimizer instruction identifier is out of bounds of the code container"));
    REQUIRE(instr_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction"));

    struct kefir_opt_instruction *instr = &code->code[instr_id];
    REQUIRE(instr->block_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Requested optimizer instruction was previously dropped"));
    *instr_ptr = instr;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_new_instruction(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                        struct kefir_opt_code_block *block,
                                                        const struct kefir_opt_operation *operation,
                                                        kefir_opt_instruction_ref_t *instr_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(operation != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer operation"));
    REQUIRE(instr_id != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to instruction identifier"));

    REQUIRE_OK(ensure_code_container_capacity(mem, code));
    struct kefir_opt_instruction *instr = &code->code[code->length];
    instr->id = code->length;
    instr->operation = *operation;
    instr->block_id = block->id;
    instr->control_flow.prev = KEFIR_ID_NONE;
    instr->control_flow.next = KEFIR_ID_NONE;
    instr->siblings.prev = block->content.tail;
    instr->siblings.next = KEFIR_ID_NONE;

    if (block->content.tail != KEFIR_ID_NONE) {
        struct kefir_opt_instruction *prev_instr = &code->code[block->content.tail];
        REQUIRE(prev_instr->siblings.next == KEFIR_ID_NONE,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected previous instruction in block to have no successors"));
        prev_instr->siblings.next = instr->id;
    } else {
        block->content.head = instr->id;
    }
    block->content.tail = instr->id;

    code->length++;
    *instr_id = instr->id;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_instruction_move_after(const struct kefir_opt_code_container *code,
                                                               kefir_opt_instruction_ref_t target_ref,
                                                               kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(target_ref != instr_ref,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Target and source optimizer instruction references must differ"));

    struct kefir_opt_instruction *insert_after = NULL, *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
    if (target_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_instr(code, target_ref, &insert_after));
        REQUIRE(insert_after->block_id == instr->block_id,
                KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                                "Both optimizer instructions shall be located within the same block"));
    }

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(code, instr->block_id, &block));

    struct kefir_opt_instruction *prev_instr = NULL, *next_instr = NULL;
    if (instr->siblings.prev != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr->siblings.prev, &prev_instr));
    }
    if (instr->siblings.next != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr->siblings.next, &next_instr));
    }

    if (prev_instr != NULL) {
        prev_instr->siblings.next = instr->siblings.next;
    }
    if (next_instr != NULL) {
        next_instr->siblings.prev = instr->siblings.prev;
    }

    if (block->content.head == instr->id) {
        block->content.head = instr->siblings.next;
    }
    if (block->content.tail == instr->id) {
        block->content.tail = instr->siblings.prev;
    }

    instr->siblings.prev = KEFIR_ID_NONE;
    instr->siblings.next = KEFIR_ID_NONE;

    if (insert_after == NULL) {
        if (block->content.head != KEFIR_ID_NONE) {
            struct kefir_opt_instruction *head_instr = NULL;
            REQUIRE_OK(kefir_opt_code_container_instr(code, block->content.head, &head_instr));
            REQUIRE(head_instr->siblings.prev == KEFIR_ID_NONE,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                    "Expected head instruction of optimizer code block to have no predecessors"));
            head_instr->siblings.prev = instr->id;
        }

        instr->siblings.prev = KEFIR_ID_NONE;
        instr->siblings.next = block->content.head;

        block->content.head = instr->id;
        if (block->content.tail == KEFIR_ID_NONE) {
            block->content.tail = instr->id;
        }
    } else {
        if (insert_after->siblings.next != KEFIR_ID_NONE) {
            struct kefir_opt_instruction *next_instr = NULL;
            REQUIRE_OK(kefir_opt_code_container_instr(code, instr->siblings.next, &next_instr));
            next_instr->siblings.prev = instr->id;
        }
        instr->siblings.prev = insert_after->id;
        instr->siblings.next = insert_after->siblings.next;

        insert_after->siblings.next = instr->id;

        if (block->content.tail == insert_after->id) {
            block->content.tail = instr->id;
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_add_control(const struct kefir_opt_code_container *code,
                                                    struct kefir_opt_code_block *block,
                                                    kefir_opt_instruction_ref_t instr_id) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));

    REQUIRE_OK(kefir_opt_code_container_insert_control(code, block, block->control_flow.tail, instr_id));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_insert_control(const struct kefir_opt_code_container *code,
                                                       struct kefir_opt_code_block *block,
                                                       kefir_opt_instruction_ref_t after_instr_id,
                                                       kefir_opt_instruction_ref_t instr_id) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(
        after_instr_id != KEFIR_ID_NONE || block != NULL,
        KEFIR_SET_ERROR(
            KEFIR_INVALID_PARAMETER,
            "Expected either valid optimizer instruction identifier for insert location, or valie optimizer block"));

    struct kefir_opt_instruction *after_instr = NULL;
    if (after_instr_id != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_instr(code, after_instr_id, &after_instr));
        if (block != NULL) {
            REQUIRE(
                after_instr->block_id == block->id,
                KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Provided target instruction is not a part of specified block"));
        } else {
            REQUIRE_OK(kefir_opt_code_container_block(code, after_instr->block_id, &block));
        }
    }

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_id, &instr));
    REQUIRE(instr->block_id == block->id,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Provided instruction is not a part of specified block"));
    REQUIRE(instr->control_flow.prev == KEFIR_ID_NONE && instr->control_flow.next == KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Provided instruction is already a part of control flow"));

    if (after_instr == NULL) {
        if (block->control_flow.head != KEFIR_ID_NONE) {
            struct kefir_opt_instruction *head_instr = NULL;
            REQUIRE_OK(kefir_opt_code_container_instr(code, block->control_flow.head, &head_instr));
            head_instr->control_flow.prev = instr->id;
        }
        instr->control_flow.next = block->control_flow.head;
        block->control_flow.head = instr->id;
        if (block->control_flow.tail == KEFIR_ID_NONE) {
            block->control_flow.tail = instr->id;
        }
    } else {
        if (after_instr->control_flow.next != KEFIR_ID_NONE) {
            struct kefir_opt_instruction *next_instr = NULL;
            REQUIRE_OK(kefir_opt_code_container_instr(code, after_instr->control_flow.next, &next_instr));
            next_instr->control_flow.prev = instr->id;
        }

        instr->control_flow.prev = after_instr->id;
        instr->control_flow.next = after_instr->control_flow.next;
        after_instr->control_flow.next = instr->id;

        if (block->control_flow.tail == after_instr->id) {
            block->control_flow.tail = instr->id;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t control_flow_remove(const struct kefir_opt_code_container *code,
                                          struct kefir_opt_code_block *block, struct kefir_opt_instruction *instr) {
    if (block->control_flow.head == instr->id) {
        block->control_flow.head = instr->control_flow.next;
    }

    if (block->control_flow.tail == instr->id) {
        block->control_flow.tail = instr->control_flow.prev;
    }

    if (instr->control_flow.prev != KEFIR_ID_NONE) {
        struct kefir_opt_instruction *prev_instr = NULL;
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr->control_flow.prev, &prev_instr));

        prev_instr->control_flow.next = instr->control_flow.next;
    }

    if (instr->control_flow.next != KEFIR_ID_NONE) {
        struct kefir_opt_instruction *next_instr = NULL;
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr->control_flow.next, &next_instr));

        next_instr->control_flow.prev = instr->control_flow.prev;
    }

    instr->control_flow.prev = KEFIR_ID_NONE;
    instr->control_flow.next = KEFIR_ID_NONE;

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_drop_control(const struct kefir_opt_code_container *code,
                                                     kefir_opt_instruction_ref_t instr_id) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_id, &instr));
    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(code, instr->block_id, &block));

    REQUIRE_OK(control_flow_remove(code, block, instr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_new_phi(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                kefir_opt_block_id_t block_id, kefir_opt_phi_id_t *phi_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(phi_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi identifier"));

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(code, block_id, &block));

    struct kefir_opt_phi_node *phi_node = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_phi_node));
    REQUIRE(phi_node != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer phi node"));

    phi_node->block_id = block_id;
    phi_node->node_id = code->next_phi_node_id;

    kefir_result_t res = kefir_hashtree_init(&phi_node->links, &kefir_hashtree_uint_ops);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, phi_node);
        return res;
    });

    res = kefir_hashtree_insert(mem, &code->phi_nodes, (kefir_hashtree_key_t) phi_node->node_id,
                                (kefir_hashtree_value_t) phi_node);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &phi_node->links);
        KEFIR_FREE(mem, phi_node);
        return res;
    });

    phi_node->siblings.prev = block->phi_nodes.tail;
    if (phi_node->siblings.prev != KEFIR_ID_NONE) {
        struct kefir_opt_phi_node *prev_phi_node = NULL;
        REQUIRE_OK(kefir_opt_code_container_phi(code, phi_node->siblings.prev, &prev_phi_node));
        REQUIRE(prev_phi_node->siblings.next == KEFIR_ID_NONE,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected optimizer phi node to be the last in the block"));
        prev_phi_node->siblings.next = phi_node->node_id;
    }
    phi_node->siblings.next = KEFIR_ID_NONE;
    block->phi_nodes.tail = phi_node->node_id;
    if (block->phi_nodes.head == KEFIR_ID_NONE) {
        block->phi_nodes.head = phi_node->node_id;
    }

    code->next_phi_node_id++;
    *phi_ptr = phi_node->node_id;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_phi(const struct kefir_opt_code_container *code, kefir_opt_phi_id_t phi_ref,
                                            struct kefir_opt_phi_node **phi_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(phi_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi node"));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&code->phi_nodes, (kefir_hashtree_key_t) phi_ref, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Cannot find requested optimizer phi node");
    }
    REQUIRE_OK(res);

    *phi_ptr = (struct kefir_opt_phi_node *) node->value;

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_phi_attach(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                   kefir_opt_phi_id_t phi_ref, kefir_opt_block_id_t block_id,
                                                   kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    struct kefir_opt_phi_node *phi_node = NULL;
    REQUIRE_OK(kefir_opt_code_container_phi(code, phi_ref, &phi_node));

    REQUIRE(phi_node->block_id != block_id,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Optimizer phi node cannot link itself"));
    REQUIRE(!kefir_hashtree_has(&phi_node->links, (kefir_hashtree_key_t) block_id),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Optimizer phi node already links provided block"));

    REQUIRE_OK(kefir_hashtree_insert(mem, &phi_node->links, (kefir_hashtree_key_t) block_id,
                                     (kefir_hashtree_value_t) instr_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_new_call(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                 kefir_opt_block_id_t block_id, kefir_id_t func_decl_id,
                                                 kefir_size_t argc, kefir_opt_call_id_t *call_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(call_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer call identifier"));

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(code, block_id, &block));

    struct kefir_opt_call_node *call_node = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_call_node));
    REQUIRE(call_node != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer call node"));

    call_node->block_id = block_id;
    call_node->node_id = code->next_call_node_id;
    call_node->function_declaration_id = func_decl_id;
    call_node->argument_count = argc;
    if (argc > 0) {
        call_node->arguments = KEFIR_MALLOC(mem, sizeof(kefir_opt_instruction_ref_t) * argc);
        REQUIRE_ELSE(call_node->arguments != NULL, {
            KEFIR_FREE(mem, call_node);
            return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer call node arguments");
        });
        for (kefir_size_t i = 0; i < argc; i++) {
            call_node->arguments[i] = KEFIR_ID_NONE;
        }
    } else {
        call_node->arguments = NULL;
    }

    kefir_result_t res = kefir_hashtree_insert(mem, &code->call_nodes, (kefir_hashtree_key_t) call_node->node_id,
                                               (kefir_hashtree_value_t) call_node);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, call_node->arguments);
        KEFIR_FREE(mem, call_node);
        return res;
    });

    call_node->siblings.prev = block->call_nodes.tail;
    if (call_node->siblings.prev != KEFIR_ID_NONE) {
        struct kefir_opt_call_node *prev_call_node = NULL;
        REQUIRE_OK(kefir_opt_code_container_call(code, call_node->siblings.prev, &prev_call_node));
        REQUIRE(prev_call_node->siblings.next == KEFIR_ID_NONE,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected optimizer call node to be the last in the block"));
        prev_call_node->siblings.next = call_node->node_id;
    }
    call_node->siblings.next = KEFIR_ID_NONE;
    block->call_nodes.tail = call_node->node_id;
    if (block->call_nodes.head == KEFIR_ID_NONE) {
        block->call_nodes.head = call_node->node_id;
    }

    code->next_call_node_id++;
    *call_ptr = call_node->node_id;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_call(const struct kefir_opt_code_container *code, kefir_opt_call_id_t call_ref,
                                             struct kefir_opt_call_node **call_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(call_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer call node"));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&code->call_nodes, (kefir_hashtree_key_t) call_ref, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Cannot find requested optimizer call node");
    }
    REQUIRE_OK(res);

    *call_ptr = (struct kefir_opt_call_node *) node->value;

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_call_set_argument(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                          kefir_opt_call_id_t call_ref, kefir_size_t argument_index,
                                                          kefir_opt_instruction_ref_t argument_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    struct kefir_opt_call_node *call_node = NULL;
    REQUIRE_OK(kefir_opt_code_container_call(code, call_ref, &call_node));

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(code, argument_ref, &instr));

    REQUIRE(call_node->block_id == instr->block_id,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                            "Optimizer call node and argument instruction shall be located within the same block"));
    REQUIRE(argument_index < call_node->argument_count,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided argument index exceeds optimizer call node argument count"));

    call_node->arguments[argument_index] = argument_ref;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_instr_head(const struct kefir_opt_code_container *code,
                                               const struct kefir_opt_code_block *block,
                                               const struct kefir_opt_instruction **instr_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(instr_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction"));

    struct kefir_opt_instruction *ptr = NULL;
    if (block->content.head != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_instr(code, block->content.head, &ptr));
    }

    *instr_ptr = ptr;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_instr_tail(const struct kefir_opt_code_container *code,
                                               const struct kefir_opt_code_block *block,
                                               const struct kefir_opt_instruction **instr_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(instr_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction"));

    struct kefir_opt_instruction *ptr = NULL;
    if (block->content.tail != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_instr(code, block->content.tail, &ptr));
    }

    *instr_ptr = ptr;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_instr_control_head(const struct kefir_opt_code_container *code,
                                                       const struct kefir_opt_code_block *block,
                                                       const struct kefir_opt_instruction **instr_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(instr_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction"));

    struct kefir_opt_instruction *ptr = NULL;
    if (block->control_flow.head != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_instr(code, block->control_flow.head, &ptr));
    }

    *instr_ptr = ptr;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_instr_control_tail(const struct kefir_opt_code_container *code,
                                                       const struct kefir_opt_code_block *block,
                                                       const struct kefir_opt_instruction **instr_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(instr_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction"));

    struct kefir_opt_instruction *ptr = NULL;
    if (block->control_flow.tail != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_instr(code, block->control_flow.tail, &ptr));
    }

    *instr_ptr = ptr;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_phi_head(const struct kefir_opt_code_container *code,
                                             const struct kefir_opt_code_block *block,
                                             const struct kefir_opt_phi_node **phi_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(phi_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi node"));

    struct kefir_opt_phi_node *ptr = NULL;
    if (block->phi_nodes.head != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_phi(code, block->phi_nodes.head, &ptr));
    }

    *phi_ptr = ptr;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_phi_tail(const struct kefir_opt_code_container *code,
                                             const struct kefir_opt_code_block *block,
                                             const struct kefir_opt_phi_node **phi_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(phi_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi node"));

    struct kefir_opt_phi_node *ptr = NULL;
    if (block->phi_nodes.tail != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_phi(code, block->phi_nodes.tail, &ptr));
    }

    *phi_ptr = ptr;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_call_head(const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_code_block *block,
                                              const struct kefir_opt_call_node **call_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(call_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer call node"));

    struct kefir_opt_call_node *ptr = NULL;
    if (block->call_nodes.head != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_call(code, block->call_nodes.head, &ptr));
    }

    *call_ptr = ptr;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_call_tail(const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_code_block *block,
                                              const struct kefir_opt_call_node **call_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(call_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer call node"));

    struct kefir_opt_call_node *ptr = NULL;
    if (block->call_nodes.tail != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_call(code, block->call_nodes.tail, &ptr));
    }

    *call_ptr = ptr;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_instruction_prev_sibling(const struct kefir_opt_code_container *code,
                                                  const struct kefir_opt_instruction *instr,
                                                  const struct kefir_opt_instruction **instr_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));
    REQUIRE(instr_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction"));

    struct kefir_opt_instruction *ptr = NULL;
    if (instr->siblings.prev != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr->siblings.prev, &ptr));
    }

    *instr_ptr = ptr;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_instruction_next_sibling(const struct kefir_opt_code_container *code,
                                                  const struct kefir_opt_instruction *instr,
                                                  const struct kefir_opt_instruction **instr_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));
    REQUIRE(instr_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction"));

    struct kefir_opt_instruction *ptr = NULL;
    if (instr->siblings.next != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr->siblings.next, &ptr));
    }

    *instr_ptr = ptr;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_instruction_prev_control(const struct kefir_opt_code_container *code,
                                                  const struct kefir_opt_instruction *instr,
                                                  const struct kefir_opt_instruction **instr_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));
    REQUIRE(instr_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction"));

    struct kefir_opt_instruction *ptr = NULL;
    if (instr->control_flow.prev != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr->control_flow.prev, &ptr));
    }

    *instr_ptr = ptr;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_instruction_next_control(const struct kefir_opt_code_container *code,
                                                  const struct kefir_opt_instruction *instr,
                                                  const struct kefir_opt_instruction **instr_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));
    REQUIRE(instr_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction"));

    struct kefir_opt_instruction *ptr = NULL;
    if (instr->control_flow.next != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr->control_flow.next, &ptr));
    }

    *instr_ptr = ptr;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_phi_prev_sibling(const struct kefir_opt_code_container *code,
                                          const struct kefir_opt_phi_node *phi,
                                          const struct kefir_opt_phi_node **phi_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(phi != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer phi node"));
    REQUIRE(phi_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi node"));

    struct kefir_opt_phi_node *ptr = NULL;
    if (phi->siblings.prev != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_phi(code, phi->siblings.prev, &ptr));
    }

    *phi_ptr = ptr;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_phi_next_sibling(const struct kefir_opt_code_container *code,
                                          const struct kefir_opt_phi_node *phi,
                                          const struct kefir_opt_phi_node **phi_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(phi != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer phi node"));
    REQUIRE(phi_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi node"));

    struct kefir_opt_phi_node *ptr = NULL;
    if (phi->siblings.next != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_phi(code, phi->siblings.next, &ptr));
    }

    *phi_ptr = ptr;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_call_prev_sibling(const struct kefir_opt_code_container *code,
                                           const struct kefir_opt_call_node *call,
                                           const struct kefir_opt_call_node **call_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(call != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer call node"));
    REQUIRE(call_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer call node"));

    struct kefir_opt_call_node *ptr = NULL;
    if (call->siblings.prev != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_call(code, call->siblings.prev, &ptr));
    }

    *call_ptr = ptr;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_call_next_sibling(const struct kefir_opt_code_container *code,
                                           const struct kefir_opt_call_node *call,
                                           const struct kefir_opt_call_node **call_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(call != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer call node"));
    REQUIRE(call_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer call node"));

    struct kefir_opt_call_node *ptr = NULL;
    if (call->siblings.next != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_call(code, call->siblings.next, &ptr));
    }

    *call_ptr = ptr;
    return KEFIR_OK;
}

const struct kefir_opt_code_block *kefir_opt_code_container_iter(const struct kefir_opt_code_container *code,
                                                                 struct kefir_opt_code_container_iterator *iter) {
    REQUIRE(code != NULL, NULL);
    REQUIRE(iter != NULL, NULL);

    const struct kefir_hashtree_node *node = kefir_hashtree_iter(&code->blocks, &iter->iter);
    if (node != NULL) {
        return (struct kefir_opt_code_block *) node->value;
    } else {
        return NULL;
    }
}

const struct kefir_opt_code_block *kefir_opt_code_container_next(struct kefir_opt_code_container_iterator *iter) {
    REQUIRE(iter != NULL, NULL);

    const struct kefir_hashtree_node *node = kefir_hashtree_next(&iter->iter);
    if (node != NULL) {
        return (struct kefir_opt_code_block *) node->value;
    } else {
        return NULL;
    }
}

kefir_result_t kefir_opt_phi_node_link_iter(const struct kefir_opt_phi_node *phi_node,
                                            struct kefir_opt_phi_node_link_iterator *iter,
                                            kefir_opt_block_id_t *block_id_ptr,
                                            kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(phi_node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer phi node"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi node link iterator"));

    const struct kefir_hashtree_node *node = kefir_hashtree_iter(&phi_node->links, &iter->iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);
    ASSIGN_PTR(block_id_ptr, (kefir_opt_block_id_t) node->key);
    ASSIGN_PTR(instr_ref_ptr, (kefir_opt_instruction_ref_t) node->value);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_phi_node_link_next(struct kefir_opt_phi_node_link_iterator *iter,
                                            kefir_opt_block_id_t *block_id_ptr,
                                            kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi node link iterator"));

    const struct kefir_hashtree_node *node = kefir_hashtree_next(&iter->iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);
    ASSIGN_PTR(block_id_ptr, (kefir_opt_block_id_t) node->key);
    ASSIGN_PTR(instr_ref_ptr, (kefir_opt_instruction_ref_t) node->value);
    return KEFIR_OK;
}
