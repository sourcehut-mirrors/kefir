/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#ifndef KEFIR_OPTIMIZER_CODE_H_
#define KEFIR_OPTIMIZER_CODE_H_

#include "kefir/optimizer/opcode_defs.h"
#include "kefir/core/basic-types.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/hashtreeset.h"

typedef enum kefir_opt_opcode {
#define KEFIR_OPT_OPCODE(_id, _name, _class) KEFIR_OPT_OPCODE_##_id
    KEFIR_OPTIMIZER_OPCODE_DEFS(KEFIR_OPT_OPCODE, COMMA)
#undef KEFIR_OPT_OPCODE
} kefir_opt_opcode_t;

typedef kefir_id_t kefir_opt_block_id_t;
typedef kefir_id_t kefir_opt_instruction_ref_t;
typedef kefir_id_t kefir_opt_phi_id_t;
typedef kefir_id_t kefir_opt_call_id_t;
typedef kefir_id_t kefir_opt_inline_assembly_id_t;

typedef struct kefir_opt_memory_access_flags {
    kefir_bool_t volatile_access;
} kefir_opt_memory_access_flags_t;

typedef enum kefir_opt_memory_order { KEFIR_OPT_MEMORY_ORDER_SEQ_CST } kefir_opt_memory_order_t;

typedef enum kefir_opt_operation_reference_index {
    KEFIR_OPT_BITFIELD_BASE_REF = 0,
    KEFIR_OPT_BITFIELD_VALUE_REF = 1,
    KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF = 0,
    KEFIR_OPT_MEMORY_ACCESS_VALUE_REF = 1,
    KEFIR_OPT_STACK_ALLOCATION_SIZE_REF = 0,
    KEFIR_OPT_STACK_ALLOCATION_ALIGNMENT_REF = 1
} kefir_opt_operation_reference_index_t;

typedef enum kefir_opt_branch_condition_variant {
    KEFIR_OPT_BRANCH_CONDITION_8BIT,
    KEFIR_OPT_BRANCH_CONDITION_16BIT,
    KEFIR_OPT_BRANCH_CONDITION_32BIT,
    KEFIR_OPT_BRANCH_CONDITION_64BIT
} kefir_opt_branch_condition_variant_t;

typedef struct kefir_opt_operation_parameters {
    kefir_opt_instruction_ref_t refs[3];
    struct {
        kefir_id_t type_id;
        kefir_size_t type_index;
    } type;
    union {
        kefir_opt_phi_id_t phi_ref;
        kefir_opt_inline_assembly_id_t inline_asm_ref;
        kefir_size_t index;
        kefir_id_t ir_ref;
        struct {
            union {
                kefir_id_t global_ref;
                kefir_size_t local_index;
            };
            kefir_int64_t offset;
        } variable;

        struct {
            kefir_opt_block_id_t target_block;
            kefir_opt_block_id_t alternative_block;
            kefir_opt_branch_condition_variant_t condition_variant;
            kefir_opt_instruction_ref_t condition_ref;
        } branch;

        union {
            kefir_int64_t integer;
            kefir_uint64_t uinteger;
            kefir_float32_t float32;
            kefir_float64_t float64;
            kefir_long_double_t long_double;
            kefir_id_t string_ref;
            kefir_opt_block_id_t block_ref;
        } imm;

        struct {
            struct kefir_opt_memory_access_flags flags;
        } memory_access;

        struct {
            kefir_size_t offset;
            kefir_size_t length;
        } bitfield;

        struct {
            kefir_bool_t within_scope;
        } stack_allocation;

        struct {
            kefir_opt_call_id_t call_ref;
            kefir_opt_instruction_ref_t indirect_ref;
        } function_call;

        struct {
            kefir_opt_memory_order_t model;
        } atomic_op;
    };
} kefir_opt_operation_parameters_t;

typedef struct kefir_opt_operation {
    kefir_opt_opcode_t opcode;
    struct kefir_opt_operation_parameters parameters;
} kefir_opt_operation_t;

typedef struct kefir_opt_instruction_link {
    kefir_opt_instruction_ref_t prev;
    kefir_opt_instruction_ref_t next;
} kefir_opt_instruction_link_t;

typedef struct kefir_opt_instruction {
    kefir_opt_instruction_ref_t id;
    kefir_opt_block_id_t block_id;

    struct kefir_opt_operation operation;
    struct kefir_opt_instruction_link siblings;
    struct kefir_opt_instruction_link control_flow;
} kefir_opt_instruction_t;

typedef struct kefir_opt_code_instruction_list {
    kefir_opt_instruction_ref_t head;
    kefir_opt_instruction_ref_t tail;
} kefir_opt_code_instruction_list_t;

