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
#include "kefir/core/error.h"
#include "kefir/core/util.h"

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

static kefir_result_t extract_inputs_typed_ref1(const struct kefir_opt_code_container *code,
                                                const struct kefir_opt_instruction *instr, kefir_bool_t resolve_phi,
                                                kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                                void *payload) {
    UNUSED(code);
    UNUSED(resolve_phi);
    INPUT_CALLBACK(instr->operation.parameters.refs[0], callback, payload);
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
                                                   kefir_opt_instruction_ref_t instr_ref, kefir_opt_block_id_t block_id,
                                                   kefir_opt_instruction_ref_t *copied_instr_ref) {
    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_PHI) {
        REQUIRE_OK(kefir_opt_code_container_phi_link_for(code, instr->operation.parameters.phi_ref, block_id,
                                                         copied_instr_ref));
    } else {
        REQUIRE_OK(kefir_opt_code_container_copy_instruction(mem, code, block_id, instr_ref, copied_instr_ref));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_merge_into(struct kefir_mem *mem, struct kefir_opt_code_container *code,
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

        REQUIRE_OK(copy_instruction_resolve_phi(mem, code, instr_ref, target_block_id, &replacement_ref));
        kefir_bool_t is_control_flow;
        REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(code, replacement_ref, &is_control_flow));
        if (!is_control_flow) {
            REQUIRE_OK(kefir_opt_code_container_add_control(code, target_block_id, replacement_ref));
        }
        REQUIRE_OK(kefir_opt_code_container_replace_references(mem, code, replacement_ref, instr_ref));
    }
    REQUIRE_OK(res);

    for (res = kefir_opt_code_block_instr_head(code, source_block, &instr_ref);
         res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
         res = kefir_opt_instruction_next_sibling(code, instr_ref, &instr_ref)) {
        kefir_bool_t is_control_flow;
        REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(code, instr_ref, &is_control_flow));
        if (!is_control_flow) {
            REQUIRE_OK(copy_instruction_resolve_phi(mem, code, instr_ref, target_block_id, &replacement_ref));
            REQUIRE_OK(kefir_opt_code_container_replace_references(mem, code, replacement_ref, instr_ref));
        }
    }
    REQUIRE_OK(res);

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
        REQUIRE_OK(kefir_opt_code_container_phi_link_for(code, phi_ref, old_predecessor_block_id, &instr_ref));
        REQUIRE_OK(kefir_opt_code_container_phi_attach(mem, code, phi_ref, new_predecessor_block_id, instr_ref));
    }
    REQUIRE_OK(res);

    return KEFIR_OK;
}
