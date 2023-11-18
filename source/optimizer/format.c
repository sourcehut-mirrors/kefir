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

static kefir_result_t format_operation_branch(struct kefir_json_output *json, const struct kefir_opt_operation *oper) {
    switch (oper->opcode) {
        case KEFIR_OPT_OPCODE_JUMP:
            REQUIRE_OK(kefir_json_output_object_key(json, "target_block"));
            REQUIRE_OK(id_format(json, oper->parameters.branch.target_block));
            break;

        case KEFIR_OPT_OPCODE_BRANCH:
            REQUIRE_OK(kefir_json_output_object_key(json, "target_block"));
            REQUIRE_OK(id_format(json, oper->parameters.branch.target_block));
            REQUIRE_OK(kefir_json_output_object_key(json, "alternative_block"));
            REQUIRE_OK(id_format(json, oper->parameters.branch.alternative_block));
            REQUIRE_OK(kefir_json_output_object_key(json, "condition"));
            REQUIRE_OK(id_format(json, oper->parameters.branch.condition_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer operation code");
    }
    return KEFIR_OK;
}

static kefir_result_t format_operation_cmp_branch(struct kefir_json_output *json,
                                                  const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "target_block"));
    REQUIRE_OK(id_format(json, oper->parameters.branch.target_block));
    REQUIRE_OK(kefir_json_output_object_key(json, "alternative_block"));
    REQUIRE_OK(id_format(json, oper->parameters.branch.alternative_block));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    kefir_bool_t both_refs = true;
    switch (oper->parameters.branch.comparison.type) {
        case KEFIR_OPT_COMPARE_BRANCH_INT_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int_equals"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_EQUALS_CONST:
            REQUIRE_OK(kefir_json_output_string(json, "int_equals_const"));
            both_refs = false;
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_NOT_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int_not_equals"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_NOT_EQUALS_CONST:
            REQUIRE_OK(kefir_json_output_string(json, "int_not_equals_const"));
            both_refs = false;
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_GREATER:
            REQUIRE_OK(kefir_json_output_string(json, "int_greater"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_GREATER_CONST:
            REQUIRE_OK(kefir_json_output_string(json, "int_greater_const"));
            both_refs = false;
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_GREATER_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int_greater_or_equals"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_GREATER_OR_EQUALS_CONST:
            REQUIRE_OK(kefir_json_output_string(json, "int_greater_or_equals_const"));
            both_refs = false;
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_LESS:
            REQUIRE_OK(kefir_json_output_string(json, "int_less"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_LESS_CONST:
            REQUIRE_OK(kefir_json_output_string(json, "int_less_const"));
            both_refs = false;
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_LESS_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int_less_or_equals"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_LESS_OR_EQUALS_CONST:
            REQUIRE_OK(kefir_json_output_string(json, "int_less_or_equals_const"));
            both_refs = false;
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE:
            REQUIRE_OK(kefir_json_output_string(json, "int_above"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE_CONST:
            REQUIRE_OK(kefir_json_output_string(json, "int_above_const"));
            both_refs = false;
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int_above_or_equals"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE_OR_EQUALS_CONST:
            REQUIRE_OK(kefir_json_output_string(json, "int_above_or_equals_const"));
            both_refs = false;
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_BELOW:
            REQUIRE_OK(kefir_json_output_string(json, "int_below"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_BELOW_CONST:
            REQUIRE_OK(kefir_json_output_string(json, "int_below_const"));
            both_refs = false;
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_BELOW_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int_below_or_equals"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_INT_BELOW_OR_EQUALS_CONST:
            REQUIRE_OK(kefir_json_output_string(json, "int_below_or_equals_const"));
            both_refs = false;
            break;

        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "float32_equals"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_NOT_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "float32_not_equals"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_GREATER:
            REQUIRE_OK(kefir_json_output_string(json, "float32_greater"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_GREATER_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "float32_greater_or_equals"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_LESS:
            REQUIRE_OK(kefir_json_output_string(json, "float32_less"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_LESS_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "float32_less_or_equals"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "float64_equals"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_NOT_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "float64_not_equals"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_GREATER:
            REQUIRE_OK(kefir_json_output_string(json, "float64_greater"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_GREATER_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "float64_greater_or_equals"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_LESS:
            REQUIRE_OK(kefir_json_output_string(json, "float64_less"));
            break;

        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_LESS_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "float64_less_or_equals"));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer branch comparison type");
    }

    REQUIRE_OK(kefir_json_output_object_key(json, "ref1"));
    REQUIRE_OK(id_format(json, oper->parameters.branch.comparison.refs[0]));
    if (both_refs) {
        REQUIRE_OK(kefir_json_output_object_key(json, "ref2"));
        REQUIRE_OK(id_format(json, oper->parameters.branch.comparison.refs[1]));
    } else {
        REQUIRE_OK(kefir_json_output_object_key(json, "imm"));
        REQUIRE_OK(kefir_json_output_integer(json, oper->parameters.branch.comparison.integer));
    }
    return KEFIR_OK;
}

static kefir_result_t format_operation_ref1(struct kefir_json_output *json, const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    return KEFIR_OK;
}

static kefir_result_t format_operation_ref1_imm(struct kefir_json_output *json,
                                                const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(id_format(json, oper->parameters.ref_imm.refs[0]));
    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
    switch (oper->opcode) {
        case KEFIR_OPT_OPCODE_INT_ADD_CONST:
        case KEFIR_OPT_OPCODE_INT_SUB_CONST:
        case KEFIR_OPT_OPCODE_INT_MUL_CONST:
        case KEFIR_OPT_OPCODE_INT_AND_CONST:
        case KEFIR_OPT_OPCODE_INT_OR_CONST:
        case KEFIR_OPT_OPCODE_INT_XOR_CONST:
        case KEFIR_OPT_OPCODE_INT_EQUALS_CONST:
            REQUIRE_OK(kefir_json_output_integer(json, oper->parameters.ref_imm.integer));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer operation code");
    }
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
    REQUIRE_OK(id_format(json, oper->parameters.typed_refs.ref[0]));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(id_format(json, oper->parameters.typed_refs.type_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.typed_refs.type_index));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_typed_ref2(struct kefir_json_output *json,
                                                  const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "target"));
    REQUIRE_OK(id_format(json, oper->parameters.typed_refs.ref[0]));
    REQUIRE_OK(kefir_json_output_object_key(json, "source"));
    REQUIRE_OK(id_format(json, oper->parameters.typed_refs.ref[1]));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(id_format(json, oper->parameters.typed_refs.type_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.typed_refs.type_index));
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

static kefir_result_t format_operation_index(struct kefir_json_output *json, const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.index));
    return KEFIR_OK;
}

static kefir_result_t format_operation_phi_ref(struct kefir_json_output *json, const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "phi_ref"));
    REQUIRE_OK(id_format(json, oper->parameters.phi_ref));
    return KEFIR_OK;
}

static kefir_result_t format_operation_inline_asm(struct kefir_json_output *json,
                                                  const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "inline_asm_ref"));
    REQUIRE_OK(id_format(json, oper->parameters.inline_asm_ref));
    return KEFIR_OK;
}

static kefir_result_t format_operation_immediate(struct kefir_json_output *json,
                                                 const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
    switch (oper->opcode) {
        case KEFIR_OPT_OPCODE_INT_CONST:
            REQUIRE_OK(kefir_json_output_integer(json, oper->parameters.imm.integer));
            break;

        case KEFIR_OPT_OPCODE_UINT_CONST:
            REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.imm.uinteger));
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_CONST:
            REQUIRE_OK(kefir_json_output_float(json, oper->parameters.imm.float32));
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_CONST:
            REQUIRE_OK(kefir_json_output_float(json, oper->parameters.imm.float64));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST:
            REQUIRE_OK(kefir_json_output_long_double(json, oper->parameters.imm.long_double.value));
            REQUIRE_OK(kefir_json_output_object_key(json, "storage"));
            REQUIRE_OK(id_format(json, oper->parameters.imm.long_double.storage));
            break;

        case KEFIR_OPT_OPCODE_STRING_REF:
            REQUIRE_OK(id_format(json, oper->parameters.imm.string_ref));
            break;

        case KEFIR_OPT_OPCODE_BLOCK_LABEL:
            REQUIRE_OK(id_format(json, oper->parameters.imm.block_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer operation code");
    }
    return KEFIR_OK;
}

static kefir_result_t format_operation_none(struct kefir_json_output *json, const struct kefir_opt_operation *oper) {
    UNUSED(json);
    UNUSED(oper);
    return KEFIR_OK;
}

static kefir_result_t instr_format(struct kefir_json_output *json, const struct kefir_opt_instruction *instr,
                                   const struct kefir_opt_code_analysis *code_analysis) {
    REQUIRE(code_analysis == NULL || code_analysis->instructions[instr->id].reachable, KEFIR_OK);

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

    REQUIRE_OK(kefir_json_output_object_key(json, "properties"));
    if (code_analysis != NULL) {
        const struct kefir_opt_code_analysis_instruction_properties *instr_props = NULL;
        REQUIRE_OK(kefir_opt_code_analysis_instruction_properties(code_analysis, instr->id, &instr_props));

        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "reachable"));
        REQUIRE_OK(kefir_json_output_boolean(json, instr_props->reachable));
        REQUIRE_OK(kefir_json_output_object_key(json, "linear_index"));
        if (instr_props->linear_position != KEFIR_OPT_CODE_ANALYSIS_LINEAR_INDEX_UNDEFINED) {
            REQUIRE_OK(kefir_json_output_uinteger(json, instr_props->linear_position));
        } else {
            REQUIRE_OK(kefir_json_output_null(json));
        }
        REQUIRE_OK(kefir_json_output_object_key(json, "linear_liveness"));
        if (instr_props->reachable) {
            const struct kefir_opt_instruction_liveness_interval *instr_liveness =
                &code_analysis->liveness.intervals[instr_props->linear_position];
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "alias"));
            REQUIRE_OK(id_format(json, instr_liveness->alias_ref));
            REQUIRE_OK(kefir_json_output_object_key(json, "begin"));
            REQUIRE_OK(kefir_json_output_uinteger(json, instr_liveness->range.begin));
            REQUIRE_OK(kefir_json_output_object_key(json, "end"));
            REQUIRE_OK(kefir_json_output_uinteger(json, instr_liveness->range.end));
            REQUIRE_OK(kefir_json_output_object_end(json));
        } else {
            REQUIRE_OK(kefir_json_output_null(json));
        }
        REQUIRE_OK(kefir_json_output_object_end(json));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t phi_format(struct kefir_json_output *json, const struct kefir_opt_phi_node *phi,
                                 const struct kefir_opt_code_analysis *code_analysis) {
    REQUIRE(code_analysis == NULL || code_analysis->instructions[phi->output_ref].reachable, KEFIR_OK);

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

static kefir_result_t inline_asm_format(struct kefir_json_output *json,
                                        const struct kefir_opt_inline_assembly_node *inline_asm) {
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(kefir_json_output_uinteger(json, inline_asm->node_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "inline_asm_id"));
    REQUIRE_OK(kefir_json_output_uinteger(json, inline_asm->inline_asm_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "parameters"));
    REQUIRE_OK(kefir_json_output_array_begin(json));

    for (kefir_size_t i = 0; i < inline_asm->parameter_count; i++) {
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "load_store"));
        REQUIRE_OK(id_format(json, inline_asm->parameters[i].load_store_ref));
        REQUIRE_OK(kefir_json_output_object_key(json, "read"));
        REQUIRE_OK(id_format(json, inline_asm->parameters[i].read_ref));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }

    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "default_jump_target"));
    REQUIRE_OK(id_format(json, inline_asm->default_jump_target));
    REQUIRE_OK(kefir_json_output_object_key(json, "jump_targets"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->jump_targets, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_id_t, target_id, node->key);
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block, node->value);

        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "id"));
        REQUIRE_OK(id_format(json, target_id));
        REQUIRE_OK(kefir_json_output_object_key(json, "target_block"));
        REQUIRE_OK(id_format(json, target_block));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t code_block_format(struct kefir_json_output *json, const struct kefir_opt_code_container *code,
                                        const struct kefir_opt_code_block *block,
                                        const struct kefir_opt_code_analysis *code_analysis) {
    REQUIRE(code_analysis == NULL || code_analysis->blocks[block->id].reachable, KEFIR_OK);

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(kefir_json_output_uinteger(json, block->id));
    REQUIRE_OK(kefir_json_output_object_key(json, "control_flow"));
    REQUIRE_OK(id_format(json, block->control_flow.head));

    REQUIRE_OK(kefir_json_output_object_key(json, "phi"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    kefir_result_t res;
    struct kefir_opt_phi_node *phi = NULL;
    for (res = kefir_opt_code_block_phi_head(code, block, &phi); res == KEFIR_OK && phi != NULL;
         res = kefir_opt_phi_next_sibling(code, phi, &phi)) {

        REQUIRE_OK(phi_format(json, phi, code_analysis));
    }
    REQUIRE_OK(res);
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "calls"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    struct kefir_opt_call_node *call = NULL;
    for (res = kefir_opt_code_block_call_head(code, block, &call); res == KEFIR_OK && call != NULL;
         res = kefir_opt_call_next_sibling(code, call, &call)) {

        REQUIRE_OK(call_format(json, call));
    }
    REQUIRE_OK(res);
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "inline_assembly"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    struct kefir_opt_inline_assembly_node *inline_asm = NULL;
    for (res = kefir_opt_code_block_inline_assembly_head(code, block, &inline_asm);
         res == KEFIR_OK && inline_asm != NULL;
         res = kefir_opt_inline_assembly_next_sibling(code, inline_asm, &inline_asm)) {

        REQUIRE_OK(inline_asm_format(json, inline_asm));
    }
    REQUIRE_OK(res);
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "code"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    struct kefir_opt_instruction *instr = NULL;
    for (res = kefir_opt_code_block_instr_head(code, block, &instr); res == KEFIR_OK && instr != NULL;
         res = kefir_opt_instruction_next_sibling(code, instr, &instr)) {

        REQUIRE_OK(instr_format(json, instr, code_analysis));
    }
    REQUIRE_OK(res);

    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "properties"));
    if (code_analysis != NULL) {
        const struct kefir_opt_code_analysis_block_properties *block_props = NULL;
        REQUIRE_OK(kefir_opt_code_analysis_block_properties(code_analysis, block->id, &block_props));

        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "reachable"));
        REQUIRE_OK(kefir_json_output_boolean(json, block_props->reachable));
        REQUIRE_OK(kefir_json_output_object_key(json, "linear_position"));
        if (block_props->reachable) {
            REQUIRE_OK(kefir_json_output_uinteger(json, block_props->linear_position));
        } else {
            REQUIRE_OK(kefir_json_output_null(json));
        }
        REQUIRE_OK(kefir_json_output_object_key(json, "linear_interval"));
        if (block_props->linear_range.begin_index != KEFIR_OPT_CODE_ANALYSIS_LINEAR_INDEX_UNDEFINED) {
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "begin"));
            REQUIRE_OK(kefir_json_output_uinteger(json, block_props->linear_range.begin_index));
            REQUIRE_OK(kefir_json_output_object_key(json, "end"));
            REQUIRE_OK(kefir_json_output_uinteger(json, block_props->linear_range.end_index));
            REQUIRE_OK(kefir_json_output_object_end(json));
        } else {
            REQUIRE_OK(kefir_json_output_null(json));
        }

        REQUIRE_OK(kefir_json_output_object_key(json, "successors"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        for (const struct kefir_list_entry *iter = kefir_list_head(&block_props->successors); iter != NULL;
             kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) iter->value);
            REQUIRE_OK(kefir_json_output_uinteger(json, block_id));
        }
        REQUIRE_OK(kefir_json_output_array_end(json));

        REQUIRE_OK(kefir_json_output_object_key(json, "predecessors"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        for (const struct kefir_list_entry *iter = kefir_list_head(&block_props->predecessors); iter != NULL;
             kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) iter->value);
            REQUIRE_OK(kefir_json_output_uinteger(json, block_id));
        }
        REQUIRE_OK(kefir_json_output_array_end(json));
        REQUIRE_OK(kefir_json_output_object_end(json));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_format(struct kefir_json_output *json, const struct kefir_opt_code_container *code,
                                     const struct kefir_opt_code_analysis *code_analysis) {
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid json output"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_json_output_object_begin(json));

    struct kefir_opt_code_container_iterator iter;
    REQUIRE_OK(kefir_json_output_object_key(json, "entry_point"));
    REQUIRE_OK(id_format(json, code->entry_point));
    REQUIRE_OK(kefir_json_output_object_key(json, "blocks"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {
        REQUIRE_OK(code_block_format(json, code, block, code_analysis));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "properties"));
    if (code_analysis != NULL) {
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "linear_length"));
        REQUIRE_OK(kefir_json_output_uinteger(json, code_analysis->linearization_length));
        REQUIRE_OK(kefir_json_output_object_end(json));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_function(struct kefir_json_output *json, const struct kefir_opt_function *function,
                                      const struct kefir_opt_code_analysis *code_analysis) {
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
    REQUIRE_OK(kefir_opt_code_format(json, &function->code, code_analysis));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_format(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                       const struct kefir_opt_module_analysis *analysis) {
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid json output"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));

    REQUIRE_OK(kefir_json_output_object_begin(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "functions"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->functions, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        const struct kefir_opt_code_analysis *code_analysis = NULL;
        if (analysis != NULL) {
            REQUIRE_OK(kefir_opt_module_analysis_get_function(analysis, (kefir_id_t) node->key, &code_analysis));
        }

        ASSIGN_DECL_CAST(const struct kefir_opt_function *, function, node->value);
        REQUIRE_OK(format_function(json, function, code_analysis));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}
