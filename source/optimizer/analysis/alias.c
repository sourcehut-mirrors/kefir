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

#include "kefir/optimizer/alias.h"
#include "kefir/ir/module.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_bool_t offset_alias(kefir_int64_t offset1, kefir_size_t size1, kefir_int64_t offset2, kefir_size_t size2) {
    return (offset2 >= offset1 && offset2 < (kefir_int64_t) (offset1 + size1)) ||
           (offset1 >= offset2 && offset1 < (kefir_int64_t) (offset2 + size2));
}

static kefir_result_t same_global_refs(const struct kefir_ir_module *ir_module, kefir_id_t ref1, kefir_id_t ref2,
                                       kefir_bool_t *same_refs) {
    if (ref1 == ref2) {
        *same_refs = true;
        return KEFIR_OK;
    }
    const char *symbol1 = kefir_ir_module_get_named_symbol(ir_module, ref1);
    REQUIRE(symbol1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find named IR symbol"));

    const char *symbol2 = kefir_ir_module_get_named_symbol(ir_module, ref2);
    REQUIRE(symbol2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find named IR symbol"));

    const struct kefir_ir_identifier *ir_identifier1, *ir_identifier2;
    REQUIRE_OK(kefir_ir_module_get_identifier(ir_module, symbol1, &ir_identifier1));
    REQUIRE_OK(kefir_ir_module_get_identifier(ir_module, symbol2, &ir_identifier2));

    const char *name1 = ir_identifier1->alias != NULL ? ir_identifier1->alias : ir_identifier1->symbol;
    const char *name2 = ir_identifier2->alias != NULL ? ir_identifier2->alias : ir_identifier2->symbol;

    *same_refs = strcmp(name1, name2) == 0;
    return KEFIR_OK;
}

static kefir_result_t may_alias_impl(const struct kefir_opt_code_container *code,
                                     const struct kefir_opt_code_escape_analysis *escapes,
                                     const struct kefir_ir_module *ir_module, kefir_opt_instruction_ref_t location_ref1,
                                     kefir_size_t size1, kefir_int64_t offset1,
                                     kefir_opt_instruction_ref_t location_ref2, kefir_size_t size2,
                                     kefir_int64_t offset2, kefir_bool_t pessimistic_aliasing,
                                     kefir_bool_t *may_alias) {
    const struct kefir_opt_instruction *location1, *location2;
    REQUIRE_OK(kefir_opt_code_container_instr(code, location_ref1, &location1));
    REQUIRE_OK(kefir_opt_code_container_instr(code, location_ref2, &location2));

    *may_alias = true;
    if ((location1->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL &&
         location2->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL) ||
        (location1->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL &&
         location2->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL)) {
        kefir_bool_t same_refs = true;
        REQUIRE_OK(same_global_refs(ir_module, location1->operation.parameters.variable.global_ref,
                                    location2->operation.parameters.variable.global_ref, &same_refs));
        if (!same_refs) {
            *may_alias = false;
        } else {
            *may_alias =
                pessimistic_aliasing || offset_alias(location1->operation.parameters.variable.offset + offset1, size1,
                                                     location2->operation.parameters.variable.offset + offset2, size2);
        }
    } else if (location1->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
               location2->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
        if (location1->id != location2->id) {
            *may_alias = false;
        } else {
            *may_alias = pessimistic_aliasing || offset_alias(offset1, size1, offset2, size2);
        }
    } else if ((location1->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL ||
                location1->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL ||
                location1->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) &&
               (location2->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL ||
                location2->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL ||
                location2->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) &&
               location1->operation.opcode != location2->operation.opcode) {
        *may_alias = false;
    } else if (location1->operation.opcode == KEFIR_OPT_OPCODE_REF_LOCAL) {
        REQUIRE_OK(may_alias_impl(code, escapes, ir_module, location1->operation.parameters.refs[0], size1,
                                  offset1 + location1->operation.parameters.offset, location_ref2, size2, offset2,
                                  pessimistic_aliasing, may_alias));
    } else if (location2->operation.opcode == KEFIR_OPT_OPCODE_REF_LOCAL) {
        REQUIRE_OK(may_alias_impl(code, escapes, ir_module, location_ref1, size1, offset1,
                                  location2->operation.parameters.refs[0], size2,
                                  offset2 + location2->operation.parameters.offset, pessimistic_aliasing, may_alias));
    } else if (escapes != NULL && location1->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
               !kefir_opt_code_escape_analysis_has_escapes(escapes, location_ref1)) {
        *may_alias = false;
    } else if (escapes != NULL && location2->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
               !kefir_opt_code_escape_analysis_has_escapes(escapes, location_ref2)) {
        *may_alias = false;
    } else if (location1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ADD) {
        const struct kefir_opt_instruction *arg1_instr, *arg2_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, location1->operation.parameters.refs[0], &arg1_instr));
        REQUIRE_OK(kefir_opt_code_container_instr(code, location1->operation.parameters.refs[1], &arg2_instr));
        if (arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
            arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            REQUIRE_OK(may_alias_impl(code, escapes, ir_module, location1->operation.parameters.refs[1], size1,
                                      offset1 + arg1_instr->operation.parameters.imm.integer, location_ref2, size2,
                                      offset2, pessimistic_aliasing, may_alias));
        } else if (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                   arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            REQUIRE_OK(may_alias_impl(code, escapes, ir_module, location1->operation.parameters.refs[0], size1,
                                      offset1 + arg2_instr->operation.parameters.imm.integer, location_ref2, size2,
                                      offset2, pessimistic_aliasing, may_alias));
        } else if (!pessimistic_aliasing) {
            kefir_bool_t may_alias_left = true, may_alias_right = true;
            REQUIRE_OK(may_alias_impl(code, escapes, ir_module, arg1_instr->id, size1, offset1, location_ref2, size2,
                                      offset2, true, &may_alias_left));
            REQUIRE_OK(may_alias_impl(code, escapes, ir_module, arg2_instr->id, size1, offset1, location_ref2, size2,
                                      offset2, true, &may_alias_right));
            *may_alias = may_alias_left && may_alias_right;
        }
    } else if (location2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ADD) {
        const struct kefir_opt_instruction *arg1_instr, *arg2_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, location2->operation.parameters.refs[0], &arg1_instr));
        REQUIRE_OK(kefir_opt_code_container_instr(code, location2->operation.parameters.refs[1], &arg2_instr));
        if (arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
            arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            REQUIRE_OK(may_alias_impl(
                code, escapes, ir_module, location_ref1, size1, offset1, location2->operation.parameters.refs[1], size2,
                offset2 + arg1_instr->operation.parameters.imm.integer, pessimistic_aliasing, may_alias));
        } else if (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                   arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            REQUIRE_OK(may_alias_impl(
                code, escapes, ir_module, location_ref1, size1, offset1, location2->operation.parameters.refs[0], size2,
                offset2 + arg2_instr->operation.parameters.imm.integer, pessimistic_aliasing, may_alias));
        } else if (!pessimistic_aliasing) {
            kefir_bool_t may_alias_left = true, may_alias_right = true;
            REQUIRE_OK(may_alias_impl(code, escapes, ir_module, location_ref1, size1, offset1, arg1_instr->id, size2,
                                      offset2, true, &may_alias_left));
            REQUIRE_OK(may_alias_impl(code, escapes, ir_module, location_ref1, size1, offset1, arg2_instr->id, size2,
                                      offset2, true, &may_alias_right));
            *may_alias = may_alias_left && may_alias_right;
        }
    } else if (location1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SUB) {
        const struct kefir_opt_instruction *arg1_instr, *arg2_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, location1->operation.parameters.refs[0], &arg1_instr));
        REQUIRE_OK(kefir_opt_code_container_instr(code, location1->operation.parameters.refs[1], &arg2_instr));
        if (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
            arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            REQUIRE_OK(may_alias_impl(code, escapes, ir_module, location1->operation.parameters.refs[0], size1,
                                      offset1 - arg2_instr->operation.parameters.imm.integer, location_ref2, size2,
                                      offset2, pessimistic_aliasing, may_alias));
        } else if (!pessimistic_aliasing) {
            kefir_bool_t may_alias_left = true, may_alias_right = true;
            REQUIRE_OK(may_alias_impl(code, escapes, ir_module, arg1_instr->id, size1, offset1, location_ref2, size2,
                                      offset2, true, &may_alias_left));
            REQUIRE_OK(may_alias_impl(code, escapes, ir_module, arg2_instr->id, size1, offset1, location_ref2, size2,
                                      offset2, true, &may_alias_right));
            *may_alias = may_alias_left && may_alias_right;
        }
    } else if (location2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SUB) {
        const struct kefir_opt_instruction *arg1_instr, *arg2_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, location2->operation.parameters.refs[0], &arg1_instr));
        REQUIRE_OK(kefir_opt_code_container_instr(code, location2->operation.parameters.refs[1], &arg2_instr));
        if (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
            arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            REQUIRE_OK(may_alias_impl(
                code, escapes, ir_module, location_ref1, size1, offset1, location2->operation.parameters.refs[0], size2,
                offset2 - arg2_instr->operation.parameters.imm.integer, pessimistic_aliasing, may_alias));
        } else if (!pessimistic_aliasing) {
            kefir_bool_t may_alias_left = true, may_alias_right = true;
            REQUIRE_OK(may_alias_impl(code, escapes, ir_module, location_ref1, size1, offset1, arg1_instr->id, size2,
                                      offset2, true, &may_alias_left));
            REQUIRE_OK(may_alias_impl(code, escapes, ir_module, location_ref1, size1, offset1, arg2_instr->id, size2,
                                      offset2, true, &may_alias_right));
            *may_alias = may_alias_left && may_alias_right;
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_may_alias(const struct kefir_opt_code_container *code,
                                        const struct kefir_opt_code_escape_analysis *escapes,
                                        const struct kefir_ir_module *ir_module,
                                        kefir_opt_instruction_ref_t location_ref1, kefir_size_t size1,
                                        kefir_int64_t offset1, kefir_opt_instruction_ref_t location_ref2,
                                        kefir_size_t size2, kefir_int64_t offset2, kefir_bool_t *may_alias) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(escapes != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer escape analysis"));
    REQUIRE(ir_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(may_alias != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    REQUIRE_OK(may_alias_impl(code, escapes, ir_module, location_ref1, size1, offset1, location_ref2, size2, offset2,
                              false, may_alias));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_must_alias(const struct kefir_opt_code_container *code,
                                         const struct kefir_ir_module *ir_module,
                                         kefir_opt_instruction_ref_t location_ref1, kefir_size_t size1,
                                         kefir_int64_t offset1, kefir_opt_instruction_ref_t location_ref2,
                                         kefir_size_t size2, kefir_int64_t offset2, kefir_bool_t *must_alias) {
    UNUSED(size1);
    UNUSED(size2);
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(ir_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(must_alias != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    const struct kefir_opt_instruction *location1, *location2;
    REQUIRE_OK(kefir_opt_code_container_instr(code, location_ref1, &location1));
    REQUIRE_OK(kefir_opt_code_container_instr(code, location_ref2, &location2));

    *must_alias = false;
    if (location1->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL &&
        location2->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL) {
        kefir_bool_t same_refs = true;
        REQUIRE_OK(same_global_refs(ir_module, location1->operation.parameters.variable.global_ref,
                                    location2->operation.parameters.variable.global_ref, &same_refs));
        *must_alias = same_refs && (location1->operation.parameters.variable.offset + offset1) ==
                                       (location2->operation.parameters.variable.offset + offset2);
    } else if (location1->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL &&
               location2->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL &&
               location1->operation.parameters.variable.global_ref ==
                   location2->operation.parameters.variable.global_ref) {
        kefir_bool_t same_refs = true;
        REQUIRE_OK(same_global_refs(ir_module, location1->operation.parameters.variable.global_ref,
                                    location2->operation.parameters.variable.global_ref, &same_refs));
        *must_alias = same_refs && (location1->operation.parameters.variable.offset + offset1) ==
                                       (location2->operation.parameters.variable.offset + offset2);
    } else if (location1->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
               location2->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL && location1->id == location2->id) {
        *must_alias = offset1 == offset2;
    } else if (location1->operation.opcode == KEFIR_OPT_OPCODE_REF_LOCAL) {
        REQUIRE_OK(kefir_opt_code_must_alias(code, ir_module, location1->operation.parameters.refs[0], size1,
                                             offset1 + location1->operation.parameters.offset, location_ref2, size2,
                                             offset2, must_alias));
    } else if (location2->operation.opcode == KEFIR_OPT_OPCODE_REF_LOCAL) {
        REQUIRE_OK(kefir_opt_code_must_alias(code, ir_module, location_ref1, size1, offset1,
                                             location2->operation.parameters.refs[0], size2,
                                             offset2 + location2->operation.parameters.offset, must_alias));
    } else if (location1->operation.opcode == KEFIR_OPT_OPCODE_GET_ARGUMENT &&
               location2->operation.opcode == KEFIR_OPT_OPCODE_GET_ARGUMENT &&
               location1->operation.parameters.index == location2->operation.parameters.index) {
        *must_alias = offset1 == offset2;
    } else if (location1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ADD) {
        const struct kefir_opt_instruction *arg1_instr, *arg2_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, location1->operation.parameters.refs[0], &arg1_instr));
        REQUIRE_OK(kefir_opt_code_container_instr(code, location1->operation.parameters.refs[1], &arg2_instr));
        if (arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
            arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            REQUIRE_OK(kefir_opt_code_must_alias(code, ir_module, location1->operation.parameters.refs[1], size1,
                                                 offset1 + arg1_instr->operation.parameters.imm.integer, location_ref2,
                                                 size2, offset2, must_alias));
        } else if (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                   arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            REQUIRE_OK(kefir_opt_code_must_alias(code, ir_module, location1->operation.parameters.refs[0], size1,
                                                 offset1 + arg2_instr->operation.parameters.imm.integer, location_ref2,
                                                 size2, offset2, must_alias));
        }
    } else if (location2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ADD) {
        const struct kefir_opt_instruction *arg1_instr, *arg2_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, location2->operation.parameters.refs[0], &arg1_instr));
        REQUIRE_OK(kefir_opt_code_container_instr(code, location2->operation.parameters.refs[1], &arg2_instr));
        if (arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
            arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            REQUIRE_OK(kefir_opt_code_must_alias(code, ir_module, location_ref1, size1, offset1,
                                                 location2->operation.parameters.refs[1], size2,
                                                 offset2 + arg1_instr->operation.parameters.imm.integer, must_alias));
        } else if (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                   arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            REQUIRE_OK(kefir_opt_code_must_alias(code, ir_module, location_ref1, size1, offset1,
                                                 location2->operation.parameters.refs[0], size2,
                                                 offset2 + arg2_instr->operation.parameters.imm.integer, must_alias));
        }
    } else if (location1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SUB) {
        const struct kefir_opt_instruction *arg2_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, location1->operation.parameters.refs[1], &arg2_instr));
        if (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
            arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            REQUIRE_OK(kefir_opt_code_must_alias(code, ir_module, location1->operation.parameters.refs[0], size1,
                                                 offset1 - arg2_instr->operation.parameters.imm.integer, location_ref2,
                                                 size2, offset2, must_alias));
        }
    } else if (location2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SUB) {
        const struct kefir_opt_instruction *arg2_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, location2->operation.parameters.refs[1], &arg2_instr));
        if (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
            arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            REQUIRE_OK(kefir_opt_code_must_alias(code, ir_module, location_ref1, size1, offset1,
                                                 location2->operation.parameters.refs[0], size2,
                                                 offset2 - arg2_instr->operation.parameters.imm.integer, must_alias));
        }
    } else if ((location1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                location1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) &&
               (location2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                location2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {
        *must_alias = location1->operation.parameters.imm.integer + offset1 ==
                      location2->operation.parameters.imm.integer + offset2;
    }
    return KEFIR_OK;
}
