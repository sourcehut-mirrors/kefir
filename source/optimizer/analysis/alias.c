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
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_bool_t offset_alias(kefir_int64_t offset1, kefir_size_t size1, kefir_int64_t offset2, kefir_size_t size2) {
    return (offset2 >= offset1 && offset2 < (kefir_int64_t) (offset1 + size1)) ||
           (offset1 >= offset2 && offset1 < (kefir_int64_t) (offset2 + size2));
}

kefir_result_t kefir_opt_code_may_alias(const struct kefir_opt_code_container *code,
                                        kefir_opt_instruction_ref_t location_ref1, kefir_size_t size1,
                                        kefir_int64_t offset1, kefir_opt_instruction_ref_t location_ref2,
                                        kefir_size_t size2, kefir_int64_t offset2, kefir_bool_t *may_alias) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(may_alias != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    const struct kefir_opt_instruction *location1, *location2;
    REQUIRE_OK(kefir_opt_code_container_instr(code, location_ref1, &location1));
    REQUIRE_OK(kefir_opt_code_container_instr(code, location_ref2, &location2));

    *may_alias = true;
    if ((location1->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL &&
         location2->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL) ||
        (location1->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL &&
         location2->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL)) {
        if (location1->operation.parameters.variable.global_ref !=
            location2->operation.parameters.variable.global_ref) {
            *may_alias = false;
        } else {
            *may_alias = offset_alias(location1->operation.parameters.variable.offset + offset1, size1,
                                      location2->operation.parameters.variable.offset + offset2, size2);
        }
    } else if (location1->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
               location2->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
        if (location1->id != location2->id) {
            *may_alias = false;
        } else {
            *may_alias = offset_alias(offset1, size1, offset2, size2);
        }
    } else if ((location1->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL ||
                location1->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL ||
                location1->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) &&
               (location2->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL ||
                location2->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL ||
                location2->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) &&
               location1->operation.opcode != location2->operation.opcode) {
        *may_alias = false;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_must_alias(const struct kefir_opt_code_container *code,
                                         kefir_opt_instruction_ref_t location_ref1, kefir_size_t size1,
                                         kefir_int64_t offset1, kefir_opt_instruction_ref_t location_ref2,
                                         kefir_size_t size2, kefir_int64_t offset2, kefir_bool_t *must_alias) {
    UNUSED(size1);
    UNUSED(size2);
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(must_alias != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    const struct kefir_opt_instruction *location1, *location2;
    REQUIRE_OK(kefir_opt_code_container_instr(code, location_ref1, &location1));
    REQUIRE_OK(kefir_opt_code_container_instr(code, location_ref2, &location2));

    *must_alias = false;
    if (location1->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL &&
        location2->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL &&
        location1->operation.parameters.variable.global_ref == location2->operation.parameters.variable.global_ref) {
        *must_alias = (location1->operation.parameters.variable.offset + offset1) ==
                      (location2->operation.parameters.variable.offset + offset2);
    } else if (location1->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL &&
               location2->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL &&
               location1->operation.parameters.variable.global_ref ==
                   location2->operation.parameters.variable.global_ref &&
               location1->operation.parameters.variable.offset == location2->operation.parameters.variable.offset) {
        *must_alias = (location1->operation.parameters.variable.offset + offset1) ==
                      (location2->operation.parameters.variable.offset + offset2);
    } else if (location1->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
               location2->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL && location1->id == location2->id) {
        *must_alias = offset1 == offset2;
    }
    return KEFIR_OK;
}
