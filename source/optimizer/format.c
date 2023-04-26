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

static kefir_result_t id_format(struct kefir_json_output *json, kefir_opt_id_t id) {
    if (id == KEFIR_OPT_ID_NONE) {
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

static kefir_result_t format_operation_arg1(struct kefir_json_output *json, const struct kefir_opt_operation *oper) {
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    return KEFIR_OK;
}

static kefir_result_t instr_format(struct kefir_json_output *json, const struct kefir_opt_instruction *instr) {
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));

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

static kefir_result_t code_block_format(struct kefir_json_output *json, const struct kefir_opt_code_container *code,
                                        const struct kefir_opt_code_block *block) {
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(kefir_json_output_uinteger(json, block->id));
    REQUIRE_OK(kefir_json_output_object_key(json, "control_flow"));
    REQUIRE_OK(id_format(json, block->control_flow.head));
    REQUIRE_OK(kefir_json_output_object_key(json, "code"));
    REQUIRE_OK(kefir_json_output_array_begin(json));

    kefir_result_t res;
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
