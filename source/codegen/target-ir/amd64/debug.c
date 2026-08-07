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

#include "kefir/codegen/target-ir/amd64/debug.h"
#include "kefir/codegen/target-ir/amd64/regalloc.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_amd64_target_ir_find_value_location(
    const struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_regalloc *regalloc, kefir_asmcmp_debug_info_value_location_reference_t location_ref,
    struct kefir_asmcmp_debug_info_value_location *location) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));
    REQUIRE(location != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp debug value location"));

    kefir_codegen_target_ir_value_ref_t value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(location_ref);
    const struct kefir_codegen_target_ir_value_type *value_type;
    kefir_result_t res = kefir_codegen_target_ir_code_value_props(code, value_ref, &value_type);
    if (res == KEFIR_NOT_FOUND) {
        location->type = KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_NONE;
        return KEFIR_OK;
    }

    switch (value_type->kind) {
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_UNSPECIFIED:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_RESOURCE:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT:
            location->type = KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_NONE;
            break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE: {
            kefir_codegen_target_ir_regalloc_allocation_t allocation;
            res = kefir_codegen_target_ir_regalloc_get(regalloc, value_ref, &allocation);
            if (res == KEFIR_NOT_FOUND) {
                location->type = KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_NONE;
                return KEFIR_OK;
            }
            REQUIRE_OK(res);

            union kefir_codegen_target_ir_amd64_regalloc_entry entry = {.allocation = allocation};
            switch (entry.type) {
                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_NA:
                    location->type = KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_NONE;
                    break;

                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL:
                    location->type = KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_SPILL_AREA;
                    location->spill_area.index = entry.spill_area.index;
                    location->spill_area.length = entry.spill_area.length;
                    break;

                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP:
                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE:
                    location->type = KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_REGISTER;
                    location->reg = entry.reg.value;
                    break;
            }
        } break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE:
            location->type = KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_LOCAL_VARIABLE;
            location->local_variable.identifier = value_type->parameters.local_variable.identifier;
            location->local_variable.offset = value_type->parameters.local_variable.offset;
            break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_EXTERNAL_MEMORY:
            location->type = KEFIR_ASMCMP_DEBUG_INFO_VALUE_LOCATION_MEMORY_POINTER;
            location->memory.base_reg = value_type->parameters.memory.base_reg;
            location->memory.offset = value_type->parameters.memory.offset;
            break;
    }

    return KEFIR_OK;
}
