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

#include "kefir/optimizer/inline.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/optimizer/builder.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct do_inline_param {
    struct kefir_mem *mem;
    const struct kefir_opt_module *module;
    const struct kefir_opt_function *src_function;
    struct kefir_opt_function *dst_function;
    struct kefir_opt_code_container *dst_code;
    kefir_opt_block_id_t inline_predecessor_block_id;
    kefir_opt_block_id_t inline_successor_block_id;
    kefir_opt_call_id_t original_call_ref;
    kefir_opt_instruction_ref_t original_call_instr_ref;
    kefir_opt_phi_id_t result_phi_ref;
    kefir_opt_instruction_ref_t result_phi_instr;

    struct kefir_hashtree block_mapping;
    struct kefir_hashtree instr_mapping;
};

static kefir_result_t map_block(struct do_inline_param *param, kefir_opt_block_id_t block_id,
                                kefir_opt_block_id_t *mapped_block_id_ptr) {
    if (block_id == KEFIR_ID_NONE) {
        *mapped_block_id_ptr = KEFIR_ID_NONE;
        return KEFIR_OK;
    }
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&param->block_mapping, (kefir_hashtree_key_t) block_id, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_PTR(mapped_block_id_ptr, (kefir_opt_block_id_t) node->value);
        return KEFIR_OK;
    }

    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(kefir_opt_code_container_new_block(param->mem, param->dst_code, false, &mapped_block_id));
    REQUIRE_OK(kefir_hashtree_insert(param->mem, &param->block_mapping, (kefir_hashtree_key_t) block_id,
                                     (kefir_hashtree_value_t) mapped_block_id));
    REQUIRE_OK(kefir_opt_function_block_inlined_from(param->mem, param->dst_function, mapped_block_id,
                                                     param->src_function, block_id));
    REQUIRE_OK(kefir_opt_function_block_inlined_from(param->mem, param->dst_function, mapped_block_id,
                                                     param->dst_function, param->inline_predecessor_block_id));
    ASSIGN_PTR(mapped_block_id_ptr, mapped_block_id);
    return KEFIR_OK;
}

static kefir_result_t do_inline_instr(kefir_opt_instruction_ref_t, void *);

static kefir_result_t get_instr_ref_mapping(struct do_inline_param *param, kefir_opt_instruction_ref_t instr_ref,
                                            kefir_opt_instruction_ref_t *mapped_instr_ref) {
    if (instr_ref == KEFIR_ID_NONE) {
        *mapped_instr_ref = KEFIR_ID_NONE;
        return KEFIR_OK;
    }

    if (!kefir_hashtree_has(&param->instr_mapping, (kefir_hashtree_key_t) instr_ref)) {
        REQUIRE_OK(do_inline_instr(instr_ref, param));
    }

    struct kefir_hashtree_node *node;
    REQUIRE_OK(kefir_hashtree_at(&param->instr_mapping, (kefir_hashtree_key_t) instr_ref, &node));
    *mapped_instr_ref = (kefir_opt_instruction_ref_t) node->value;
    return KEFIR_OK;
}

static kefir_result_t inline_operation_ref1(struct do_inline_param *param, const struct kefir_opt_instruction *instr,
                                            kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    kefir_opt_instruction_ref_t mapped_ref1;
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[0], &mapped_ref1));
    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {.opcode = instr->operation.opcode,
                                       .parameters = {.refs = {mapped_ref1, KEFIR_ID_NONE, KEFIR_ID_NONE}}},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_ref2(struct do_inline_param *param, const struct kefir_opt_instruction *instr,
                                            kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    kefir_opt_instruction_ref_t mapped_ref1, mapped_ref2;
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[0], &mapped_ref1));
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[1], &mapped_ref2));
    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {.opcode = instr->operation.opcode,
                                       .parameters = {.refs = {mapped_ref1, mapped_ref2, KEFIR_ID_NONE}}},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_ref3_cond(struct do_inline_param *param,
                                                 const struct kefir_opt_instruction *instr,
                                                 kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    kefir_opt_instruction_ref_t mapped_ref1, mapped_ref2, mapped_ref3;
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[0], &mapped_ref1));
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[1], &mapped_ref2));
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[2], &mapped_ref3));
    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {
            .opcode = instr->operation.opcode,
            .parameters = {.refs = {mapped_ref1, mapped_ref2, mapped_ref3},
                           .condition_variant = instr->operation.parameters.condition_variant}},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_ref4_compare(struct do_inline_param *param,
                                                    const struct kefir_opt_instruction *instr,
                                                    kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    kefir_opt_instruction_ref_t mapped_ref1, mapped_ref2, mapped_ref3, mapped_ref4;
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[0], &mapped_ref1));
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[1], &mapped_ref2));
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[2], &mapped_ref3));
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[3], &mapped_ref4));
    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {.opcode = instr->operation.opcode,
                                       .parameters = {.comparison = instr->operation.parameters.comparison,
                                                      .refs = {mapped_ref1, mapped_ref2, mapped_ref3, mapped_ref4}}},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_branch(struct do_inline_param *param, const struct kefir_opt_instruction *instr,
                                              kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id, mapped_target_block_id, mapped_alternative_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));
    REQUIRE_OK(map_block(param, instr->operation.parameters.branch.target_block, &mapped_target_block_id));
    REQUIRE_OK(map_block(param, instr->operation.parameters.branch.alternative_block, &mapped_alternative_block_id));

    kefir_opt_instruction_ref_t mapped_ref1;
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.branch.condition_ref, &mapped_ref1));
    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {
            .opcode = instr->operation.opcode,
            .parameters = {.branch = {.condition_ref = mapped_ref1,
                                      .condition_variant = instr->operation.parameters.branch.condition_variant,
                                      .target_block = mapped_target_block_id,
                                      .alternative_block = mapped_alternative_block_id}}},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_branch_compare(struct do_inline_param *param,
                                                      const struct kefir_opt_instruction *instr,
                                                      kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id, mapped_target_block_id, mapped_alternative_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));
    REQUIRE_OK(map_block(param, instr->operation.parameters.branch.target_block, &mapped_target_block_id));
    REQUIRE_OK(map_block(param, instr->operation.parameters.branch.alternative_block, &mapped_alternative_block_id));

    kefir_opt_instruction_ref_t mapped_ref1, mapped_ref2;
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[0], &mapped_ref1));
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[1], &mapped_ref2));
    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {
            .opcode = instr->operation.opcode,
            .parameters = {.refs = {mapped_ref1, mapped_ref2, KEFIR_ID_NONE},
                           .branch = {.comparison.operation = instr->operation.parameters.branch.comparison.operation,
                                      .target_block = mapped_target_block_id,
                                      .alternative_block = mapped_alternative_block_id}}},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_call_ref(struct do_inline_param *param,
                                                const struct kefir_opt_instruction *instr,
                                                kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    const struct kefir_opt_call_node *src_call_node;
    REQUIRE_OK(kefir_opt_code_container_call(&param->src_function->code,
                                             instr->operation.parameters.function_call.call_ref, &src_call_node));

    kefir_opt_instruction_ref_t mapped_ref1;
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.function_call.indirect_ref, &mapped_ref1));

    kefir_opt_call_id_t dst_call_ref;
    REQUIRE_OK(kefir_opt_code_container_new_call(param->mem, param->dst_code, mapped_block_id,
                                                 src_call_node->function_declaration_id, src_call_node->argument_count,
                                                 mapped_ref1, &dst_call_ref, mapped_instr_ref_ptr));

    for (kefir_size_t i = 0; i < src_call_node->argument_count; i++) {
        REQUIRE_OK(get_instr_ref_mapping(param, src_call_node->arguments[i], &mapped_ref1));
        REQUIRE_OK(
            kefir_opt_code_container_call_set_argument(param->mem, param->dst_code, dst_call_ref, i, mapped_ref1));
    }
    if (src_call_node->return_space != KEFIR_ID_NONE) {
        REQUIRE_OK(get_instr_ref_mapping(param, src_call_node->return_space, &mapped_ref1));
        REQUIRE_OK(
            kefir_opt_code_container_call_set_return_space(param->mem, param->dst_code, dst_call_ref, mapped_ref1));
    }
    return KEFIR_OK;
}

