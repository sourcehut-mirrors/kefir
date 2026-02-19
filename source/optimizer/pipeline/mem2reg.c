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

#include "kefir/optimizer/pipeline.h"
#include "kefir/optimizer/control_flow.h"
#include "kefir/optimizer/mem2reg_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t is_mem2reg_candidate(const struct kefir_opt_code_container *code,
                                           const struct kefir_opt_code_control_flow *control_flow,
                                           const struct kefir_ir_module *ir_module,
                                           kefir_opt_instruction_ref_t instr_ref, kefir_bool_t *skip_candidate) {
    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
    REQUIRE(instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected local allocation optimizer instruction"));

    const struct kefir_ir_type *ir_type =
        kefir_ir_module_get_named_type(ir_module, instr->operation.parameters.type.type_id);
    REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type"));
    const struct kefir_ir_typeentry *local_typeentry =
        kefir_ir_type_at(ir_type, instr->operation.parameters.type.type_index);
    REQUIRE(local_typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to fetch local variable type"));

    *skip_candidate = false;
    switch (local_typeentry->typecode) {
        case KEFIR_IR_TYPE_INT8:
        case KEFIR_IR_TYPE_INT16:
        case KEFIR_IR_TYPE_INT32:
        case KEFIR_IR_TYPE_INT64:
        case KEFIR_IR_TYPE_INT128:
        case KEFIR_IR_TYPE_FLOAT32:
        case KEFIR_IR_TYPE_FLOAT64:
        case KEFIR_IR_TYPE_LONG_DOUBLE:
        case KEFIR_IR_TYPE_COMPLEX_FLOAT32:
        case KEFIR_IR_TYPE_COMPLEX_FLOAT64:
        case KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE:
        case KEFIR_IR_TYPE_BITINT:
        case KEFIR_IR_TYPE_DECIMAL32:
        case KEFIR_IR_TYPE_DECIMAL64:
        case KEFIR_IR_TYPE_DECIMAL128:
            // Intentionally left blank
            break;

        case KEFIR_IR_TYPE_STRUCT:
        case KEFIR_IR_TYPE_ARRAY:
        case KEFIR_IR_TYPE_UNION:
        case KEFIR_IR_TYPE_BITFIELD:
        case KEFIR_IR_TYPE_NONE:
        case KEFIR_IR_TYPE_COUNT:
            *skip_candidate = true;
            break;
    }

    kefir_result_t res;
    struct kefir_opt_instruction_use_iterator use_iter;
    for (res = kefir_opt_code_container_instruction_use_instr_iter(code, instr_ref, &use_iter);
         !*skip_candidate && res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_instruction *use_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, use_iter.use_instr_ref, &use_instr));

        kefir_bool_t reachable;
        REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(control_flow, use_instr->block_id, &reachable));
        if (!reachable) {
            continue;
        }

        switch (use_instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_INT8_LOAD:
            case KEFIR_OPT_OPCODE_INT16_LOAD:
            case KEFIR_OPT_OPCODE_INT32_LOAD:
            case KEFIR_OPT_OPCODE_INT64_LOAD:
            case KEFIR_OPT_OPCODE_INT128_LOAD:
            case KEFIR_OPT_OPCODE_FLOAT32_LOAD:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL32_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL64_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL128_LOAD:
            case KEFIR_OPT_OPCODE_BITINT_LOAD:
            case KEFIR_OPT_OPCODE_BITINT_LOAD_PRECISE:
                *skip_candidate = use_instr->operation.parameters.memory_access.flags.volatile_access;
                break;

            case KEFIR_OPT_OPCODE_INT8_STORE:
            case KEFIR_OPT_OPCODE_INT16_STORE:
            case KEFIR_OPT_OPCODE_INT32_STORE:
            case KEFIR_OPT_OPCODE_INT64_STORE:
            case KEFIR_OPT_OPCODE_INT128_STORE:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL64_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL128_STORE:
            case KEFIR_OPT_OPCODE_BITINT_STORE:
            case KEFIR_OPT_OPCODE_BITINT_STORE_PRECISE:
                if (instr_ref == use_instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF]) {
                    *skip_candidate = true;
                } else {
                    *skip_candidate = use_instr->operation.parameters.memory_access.flags.volatile_access;
                }
                break;

            default:
                *skip_candidate = true;
                break;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t mem2reg_scan(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                   const struct kefir_opt_code_control_flow *control_flow,
                                   const struct kefir_ir_module *ir_module, struct kefir_hashset *candidates) {
    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {

        kefir_opt_instruction_ref_t instr_ref;
        for (kefir_opt_code_block_instr_head(code, block->id, &instr_ref); instr_ref != KEFIR_ID_NONE;
             kefir_opt_instruction_next_sibling(code, instr_ref, &instr_ref)) {

            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
            if (instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
                kefir_bool_t skip_candidate = false;
                REQUIRE_OK(is_mem2reg_candidate(code, control_flow, ir_module, instr_ref, &skip_candidate));
                if (!skip_candidate) {
                    REQUIRE_OK(kefir_hashset_add(mem, candidates, (kefir_hashset_key_t) instr_ref));
                }
            }
        }
    }

    return KEFIR_OK;
}

static kefir_result_t mem2reg_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                    struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                    const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct kefir_opt_code_control_flow control_flow;
    struct kefir_hashset candidates;
    REQUIRE_OK(kefir_opt_code_control_flow_init(&control_flow));
    REQUIRE_OK(kefir_hashset_init(&candidates, &kefir_hashtable_uint_ops));

    kefir_result_t res = kefir_opt_code_control_flow_build(mem, &control_flow, &func->code);
    REQUIRE_CHAIN(&res, mem2reg_scan(mem, &func->code, &control_flow, module->ir_module, &candidates));
    REQUIRE_CHAIN(&res, kefir_opt_code_util_mem2reg_apply(mem, module->ir_module, &func->code, &func->debug_info,
                                                          &control_flow, &candidates));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_control_flow_free(mem, &control_flow);
        kefir_hashset_free(mem, &candidates);
        return res;
    });
    res = kefir_opt_code_control_flow_free(mem, &control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &candidates);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &candidates));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassMem2Reg = {
    .name = "mem2reg", .apply = mem2reg_apply, .payload = NULL};
