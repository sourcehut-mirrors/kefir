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
#include <string.h>

static kefir_result_t propagate_jump_impl(struct kefir_mem *mem, struct kefir_asmcmp_context *context, struct kefir_hashtreeset *jump_chain) {
    for (kefir_asmcmp_instruction_index_t instr_index = kefir_asmcmp_context_instr_head(context), next_instr_index;
         instr_index != KEFIR_ASMCMP_INDEX_NONE; instr_index = next_instr_index) {

        kefir_asmcmp_instruction_index_t jump_target_instr_idx;
        struct kefir_asmcmp_instruction *instr, *jump_target_instr;
        REQUIRE_OK(kefir_asmcmp_context_instr_at(context, instr_index, &instr));

        next_instr_index = kefir_asmcmp_context_instr_next(context, instr_index);

        switch (instr->opcode) {
            case KEFIR_ASMCMP_AMD64_OPCODE(jmp):
            case KEFIR_ASMCMP_AMD64_OPCODE(jz):
            case KEFIR_ASMCMP_AMD64_OPCODE(jnz):
            case KEFIR_ASMCMP_AMD64_OPCODE(je):
            case KEFIR_ASMCMP_AMD64_OPCODE(jne):
            case KEFIR_ASMCMP_AMD64_OPCODE(js):
            case KEFIR_ASMCMP_AMD64_OPCODE(jns):
            case KEFIR_ASMCMP_AMD64_OPCODE(jp):
            case KEFIR_ASMCMP_AMD64_OPCODE(jnp):
            case KEFIR_ASMCMP_AMD64_OPCODE(ja):
            case KEFIR_ASMCMP_AMD64_OPCODE(jae):
            case KEFIR_ASMCMP_AMD64_OPCODE(jb):
            case KEFIR_ASMCMP_AMD64_OPCODE(jbe):
            case KEFIR_ASMCMP_AMD64_OPCODE(jg):
            case KEFIR_ASMCMP_AMD64_OPCODE(jge):
            case KEFIR_ASMCMP_AMD64_OPCODE(jl):
            case KEFIR_ASMCMP_AMD64_OPCODE(jle):
                if (instr->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL) {
                    REQUIRE_OK(kefir_asmcmp_context_label_at(context, instr->args[0].internal_label, &jump_target_instr_idx));
                    REQUIRE_OK(kefir_asmcmp_context_instr_at(context, jump_target_instr_idx, &jump_target_instr));

                    if (jump_target_instr->opcode == KEFIR_ASMCMP_AMD64_OPCODE(jmp) && !kefir_hashtreeset_has(jump_chain, jump_target_instr_idx)) {
                        instr->args[0] = jump_target_instr->args[0];
                        next_instr_index = instr_index;
                        REQUIRE_OK(kefir_hashtreeset_add(mem, jump_chain, (kefir_hashtreeset_entry_t) jump_target_instr_idx));
                    } else {
                        REQUIRE_OK(kefir_hashtreeset_clean(mem, jump_chain));
                    }
                } else {
                    REQUIRE_OK(kefir_hashtreeset_clean(mem, jump_chain));
                }
                break;

            default:
                REQUIRE_OK(kefir_hashtreeset_clean(mem, jump_chain));
                break;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t propagate_jump_apply(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                 const struct kefir_asmcmp_pipeline_pass *pass) {
    UNUSED(mem);
    UNUSED(context);
    UNUSED(pass);

    struct kefir_hashtreeset jump_chain;
    REQUIRE_OK(kefir_hashtreeset_init(&jump_chain, &kefir_hashtree_uint_ops));
    kefir_result_t res = propagate_jump_impl(mem, context, &jump_chain);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &jump_chain);
        return res;
    });
    REQUIRE_OK(kefir_hashtreeset_free(mem, &jump_chain));
    return KEFIR_OK;
}

const struct kefir_asmcmp_pipeline_pass KefirAsmcmpAmd64PropagateJumpPass = {
    .name = "amd64-propagate-jump", .type = KEFIR_ASMCMP_PIPELINE_PASS_DEVIRTUAL, .apply = propagate_jump_apply, .payload = NULL};
