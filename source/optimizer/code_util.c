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
                                               const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                               kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                               void *payload) {
    UNUSED(code);
    UNUSED(block_local);
    INPUT_CALLBACK(instr->operation.parameters.memory_access.location, callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.memory_access.value, callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_load_mem(const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                              kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                              void *payload) {
    UNUSED(code);
    UNUSED(block_local);
    INPUT_CALLBACK(instr->operation.parameters.memory_access.location, callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_stack_alloc(const struct kefir_opt_code_container *code,
                                                 const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                                 kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                                 void *payload) {
    UNUSED(code);
    UNUSED(block_local);
    INPUT_CALLBACK(instr->operation.parameters.stack_allocation.alignment_ref, callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.stack_allocation.size_ref, callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_bitfield(const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                              kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                              void *payload) {
    UNUSED(code);
    UNUSED(block_local);
    INPUT_CALLBACK(instr->operation.parameters.bitfield.base_ref, callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.bitfield.value_ref, callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_branch(const struct kefir_opt_code_container *code,
                                            const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                            kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                            void *payload) {
    UNUSED(code);
    UNUSED(block_local);
    INPUT_CALLBACK(instr->operation.parameters.branch.condition_ref, callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_typed_ref1(const struct kefir_opt_code_container *code,
                                                const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                                kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                                void *payload) {
    UNUSED(code);
    UNUSED(block_local);
    INPUT_CALLBACK(instr->operation.parameters.typed_refs.ref[0], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_typed_ref2(const struct kefir_opt_code_container *code,
                                                const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                                kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                                void *payload) {
    UNUSED(code);
    UNUSED(block_local);
    INPUT_CALLBACK(instr->operation.parameters.typed_refs.ref[0], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.typed_refs.ref[1], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_ref1(const struct kefir_opt_code_container *code,
                                          const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                          kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                          void *payload) {
    UNUSED(code);
    UNUSED(block_local);
    INPUT_CALLBACK(instr->operation.parameters.refs[0], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_ref1_imm(const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                              kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                              void *payload) {
    UNUSED(code);
    UNUSED(block_local);
    INPUT_CALLBACK(instr->operation.parameters.ref_imm.refs[0], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_ref2(const struct kefir_opt_code_container *code,
                                          const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                          kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                          void *payload) {
    UNUSED(code);
    UNUSED(block_local);
    INPUT_CALLBACK(instr->operation.parameters.refs[0], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.refs[1], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_atomic_op(const struct kefir_opt_code_container *code,
                                               const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                               kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                               void *payload) {
    UNUSED(code);
    UNUSED(block_local);
    INPUT_CALLBACK(instr->operation.parameters.atomic_op.ref[0], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.atomic_op.ref[1], callback, payload);
    INPUT_CALLBACK(instr->operation.parameters.atomic_op.ref[2], callback, payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_variable(const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                              kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                              void *payload) {
    UNUSED(code);
    UNUSED(instr);
    UNUSED(block_local);
    UNUSED(callback);
    UNUSED(payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_immediate(const struct kefir_opt_code_container *code,
                                               const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                               kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                               void *payload) {
    UNUSED(code);
    UNUSED(instr);
    UNUSED(block_local);
    UNUSED(callback);
    UNUSED(payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_index(const struct kefir_opt_code_container *code,
                                           const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                           kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                           void *payload) {
    UNUSED(code);
    UNUSED(instr);
    UNUSED(block_local);
    UNUSED(callback);
    UNUSED(payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_none(const struct kefir_opt_code_container *code,
                                          const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                          kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                          void *payload) {
    UNUSED(code);
    UNUSED(instr);
    UNUSED(block_local);
    UNUSED(callback);
    UNUSED(payload);
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_call_ref(const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                              kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                              void *payload) {
    UNUSED(block_local);
    struct kefir_opt_call_node *call = NULL;
    REQUIRE_OK(kefir_opt_code_container_call(code, instr->operation.parameters.function_call.call_ref, &call));
    INPUT_CALLBACK(instr->operation.parameters.function_call.indirect_ref, callback, payload);
    for (kefir_size_t i = 0; i < call->argument_count; i++) {
        INPUT_CALLBACK(call->arguments[i], callback, payload);
    }
    return KEFIR_OK;
}

static kefir_result_t extract_inputs_phi_ref(const struct kefir_opt_code_container *code,
                                             const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                             kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                             void *payload) {
    if (!block_local) {
        struct kefir_opt_phi_node *phi_node;
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
                                                const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                                kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                                void *payload) {
    UNUSED(block_local);
    struct kefir_opt_inline_assembly_node *inline_asm = NULL;
    REQUIRE_OK(kefir_opt_code_container_inline_assembly(code, instr->operation.parameters.inline_asm_ref, &inline_asm));
    for (kefir_size_t i = 0; i < inline_asm->parameter_count; i++) {
        INPUT_CALLBACK(inline_asm->parameters[i].read_ref, callback, payload);
        INPUT_CALLBACK(inline_asm->parameters[i].load_store_ref, callback, payload);
    }
    return KEFIR_OK;
}

#undef INPUT_CALLBACK

kefir_result_t kefir_opt_instruction_extract_inputs(const struct kefir_opt_code_container *code,
                                                    const struct kefir_opt_instruction *instr, kefir_bool_t block_local,
                                                    kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
                                                    void *payload) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));
    REQUIRE(callback != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction input callback"));

    switch (instr->operation.opcode) {
#define OPCODE_DEF(_id, _symbolic, _class)                                                \
    case KEFIR_OPT_OPCODE_##_id:                                                          \
        REQUIRE_OK(extract_inputs_##_class(code, instr, block_local, callback, payload)); \
        break;

        KEFIR_OPTIMIZER_OPCODE_DEFS(OPCODE_DEF, )
#undef OPCODE_DEF
    }

    return KEFIR_OK;
}
