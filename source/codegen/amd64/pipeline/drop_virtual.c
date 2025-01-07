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

#define KEFIR_CODEGEN_ASMCMP_PIPELINE_INTERNAL
#include "kefir/codegen/asmcmp/pipeline.h"
#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t drop_virtual_apply(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                 const struct kefir_asmcmp_pipeline_pass *pass) {
    UNUSED(mem);
    UNUSED(context);
    UNUSED(pass);

    for (kefir_asmcmp_instruction_index_t instr_index = kefir_asmcmp_context_instr_head(context), next_instr_index;
         instr_index != KEFIR_ASMCMP_INDEX_NONE; instr_index = next_instr_index) {

        struct kefir_asmcmp_instruction *instr;
        REQUIRE_OK(kefir_asmcmp_context_instr_at(context, instr_index, &instr));

        next_instr_index = kefir_asmcmp_context_instr_next(context, instr_index);

        kefir_bool_t drop_instr = false;
        switch (instr->opcode) {
            case KEFIR_ASMCMP_AMD64_OPCODE(touch_virtual_register):
            case KEFIR_ASMCMP_AMD64_OPCODE(vreg_lifetime_range_begin):
            case KEFIR_ASMCMP_AMD64_OPCODE(vreg_lifetime_range_end):
            case KEFIR_ASMCMP_AMD64_OPCODE(noop):
                drop_instr = true;
                break;

            case KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link):
                if (instr->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER &&
                    instr->args[1].type == KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER &&
                    instr->args[0].phreg == instr->args[1].phreg) {
                    drop_instr = true;
                }
                break;

            default:
                // Intentionally left blank
                break;
        }

        if (drop_instr) {
            if (next_instr_index != KEFIR_ASMCMP_INDEX_NONE) {
                REQUIRE_OK(kefir_asmcmp_context_move_labels(mem, context, next_instr_index, instr_index));
                REQUIRE_OK(kefir_asmcmp_context_instr_drop(context, instr_index));
            } else {
                instr->opcode = KEFIR_ASMCMP_AMD64_OPCODE(noop);
            }
        }
    }
    return KEFIR_OK;
}

const struct kefir_asmcmp_pipeline_pass KefirAsmcmpAmd64DropVirtualPass = {
    .name = "amd64-drop-virtual", .type = KEFIR_ASMCMP_PIPELINE_PASS_DEVIRTUAL, .apply = drop_virtual_apply, .payload = NULL};
