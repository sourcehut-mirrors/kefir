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

#include "kefir/optimizer/format.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t id_format(struct kefir_json_output *json, kefir_id_t id) {
    if (id == KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_json_output_null(json));
    } else {
        REQUIRE_OK(kefir_json_output_uinteger(json, id));
    }
    return KEFIR_OK;
}

static kefir_result_t format_operation_jump(struct kefir_json_output *json, const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "target_block"));
    REQUIRE_OK(id_format(json, oper->parameters.branch.target_block));
    return KEFIR_OK;
}

static kefir_result_t format_operation_branch(struct kefir_json_output *json, const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "target_block"));
    REQUIRE_OK(id_format(json, oper->parameters.branch.target_block));
    REQUIRE_OK(kefir_json_output_object_key(json, "alternative_block"));
    REQUIRE_OK(id_format(json, oper->parameters.branch.alternative_block));
    REQUIRE_OK(kefir_json_output_object_key(json, "condition"));
    REQUIRE_OK(id_format(json, oper->parameters.branch.condition_ref));
    return KEFIR_OK;
}

static kefir_result_t format_operation_ref1(struct kefir_json_output *json, const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    return KEFIR_OK;
}

static kefir_result_t format_operation_ref2(struct kefir_json_output *json, const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "args"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_ref3(struct kefir_json_output *json, const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "args"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
    REQUIRE_OK(id_format(json, oper->parameters.refs[2]));
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_load_mem(struct kefir_json_output *json,
                                                const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "location"));
    REQUIRE_OK(id_format(json, oper->parameters.memory_access.location));
    REQUIRE_OK(kefir_json_output_object_key(json, "flags"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "volatile"));
    REQUIRE_OK(kefir_json_output_boolean(json, oper->parameters.memory_access.flags.volatile_access));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_store_mem(struct kefir_json_output *json,
                                                 const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "location"));
    REQUIRE_OK(id_format(json, oper->parameters.memory_access.location));
    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
    REQUIRE_OK(id_format(json, oper->parameters.memory_access.value));
    REQUIRE_OK(kefir_json_output_object_key(json, "flags"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "volatile"));
    REQUIRE_OK(kefir_json_output_boolean(json, oper->parameters.memory_access.flags.volatile_access));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_mem_op(struct kefir_json_output *json, const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "target"));
    REQUIRE_OK(id_format(json, oper->parameters.memory_operation.target));
    REQUIRE_OK(kefir_json_output_object_key(json, "source"));
    REQUIRE_OK(id_format(json, oper->parameters.memory_operation.source));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(id_format(json, oper->parameters.memory_operation.type_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.memory_operation.type_index));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_bitfield(struct kefir_json_output *json,
                                                const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "base"));
    REQUIRE_OK(id_format(json, oper->parameters.bitfield.base_ref));
    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
    REQUIRE_OK(id_format(json, oper->parameters.bitfield.value_ref));
    REQUIRE_OK(kefir_json_output_object_key(json, "offset"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.bitfield.offset));
    REQUIRE_OK(kefir_json_output_object_key(json, "length"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.bitfield.length));
    return KEFIR_OK;
}

static kefir_result_t format_operation_typed_ref1(struct kefir_json_output *json,
                                                  const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "ref"));
    REQUIRE_OK(id_format(json, oper->parameters.typed_ref.ref));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(id_format(json, oper->parameters.typed_ref.type_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.typed_ref.type_index));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_stack_alloc(struct kefir_json_output *json,
                                                   const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "size_ref"));
    REQUIRE_OK(id_format(json, oper->parameters.stack_allocation.size_ref));
    REQUIRE_OK(kefir_json_output_object_key(json, "alignment_ref"));
    REQUIRE_OK(id_format(json, oper->parameters.stack_allocation.alignment_ref));
    REQUIRE_OK(kefir_json_output_object_key(json, "within_scope"));
    REQUIRE_OK(kefir_json_output_boolean(json, oper->parameters.stack_allocation.within_scope));
    return KEFIR_OK;
}

static kefir_result_t format_operation_call_ref(struct kefir_json_output *json,
                                                const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "call_ref"));
    REQUIRE_OK(id_format(json, oper->parameters.function_call.call_ref));
    REQUIRE_OK(kefir_json_output_object_key(json, "indirect_ref"));
    REQUIRE_OK(id_format(json, oper->parameters.function_call.indirect_ref));
    return KEFIR_OK;
}

static kefir_result_t format_operation_ir_ref(struct kefir_json_output *json, const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "ir_ref"));
    REQUIRE_OK(id_format(json, oper->parameters.ir_ref));
    return KEFIR_OK;
}

static kefir_result_t format_operation_block_ref(struct kefir_json_output *json,
                                                 const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "block_ref"));
    REQUIRE_OK(id_format(json, oper->parameters.block_ref));
    return KEFIR_OK;
}

static kefir_result_t format_operation_imm_int(struct kefir_json_output *json, const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
    REQUIRE_OK(kefir_json_output_integer(json, oper->parameters.imm.integer));
    return KEFIR_OK;
}

static kefir_result_t format_operation_imm_uint(struct kefir_json_output *json,
                                                const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
    REQUIRE_OK(kefir_json_output_integer(json, oper->parameters.imm.integer));
    return KEFIR_OK;
}

static kefir_result_t format_operation_imm_float32(struct kefir_json_output *json,
                                                   const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
    REQUIRE_OK(kefir_json_output_float(json, oper->parameters.imm.float32));
    return KEFIR_OK;
}

