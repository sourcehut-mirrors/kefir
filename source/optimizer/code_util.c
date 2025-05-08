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

#include "kefir/optimizer/code_util.h"
#include "kefir/optimizer/builder.h"
#include "kefir/optimizer/structure.h"
#include "kefir/core/queue.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

#define INPUT_CALLBACK(_ref, _callback, _payload)         \
    do {                                                  \
        if ((_ref) != KEFIR_ID_NONE) {                    \
            REQUIRE_OK((_callback) ((_ref), (_payload))); \
        }                                                 \
    } while (0)

static kefir_result_t extract_inputs_store_mem(const struct kefir_opt_code_container *code,
                                               const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                               kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                               void *payload) {
    UNUSED(code);
    UNUSED(resolve_phi);
    INPUT_CALLBACK(instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_load_mem(const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                              kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                              void *payload) {
    UNUSED(code);
    UNUSED(resolve_phi);
    INPUT_CALLBACK(instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_stack_alloc(const struct kefir_opt_code_container *code,
                                                 const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                                 kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                                 void *payload) {
    UNUSED(code);
    UNUSED(resolve_phi);
    INPUT_CALLBACK(instr->operation.parameters.refs[KEFIR_OPT_STACK_ALLOCATION_ALIGNMENT_REF], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[KEFIR_OPT_STACK_ALLOCATION_SIZE_REF], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_bitfield(const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                              kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                              void *payload) {
    UNUSED(code);
    UNUSED(resolve_phi);
    INPUT_CALLBACK(instr->operation.parameters.refs[KEFIR_OPT_BITFIELD_BASE_REF], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[KEFIR_OPT_BITFIELD_VALUE_REF], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_branch(const struct kefir_opt_code_container *code,
                                            const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                            kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                            void *payload) {
    UNUSED(code);
    UNUSED(resolve_phi);
    INPUT_CALLBACK(instr->operation.parameters.branch.condition_ref, callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_branch_compare(const struct kefir_opt_code_container *code,
                                                    const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                                    kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                                    void *payload) {
    UNUSED(code);
    UNUSED(resolve_phi);
    INPUT_CALLBACK(instr->operation.parameters.refs[0], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[1], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_typed_ref2(const struct kefir_opt_code_container *code,
                                                const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                                kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                                void *payload) {
    UNUSED(code);
    UNUSED(resolve_phi);
    INPUT_CALLBACK(instr->operation.parameters.refs[0], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[1], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_ref3_cond(const struct kefir_opt_code_container *code,
                                               const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                               kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                               void *payload) {
    UNUSED(code);
    UNUSED(resolve_phi);
    INPUT_CALLBACK(instr->operation.parameters.refs[0], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[1], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[2], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_ref4_compare(const struct kefir_opt_code_container *code,
                                                  const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                                  kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                                  void *payload) {
    UNUSED(code);
    UNUSED(resolve_phi);
    INPUT_CALLBACK(instr->operation.parameters.refs[0], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[1], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[2], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[3], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_compare_ref2(const struct kefir_opt_code_container *code,
                                                  const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                                  kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                                  void *payload) {
    UNUSED(code);
    UNUSED(resolve_phi);
    INPUT_CALLBACK(instr->operation.parameters.refs[0], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[1], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_overflow_arith(const struct kefir_opt_code_container *code,
                                                    const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                                    kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                                    void *payload) {
    UNUSED(code);
    UNUSED(resolve_phi);
    INPUT_CALLBACK(instr->operation.parameters.refs[0], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[1], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[2], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_ref1(const struct kefir_opt_code_container *code,
                                          const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                          kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                          void *payload) {
    UNUSED(code);
    UNUSED(resolve_phi);
    INPUT_CALLBACK(instr->operation.parameters.refs[0], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_ref2(const struct kefir_opt_code_container *code,
                                          const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                          kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                          void *payload) {
    UNUSED(code);
    UNUSED(resolve_phi);
    INPUT_CALLBACK(instr->operation.parameters.refs[0], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[1], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_ref_offset(const struct kefir_opt_code_container *code,
                                                const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                                kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                                void *payload) {
    UNUSED(code);
    UNUSED(resolve_phi);
    INPUT_CALLBACK(instr->operation.parameters.refs[0], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_atomic_op(const struct kefir_opt_code_container *code,
                                               const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                               kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                               void *payload) {
    UNUSED(code);
    UNUSED(resolve_phi);
    INPUT_CALLBACK(instr->operation.parameters.refs[0], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[1], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[2], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_variable(const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                              kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                              void *payload) {
    UNUSED(code);
    UNUSED(instr);
    UNUSED(resolve_phi);
    UNUSED(callback);
    UNUSED(payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_immediate(const struct kefir_opt_code_container *code,
                                               const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                               kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                               void *payload) {
    UNUSED(code);
    UNUSED(instr);
    UNUSED(resolve_phi);
    UNUSED(callback);
    UNUSED(payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_index(const struct kefir_opt_code_container *code,
                                           const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                           kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                           void *payload) {
    UNUSED(code);
    UNUSED(instr);
    UNUSED(resolve_phi);
    UNUSED(callback);
    UNUSED(payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_type(const struct kefir_opt_code_container *code,
                                          const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                          kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                          void *payload) {
    UNUSED(code);
    UNUSED(instr);
    UNUSED(resolve_phi);
    UNUSED(callback);
    UNUSED(payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_none(const struct kefir_opt_code_container *code,
                                          const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                          kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                          void *payload) {
    UNUSED(code);
    UNUSED(instr);
    UNUSED(resolve_phi);
    UNUSED(callback);
    UNUSED(payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_call_ref(const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                              kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                              void *payload) {
    UNUSED(resolve_phi);
    const struct kefir_opt_call_node *call = NULL;
    REQUIRE_OK(kefir_opt_code_container_call(code, instr->operation.parameters.function_call.call_ref, &call));
    INPUT_CALLBACK(instr->operation.parameters.function_call.indirect_ref, callback, payload);
    for (kefir_size_t i = 0; i < call->argument_count; i++) {
        INPUT_CALLBACK(call->arguments[i], callback, payload);
    }
    INPUT_CALLBACK(call->return_space, callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_phi_ref(const struct kefir_opt_code_container *code,
                                             const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                             kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                             void *payload) {
    if (!resolve_phi) {
        const struct kefir_opt_phi_node *phi_node;
        REQUIRE_OK(kefir_opt_code_container_phi(code, instr->operation.parameters.phi_ref, &phi_node));
        struct kefir_opt_phi_node_link_iterator iter;
        kefir_opt_block_id_t block_id;
        kefir_opt_instruction_ref_t instr_ref;
        kefir_result_t res;
        for (res = kefir_opt_phi_node_link_iter(phi_node, &iter, &block_id, &instr_ref); res == KEFIR_OK;
             res = kefir_opt_phi_node_link_next(&iter, &block_id, &instr_ref)) {
            INPUT_CALLBACK(instr_ref, callback, payload);
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_inline_asm(const struct kefir_opt_code_container *code,
                                                const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                                kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                                void *payload) {
    UNUSED(resolve_phi);
    const struct kefir_opt_inline_assembly_node *inline_asm = NULL;
    REQUIRE_OK(kefir_opt_code_container_inline_assembly(code, instr->operation.parameters.inline_asm_ref, &inline_asm));
    for (kefir_size_t i = 0; i < inline_asm->parameter_count; i++) {
        INPUT_CALLBACK(inline_asm->parameters[i].read_ref, callback, payload);
        INPUT_CALLBACK(inline_asm->parameters[i].load_store_ref, callback, payload);
    }
    return KEFIR_OK;
}

#undef INPUT_CALLBACK

kefir_result_t kefir_opt_instruction_extract_inputs(const struct kefir_opt_code_container *code,
                                                    const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                                    kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                                    void *payload) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));
    REQUIRE(callback != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction input callback"));

    switch (instr->operation.opcode) {
#define OPCODE_DEF(_id, _symbolic, _class)                                                \
    case KEFIR_OPT_OPCODE_##_id:                                                          \
        REQUIRE_OK(extract_inputs_##_class(code, instr, resolve_phi, callback, payload)); \
        break;

        KEFIR_OPTIMIZER_OPCODE_DEFS(OPCODE_DEF, )
#undef OPCODE_DEF
    }

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_instruction_is_control_flow(const struct kefir_opt_code_container *code,
                                                          kefir_opt_instruction_ref_t instr_ref,
                                                          kefir_bool_t *result_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));

    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(code, instr->block_id, &block));

    *result_ptr = block->control_flow.head == instr_ref || instr->control_flow.prev != KEFIR_ID_NONE ||
                  instr->control_flow.next != KEFIR_ID_NONE;
    return KEFIR_OK;
}

static kefir_result_t copy_instruction_resolve_phi(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                   struct kefir_opt_code_debug_info *debug_info,
                                                   kefir_opt_instruction_ref_t instr_ref, kefir_opt_block_id_t block_id,
                                                   kefir_opt_instruction_ref_t *copied_instr_ref) {
    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_PHI) {
        REQUIRE_OK(kefir_opt_code_container_phi_link_for(code, instr->operation.parameters.phi_ref, block_id,
                                                         copied_instr_ref));
    } else {
        if (debug_info != NULL) {
            REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor_of(debug_info, instr_ref));
        }
        REQUIRE_OK(kefir_opt_code_container_copy_instruction(mem, code, block_id, instr_ref, copied_instr_ref));
        if (debug_info != NULL) {
            REQUIRE_OK(kefir_opt_code_debug_info_replace_local_variable(mem, debug_info, instr_ref, *copied_instr_ref));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_merge_into(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                               struct kefir_opt_code_debug_info *debug_info,
                                               kefir_opt_block_id_t target_block_id,
                                               kefir_opt_block_id_t source_block_id, kefir_bool_t merge_control_tail) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));

    const struct kefir_opt_code_block *source_block;
    REQUIRE_OK(kefir_opt_code_container_block(code, source_block_id, &source_block));

    kefir_result_t res;
    kefir_opt_instruction_ref_t instr_ref, replacement_ref, tail_control_ref;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(code, source_block, &tail_control_ref));
    for (res = kefir_opt_code_block_instr_control_head(code, source_block, &instr_ref);
         res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
         res = kefir_opt_instruction_next_control(code, instr_ref, &instr_ref)) {
        if (!merge_control_tail && instr_ref == tail_control_ref) {
            continue;
        }

        REQUIRE_OK(copy_instruction_resolve_phi(mem, code, debug_info, instr_ref, target_block_id, &replacement_ref));
        kefir_bool_t is_control_flow;
        REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(code, replacement_ref, &is_control_flow));
        if (!is_control_flow) {
            REQUIRE_OK(kefir_opt_code_container_add_control(code, target_block_id, replacement_ref));
        }
        REQUIRE_OK(kefir_opt_code_container_replace_references(mem, code, replacement_ref, instr_ref));
        REQUIRE_OK(kefir_opt_code_debug_info_replace_local_variable(mem, debug_info, instr_ref, replacement_ref));
    }
    REQUIRE_OK(res);

    for (res = kefir_opt_code_block_instr_head(code, source_block, &instr_ref);
         res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
         res = kefir_opt_instruction_next_sibling(code, instr_ref, &instr_ref)) {
        kefir_bool_t is_control_flow;
        REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(code, instr_ref, &is_control_flow));
        if (!is_control_flow) {
            REQUIRE_OK(
                copy_instruction_resolve_phi(mem, code, debug_info, instr_ref, target_block_id, &replacement_ref));
            REQUIRE_OK(kefir_opt_code_container_replace_references(mem, code, replacement_ref, instr_ref));
            REQUIRE_OK(kefir_opt_code_debug_info_replace_local_variable(mem, debug_info, instr_ref, replacement_ref));
        }
    }
    REQUIRE_OK(res);

    REQUIRE_OK(kefir_opt_code_container_drop_block(mem, code, source_block_id));

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_redirect_phi_links(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                       kefir_opt_block_id_t old_predecessor_block_id,
                                                       kefir_opt_block_id_t new_predecessor_block_id,
                                                       kefir_opt_block_id_t block_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));

    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(code, block_id, &block));

    kefir_opt_phi_id_t phi_ref;
    kefir_result_t res;
    for (res = kefir_opt_code_block_phi_head(code, block, &phi_ref); res == KEFIR_OK && phi_ref != KEFIR_ID_NONE;
         res = kefir_opt_phi_next_sibling(code, phi_ref, &phi_ref)) {
        kefir_opt_instruction_ref_t instr_ref;
        kefir_result_t res = kefir_opt_code_container_phi_link_for(code, phi_ref, old_predecessor_block_id, &instr_ref);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        REQUIRE_OK(kefir_opt_code_container_phi_drop_link(mem, code, phi_ref, old_predecessor_block_id));
        REQUIRE_OK(kefir_opt_code_container_phi_drop_link(mem, code, phi_ref, new_predecessor_block_id));
        REQUIRE_OK(kefir_opt_code_container_phi_attach(mem, code, phi_ref, new_predecessor_block_id, instr_ref));
    }
    REQUIRE_OK(res);

    return KEFIR_OK;
}

static kefir_result_t collect_instr_after_split(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                struct kefir_opt_code_structure *structure,
                                                kefir_opt_instruction_ref_t split_instr_ref,
                                                struct kefir_queue *queue) {
    const struct kefir_opt_instruction *split_instr, *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, split_instr_ref, &split_instr));

    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(code, split_instr->block_id, &block));

    kefir_result_t res;
    kefir_opt_instruction_ref_t instr_ref;
    for (res = kefir_opt_code_block_instr_control_head(code, block, &instr_ref);
         res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
         res = kefir_opt_instruction_next_control(code, instr_ref, &instr_ref)) {
        kefir_bool_t sequenced_before;
        REQUIRE_OK(kefir_opt_code_structure_is_sequenced_before(mem, structure, instr_ref, split_instr_ref,
                                                                &sequenced_before));
        if (sequenced_before || instr_ref == split_instr_ref) {
            continue;
        }

        REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
        if (instr->operation.opcode == KEFIR_OPT_OPCODE_PHI) {
            continue;
        }

        REQUIRE_OK(kefir_queue_push(mem, queue, (kefir_queue_entry_t) instr_ref));
    }
    REQUIRE_OK(res);

    for (res = kefir_opt_code_block_instr_head(code, block, &instr_ref); res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
         res = kefir_opt_instruction_next_sibling(code, instr_ref, &instr_ref)) {
        kefir_bool_t sequenced_before, is_control_flow;
        REQUIRE_OK(kefir_opt_code_structure_is_sequenced_before(mem, structure, instr_ref, split_instr_ref,
                                                                &sequenced_before));
        REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(code, instr_ref, &is_control_flow));
        if (sequenced_before || instr_ref == split_instr_ref || is_control_flow) {
            continue;
        }

        REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
        if (instr->operation.opcode == KEFIR_OPT_OPCODE_PHI) {
            continue;
        }

        REQUIRE_OK(kefir_queue_push(mem, queue, (kefir_queue_entry_t) instr_ref));
    }
    REQUIRE_OK(res);

    return KEFIR_OK;
}

static kefir_result_t split_block_after_impl(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                             struct kefir_opt_code_debug_info *debug_info,
                                             struct kefir_opt_code_structure *structure,
                                             kefir_opt_instruction_ref_t split_instr_ref,
                                             kefir_opt_block_id_t *split_block_id, struct kefir_queue *instr_queue) {
    const struct kefir_opt_instruction *split_instr, *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, split_instr_ref, &split_instr));

    kefir_opt_block_id_t new_block_id;
    REQUIRE_OK(kefir_opt_code_container_new_block(mem, code, false, &new_block_id));

    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(code, split_instr->block_id, &block));

    REQUIRE_OK(collect_instr_after_split(mem, code, structure, split_instr_ref, instr_queue));

    while (!kefir_queue_is_empty(instr_queue)) {
        kefir_queue_entry_t queue_entry;
        REQUIRE_OK(kefir_queue_pop_first(mem, instr_queue, &queue_entry));
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, queue_entry);
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));

        kefir_bool_t is_control_flow;
        REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(code, instr_ref, &is_control_flow));

        kefir_opt_instruction_ref_t replacement_ref;
        if (debug_info != NULL) {
            REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor_of(debug_info, instr_ref));
        }
        REQUIRE_OK(kefir_opt_code_container_copy_instruction(mem, code, new_block_id, instr_ref, &replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_replace_references(mem, code, replacement_ref, instr_ref));
        REQUIRE_OK(kefir_opt_code_debug_info_replace_local_variable(mem, debug_info, instr_ref, replacement_ref));
        if (is_control_flow) {
            REQUIRE_OK(kefir_opt_code_container_add_control(code, new_block_id, replacement_ref));
            REQUIRE_OK(kefir_opt_code_container_drop_control(code, instr_ref));
        }
        REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, code, instr_ref));
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&structure->blocks[block->id].successors); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_opt_code_block_redirect_phi_links(mem, code, block->id, new_block_id, successor_block_id));
    }

    REQUIRE_OK(kefir_opt_code_builder_finalize_jump(mem, code, block->id, new_block_id, NULL));
    REQUIRE_OK(kefir_opt_code_structure_free(mem, structure));
    REQUIRE_OK(kefir_opt_code_structure_init(structure));
    REQUIRE_OK(kefir_opt_code_structure_build(mem, structure, code));
    *split_block_id = new_block_id;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_split_block_after(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                struct kefir_opt_code_debug_info *debug_info,
                                                struct kefir_opt_code_structure *structure,
                                                kefir_opt_instruction_ref_t split_instr_ref,
                                                kefir_opt_block_id_t *split_block_id) {
    struct kefir_queue instr_queue;
    REQUIRE_OK(kefir_queue_init(&instr_queue));
    kefir_result_t res =
        split_block_after_impl(mem, code, debug_info, structure, split_instr_ref, split_block_id, &instr_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_queue_free(mem, &instr_queue);
        return res;
    });
    REQUIRE_OK(kefir_queue_free(mem, &instr_queue));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_instruction_is_side_effect_free(const struct kefir_opt_instruction *instr,
                                                         kefir_bool_t *result_ptr) {
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT_CONST:
        case KEFIR_OPT_OPCODE_UINT_CONST:
        case KEFIR_OPT_OPCODE_FLOAT32_CONST:
        case KEFIR_OPT_OPCODE_FLOAT64_CONST:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST:
        case KEFIR_OPT_OPCODE_STRING_REF:
        case KEFIR_OPT_OPCODE_BLOCK_LABEL:
        case KEFIR_OPT_OPCODE_INT_PLACEHOLDER:
        case KEFIR_OPT_OPCODE_FLOAT32_PLACEHOLDER:
        case KEFIR_OPT_OPCODE_FLOAT64_PLACEHOLDER:
        case KEFIR_OPT_OPCODE_INT8_TO_BOOL:
        case KEFIR_OPT_OPCODE_INT16_TO_BOOL:
        case KEFIR_OPT_OPCODE_INT32_TO_BOOL:
        case KEFIR_OPT_OPCODE_INT64_TO_BOOL:
        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS:
        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS:
        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS:
        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS:
        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS:
        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS:
        case KEFIR_OPT_OPCODE_SCALAR_COMPARE:
        case KEFIR_OPT_OPCODE_INT8_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT16_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT32_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT64_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT8_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT16_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT32_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT64_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT8_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT16_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT32_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT64_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT8_ADD:
        case KEFIR_OPT_OPCODE_INT16_ADD:
        case KEFIR_OPT_OPCODE_INT32_ADD:
        case KEFIR_OPT_OPCODE_INT64_ADD:
        case KEFIR_OPT_OPCODE_INT8_SUB:
        case KEFIR_OPT_OPCODE_INT16_SUB:
        case KEFIR_OPT_OPCODE_INT32_SUB:
        case KEFIR_OPT_OPCODE_INT64_SUB:
        case KEFIR_OPT_OPCODE_INT8_MUL:
        case KEFIR_OPT_OPCODE_INT16_MUL:
        case KEFIR_OPT_OPCODE_INT32_MUL:
        case KEFIR_OPT_OPCODE_INT64_MUL:
        case KEFIR_OPT_OPCODE_UINT8_MUL:
        case KEFIR_OPT_OPCODE_UINT16_MUL:
        case KEFIR_OPT_OPCODE_UINT32_MUL:
        case KEFIR_OPT_OPCODE_UINT64_MUL:
        case KEFIR_OPT_OPCODE_INT8_AND:
        case KEFIR_OPT_OPCODE_INT16_AND:
        case KEFIR_OPT_OPCODE_INT32_AND:
        case KEFIR_OPT_OPCODE_INT64_AND:
        case KEFIR_OPT_OPCODE_INT8_OR:
        case KEFIR_OPT_OPCODE_INT16_OR:
        case KEFIR_OPT_OPCODE_INT32_OR:
        case KEFIR_OPT_OPCODE_INT64_OR:
        case KEFIR_OPT_OPCODE_INT8_XOR:
        case KEFIR_OPT_OPCODE_INT16_XOR:
        case KEFIR_OPT_OPCODE_INT32_XOR:
        case KEFIR_OPT_OPCODE_INT64_XOR:
        case KEFIR_OPT_OPCODE_INT8_LSHIFT:
        case KEFIR_OPT_OPCODE_INT16_LSHIFT:
        case KEFIR_OPT_OPCODE_INT32_LSHIFT:
        case KEFIR_OPT_OPCODE_INT64_LSHIFT:
        case KEFIR_OPT_OPCODE_INT8_RSHIFT:
        case KEFIR_OPT_OPCODE_INT16_RSHIFT:
        case KEFIR_OPT_OPCODE_INT32_RSHIFT:
        case KEFIR_OPT_OPCODE_INT64_RSHIFT:
        case KEFIR_OPT_OPCODE_INT8_ARSHIFT:
        case KEFIR_OPT_OPCODE_INT16_ARSHIFT:
        case KEFIR_OPT_OPCODE_INT32_ARSHIFT:
        case KEFIR_OPT_OPCODE_INT64_ARSHIFT:
        case KEFIR_OPT_OPCODE_INT8_NOT:
        case KEFIR_OPT_OPCODE_INT16_NOT:
        case KEFIR_OPT_OPCODE_INT32_NOT:
        case KEFIR_OPT_OPCODE_INT64_NOT:
        case KEFIR_OPT_OPCODE_INT8_NEG:
        case KEFIR_OPT_OPCODE_INT16_NEG:
        case KEFIR_OPT_OPCODE_INT32_NEG:
        case KEFIR_OPT_OPCODE_INT64_NEG:
            *result_ptr = true;
            break;

        default:
            *result_ptr = false;
            break;
    }

    return KEFIR_OK;
}

kefir_result_t kefir_opt_instruction_get_sole_use(const struct kefir_opt_code_container *code,
                                                  kefir_opt_instruction_ref_t instr_ref,
                                                  kefir_opt_instruction_ref_t *use_instr_ref_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(use_instr_ref_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction reference"));

    *use_instr_ref_ptr = KEFIR_ID_NONE;

    struct kefir_opt_instruction_use_iterator use_iter;
    kefir_result_t res = kefir_opt_code_container_instruction_use_instr_iter(code, instr_ref, &use_iter);
    REQUIRE(res != KEFIR_ITERATOR_END, KEFIR_OK);
    REQUIRE_OK(res);
    const kefir_opt_instruction_ref_t use_instr_ref = use_iter.use_instr_ref;
    res = kefir_opt_code_container_instruction_use_next(&use_iter);
    REQUIRE(res == KEFIR_ITERATOR_END, res);

    *use_instr_ref_ptr = use_instr_ref;

    return KEFIR_OK;
}

static kefir_result_t kefir_opt_move_instruction(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                 struct kefir_opt_code_debug_info *debug_info,
                                                 kefir_opt_instruction_ref_t instr_ref,
                                                 kefir_opt_block_id_t target_block_id,
                                                 kefir_opt_instruction_ref_t *moved_instr_ref_ptr) {
    kefir_bool_t is_control_flow;
    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(code, instr_ref, &is_control_flow));
    REQUIRE(!is_control_flow,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to move instruction which is a part of block control flow"));

    kefir_opt_instruction_ref_t moved_instr_ref;
    REQUIRE_OK(kefir_opt_code_container_copy_instruction(mem, code, target_block_id, instr_ref, &moved_instr_ref));
    REQUIRE_OK(kefir_opt_code_container_replace_references(mem, code, moved_instr_ref, instr_ref));
    REQUIRE_OK(kefir_opt_code_debug_info_replace_local_variable(mem, debug_info, instr_ref, moved_instr_ref));
    REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, code, instr_ref));
    ASSIGN_PTR(moved_instr_ref_ptr, moved_instr_ref);

    return KEFIR_OK;
}

struct move_instrs_param {
    struct kefir_mem *mem;
    struct kefir_opt_code_container *code;
    struct kefir_opt_code_debug_info *debug;
    kefir_opt_block_id_t source_block_id;
    kefir_opt_block_id_t target_block_id;
    struct kefir_list move_queue;
    kefir_opt_instruction_ref_t moved_instr_ref;
};

static kefir_result_t do_move_scan_deps(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct move_instrs_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid instruction move parameter"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(param->code, instr_ref, &instr));
    REQUIRE(instr->block_id == param->source_block_id, KEFIR_OK);
    REQUIRE_OK(kefir_list_insert_after(param->mem, &param->move_queue, kefir_list_tail(&param->move_queue),
                                       (void *) (kefir_uptr_t) instr_ref));
    return KEFIR_OK;
}

static kefir_result_t do_move_with_deps(struct move_instrs_param *param) {
    for (struct kefir_list_entry *iter = kefir_list_head(&param->move_queue); iter != NULL;
         iter = kefir_list_head(&param->move_queue)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(param->mem, &param->move_queue, iter));

        const struct kefir_opt_instruction *instr;
        kefir_result_t res = kefir_opt_code_container_instr(param->code, instr_ref, &instr);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        REQUIRE_OK(kefir_opt_instruction_extract_inputs(param->code, instr, true, do_move_scan_deps, param));

        if (instr->block_id == param->source_block_id) {
            kefir_opt_instruction_ref_t moved_instr_ref = KEFIR_ID_NONE;
            REQUIRE_OK(kefir_opt_move_instruction(param->mem, param->code, param->debug, instr_ref,
                                                  param->target_block_id, &moved_instr_ref));
            if (param->moved_instr_ref == KEFIR_ID_NONE) {
                param->moved_instr_ref = moved_instr_ref;
            }
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_move_instruction_with_local_dependencies(struct kefir_mem *mem,
                                                                  struct kefir_opt_code_container *code,
                                                                  struct kefir_opt_code_debug_info *debug,
                                                                  kefir_opt_instruction_ref_t instr_ref,
                                                                  kefir_opt_block_id_t target_block_id,
                                                                  kefir_opt_instruction_ref_t *moved_instr_ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(debug != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code debug information"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));

    struct move_instrs_param param = {.mem = mem,
                                      .code = code,
                                      .debug = debug,
                                      .source_block_id = instr->block_id,
                                      .target_block_id = target_block_id,
                                      .moved_instr_ref = KEFIR_ID_NONE};
    REQUIRE_OK(kefir_list_init(&param.move_queue));

    kefir_result_t res = kefir_list_insert_after(mem, &param.move_queue, kefir_list_tail(&param.move_queue),
                                                 (void *) (kefir_uptr_t) instr_ref);
    REQUIRE_CHAIN(&res, do_move_with_deps(&param));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &param.move_queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &param.move_queue));

    ASSIGN_PTR(moved_instr_ref_ptr, param.moved_instr_ref);
    return KEFIR_OK;
}

static kefir_result_t can_move_isolated_instruction(const struct kefir_opt_code_structure *structure,
                                                    kefir_opt_instruction_ref_t instr_ref,
                                                    kefir_opt_block_id_t target_block_id,
                                                    const struct kefir_hashtreeset *moved_instr,
                                                    const struct kefir_opt_can_move_instruction_ignore_use *ignore_use,
                                                    kefir_bool_t *can_move_ptr) {
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));
    REQUIRE(can_move_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    *can_move_ptr = false;

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(structure->code, instr_ref, &instr));
    REQUIRE(instr->block_id != target_block_id, KEFIR_OK);

    kefir_bool_t is_control_flow;
    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(structure->code, instr_ref, &is_control_flow));
    REQUIRE(!is_control_flow, KEFIR_OK);

    kefir_bool_t is_side_effect_free;
    REQUIRE_OK(kefir_opt_instruction_is_side_effect_free(instr, &is_side_effect_free));
    REQUIRE(is_side_effect_free, KEFIR_OK);

    struct kefir_opt_instruction_use_iterator use_iter;
    kefir_result_t res;
    for (res = kefir_opt_code_container_instruction_use_instr_iter(structure->code, instr_ref, &use_iter);
         res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        if (kefir_hashtreeset_has(moved_instr, (kefir_hashtreeset_entry_t) use_iter.use_instr_ref)) {
            continue;
        }
        if (ignore_use != NULL && ignore_use->callback != NULL) {
            kefir_bool_t ignore_use_flag = false;
            REQUIRE_OK(ignore_use->callback(instr_ref, use_iter.use_instr_ref, &ignore_use_flag, ignore_use->payload));
            if (ignore_use_flag) {
                continue;
            }
        }
        const struct kefir_opt_instruction *use_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(structure->code, use_iter.use_instr_ref, &use_instr));
        REQUIRE(use_instr->block_id != target_block_id || use_instr->operation.opcode != KEFIR_OPT_OPCODE_PHI,
                KEFIR_OK);
        kefir_bool_t is_dominator;
        REQUIRE_OK(
            kefir_opt_code_structure_is_dominator(structure, use_instr->block_id, target_block_id, &is_dominator));
        REQUIRE(is_dominator, KEFIR_OK);
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    *can_move_ptr = true;
    return KEFIR_OK;
}

struct can_move_param {
    struct kefir_mem *mem;
    const struct kefir_opt_code_structure *structure;
    kefir_opt_block_id_t source_block_id;
    kefir_opt_block_id_t target_block_id;
    struct kefir_hashtreeset moved_instr;
    const struct kefir_opt_can_move_instruction_ignore_use *ignore_use;
    kefir_bool_t can_move;
};

static kefir_result_t can_move_instr(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct can_move_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid instruction move parameter"));
    REQUIRE(param->can_move, KEFIR_OK);

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(param->structure->code, instr_ref, &instr));
    if (instr->block_id != param->source_block_id) {
        kefir_bool_t is_dominator;
        REQUIRE_OK(kefir_opt_code_structure_is_dominator(param->structure, param->target_block_id, instr->block_id,
                                                         &is_dominator));
        if (!is_dominator) {
            param->can_move = false;
        }
        return KEFIR_OK;
    }

    kefir_bool_t can_move_isolated = false;
    REQUIRE_OK(can_move_isolated_instruction(param->structure, instr_ref, param->target_block_id, &param->moved_instr,
                                             param->ignore_use, &can_move_isolated));
    if (!can_move_isolated) {
        param->can_move = false;
        return KEFIR_OK;
    }

    REQUIRE_OK(kefir_hashtreeset_add(param->mem, &param->moved_instr, (kefir_hashtreeset_entry_t) instr_ref));
    REQUIRE_OK(kefir_opt_instruction_extract_inputs(param->structure->code, instr, true, can_move_instr, payload));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_can_move_instruction_with_local_dependencies(
    struct kefir_mem *mem, const struct kefir_opt_code_structure *structure, kefir_opt_instruction_ref_t instr_ref,
    kefir_opt_block_id_t target_block_id, const struct kefir_opt_can_move_instruction_ignore_use *ignore_use,
    kefir_bool_t *can_move_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));
    REQUIRE(can_move_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(structure->code, instr_ref, &instr));

    struct can_move_param param = {.mem = mem,
                                   .structure = structure,
                                   .source_block_id = instr->block_id,
                                   .target_block_id = target_block_id,
                                   .ignore_use = ignore_use,
                                   .can_move = true};
    REQUIRE_OK(kefir_hashtreeset_init(&param.moved_instr, &kefir_hashtree_uint_ops));
    kefir_result_t res = can_move_instr(instr_ref, &param);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &param.moved_instr);
        return KEFIR_OK;
    });
    REQUIRE_OK(kefir_hashtreeset_free(mem, &param.moved_instr));

    *can_move_ptr = param.can_move;

    return KEFIR_OK;
}
