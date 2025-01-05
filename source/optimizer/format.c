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
            REQUIRE_OK(kefir_json_output_object_key(json, "condition_variant"));
            switch (oper->parameters.branch.condition_variant) {
                case KEFIR_OPT_BRANCH_CONDITION_8BIT:
                    REQUIRE_OK(kefir_json_output_string(json, "8bit"));
                    break;

                case KEFIR_OPT_BRANCH_CONDITION_16BIT:
                    REQUIRE_OK(kefir_json_output_string(json, "16bit"));
                    break;

                case KEFIR_OPT_BRANCH_CONDITION_32BIT:
                    REQUIRE_OK(kefir_json_output_string(json, "32bit"));
                    break;

                case KEFIR_OPT_BRANCH_CONDITION_64BIT:
                    REQUIRE_OK(kefir_json_output_string(json, "64bit"));
                    break;
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer operation code");
    }
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

static kefir_result_t format_operation_load_mem(struct kefir_json_output *json,
                                                const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "location"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF]));
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
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF]));
    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF]));
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
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_BITFIELD_BASE_REF]));
    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_BITFIELD_VALUE_REF]));
    REQUIRE_OK(kefir_json_output_object_key(json, "offset"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.bitfield.offset));
    REQUIRE_OK(kefir_json_output_object_key(json, "length"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.bitfield.length));
    return KEFIR_OK;
}

static kefir_result_t format_operation_typed_ref1(struct kefir_json_output *json,
                                                  const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "ref"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(id_format(json, oper->parameters.type.type_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.type.type_index));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_typed_ref2(struct kefir_json_output *json,
                                                  const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "target"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(kefir_json_output_object_key(json, "source"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(id_format(json, oper->parameters.type.type_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.type.type_index));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_overflow_arith(struct kefir_json_output *json,
                                                      const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "args"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
    REQUIRE_OK(id_format(json, oper->parameters.refs[2]));
    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(id_format(json, oper->parameters.type.type_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.type.type_index));
    REQUIRE_OK(kefir_json_output_object_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "signedness"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    REQUIRE_OK(kefir_json_output_boolean(json, (oper->parameters.overflow_arith.signedness & 1) == 1));
    REQUIRE_OK(kefir_json_output_boolean(json, ((oper->parameters.overflow_arith.signedness >> 1) & 1) == 1));
    REQUIRE_OK(kefir_json_output_boolean(json, ((oper->parameters.overflow_arith.signedness >> 2) & 1) == 1));
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_stack_alloc(struct kefir_json_output *json,
                                                   const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "size_ref"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_STACK_ALLOCATION_SIZE_REF]));
    REQUIRE_OK(kefir_json_output_object_key(json, "alignment_ref"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_STACK_ALLOCATION_ALIGNMENT_REF]));
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

static kefir_result_t format_operation_index(struct kefir_json_output *json, const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.index));
    return KEFIR_OK;
}

static kefir_result_t format_operation_variable(struct kefir_json_output *json,
                                                const struct kefir_opt_operation *oper) {
    switch (oper->opcode) {
        case KEFIR_OPT_OPCODE_GET_GLOBAL:
        case KEFIR_OPT_OPCODE_GET_THREAD_LOCAL:
            REQUIRE_OK(kefir_json_output_object_key(json, "global_ref"));
            REQUIRE_OK(id_format(json, oper->parameters.variable.global_ref));
            break;

        case KEFIR_OPT_OPCODE_GET_LOCAL:
            REQUIRE_OK(kefir_json_output_object_key(json, "local_index"));
            REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.variable.local_index));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer instruction opcode");
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "offset"));
    REQUIRE_OK(kefir_json_output_integer(json, oper->parameters.variable.offset));
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