static kefir_result_t format_operation_imm_float64(struct kefir_json_output *json,
                                                   const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
    REQUIRE_OK(kefir_json_output_float(json, oper->parameters.imm.float64));
    return KEFIR_OK;
}

static kefir_result_t format_operation_imm_long_double(struct kefir_json_output *json,
                                                       const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
    REQUIRE_OK(kefir_json_output_long_double(json, oper->parameters.imm.long_double.value));
    REQUIRE_OK(kefir_json_output_object_key(json, "base_ref"));
    REQUIRE_OK(id_format(json, oper->parameters.imm.long_double.base_ref));
    return KEFIR_OK;
}

static kefir_result_t format_operation_none(struct kefir_json_output *json, const struct kefir_opt_operation *oper) {
    UNUSED(json);
    UNUSED(oper);
    return KEFIR_OK;
}

static kefir_result_t instr_format(struct kefir_json_output *json, const struct kefir_opt_instruction *instr) {
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(kefir_json_output_uinteger(json, instr->id));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));

    switch (instr->operation.opcode) {
#define OPCODE(_id, _name, _class)                                      \
    case KEFIR_OPT_OPCODE_##_id:                                        \
        REQUIRE_OK(kefir_json_output_string(json, (_name)));            \
        REQUIRE_OK(format_operation_##_class(json, &instr->operation)); \
        break;

        KEFIR_OPTIMIZER_OPCODE_DEFS(OPCODE, )
#undef OPCODE
    }

    REQUIRE_OK(kefir_json_output_object_key(json, "control_next"));
    REQUIRE_OK(id_format(json, instr->control_flow.next));

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t phi_format(struct kefir_json_output *json, const struct kefir_opt_phi_node *phi) {
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(kefir_json_output_uinteger(json, phi->node_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "links"));
    REQUIRE_OK(kefir_json_output_array_begin(json));

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&phi->links, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, node->key);
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, node->value);
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "block_id"));
        REQUIRE_OK(kefir_json_output_uinteger(json, block_id));
        REQUIRE_OK(kefir_json_output_object_key(json, "instr_ref"));
        REQUIRE_OK(kefir_json_output_uinteger(json, instr_ref));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }

    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t call_format(struct kefir_json_output *json, const struct kefir_opt_call_node *call) {
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(kefir_json_output_uinteger(json, call->node_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "function_declaration_id"));
    REQUIRE_OK(kefir_json_output_uinteger(json, call->function_declaration_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "arguments"));
    REQUIRE_OK(kefir_json_output_array_begin(json));

    for (kefir_size_t i = 0; i < call->argument_count; i++) {
        REQUIRE_OK(id_format(json, call->arguments[i]));
    }

    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t code_block_format(struct kefir_json_output *json, const struct kefir_opt_code_container *code,
                                        const struct kefir_opt_code_block *block) {
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(kefir_json_output_uinteger(json, block->id));
    REQUIRE_OK(kefir_json_output_object_key(json, "control_flow"));
    REQUIRE_OK(id_format(json, block->control_flow.head));

    REQUIRE_OK(kefir_json_output_object_key(json, "phi"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    kefir_result_t res;
    const struct kefir_opt_phi_node *phi = NULL;
    for (res = kefir_opt_code_block_phi_head(code, block, &phi); res == KEFIR_OK && phi != NULL;
         res = kefir_opt_phi_next_sibling(code, phi, &phi)) {

        REQUIRE_OK(phi_format(json, phi));
    }
    REQUIRE_OK(res);
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "calls"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    const struct kefir_opt_call_node *call = NULL;
    for (res = kefir_opt_code_block_call_head(code, block, &call); res == KEFIR_OK && call != NULL;
         res = kefir_opt_call_next_sibling(code, call, &call)) {

        REQUIRE_OK(call_format(json, call));
    }
    REQUIRE_OK(res);
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "code"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    const struct kefir_opt_instruction *instr = NULL;
    for (res = kefir_opt_code_block_instr_head(code, block, &instr); res == KEFIR_OK && instr != NULL;
         res = kefir_opt_instruction_next_sibling(code, instr, &instr)) {

        REQUIRE_OK(instr_format(json, instr));
    }
    REQUIRE_OK(res);

    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_format(struct kefir_json_output *json, const struct kefir_opt_code_container *code) {
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid json output"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_json_output_object_begin(json));

    struct kefir_opt_code_container_iterator iter;
    REQUIRE_OK(kefir_json_output_object_key(json, "entry_point"));
    REQUIRE_OK(id_format(json, code->entry_point));
    REQUIRE_OK(kefir_json_output_object_key(json, "blocks"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_opt_code_block *block = kefir_opt_code_container_iter(code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {
        REQUIRE_OK(code_block_format(json, code, block));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_function(struct kefir_json_output *json, const struct kefir_opt_function *function) {
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(kefir_json_output_uinteger(json, function->ir_func->declaration->id));
    REQUIRE_OK(kefir_json_output_object_key(json, "name"));
    if (function->ir_func->declaration->name != NULL) {
        REQUIRE_OK(kefir_json_output_string(json, function->ir_func->declaration->name));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "code"));
    REQUIRE_OK(kefir_opt_code_format(json, &function->code));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_format(struct kefir_json_output *json, const struct kefir_opt_module *module) {
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid json output"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));

    REQUIRE_OK(kefir_json_output_object_begin(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "functions"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->functions, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(const struct kefir_opt_function *, function, node->value);
        REQUIRE_OK(format_function(json, function));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}