typedef struct kefir_opt_code_block {
    kefir_opt_block_id_t id;

    struct kefir_opt_code_instruction_list content;
    struct kefir_opt_code_instruction_list control_flow;
    struct {
        kefir_opt_phi_id_t head;
        kefir_opt_phi_id_t tail;
    } phi_nodes;

    struct {
        kefir_opt_call_id_t head;
        kefir_opt_call_id_t tail;
    } call_nodes;

    struct {
        kefir_opt_inline_assembly_id_t head;
        kefir_opt_inline_assembly_id_t tail;
    } inline_assembly_nodes;

    struct kefir_hashtreeset public_labels;
} kefir_opt_code_block_t;

typedef struct kefir_opt_phi_node {
    kefir_opt_block_id_t block_id;
    kefir_opt_phi_id_t node_id;
    kefir_size_t number_of_links;
    struct kefir_hashtree links;
    kefir_opt_instruction_ref_t output_ref;

    struct {
        kefir_opt_phi_id_t prev;
        kefir_opt_phi_id_t next;
    } siblings;
} kefir_opt_phi_node_t;

typedef struct kefir_opt_call_node {
    kefir_opt_block_id_t block_id;
    kefir_opt_call_id_t node_id;
    kefir_id_t function_declaration_id;

    kefir_size_t argument_count;
    kefir_opt_instruction_ref_t *arguments;

    struct {
        kefir_opt_call_id_t prev;
        kefir_opt_call_id_t next;
    } siblings;
} kefir_opt_call_node_t;

typedef struct kefir_opt_inline_assembly_parameter {
    kefir_opt_instruction_ref_t read_ref;
    kefir_opt_instruction_ref_t load_store_ref;
} kefir_opt_inline_assembly_parameter_t;

typedef struct kefir_opt_inline_assembly_node {
    kefir_opt_block_id_t block_id;
    kefir_opt_inline_assembly_id_t node_id;
    kefir_id_t inline_asm_id;

    kefir_size_t parameter_count;
    struct kefir_opt_inline_assembly_parameter *parameters;

    kefir_opt_block_id_t default_jump_target;
    struct kefir_hashtree jump_targets;

    struct {
        kefir_opt_inline_assembly_id_t prev;
        kefir_opt_inline_assembly_id_t next;
    } siblings;
} kefir_opt_inline_assembly_node_t;

typedef struct kefir_opt_code_container {
    struct kefir_opt_instruction *code;
    kefir_size_t length;
    kefir_size_t capacity;

    struct kefir_hashtree blocks;
    kefir_opt_block_id_t next_block_id;

    struct kefir_opt_phi_node *phi_nodes;
    kefir_size_t phi_nodes_length;
    kefir_size_t phi_nodes_capacity;

    struct kefir_hashtree call_nodes;
    kefir_opt_call_id_t next_call_node_id;

    struct kefir_hashtree inline_assembly;
    kefir_opt_call_id_t next_inline_assembly_id;

    kefir_opt_block_id_t entry_point;

    struct kefir_hashtree uses;
} kefir_opt_code_container_t;

typedef struct kefir_opt_code_block_public_label_iterator {
    struct kefir_hashtreeset_iterator iter;
    const char *public_label;
} kefir_opt_code_block_public_label_iterator_t;

kefir_result_t kefir_opt_code_container_init(struct kefir_opt_code_container *);
kefir_result_t kefir_opt_code_container_free(struct kefir_mem *, struct kefir_opt_code_container *);

kefir_bool_t kefir_opt_code_container_is_empty(const struct kefir_opt_code_container *);
kefir_size_t kefir_opt_code_container_length(const struct kefir_opt_code_container *);
kefir_result_t kefir_opt_code_container_new_block(struct kefir_mem *, struct kefir_opt_code_container *, kefir_bool_t,
                                                  kefir_opt_block_id_t *);
kefir_result_t kefir_opt_code_container_block(const struct kefir_opt_code_container *, kefir_opt_block_id_t,
                                              const struct kefir_opt_code_block **);
kefir_result_t kefir_opt_code_container_block_count(const struct kefir_opt_code_container *, kefir_size_t *);

kefir_result_t kefir_opt_code_container_add_block_public_label(struct kefir_mem *, struct kefir_opt_code_container *,
                                                               kefir_opt_block_id_t, const char *);
kefir_result_t kefir_opt_code_container_block_public_labels_iter(const struct kefir_opt_code_container *,
                                                                 kefir_opt_block_id_t,
                                                                 struct kefir_opt_code_block_public_label_iterator *);
kefir_result_t kefir_opt_code_container_block_public_labels_next(struct kefir_opt_code_block_public_label_iterator *);

kefir_result_t kefir_opt_code_container_instr(const struct kefir_opt_code_container *, kefir_opt_instruction_ref_t,
                                              const struct kefir_opt_instruction **);