static kefir_result_t format_operation_atomic_op(struct kefir_json_output *json,
                                                 const struct kefir_opt_operation *oper) {
    switch (oper->opcode) {
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD8:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD16:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD32:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD64:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_FLOAT32:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_FLOAT64:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(kefir_json_output_object_key(json, "location"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_STORE8:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE16:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE32:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE64:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE_LONG_DOUBLE:
            REQUIRE_OK(kefir_json_output_object_key(json, "location"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_COPY_MEMORY_FROM:
        case KEFIR_OPT_OPCODE_ATOMIC_COPY_MEMORY_TO:
            REQUIRE_OK(kefir_json_output_object_key(json, "location"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
            REQUIRE_OK(kefir_json_output_object_key(json, "source"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
            REQUIRE_OK(kefir_json_output_object_key(json, "type_id"));
            REQUIRE_OK(id_format(json, oper->parameters.type.type_id));
            REQUIRE_OK(kefir_json_output_object_key(json, "type_index"));
            REQUIRE_OK(id_format(json, oper->parameters.type.type_index));
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG8:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG16:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG32:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG64:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(kefir_json_output_object_key(json, "location"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
            REQUIRE_OK(kefir_json_output_object_key(json, "compare_value"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
            REQUIRE_OK(kefir_json_output_object_key(json, "new_value"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[2]));
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_MEMORY:
            REQUIRE_OK(kefir_json_output_object_key(json, "location"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
            REQUIRE_OK(kefir_json_output_object_key(json, "compare_value"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
            REQUIRE_OK(kefir_json_output_object_key(json, "new_value"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[2]));
            REQUIRE_OK(kefir_json_output_object_key(json, "type_id"));
            REQUIRE_OK(id_format(json, oper->parameters.type.type_id));
            REQUIRE_OK(kefir_json_output_object_key(json, "type_index"));
            REQUIRE_OK(id_format(json, oper->parameters.type.type_index));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer operation code");
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "atomic_model"));
    switch (oper->parameters.atomic_op.model) {
        case KEFIR_OPT_MEMORY_ORDER_SEQ_CST:
            REQUIRE_OK(kefir_json_output_string(json, "seq_cst"));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected atomic model");
    }
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
            REQUIRE_OK(kefir_json_output_long_double(json, oper->parameters.imm.long_double));
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
                                   const struct kefir_opt_code_analysis *code_analysis,
                                   const struct kefir_opt_code_debug_info *debug_info) {
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

    if (debug_info != NULL) {
        REQUIRE_OK(kefir_json_output_object_key(json, "ir_instruction"));
        kefir_size_t instruction_location;
        REQUIRE_OK(kefir_opt_code_debug_info_instruction_location(debug_info, instr->id, &instruction_location));
        if (instruction_location != KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE) {
            REQUIRE_OK(kefir_json_output_uinteger(json, instruction_location));
        } else {
            REQUIRE_OK(kefir_json_output_null(json));
        }
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
                                        const struct kefir_opt_code_analysis *code_analysis,
                                        const struct kefir_opt_code_debug_info *debug_info) {
    REQUIRE(code_analysis == NULL || code_analysis->blocks[block->id].reachable, KEFIR_OK);

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(kefir_json_output_uinteger(json, block->id));
    REQUIRE_OK(kefir_json_output_object_key(json, "control_flow"));
    REQUIRE_OK(id_format(json, block->control_flow.head));

    REQUIRE_OK(kefir_json_output_object_key(json, "phi"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    kefir_result_t res;
    kefir_opt_phi_id_t phi_ref;
    const struct kefir_opt_phi_node *phi = NULL;
    for (res = kefir_opt_code_block_phi_head(code, block, &phi_ref); res == KEFIR_OK && phi_ref != KEFIR_ID_NONE;
         res = kefir_opt_phi_next_sibling(code, phi_ref, &phi_ref)) {

        REQUIRE_OK(kefir_opt_code_container_phi(code, phi_ref, &phi));
        REQUIRE_OK(phi_format(json, phi, code_analysis));
    }
    REQUIRE_OK(res);
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "calls"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    kefir_opt_call_id_t call_ref;
    const struct kefir_opt_call_node *call = NULL;
    for (res = kefir_opt_code_block_call_head(code, block, &call_ref); res == KEFIR_OK && call_ref != KEFIR_ID_NONE;
         res = kefir_opt_call_next_sibling(code, call_ref, &call_ref)) {

        REQUIRE_OK(kefir_opt_code_container_call(code, call_ref, &call));
        REQUIRE_OK(call_format(json, call));
    }
    REQUIRE_OK(res);
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "inline_assembly"));
    REQUIRE_OK(kefir_json_output_array_begin(json));

    kefir_opt_inline_assembly_id_t inline_asm_id;
    const struct kefir_opt_inline_assembly_node *inline_asm = NULL;
    for (res = kefir_opt_code_block_inline_assembly_head(code, block, &inline_asm_id);
         res == KEFIR_OK && inline_asm_id != KEFIR_ID_NONE;
         res = kefir_opt_inline_assembly_next_sibling(code, inline_asm_id, &inline_asm_id)) {

        REQUIRE_OK(kefir_opt_code_container_inline_assembly(code, inline_asm_id, &inline_asm));
        REQUIRE_OK(inline_asm_format(json, inline_asm));
    }
    REQUIRE_OK(res);
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "code"));
    REQUIRE_OK(kefir_json_output_array_begin(json));

    kefir_opt_instruction_ref_t instr_ref;
    const struct kefir_opt_instruction *instr = NULL;
    for (res = kefir_opt_code_block_instr_head(code, block, &instr_ref); res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
         res = kefir_opt_instruction_next_sibling(code, instr_ref, &instr_ref)) {

        REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
        REQUIRE_OK(instr_format(json, instr, code_analysis, debug_info));
    }
    REQUIRE_OK(res);
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "public_labels"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    struct kefir_hashtreeset_iterator iter;
    for (res = kefir_hashtreeset_iter(&block->public_labels, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, alternative_label, iter.entry);
        REQUIRE_OK(kefir_json_output_string(json, alternative_label));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
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
                                     const struct kefir_opt_code_analysis *code_analysis,
                                     const struct kefir_opt_code_debug_info *debug_info) {
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
        REQUIRE_OK(code_block_format(json, code, block, code_analysis, debug_info));
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

    if (debug_info != NULL) {
        REQUIRE_OK(kefir_json_output_object_key(json, "debug"));
        REQUIRE_OK(kefir_json_output_object_begin(json));

        REQUIRE_OK(kefir_json_output_object_key(json, "local_variables"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        struct kefir_opt_code_debug_info_local_variable_iterator iter;
        struct kefir_opt_code_debug_info_local_variable_ref_iterator ref_iter;
        kefir_size_t local_variable;
        kefir_opt_instruction_ref_t instr_ref;
        kefir_result_t res;

        for (res = kefir_opt_code_debug_info_local_variable_iter(debug_info, &iter, &local_variable); res == KEFIR_OK;
             res = kefir_opt_code_debug_info_local_variable_next(&iter, &local_variable)) {
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "variable"));
            REQUIRE_OK(kefir_json_output_uinteger(json, local_variable));
            REQUIRE_OK(kefir_json_output_object_key(json, "refs"));
            REQUIRE_OK(kefir_json_output_array_begin(json));
            for (res = kefir_opt_code_debug_info_local_variable_ref_iter(debug_info, &ref_iter, local_variable,
                                                                         &instr_ref);
                 res == KEFIR_OK; res = kefir_opt_code_debug_info_local_variable_ref_next(&ref_iter, &instr_ref)) {
                REQUIRE_OK(kefir_json_output_uinteger(json, instr_ref));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
            REQUIRE_OK(kefir_json_output_array_end(json));
            REQUIRE_OK(kefir_json_output_object_end(json));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(kefir_json_output_array_end(json));

        REQUIRE_OK(kefir_json_output_object_end(json));
    }

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_function(struct kefir_json_output *json, const struct kefir_opt_function *function,
                                      const struct kefir_opt_code_analysis *code_analysis, kefir_bool_t debug_info) {
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
    REQUIRE_OK(kefir_opt_code_format(json, &function->code, code_analysis, debug_info ? &function->debug_info : NULL));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_format(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                       const struct kefir_opt_module_analysis *analysis, kefir_bool_t debug_info) {
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
        REQUIRE_OK(format_function(json, function, code_analysis, debug_info));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}
