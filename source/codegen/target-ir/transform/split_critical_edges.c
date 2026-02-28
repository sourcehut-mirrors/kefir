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

#include "kefir/codegen/target-ir/transform.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t update_operand(struct kefir_codegen_target_ir_operand *operand,
                                     kefir_codegen_target_ir_block_ref_t source_block_ref,
                                     kefir_codegen_target_ir_block_ref_t target_block_ref) {
    switch (operand->type) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_PHYSICAL_REGISTER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_NATIVE:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_EXTERNAL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NATIVE_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_EXTERNAL_LABEL:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UPSILON:
            // Intentionally left blank
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF:
            if (operand->block_ref == source_block_ref) {
                operand->block_ref = target_block_ref;
            }
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF:
            if (operand->rip_indirection.block_ref == source_block_ref) {
                operand->rip_indirection.block_ref = target_block_ref;
            }
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT:
            switch (operand->indirect.type) {
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS:
                    if (operand->indirect.base.block_ref == source_block_ref) {
                        operand->indirect.base.block_ref = target_block_ref;
                    }
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_PHYSICAL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_IMMEDIATE_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_NATIVE_LABEL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_EXTERNAL_LABEL_BASIS:
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_SPILL_AREA_BASIS:
                    // Intentionally left blank
                    break;
            }
    }

    return KEFIR_OK;
}

static kefir_result_t split_edge(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
                                 kefir_codegen_target_ir_block_ref_t source_block_ref,
                                 kefir_codegen_target_ir_block_ref_t target_block_ref) {
    REQUIRE(!kefir_codegen_target_ir_code_is_gate_block(code, source_block_ref), KEFIR_OK);

    kefir_codegen_target_ir_block_ref_t split_block_ref;
    REQUIRE_OK(kefir_codegen_target_ir_code_new_block(mem, code, &split_block_ref));

    const kefir_codegen_target_ir_instruction_ref_t source_block_tail_ref =
        kefir_codegen_target_ir_code_block_control_tail(code, source_block_ref);
    const struct kefir_codegen_target_ir_instruction *source_block_tail;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, source_block_tail_ref, &source_block_tail));

    kefir_result_t res;
    kefir_codegen_target_ir_instruction_ref_t new_tail_ref;
    if (source_block_tail->operation.opcode == code->klass->inline_asm_opcode) {
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(
            mem, code, source_block_ref, source_block_tail_ref,
            &(struct kefir_codegen_target_ir_operation) {
                .opcode = source_block_tail->operation.opcode,
                .inline_asm_node.target_block_ref = source_block_tail->operation.inline_asm_node.target_block_ref},
            &source_block_tail->metadata, &new_tail_ref));

        struct kefir_codegen_target_ir_code_inline_assembly_fragment_iterator iter;
        const struct kefir_codegen_target_ir_inline_assembly_fragment *fragment;
        for (res = kefir_codegen_target_ir_code_inline_assembly_fragment_iter(code, &iter, source_block_tail_ref,
                                                                              &fragment);
             res == KEFIR_OK; res = kefir_codegen_target_ir_code_inline_assembly_fragment_next(&iter, &fragment)) {
            switch (fragment->type) {
                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_TEXT:
                    REQUIRE_OK(kefir_codegen_target_ir_code_inline_assembly_text_fragment(mem, code, new_tail_ref,
                                                                                          fragment->text));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_OPERAND: {
                    struct kefir_codegen_target_ir_operand operand = fragment->operand;
                    REQUIRE_OK(update_operand(&operand, source_block_ref, split_block_ref));
                    REQUIRE_OK(kefir_codegen_target_ir_code_inline_assembly_operand_fragment(mem, code, new_tail_ref,
                                                                                             &operand));
                } break;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    } else {
        struct kefir_codegen_target_ir_operation new_tail = source_block_tail->operation;
        for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
            REQUIRE_OK(update_operand(&new_tail.parameters[i], target_block_ref, split_block_ref));
        }
        struct kefir_codegen_target_ir_instruction_metadata metadata = source_block_tail->metadata;
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, source_block_ref, source_block_tail_ref,
                                                                &new_tail, &metadata, &new_tail_ref));
    }
    REQUIRE_OK(kefir_codegen_target_ir_code_replace_instruction(mem, code, new_tail_ref, source_block_tail_ref));
    REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, source_block_tail_ref));

    struct kefir_codegen_target_ir_value_phi_node_iterator phi_node_iter;
    kefir_codegen_target_ir_instruction_ref_t phi_ref;
    for (res = kefir_codegen_target_ir_code_phi_node_iter(code, &phi_node_iter, target_block_ref, &phi_ref);
         res == KEFIR_OK; res = kefir_codegen_target_ir_code_phi_node_next(&phi_node_iter, &phi_ref)) {
        kefir_codegen_target_ir_value_ref_t link_value_ref;
        REQUIRE_OK(kefir_codegen_target_ir_code_phi_link_for(code, phi_ref, source_block_ref, &link_value_ref));
        REQUIRE_OK(kefir_codegen_target_ir_code_phi_attach(mem, code, phi_ref, split_block_ref, link_value_ref));
        REQUIRE_OK(kefir_codegen_target_ir_code_phi_drop(mem, code, phi_ref, source_block_ref));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_codegen_target_ir_operation jump_operation = {0};
    REQUIRE_OK(code->klass->make_unconditional_jump(target_block_ref, &jump_operation, code->klass->payload));
    REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(mem, code, split_block_ref, KEFIR_ID_NONE, &jump_operation,
                                                            NULL, NULL));
    return KEFIR_OK;
}

static kefir_result_t split_critical_edges(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
                                           struct kefir_hashset *critical_edges,
                                           struct kefir_codegen_target_ir_control_flow *control_flow) {
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_build(mem, control_flow));

    kefir_result_t res;
    kefir_hashset_key_t key;
    struct kefir_hashset_iterator iter;
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        if (!kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref)) {
            continue;
        }

        for (res = kefir_hashset_iter(&control_flow->blocks[block_ref].successors, &iter, &key); res == KEFIR_OK;
             res = kefir_hashset_next(&iter, &key)) {
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, successor_block_ref, key);
            if (kefir_codegen_target_ir_control_flow_is_critical_edge(control_flow, block_ref, successor_block_ref)) {
                const kefir_uint64_t edge = (((kefir_uint64_t) block_ref) << 32) | (kefir_uint32_t) successor_block_ref;
                REQUIRE_OK(kefir_hashset_add(mem, critical_edges, (kefir_hashset_key_t) edge));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    for (res = kefir_hashset_iter(critical_edges, &iter, &key); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &key)) {
        ASSIGN_DECL_CAST(kefir_uint64_t, edge, key);
        kefir_codegen_target_ir_block_ref_t block_ref = edge >> 32;
        kefir_codegen_target_ir_block_ref_t successor_block_ref = (kefir_uint32_t) edge;
        REQUIRE_OK(split_edge(mem, code, block_ref, successor_block_ref));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_transform_split_critical_edges(struct kefir_mem *mem,
                                                                      struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    struct kefir_hashset critical_edges;
    REQUIRE_OK(kefir_hashset_init(&critical_edges, &kefir_hashtable_uint_ops));

    struct kefir_codegen_target_ir_control_flow control_flow;
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_init(&control_flow, code));

    kefir_result_t res = split_critical_edges(mem, code, &critical_edges, &control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_control_flow_free(mem, &control_flow);
        kefir_hashset_free(mem, &critical_edges);
        return res;
    });
    res = kefir_codegen_target_ir_control_flow_free(mem, &control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &critical_edges);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &critical_edges));
    return KEFIR_OK;
}