kefir_result_t kefir_opt_code_container_new_instruction(struct kefir_mem *, struct kefir_opt_code_container *,
                                                        kefir_opt_block_id_t, const struct kefir_opt_operation *,
                                                        kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_container_drop_instr(const struct kefir_opt_code_container *,
                                                   kefir_opt_instruction_ref_t);

kefir_result_t kefir_opt_code_container_instruction_move_after(const struct kefir_opt_code_container *,
                                                               kefir_opt_instruction_ref_t,
                                                               kefir_opt_instruction_ref_t);
kefir_result_t kefir_opt_code_container_replace_references(struct kefir_mem *, const struct kefir_opt_code_container *,
                                                           kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t);

kefir_result_t kefir_opt_code_container_add_control(const struct kefir_opt_code_container *, kefir_opt_block_id_t,
                                                    kefir_opt_instruction_ref_t);
kefir_result_t kefir_opt_code_container_insert_control(const struct kefir_opt_code_container *, kefir_opt_block_id_t,
                                                       kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t);
kefir_result_t kefir_opt_code_container_drop_control(const struct kefir_opt_code_container *,
                                                     kefir_opt_instruction_ref_t);

kefir_result_t kefir_opt_code_container_new_phi(struct kefir_mem *, struct kefir_opt_code_container *,
                                                kefir_opt_block_id_t, kefir_opt_phi_id_t *);
kefir_result_t kefir_opt_code_container_phi(const struct kefir_opt_code_container *, kefir_opt_phi_id_t,
                                            const struct kefir_opt_phi_node **);
kefir_result_t kefir_opt_code_container_drop_phi(const struct kefir_opt_code_container *, kefir_opt_phi_id_t);
kefir_result_t kefir_opt_code_container_phi_attach(struct kefir_mem *, struct kefir_opt_code_container *,
                                                   kefir_opt_phi_id_t, kefir_opt_block_id_t,
                                                   kefir_opt_instruction_ref_t);
kefir_result_t kefir_opt_code_container_phi_set_output(const struct kefir_opt_code_container *, kefir_opt_phi_id_t,
                                                       kefir_opt_instruction_ref_t);
kefir_result_t kefir_opt_code_container_phi_link_for(const struct kefir_opt_code_container *, kefir_opt_phi_id_t,
                                                     kefir_opt_block_id_t, kefir_opt_instruction_ref_t *);

kefir_result_t kefir_opt_code_container_new_call(struct kefir_mem *, struct kefir_opt_code_container *,
                                                 kefir_opt_block_id_t, kefir_id_t, kefir_size_t, kefir_opt_call_id_t *);
kefir_result_t kefir_opt_code_container_call(const struct kefir_opt_code_container *, kefir_opt_call_id_t,
                                             const struct kefir_opt_call_node **);
kefir_result_t kefir_opt_code_container_call_set_argument(struct kefir_mem *, struct kefir_opt_code_container *,
                                                          kefir_opt_call_id_t, kefir_size_t,
                                                          kefir_opt_instruction_ref_t);
kefir_result_t kefir_opt_code_container_call_get_argument(const struct kefir_opt_code_container *, kefir_opt_call_id_t,
                                                          kefir_size_t, kefir_opt_instruction_ref_t *);

kefir_result_t kefir_opt_code_container_new_inline_assembly(struct kefir_mem *, struct kefir_opt_code_container *,
                                                            kefir_opt_block_id_t, kefir_id_t, kefir_size_t,
                                                            kefir_opt_inline_assembly_id_t *);
kefir_result_t kefir_opt_code_container_inline_assembly(const struct kefir_opt_code_container *,
                                                        kefir_opt_inline_assembly_id_t,
                                                        const struct kefir_opt_inline_assembly_node **);
kefir_result_t kefir_opt_code_container_inline_assembly_get_parameter(
    const struct kefir_opt_code_container *, kefir_opt_inline_assembly_id_t, kefir_size_t,
    const struct kefir_opt_inline_assembly_parameter **);
kefir_result_t kefir_opt_code_container_inline_assembly_set_parameter(
    struct kefir_mem *, const struct kefir_opt_code_container *, kefir_opt_inline_assembly_id_t, kefir_size_t,
    const struct kefir_opt_inline_assembly_parameter *);
kefir_result_t kefir_opt_code_container_inline_assembly_set_default_jump_target(const struct kefir_opt_code_container *,
                                                                                kefir_opt_inline_assembly_id_t,
                                                                                kefir_opt_block_id_t);
kefir_result_t kefir_opt_code_container_inline_assembly_add_jump_target(struct kefir_mem *,
                                                                        const struct kefir_opt_code_container *,
                                                                        kefir_opt_inline_assembly_id_t, kefir_id_t,
                                                                        kefir_opt_block_id_t);
kefir_result_t kefir_opt_code_container_inline_assembly_jump_target(const struct kefir_opt_code_container *,
                                                                    kefir_opt_inline_assembly_id_t, kefir_id_t,
                                                                    kefir_opt_block_id_t *);

kefir_result_t kefir_opt_code_block_instr_head(const struct kefir_opt_code_container *,
                                               const struct kefir_opt_code_block *, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_block_instr_tail(const struct kefir_opt_code_container *,
                                               const struct kefir_opt_code_block *, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_block_instr_control_head(const struct kefir_opt_code_container *,
                                                       const struct kefir_opt_code_block *,
                                                       kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_block_instr_control_tail(const struct kefir_opt_code_container *,
                                                       const struct kefir_opt_code_block *,
                                                       kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_block_phi_head(const struct kefir_opt_code_container *,
                                             const struct kefir_opt_code_block *, kefir_opt_phi_id_t *);
kefir_result_t kefir_opt_code_block_phi_tail(const struct kefir_opt_code_container *,
                                             const struct kefir_opt_code_block *, kefir_opt_phi_id_t *);
kefir_result_t kefir_opt_code_block_call_head(const struct kefir_opt_code_container *,
                                              const struct kefir_opt_code_block *, kefir_opt_call_id_t *);
kefir_result_t kefir_opt_code_block_call_tail(const struct kefir_opt_code_container *,
                                              const struct kefir_opt_code_block *, kefir_opt_call_id_t *);
kefir_result_t kefir_opt_code_block_inline_assembly_head(const struct kefir_opt_code_container *,
                                                         const struct kefir_opt_code_block *,
                                                         kefir_opt_inline_assembly_id_t *);
kefir_result_t kefir_opt_code_block_inline_assembly_tail(const struct kefir_opt_code_container *,
                                                         const struct kefir_opt_code_block *,
                                                         kefir_opt_inline_assembly_id_t *);
kefir_result_t kefir_opt_instruction_prev_sibling(const struct kefir_opt_code_container *, kefir_opt_instruction_ref_t,
                                                  kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_instruction_next_sibling(const struct kefir_opt_code_container *, kefir_opt_instruction_ref_t,
                                                  kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_instruction_prev_control(const struct kefir_opt_code_container *, kefir_opt_instruction_ref_t,
                                                  kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_instruction_next_control(const struct kefir_opt_code_container *, kefir_opt_instruction_ref_t,
                                                  kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_phi_prev_sibling(const struct kefir_opt_code_container *, kefir_opt_phi_id_t,
                                          kefir_opt_phi_id_t *);
kefir_result_t kefir_opt_phi_next_sibling(const struct kefir_opt_code_container *, kefir_opt_phi_id_t,
                                          kefir_opt_phi_id_t *);
kefir_result_t kefir_opt_call_prev_sibling(const struct kefir_opt_code_container *, kefir_opt_call_id_t,
                                           kefir_opt_call_id_t *);
kefir_result_t kefir_opt_call_next_sibling(const struct kefir_opt_code_container *, kefir_opt_call_id_t,
                                           kefir_opt_call_id_t *);
kefir_result_t kefir_opt_inline_assembly_prev_sibling(const struct kefir_opt_code_container *,
                                                      kefir_opt_inline_assembly_id_t, kefir_opt_inline_assembly_id_t *);
kefir_result_t kefir_opt_inline_assembly_next_sibling(const struct kefir_opt_code_container *,
                                                      kefir_opt_inline_assembly_id_t, kefir_opt_inline_assembly_id_t *);

typedef struct kefir_opt_code_container_iterator {
    struct kefir_hashtree_node_iterator iter;
} kefir_opt_code_container_iterator_t;

struct kefir_opt_code_block *kefir_opt_code_container_iter(const struct kefir_opt_code_container *,
                                                           struct kefir_opt_code_container_iterator *);
struct kefir_opt_code_block *kefir_opt_code_container_next(struct kefir_opt_code_container_iterator *);

typedef struct kefir_opt_phi_node_link_iterator {
    struct kefir_hashtree_node_iterator iter;
} kefir_opt_phi_node_link_iterator_t;

kefir_result_t kefir_opt_phi_node_link_iter(const struct kefir_opt_phi_node *,
                                            struct kefir_opt_phi_node_link_iterator *, kefir_opt_block_id_t *,
                                            kefir_opt_instruction_ref_t *);

kefir_result_t kefir_opt_phi_node_link_next(struct kefir_opt_phi_node_link_iterator *, kefir_opt_block_id_t *,
                                            kefir_opt_instruction_ref_t *);

#endif
