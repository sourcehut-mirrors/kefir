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

typedef kefir_size_t kefir_opt_id_t;
#define KEFIR_OPT_ID_NONE (~(kefir_opt_id_t) 0ull)

typedef struct kefir_opt_operation {
    kefir_opt_opcode_t opcode;

    union {
        struct {
            kefir_opt_id_t target_block;
            kefir_opt_id_t alternative_block;
            kefir_opt_id_t condition;
        } branch;
    };
} kefir_opt_operation_t;

typedef struct kefir_opt_instruction_list_entry {
    kefir_opt_id_t prev;
    kefir_opt_id_t next;
} kefir_opt_instruction_list_entry_t;

typedef struct kefir_opt_instruction {
    kefir_opt_id_t id;
    kefir_opt_id_t block_id;

    struct kefir_opt_operation operation;
    struct kefir_opt_instruction_list_entry siblings;
    struct kefir_opt_instruction_list_entry control_flow;
} kefir_opt_instruction_t;

typedef struct kefir_opt_code_block_list {
    kefir_opt_id_t head;
    kefir_opt_id_t tail;
} kefir_opt_code_block_list_t;

typedef struct kefir_opt_code_block {
    kefir_opt_id_t id;

    struct kefir_opt_code_block_list content;
    struct kefir_opt_code_block_list control_flow;
} kefir_opt_code_block_t;

typedef struct kefir_opt_code_container {
    struct kefir_opt_instruction *code;
    kefir_size_t length;
    kefir_size_t capacity;

    struct kefir_hashtree blocks;
    kefir_opt_id_t next_block_id;
} kefir_opt_code_container_t;

kefir_result_t kefir_opt_code_container_init(struct kefir_opt_code_container *);
kefir_result_t kefir_opt_code_container_free(struct kefir_mem *, struct kefir_opt_code_container *);

kefir_result_t kefir_opt_code_container_new_block(struct kefir_mem *, struct kefir_opt_code_container *,
                                                  kefir_opt_id_t *);
kefir_result_t kefir_opt_code_container_block(const struct kefir_opt_code_container *, kefir_opt_id_t,
                                              struct kefir_opt_code_block **);

kefir_result_t kefir_opt_code_container_instr(const struct kefir_opt_code_container *, kefir_opt_id_t,
                                              struct kefir_opt_instruction **);
kefir_result_t kefir_opt_code_container_append(struct kefir_mem *, struct kefir_opt_code_container *, kefir_opt_id_t,
                                               const struct kefir_opt_operation *, kefir_opt_id_t *);
kefir_result_t kefir_opt_code_container_add_control(const struct kefir_opt_code_container *, kefir_opt_id_t,
                                                    kefir_opt_id_t);

kefir_result_t kefir_opt_code_block_instr_iter(const struct kefir_opt_code_container *,
                                               const struct kefir_opt_code_block *,
                                               const struct kefir_opt_instruction **);
kefir_result_t kefir_opt_code_block_instr_control_iter(const struct kefir_opt_code_container *,
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

typedef struct kefir_opt_code_block_iterator {
    struct kefir_hashtree_node_iterator iter;
} kefir_opt_code_block_iterator_t;

const struct kefir_opt_code_block *kefir_opt_code_block_iter(const struct kefir_opt_code_container *,
                                                             struct kefir_opt_code_block_iterator *);
const struct kefir_opt_code_block *kefir_opt_code_block_next(struct kefir_opt_code_block_iterator *);

#endif
