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

#ifndef KEFIR_OPTIMIZER_CODE_H_
#define KEFIR_OPTIMIZER_CODE_H_

#include "kefir/optimizer/opcode_defs.h"
#include "kefir/core/basic-types.h"
#include "kefir/core/hashtree.h"

typedef enum kefir_opt_opcode {
#define KEFIR_OPT_OPCODE(_id, _name, _class) KEFIR_OPT_OPCODE_##_id
    KEFIR_OPTIMIZER_OPCODE_DEFS(KEFIR_OPT_OPCODE, COMMA)
#undef KEFIR_OPT_OPCODE
} kefir_opt_opcode_t;

typedef kefir_id_t kefir_opt_block_id_t;
typedef kefir_id_t kefir_opt_instruction_ref_t;

typedef struct kefir_opt_memory_access_flags {
    kefir_bool_t volatile_access;
} kefir_opt_memory_access_flags_t;

typedef union kefir_opt_operation_parameters {
    struct {
        kefir_opt_block_id_t target_block;
        kefir_opt_block_id_t alternative_block;
        kefir_opt_instruction_ref_t condition_ref;
    } branch;

    kefir_id_t refs[3];

    union {
        kefir_int64_t integer;
        kefir_uint64_t uinteger;
        kefir_float32_t float32;
        kefir_float64_t float64;
    } imm;

    struct {
        kefir_opt_instruction_ref_t location;
        kefir_opt_instruction_ref_t value;
        struct kefir_opt_memory_access_flags flags;
    } memory_access;

    struct {
        kefir_opt_instruction_ref_t target;
        kefir_opt_instruction_ref_t source;
        kefir_id_t type_id;
        kefir_size_t type_index;
    } memory_operation;

    struct {
        kefir_size_t offset;
        kefir_size_t length;
        kefir_opt_instruction_ref_t base_ref;
        kefir_opt_instruction_ref_t value_ref;
    } bitfield;
} kefir_opt_operation_parameters_t;

typedef struct kefir_opt_operation {
    kefir_opt_opcode_t opcode;
    union kefir_opt_operation_parameters parameters;
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
} kefir_opt_code_block_t;

typedef struct kefir_opt_code_container {
    struct kefir_opt_instruction *code;
    kefir_size_t length;
    kefir_size_t capacity;

    struct kefir_hashtree blocks;
    kefir_opt_block_id_t next_block_id;

    kefir_opt_block_id_t entry_point;
} kefir_opt_code_container_t;

kefir_result_t kefir_opt_code_container_init(struct kefir_opt_code_container *);
kefir_result_t kefir_opt_code_container_free(struct kefir_mem *, struct kefir_opt_code_container *);

kefir_bool_t kefir_opt_code_container_is_empty(const struct kefir_opt_code_container *);
kefir_result_t kefir_opt_code_container_new_block(struct kefir_mem *, struct kefir_opt_code_container *, kefir_bool_t,
                                                  kefir_opt_block_id_t *);
kefir_result_t kefir_opt_code_container_block(const struct kefir_opt_code_container *, kefir_opt_block_id_t,
                                              struct kefir_opt_code_block **);

kefir_result_t kefir_opt_code_container_instr(const struct kefir_opt_code_container *, kefir_opt_instruction_ref_t,
                                              struct kefir_opt_instruction **);
kefir_result_t kefir_opt_code_container_new_instruction(struct kefir_mem *, struct kefir_opt_code_container *,
                                                        struct kefir_opt_code_block *,
                                                        const struct kefir_opt_operation *,
                                                        kefir_opt_instruction_ref_t *);

kefir_result_t kefir_opt_code_container_add_control(const struct kefir_opt_code_container *,
                                                    struct kefir_opt_code_block *, kefir_opt_instruction_ref_t);
kefir_result_t kefir_opt_code_container_insert_control(const struct kefir_opt_code_container *,
                                                       struct kefir_opt_code_block *, kefir_opt_instruction_ref_t,
                                                       kefir_opt_instruction_ref_t);
kefir_result_t kefir_opt_code_container_drop_control(const struct kefir_opt_code_container *,
                                                     kefir_opt_instruction_ref_t);

kefir_result_t kefir_opt_code_block_instr_head(const struct kefir_opt_code_container *,
                                               const struct kefir_opt_code_block *,
                                               const struct kefir_opt_instruction **);
kefir_result_t kefir_opt_code_block_instr_tail(const struct kefir_opt_code_container *,
                                               const struct kefir_opt_code_block *,
                                               const struct kefir_opt_instruction **);
kefir_result_t kefir_opt_code_block_instr_control_head(const struct kefir_opt_code_container *,
                                                       const struct kefir_opt_code_block *,
                                                       const struct kefir_opt_instruction **);
kefir_result_t kefir_opt_code_block_instr_control_tail(const struct kefir_opt_code_container *,
                                                       const struct kefir_opt_code_block *,
                                                       const struct kefir_opt_instruction **);
kefir_result_t kefir_opt_instruction_prev_sibling(const struct kefir_opt_code_container *,
                                                  const struct kefir_opt_instruction *,
                                                  const struct kefir_opt_instruction **);
kefir_result_t kefir_opt_instruction_next_sibling(const struct kefir_opt_code_container *,
                                                  const struct kefir_opt_instruction *,
                                                  const struct kefir_opt_instruction **);
kefir_result_t kefir_opt_instruction_prev_control(const struct kefir_opt_code_container *,
                                                  const struct kefir_opt_instruction *,
                                                  const struct kefir_opt_instruction **);
kefir_result_t kefir_opt_instruction_next_control(const struct kefir_opt_code_container *,
                                                  const struct kefir_opt_instruction *,
                                                  const struct kefir_opt_instruction **);

typedef struct kefir_opt_code_container_iterator {
    struct kefir_hashtree_node_iterator iter;
} kefir_opt_code_container_iterator_t;

const struct kefir_opt_code_block *kefir_opt_code_container_iter(const struct kefir_opt_code_container *,
                                                                 struct kefir_opt_code_container_iterator *);
const struct kefir_opt_code_block *kefir_opt_code_container_next(struct kefir_opt_code_container_iterator *);

#endif