static kefir_result_t inline_operation_immediate(struct do_inline_param *param,
                                                 const struct kefir_opt_instruction *instr,
                                                 kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_BLOCK_LABEL) {
        kefir_opt_block_id_t mapped_block_label_id;
        REQUIRE_OK(map_block(param, instr->operation.parameters.imm.block_ref, &mapped_block_label_id));
        REQUIRE_OK(kefir_opt_code_container_new_instruction(
            param->mem, param->dst_code, mapped_block_id,
            &(struct kefir_opt_operation) {.opcode = instr->operation.opcode,
                                           .parameters = {.imm.block_ref = mapped_block_label_id}},
            mapped_instr_ref_ptr));
    } else {
        REQUIRE_OK(kefir_opt_code_container_new_instruction(
            param->mem, param->dst_code, mapped_block_id,
            &(struct kefir_opt_operation) {.opcode = instr->operation.opcode,
                                           .parameters = instr->operation.parameters},
            mapped_instr_ref_ptr));
    }
    return KEFIR_OK;
}

static kefir_result_t inline_operation_none(struct do_inline_param *param, const struct kefir_opt_instruction *instr,
                                            kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {.opcode = instr->operation.opcode, .parameters = instr->operation.parameters},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_variable(struct do_inline_param *param,
                                                const struct kefir_opt_instruction *instr,
                                                kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {.opcode = instr->operation.opcode, .parameters = instr->operation.parameters},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_type(struct do_inline_param *param, const struct kefir_opt_instruction *instr,
                                            kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {.opcode = instr->operation.opcode, .parameters = instr->operation.parameters},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_ref_offset(struct do_inline_param *param,
                                                  const struct kefir_opt_instruction *instr,
                                                  kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    kefir_opt_instruction_ref_t mapped_ref1;
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[0], &mapped_ref1));

    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {.opcode = instr->operation.opcode,
                                       .parameters = {.refs = {mapped_ref1, KEFIR_ID_NONE, KEFIR_ID_NONE},
                                                      .offset = instr->operation.parameters.offset}},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_load_mem(struct do_inline_param *param,
                                                const struct kefir_opt_instruction *instr,
                                                kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    kefir_opt_instruction_ref_t mapped_ref1;
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[0], &mapped_ref1));

    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {
            .opcode = instr->operation.opcode,
            .parameters = {.refs = {mapped_ref1, KEFIR_ID_NONE, KEFIR_ID_NONE},
                           .memory_access.flags = instr->operation.parameters.memory_access.flags}},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_store_mem(struct do_inline_param *param,
                                                 const struct kefir_opt_instruction *instr,
                                                 kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    kefir_opt_instruction_ref_t mapped_ref1, mapped_ref2;
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[0], &mapped_ref1));
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[1], &mapped_ref2));

    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {
            .opcode = instr->operation.opcode,
            .parameters = {.refs = {mapped_ref1, mapped_ref2, KEFIR_ID_NONE},
                           .memory_access.flags = instr->operation.parameters.memory_access.flags}},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_typed_ref2(struct do_inline_param *param,
                                                  const struct kefir_opt_instruction *instr,
                                                  kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    kefir_opt_instruction_ref_t mapped_ref1, mapped_ref2;
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[0], &mapped_ref1));
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[1], &mapped_ref2));

    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {.opcode = instr->operation.opcode,
                                       .parameters = {.refs = {mapped_ref1, mapped_ref2, KEFIR_ID_NONE},
                                                      .type = instr->operation.parameters.type}},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_compare_ref2(struct do_inline_param *param,
                                                    const struct kefir_opt_instruction *instr,
                                                    kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    kefir_opt_instruction_ref_t mapped_ref1, mapped_ref2;
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[0], &mapped_ref1));
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[1], &mapped_ref2));

    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {.opcode = instr->operation.opcode,
                                       .parameters = {.refs = {mapped_ref1, mapped_ref2, KEFIR_ID_NONE},
                                                      .comparison = instr->operation.parameters.comparison}},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_bitfield(struct do_inline_param *param,
                                                const struct kefir_opt_instruction *instr,
                                                kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    kefir_opt_instruction_ref_t mapped_ref1, mapped_ref2;
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[0], &mapped_ref1));
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[1], &mapped_ref2));

    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {.opcode = instr->operation.opcode,
                                       .parameters = {.refs = {mapped_ref1, mapped_ref2, KEFIR_ID_NONE},
                                                      .bitfield = instr->operation.parameters.bitfield}},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_stack_alloc(struct do_inline_param *param,
                                                   const struct kefir_opt_instruction *instr,
                                                   kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    kefir_opt_instruction_ref_t mapped_ref1, mapped_ref2;
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[0], &mapped_ref1));
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[1], &mapped_ref2));

    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {
            .opcode = instr->operation.opcode,
            .parameters = {.refs = {mapped_ref1, mapped_ref2, KEFIR_ID_NONE},
                           .stack_allocation = instr->operation.parameters.stack_allocation}},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_atomic_op(struct do_inline_param *param,
                                                 const struct kefir_opt_instruction *instr,
                                                 kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    kefir_opt_instruction_ref_t mapped_ref1, mapped_ref2, mapped_ref3;
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[0], &mapped_ref1));
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[1], &mapped_ref2));
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[2], &mapped_ref3));

    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {.opcode = instr->operation.opcode,
                                       .parameters = {.refs = {mapped_ref1, mapped_ref2, mapped_ref3},
                                                      .type = instr->operation.parameters.type,
                                                      .atomic_op = instr->operation.parameters.atomic_op}},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_overflow_arith(struct do_inline_param *param,
                                                      const struct kefir_opt_instruction *instr,
                                                      kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    kefir_opt_instruction_ref_t mapped_ref1, mapped_ref2, mapped_ref3;
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[0], &mapped_ref1));
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[1], &mapped_ref2));
    REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[2], &mapped_ref3));

    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {.opcode = instr->operation.opcode,
                                       .parameters = {.refs = {mapped_ref1, mapped_ref2, mapped_ref3},
                                                      .type = instr->operation.parameters.type,
                                                      .overflow_arith = instr->operation.parameters.overflow_arith}},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_inline_asm(struct do_inline_param *param,
                                                  const struct kefir_opt_instruction *instr,
                                                  kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    const struct kefir_opt_inline_assembly_node *src_inline_asm_node;
    REQUIRE_OK(kefir_opt_code_container_inline_assembly(
        &param->src_function->code, instr->operation.parameters.inline_asm_ref, &src_inline_asm_node));

    kefir_opt_instruction_ref_t inline_asm_instr_ref;
    kefir_opt_inline_assembly_id_t inline_asm_ref;
    REQUIRE_OK(kefir_opt_code_container_new_inline_assembly(
        param->mem, param->dst_code, mapped_block_id, src_inline_asm_node->inline_asm_id,
        src_inline_asm_node->parameter_count, &inline_asm_ref, &inline_asm_instr_ref));
    ASSIGN_PTR(mapped_instr_ref_ptr, inline_asm_instr_ref);

    for (kefir_size_t i = 0; i < src_inline_asm_node->parameter_count; i++) {
        kefir_opt_instruction_ref_t mapped_ref1, mapped_ref2;
        REQUIRE_OK(get_instr_ref_mapping(param, src_inline_asm_node->parameters[i].load_store_ref, &mapped_ref1));
        REQUIRE_OK(get_instr_ref_mapping(param, src_inline_asm_node->parameters[i].read_ref, &mapped_ref2));
        REQUIRE_OK(kefir_opt_code_container_inline_assembly_set_parameter(
            param->mem, param->dst_code, inline_asm_ref, i,
            &(struct kefir_opt_inline_assembly_parameter) {.load_store_ref = mapped_ref1, .read_ref = mapped_ref2}));
    }

    if (src_inline_asm_node->default_jump_target != KEFIR_ID_NONE) {
        kefir_opt_block_id_t mapped_target_block_id;
        REQUIRE_OK(map_block(param, src_inline_asm_node->default_jump_target, &mapped_target_block_id));
        REQUIRE_OK(kefir_opt_code_container_inline_assembly_set_default_jump_target(param->dst_code, inline_asm_ref,
                                                                                    mapped_target_block_id));
    }

    struct kefir_hashtree_node_iterator iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&src_inline_asm_node->jump_targets, &iter);
         node != NULL; node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_id_t, target_id, node->key);
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block_id, node->value);
        kefir_opt_block_id_t mapped_target_block_id;
        REQUIRE_OK(map_block(param, target_block_id, &mapped_target_block_id));
        REQUIRE_OK(kefir_opt_code_container_inline_assembly_add_jump_target(param->mem, param->dst_code, inline_asm_ref,
                                                                            target_id, mapped_target_block_id));
    }
    return KEFIR_OK;
}

static kefir_result_t inline_operation_phi_ref(struct do_inline_param *param, const struct kefir_opt_instruction *instr,
                                               kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    kefir_opt_phi_id_t phi_ref;
    REQUIRE_OK(
        kefir_opt_code_container_new_phi(param->mem, param->dst_code, mapped_block_id, &phi_ref, mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t inline_operation_index(struct do_inline_param *param, const struct kefir_opt_instruction *instr,
                                             kefir_opt_instruction_ref_t *mapped_instr_ref_ptr) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        param->mem, param->dst_code, mapped_block_id,
        &(struct kefir_opt_operation) {.opcode = instr->operation.opcode, .parameters = instr->operation.parameters},
        mapped_instr_ref_ptr));
    return KEFIR_OK;
}

static kefir_result_t generate_placeholder(struct do_inline_param *param, kefir_opt_block_id_t block_id,
                                           kefir_id_t type_id, kefir_size_t type_index,
                                           kefir_opt_instruction_ref_t insert_after,
                                           kefir_opt_instruction_ref_t *instr_ref_ptr) {
    const struct kefir_ir_type *ir_type = kefir_ir_module_get_named_type(param->module->ir_module, type_id);
    REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve IR type"));
    const struct kefir_ir_typeentry *ir_typeentry = kefir_ir_type_at(ir_type, type_index);
    REQUIRE(ir_typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve IR type entry"));

    kefir_opt_instruction_ref_t zero_instr_ref;
    switch (ir_typeentry->typecode) {
        case KEFIR_IR_TYPE_INT8:
        case KEFIR_IR_TYPE_INT16:
        case KEFIR_IR_TYPE_INT32:
        case KEFIR_IR_TYPE_INT64:
        case KEFIR_IR_TYPE_BOOL:
        case KEFIR_IR_TYPE_CHAR:
        case KEFIR_IR_TYPE_SHORT:
        case KEFIR_IR_TYPE_INT:
        case KEFIR_IR_TYPE_LONG:
        case KEFIR_IR_TYPE_WORD:
            REQUIRE_OK(kefir_opt_code_builder_int_constant(param->mem, param->dst_code, block_id, 0, instr_ref_ptr));
            break;

        case KEFIR_IR_TYPE_FLOAT32:
            REQUIRE_OK(
                kefir_opt_code_builder_float32_constant(param->mem, param->dst_code, block_id, 0.0f, instr_ref_ptr));
            break;

        case KEFIR_IR_TYPE_FLOAT64:
            REQUIRE_OK(
                kefir_opt_code_builder_float64_constant(param->mem, param->dst_code, block_id, 0.0f, instr_ref_ptr));
            break;

        case KEFIR_IR_TYPE_LONG_DOUBLE:
            REQUIRE_OK(kefir_opt_code_builder_long_double_constant(param->mem, param->dst_code, block_id, 0.0L,
                                                                   instr_ref_ptr));
            break;

        case KEFIR_IR_TYPE_COMPLEX_FLOAT32:
            REQUIRE_OK(
                kefir_opt_code_builder_float32_constant(param->mem, param->dst_code, block_id, 0.0f, instr_ref_ptr));
            REQUIRE_OK(kefir_opt_code_builder_complex_float32_from(param->mem, param->dst_code, block_id,
                                                                   *instr_ref_ptr, *instr_ref_ptr, instr_ref_ptr));
            break;

        case KEFIR_IR_TYPE_COMPLEX_FLOAT64:
            REQUIRE_OK(
                kefir_opt_code_builder_float64_constant(param->mem, param->dst_code, block_id, 0.0f, instr_ref_ptr));
            REQUIRE_OK(kefir_opt_code_builder_complex_float64_from(param->mem, param->dst_code, block_id,
                                                                   *instr_ref_ptr, *instr_ref_ptr, instr_ref_ptr));
            break;

        case KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(kefir_opt_code_builder_long_double_constant(param->mem, param->dst_code, block_id, 0.0L,
                                                                   instr_ref_ptr));
            REQUIRE_OK(kefir_opt_code_builder_complex_long_double_from(param->mem, param->dst_code, block_id,
                                                                       *instr_ref_ptr, *instr_ref_ptr, instr_ref_ptr));
            break;

        default: {
            const struct kefir_opt_call_node *original_call;
            REQUIRE_OK(
                kefir_opt_code_container_call(&param->dst_function->code, param->original_call_ref, &original_call));
            REQUIRE(original_call->return_space != KEFIR_ID_NONE,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid return space for inlined call site"));
            *instr_ref_ptr = original_call->return_space;
            REQUIRE_OK(kefir_opt_code_builder_zero_memory(param->mem, param->dst_code, block_id, *instr_ref_ptr,
                                                          type_id, 0, &zero_instr_ref));
            REQUIRE_OK(
                kefir_opt_code_container_insert_control(param->dst_code, block_id, insert_after, zero_instr_ref));
        } break;
    }

    return KEFIR_OK;
}

static kefir_result_t inline_return(struct do_inline_param *param, const struct kefir_opt_instruction *instr,
                                    kefir_opt_instruction_ref_t *mapped_instr_ref) {
    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    if (kefir_ir_type_length(param->src_function->ir_func->declaration->result) > 0) {
        const struct kefir_ir_typeentry *ir_typeentry =
            kefir_ir_type_at(param->src_function->ir_func->declaration->result, 0);
        REQUIRE(ir_typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve IR type entry"));

        if (param->result_phi_ref == KEFIR_ID_NONE) {
            REQUIRE_OK(kefir_opt_code_container_new_phi(param->mem, param->dst_code, param->inline_successor_block_id,
                                                        &param->result_phi_ref, &param->result_phi_instr));
        }

        if (instr->operation.parameters.refs[0] != KEFIR_ID_NONE) {
            REQUIRE_OK(get_instr_ref_mapping(param, instr->operation.parameters.refs[0], mapped_instr_ref));

            const struct kefir_opt_call_node *call_node;
            REQUIRE_OK(kefir_opt_code_container_call(param->dst_code, param->original_call_ref, &call_node));
            if (call_node->return_space != KEFIR_ID_NONE) {
                kefir_opt_instruction_ref_t copy_instr_ref;
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(
                    param->mem, param->dst_code, mapped_block_id, call_node->return_space, *mapped_instr_ref,
                    param->src_function->ir_func->declaration->result_type_id, 0, &copy_instr_ref));

                const struct kefir_opt_code_block *mapped_block;
                REQUIRE_OK(kefir_opt_code_container_block(param->dst_code, mapped_block_id, &mapped_block));

                REQUIRE_OK(kefir_opt_code_container_insert_control(param->dst_code, mapped_block_id,
                                                                   mapped_block->control_flow.tail, copy_instr_ref));

                *mapped_instr_ref = call_node->return_space;
            }
        } else {
            const struct kefir_opt_code_block *mapped_block;
            REQUIRE_OK(kefir_opt_code_container_block(param->dst_code, mapped_block_id, &mapped_block));
            REQUIRE_OK(generate_placeholder(param, mapped_block_id,
                                            param->src_function->ir_func->declaration->result_type_id, 0,
                                            mapped_block->control_flow.tail, mapped_instr_ref));
        }
        REQUIRE_OK(kefir_opt_code_container_phi_attach(param->mem, param->dst_code, param->result_phi_ref,
                                                       mapped_block_id, *mapped_instr_ref));
    }
    REQUIRE_OK(kefir_opt_code_builder_finalize_jump(param->mem, param->dst_code, mapped_block_id,
                                                    param->inline_successor_block_id, mapped_instr_ref));

    return KEFIR_OK;
}

static kefir_result_t do_inline_instr(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct do_inline_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid inline trace payload"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&param->src_function->code, instr_ref, &instr));
    REQUIRE(!kefir_hashtree_has(&param->instr_mapping, (kefir_hashtree_key_t) instr_ref), KEFIR_OK);

    kefir_opt_block_id_t mapped_block_id;
    REQUIRE_OK(map_block(param, instr->block_id, &mapped_block_id));

    kefir_size_t src_ir_instruction_location;
    REQUIRE_OK(kefir_opt_code_debug_info_instruction_location(&param->src_function->debug_info, instr_ref,
                                                              &src_ir_instruction_location));
    REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor(
        &param->dst_function->debug_info,
        param->dst_function->debug_info_mapping.ir_code_length + src_ir_instruction_location));

    kefir_opt_instruction_ref_t mapped_instr_ref;
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_RETURN) {
        REQUIRE_OK(inline_return(param, instr, &mapped_instr_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE ||
               instr->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL) {
        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to inline function with tail calls");
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_GET_ARGUMENT) {
        const struct kefir_opt_call_node *call_node;
        REQUIRE_OK(kefir_opt_code_container_call(param->dst_code, param->original_call_ref, &call_node));
        REQUIRE(instr->operation.parameters.index < call_node->argument_count,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                "Mismatch between inlined function argument count and call site arguments"));
        mapped_instr_ref = call_node->arguments[instr->operation.parameters.index];
    } else {
        switch (instr->operation.opcode) {
#define OPCODE_DEF(_id, _symbolic, _class)                                      \
    case KEFIR_OPT_OPCODE_##_id:                                                \
        REQUIRE_OK(inline_operation_##_class(param, instr, &mapped_instr_ref)); \
        break;

            KEFIR_OPTIMIZER_OPCODE_DEFS(OPCODE_DEF, )
#undef OPCODE_DEF
        }

        kefir_bool_t is_control_flow;
        REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(&param->src_function->code, instr_ref, &is_control_flow));
        if (is_control_flow) {
            kefir_opt_instruction_ref_t control_prev;
            REQUIRE_OK(kefir_opt_instruction_prev_control(&param->src_function->code, instr_ref, &control_prev));
            kefir_opt_instruction_ref_t mapped_control_prev;
            REQUIRE_OK(get_instr_ref_mapping(param, control_prev, &mapped_control_prev));
            REQUIRE_OK(kefir_opt_code_container_insert_control(param->dst_code, mapped_block_id, mapped_control_prev,
                                                               mapped_instr_ref));
        }
    }
    REQUIRE_OK(kefir_hashtree_insert(param->mem, &param->instr_mapping, (kefir_hashtree_key_t) instr_ref,
                                     (kefir_hashtree_value_t) mapped_instr_ref));

    return KEFIR_OK;
}

static kefir_result_t inline_blocks(struct do_inline_param *param) {
    struct kefir_opt_code_container_tracer tracer = {.trace_instruction = do_inline_instr, .payload = param};
    REQUIRE_OK(kefir_opt_code_container_trace(param->mem, &param->src_function->code, &tracer));

    return KEFIR_OK;
}

static kefir_result_t map_inlined_phis(struct do_inline_param *param) {
    kefir_size_t num_of_src_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(&param->src_function->code, &num_of_src_blocks));

    for (kefir_opt_block_id_t src_block_id = 0; src_block_id < num_of_src_blocks; src_block_id++) {
        const struct kefir_opt_code_block *src_block;
        REQUIRE_OK(kefir_opt_code_container_block(&param->src_function->code, src_block_id, &src_block));
        kefir_opt_phi_id_t src_phi_ref;
        kefir_result_t res;
        for (res = kefir_opt_code_block_phi_head(&param->src_function->code, src_block, &src_phi_ref);
             res == KEFIR_OK && src_phi_ref != KEFIR_ID_NONE;
             res = kefir_opt_phi_next_sibling(&param->src_function->code, src_phi_ref, &src_phi_ref)) {

            const struct kefir_opt_phi_node *src_phi_node;
            REQUIRE_OK(kefir_opt_code_container_phi(&param->src_function->code, src_phi_ref, &src_phi_node));
            if (!kefir_hashtree_has(&param->instr_mapping, (kefir_hashtree_key_t) src_phi_node->output_ref)) {
                continue;
            }

            kefir_opt_instruction_ref_t dst_phi_instr_ref;
            REQUIRE_OK(get_instr_ref_mapping(param, src_phi_node->output_ref, &dst_phi_instr_ref));

            const struct kefir_opt_instruction *dst_phi_instr;
            REQUIRE_OK(kefir_opt_code_container_instr(param->dst_code, dst_phi_instr_ref, &dst_phi_instr));
            kefir_opt_phi_id_t dst_phi_ref = dst_phi_instr->operation.parameters.phi_ref;

            struct kefir_hashtree_node_iterator iter;
            for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&src_phi_node->links, &iter); node != NULL;
                 node = kefir_hashtree_next(&iter)) {
                ASSIGN_DECL_CAST(kefir_opt_block_id_t, link_block_id, node->key);
                ASSIGN_DECL_CAST(kefir_opt_block_id_t, link_instr_ref, node->value);

                struct kefir_hashtree_node *node;
                kefir_result_t res =
                    kefir_hashtree_at(&param->block_mapping, (kefir_hashtree_key_t) link_block_id, &node);
                if (res != KEFIR_NOT_FOUND) {
                    REQUIRE_OK(res);
                    ASSIGN_DECL_CAST(kefir_opt_block_id_t, mapped_block_id, node->value);
                    kefir_opt_instruction_ref_t mapped_instr_ref;
                    REQUIRE_OK(get_instr_ref_mapping(param, link_instr_ref, &mapped_instr_ref));
                    REQUIRE_OK(kefir_opt_code_container_phi_attach(param->mem, param->dst_code, dst_phi_ref,
                                                                   mapped_block_id, mapped_instr_ref));
                }
            }
        }
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t link_inlined_entry_block(struct do_inline_param *param) {
    kefir_opt_block_id_t inlined_entry_block_id;
    REQUIRE_OK(map_block(param, param->src_function->code.entry_point, &inlined_entry_block_id));

    const struct kefir_opt_code_block *pred_block;
    REQUIRE_OK(kefir_opt_code_container_block(param->dst_code, param->inline_predecessor_block_id, &pred_block));

    kefir_opt_instruction_ref_t tail_instr_ref;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(param->dst_code, pred_block, &tail_instr_ref));
    REQUIRE_OK(kefir_opt_code_container_drop_control(param->dst_code, tail_instr_ref));
    REQUIRE_OK(kefir_opt_code_container_drop_instr(param->mem, param->dst_code, tail_instr_ref));

    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(param->dst_code, pred_block, &tail_instr_ref));
    if (param->result_phi_instr != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_replace_references(param->mem, param->dst_code, param->result_phi_instr,
                                                               tail_instr_ref));
        REQUIRE_OK(kefir_opt_code_debug_info_replace_local_variable(param->mem, &param->dst_function->debug_info,
                                                                    tail_instr_ref, param->result_phi_instr));
    }
    REQUIRE_OK(kefir_opt_code_container_drop_control(param->dst_code, tail_instr_ref));
    REQUIRE_OK(kefir_opt_code_container_drop_instr(param->mem, param->dst_code, tail_instr_ref));

    REQUIRE_OK(kefir_opt_code_builder_finalize_jump(param->mem, param->dst_code, pred_block->id, inlined_entry_block_id,
                                                    NULL));

    return KEFIR_OK;
}

static kefir_result_t clone_entry_into(struct kefir_mem *mem, struct kefir_ir_debug_entries *entries,
                                       struct kefir_string_pool *symbols, kefir_ir_debug_entry_id_t src_entry_id,
                                       kefir_ir_debug_entry_id_t dst_parent_entry_id, kefir_size_t ir_mapping_base) {
    const struct kefir_ir_debug_entry *src_entry;
    REQUIRE_OK(kefir_ir_debug_entry_get(entries, src_entry_id, &src_entry));

    kefir_ir_debug_entry_id_t dst_entry_id;
    REQUIRE_OK(kefir_ir_debug_entry_new_child(mem, entries, dst_parent_entry_id, src_entry->tag, &dst_entry_id));

    kefir_result_t res;
    struct kefir_ir_debug_entry_attribute_iterator attr_iter;
    const struct kefir_ir_debug_entry_attribute *src_entry_attr;
    for (res = kefir_ir_debug_entry_attribute_iter(entries, src_entry_id, &attr_iter, &src_entry_attr); res == KEFIR_OK;
         res = kefir_ir_debug_entry_attribute_next(&attr_iter, &src_entry_attr)) {
        if (src_entry_attr->tag == KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_BEGIN ||
            src_entry_attr->tag == KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CODE_END) {
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(
                mem, entries, symbols, dst_entry_id,
                &(struct kefir_ir_debug_entry_attribute) {.tag = src_entry_attr->tag,
                                                          .code_index = src_entry_attr->code_index + ir_mapping_base}));
        } else {
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, entries, symbols, dst_entry_id, src_entry_attr));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_ir_debug_entry_child_iterator entry_iter;
    kefir_ir_debug_entry_id_t child_entry_id;
    for (res = kefir_ir_debug_entry_child_iter(entries, src_entry_id, &entry_iter, &child_entry_id); res == KEFIR_OK;
         res = kefir_ir_debug_entry_child_next(&entry_iter, &child_entry_id)) {
        REQUIRE_OK(clone_entry_into(mem, entries, symbols, child_entry_id, dst_parent_entry_id, ir_mapping_base));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t inline_debug_source_map(struct do_inline_param *param) {
    kefir_result_t res;
    struct kefir_ir_debug_function_source_map_iterator iter;
    const struct kefir_ir_debug_source_location *source_location;
    const kefir_size_t ir_mapping_base = param->dst_function->debug_info_mapping.ir_code_length;
    ASSIGN_DECL_CAST(struct kefir_ir_debug_function_source_map *, source_map,
                     &param->dst_function->ir_func->debug_info.source_map);
    for (res = kefir_ir_debug_function_source_map_iter(&param->src_function->ir_func->debug_info.source_map, &iter,
                                                       &source_location);
         res == KEFIR_OK; res = kefir_ir_debug_function_source_map_next(&iter, &source_location)) {
        REQUIRE_OK(kefir_ir_debug_function_source_map_insert(
            param->mem, source_map, &param->module->ir_module->symbols, &source_location->location,
            source_location->begin + ir_mapping_base, source_location->end + ir_mapping_base));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t inline_debug_entries(struct do_inline_param *param) {
    kefir_result_t res;
    struct kefir_ir_debug_entry_child_iterator entry_iter;
    kefir_ir_debug_entry_id_t child_entry_id;
    const kefir_size_t ir_mapping_base = param->dst_function->debug_info_mapping.ir_code_length;
    ASSIGN_DECL_CAST(struct kefir_ir_debug_entries *, entries, &param->module->ir_module->debug_info.entries);

    kefir_ir_debug_entry_id_t inlined_function_lexical_block_id;
    REQUIRE_OK(kefir_ir_debug_entry_new_child(param->mem, entries,
                                              param->dst_function->ir_func->debug_info.subprogram_id,
                                              KEFIR_IR_DEBUG_ENTRY_LEXICAL_BLOCK, &inlined_function_lexical_block_id));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(param->mem, entries, &param->module->ir_module->symbols,
                                                  inlined_function_lexical_block_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_CODE_BEGIN(ir_mapping_base)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(
        param->mem, entries, &param->module->ir_module->symbols, inlined_function_lexical_block_id,
        &KEFIR_IR_DEBUG_ENTRY_ATTR_CODE_END(ir_mapping_base +
                                            kefir_irblock_length(&param->src_function->ir_func->body))));

    for (res = kefir_ir_debug_entry_child_iter(entries, param->src_function->ir_func->debug_info.subprogram_id,
                                               &entry_iter, &child_entry_id);
         res == KEFIR_OK; res = kefir_ir_debug_entry_child_next(&entry_iter, &child_entry_id)) {
        const struct kefir_ir_debug_entry *src_entry;
        REQUIRE_OK(kefir_ir_debug_entry_get(entries, child_entry_id, &src_entry));
        if (src_entry->tag != KEFIR_IR_DEBUG_ENTRY_FUNCTION_PARAMETER &&
            src_entry->tag != KEFIR_IR_DEBUG_ENTRY_FUNCTION_VARARG) {
            REQUIRE_OK(clone_entry_into(param->mem, entries, &param->module->ir_module->symbols, child_entry_id,
                                        inlined_function_lexical_block_id, ir_mapping_base));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t inline_debug_allocation_info(struct do_inline_param *param) {
    kefir_result_t res;
    struct kefir_opt_code_debug_info_local_variable_iterator alloc_iter;
    kefir_uint64_t variable_id;
    for (res = kefir_opt_code_debug_info_local_variable_allocation_iter(&param->src_function->debug_info, &alloc_iter,
                                                                        &variable_id);
         res == KEFIR_OK; res = kefir_opt_code_debug_info_local_variable_allocation_next(&alloc_iter, &variable_id)) {
        const struct kefir_opt_code_debug_info_local_variable_refset *refset;
        REQUIRE_OK(kefir_opt_code_debug_info_local_variable_allocation_of(&param->src_function->debug_info, variable_id,
                                                                          &refset));

        struct kefir_hashtreeset_iterator iter;
        for (res = kefir_hashtreeset_iter(&refset->refs, &iter); res == KEFIR_OK; res = kefir_hashtreeset_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, alloc_instr_ref, iter.entry);

            kefir_opt_instruction_ref_t mapped_alloc_instr_ref;
            struct kefir_hashtree_node *node2;
            res = kefir_hashtree_at(&param->instr_mapping, (kefir_hashtree_key_t) alloc_instr_ref, &node2);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                mapped_alloc_instr_ref = node2->value;
            } else {
                kefir_opt_block_id_t mapped_block_id;
                REQUIRE_OK(map_block(param, param->src_function->code.entry_point, &mapped_block_id));
                REQUIRE_OK(kefir_opt_code_builder_int_placeholder(param->mem, param->dst_code, mapped_block_id,
                                                                  &mapped_alloc_instr_ref));
                REQUIRE_OK(kefir_hashtree_insert(param->mem, &param->instr_mapping,
                                                 (kefir_hashtree_key_t) alloc_instr_ref, mapped_alloc_instr_ref));
            }

            REQUIRE_OK(kefir_opt_code_debug_info_register_local_variable_allocation(
                param->mem, &param->dst_function->debug_info, mapped_alloc_instr_ref, variable_id));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    kefir_opt_instruction_ref_t alloc_instr_ref;
    for (res = kefir_opt_code_debug_info_local_variable_iter(&param->src_function->debug_info, &alloc_iter,
                                                             &alloc_instr_ref);
         res == KEFIR_OK; res = kefir_opt_code_debug_info_local_variable_next(&alloc_iter, &alloc_instr_ref)) {
        struct kefir_hashtree_node *node;
        kefir_opt_instruction_ref_t mapped_alloc_instr_ref;
        res = kefir_hashtree_at(&param->instr_mapping, (kefir_hashtree_key_t) alloc_instr_ref, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            mapped_alloc_instr_ref = node->value;
        } else {
            kefir_opt_block_id_t mapped_block_id;
            REQUIRE_OK(map_block(param, param->src_function->code.entry_point, &mapped_block_id));
            REQUIRE_OK(kefir_opt_code_builder_int_placeholder(param->mem, param->dst_code, mapped_block_id,
                                                              &mapped_alloc_instr_ref));
            REQUIRE_OK(kefir_hashtree_insert(param->mem, &param->instr_mapping, (kefir_hashtree_key_t) alloc_instr_ref,
                                             mapped_alloc_instr_ref));
        }

        kefir_opt_instruction_ref_t ref;
        struct kefir_opt_code_debug_info_local_variable_ref_iterator ref_iter;
        for (res = kefir_opt_code_debug_info_local_variable_ref_iter(&param->src_function->debug_info, &ref_iter,
                                                                     alloc_instr_ref, &ref);
             res == KEFIR_OK; res = kefir_opt_code_debug_info_local_variable_ref_next(&ref_iter, &ref)) {
            res = kefir_hashtree_at(&param->instr_mapping, (kefir_hashtree_key_t) ref, &node);
            if (res == KEFIR_NOT_FOUND) {
                continue;
            }
            REQUIRE_OK(res);
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, mapped_ref, node->value);
            REQUIRE_OK(kefir_opt_code_debug_info_add_local_variable_ref(param->mem, &param->dst_function->debug_info,
                                                                        mapped_alloc_instr_ref, mapped_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t do_inline_impl(struct do_inline_param *param) {
    REQUIRE_OK(inline_blocks(param));
    REQUIRE_OK(map_inlined_phis(param));
    REQUIRE_OK(link_inlined_entry_block(param));

    REQUIRE_OK(inline_debug_source_map(param));
    REQUIRE_OK(inline_debug_entries(param));
    REQUIRE_OK(inline_debug_allocation_info(param));
    param->dst_function->debug_info_mapping.ir_code_length += kefir_irblock_length(&param->src_function->ir_func->body);

    return KEFIR_OK;
}

static kefir_result_t do_inline(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                struct kefir_opt_function *dst_function, const struct kefir_opt_function *src_function,
                                kefir_opt_block_id_t inline_predecessor_block_id,
                                kefir_opt_block_id_t inline_successor_block_id, kefir_opt_call_id_t original_call_ref,
                                kefir_opt_instruction_ref_t original_call_instr_ref) {
    struct do_inline_param param = {.mem = mem,
                                    .module = module,
                                    .dst_function = dst_function,
                                    .src_function = src_function,
                                    .dst_code = &dst_function->code,
                                    .inline_predecessor_block_id = inline_predecessor_block_id,
                                    .inline_successor_block_id = inline_successor_block_id,
                                    .original_call_ref = original_call_ref,
                                    .original_call_instr_ref = original_call_instr_ref,
                                    .result_phi_ref = KEFIR_ID_NONE,
                                    .result_phi_instr = KEFIR_ID_NONE};
    REQUIRE_OK(kefir_hashtree_init(&param.block_mapping, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&param.instr_mapping, &kefir_hashtree_uint_ops));

    kefir_result_t res = do_inline_impl(&param);

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &param.block_mapping);
        kefir_hashtree_free(mem, &param.instr_mapping);
        return res;
    });
    res = kefir_hashtree_free(mem, &param.block_mapping);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &param.instr_mapping);
        return res;
    });
    REQUIRE_OK(kefir_hashtree_free(mem, &param.instr_mapping));
    return KEFIR_OK;
}

static kefir_result_t can_inline_function(const struct kefir_opt_function *callee_function,
                                          const struct kefir_opt_function *called_function,
                                          const struct kefir_opt_call_node *call_node,
                                          const struct kefir_opt_try_inline_function_call_parameters *inline_params,
                                          kefir_bool_t *can_inline_ptr) {
    kefir_bool_t can_inline;
    REQUIRE_OK(kefir_opt_function_block_can_inline(
        callee_function, call_node->block_id, called_function,
        (inline_params == NULL ? KEFIR_SIZE_MAX : inline_params->max_inline_depth), &can_inline));

    if (!called_function->ir_func->flags.inline_function || called_function->ir_func->declaration->vararg ||
        called_function->ir_func->declaration->returns_twice ||
        called_function->ir_func->declaration->id == callee_function->ir_func->declaration->id ||
        (inline_params == NULL || callee_function->num_of_inlines >= inline_params->max_inlines_per_function)) {
        can_inline = false;
    }

    kefir_size_t called_func_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(&called_function->code, &called_func_blocks));
    for (kefir_opt_block_id_t block_id = 0; can_inline && block_id < called_func_blocks; block_id++) {
        const struct kefir_opt_code_block *block;
        REQUIRE_OK(kefir_opt_code_container_block(&called_function->code, block_id, &block));
        if (!kefir_hashtreeset_empty(&block->public_labels)) {
            can_inline = false;
        }

        kefir_opt_instruction_ref_t instr_ref;
        kefir_result_t res;
        for (res = kefir_opt_code_block_instr_head(&called_function->code, block, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE && can_inline;
             res = kefir_opt_instruction_next_sibling(&called_function->code, instr_ref, &instr_ref)) {
            const struct kefir_opt_instruction *instruction;
            REQUIRE_OK(kefir_opt_code_container_instr(&called_function->code, instr_ref, &instruction));
            if ((instruction->operation.opcode == KEFIR_OPT_OPCODE_GET_ARGUMENT &&
                 instruction->operation.parameters.index >= call_node->argument_count) ||
                instruction->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE ||
                instruction->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL) {
                can_inline = false;
            }
        }
        REQUIRE_OK(res);
    }

    *can_inline_ptr = can_inline;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_try_inline_function_call(
    struct kefir_mem *mem, const struct kefir_opt_module *module, struct kefir_opt_function *func,
    struct kefir_opt_code_structure *structure,
    const struct kefir_opt_try_inline_function_call_parameters *inline_params, kefir_opt_instruction_ref_t instr_ref,
    kefir_bool_t *did_inline_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));

    ASSIGN_PTR(did_inline_ptr, false);

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_ref, &instr));
    REQUIRE(instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to inline non-function call instruction"));

    const struct kefir_opt_call_node *call_node;
    REQUIRE_OK(
        kefir_opt_code_container_call(&func->code, instr->operation.parameters.function_call.call_ref, &call_node));

    const struct kefir_ir_function_decl *ir_func_decl =
        kefir_ir_module_get_declaration(module->ir_module, call_node->function_declaration_id);
    REQUIRE(ir_func_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve IR function declaration"));
    const struct kefir_ir_function *ir_func = kefir_ir_module_get_function(module->ir_module, ir_func_decl->name);
    REQUIRE(ir_func != NULL, KEFIR_OK);

    struct kefir_opt_function *called_func;
    REQUIRE_OK(kefir_opt_module_get_function(module, ir_func->declaration->id, &called_func));

    kefir_bool_t can_inline;
    REQUIRE_OK(can_inline_function(func, called_func, call_node, inline_params, &can_inline));

    if (can_inline) {
        kefir_opt_block_id_t block_id = instr->block_id;
        kefir_opt_block_id_t split_block_id;
        REQUIRE_OK(kefir_opt_code_split_block_after(mem, &func->code, &func->debug_info, structure, instr_ref,
                                                    &split_block_id));
        REQUIRE_OK(do_inline(mem, module, func, called_func, block_id, split_block_id, call_node->node_id,
                             call_node->output_ref));
        func->num_of_inlines++;
        ASSIGN_PTR(did_inline_ptr, true);

        REQUIRE_OK(kefir_opt_code_structure_free(mem, structure));
        REQUIRE_OK(kefir_opt_code_structure_init(structure));
        REQUIRE_OK(kefir_opt_code_structure_build(mem, structure, &func->code));
    }

    return KEFIR_OK;
}
