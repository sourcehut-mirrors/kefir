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

#define KEFIR_CODEGEN_ASMCMP_PIPELINE_INTERNAL
#include "kefir/codegen/asmcmp/pipeline.h"
#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define IS_INT(_op, _value) ((_op)->type == KEFIR_ASMCMP_VALUE_TYPE_INTEGER && (_op)->int_immediate == (_value))

static kefir_result_t amd64_peephole_apply(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                           const struct kefir_asmcmp_pipeline_pass *pass) {
    UNUSED(mem);
    UNUSED(context);
    UNUSED(pass);
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp contet"));
    REQUIRE(context->klass == &KEFIR_ASMCMP_AMD64_KLASS,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected amd64 asmcmp context"));

    for (kefir_asmcmp_instruction_index_t instr_index = kefir_asmcmp_context_instr_head(context);
         instr_index != KEFIR_ASMCMP_INDEX_NONE;) {

        struct kefir_asmcmp_instruction *instr;
        REQUIRE_OK(kefir_asmcmp_context_instr_at(context, instr_index, &instr));

        kefir_asmcmp_instruction_index_t next_instr_index = kefir_asmcmp_context_instr_next(context, instr_index);

        switch (instr->opcode) {
            case KEFIR_ASMCMP_AMD64_OPCODE(jmp): {
                struct kefir_asmcmp_instruction *next_instr = NULL;
                if (next_instr_index != KEFIR_ASMCMP_INDEX_NONE) {
                    REQUIRE_OK(kefir_asmcmp_context_instr_at(context, next_instr_index, &next_instr));
                }
                if (next_instr != NULL && next_instr->opcode == KEFIR_ASMCMP_AMD64_OPCODE(jmp) &&
                    next_instr->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL) {
                    kefir_asmcmp_label_index_t label = next_instr->args[0].internal_label;

                    kefir_bool_t drop_instr = true;
                    for (kefir_asmcmp_label_index_t instr_label =
                             kefir_asmcmp_context_instr_label_head(context, next_instr_index);
                         instr_label != KEFIR_ASMCMP_INDEX_NONE;) {

                        if (instr_label == label) {
                            drop_instr = false;
                            instr_label = kefir_asmcmp_context_instr_label_next(context, instr_label);
                            continue;
                        }

                        const struct kefir_asmcmp_label *asmlabel;
                        REQUIRE_OK(kefir_asmcmp_context_get_label(context, instr_label, &asmlabel));

                        if (!asmlabel->external_dependencies) {
                            REQUIRE_OK(kefir_asmcmp_replace_labels(context, label, instr_label));
                            kefir_asmcmp_label_index_t next_label =
                                kefir_asmcmp_context_instr_label_next(context, instr_label);
                            REQUIRE_OK(kefir_asmcmp_context_unbind_label(mem, context, instr_label));
                            instr_label = next_label;
                        } else {
                            drop_instr = false;
                            instr_label = kefir_asmcmp_context_instr_label_next(context, instr_label);
                            continue;
                        }
                    }

                    if (drop_instr) {
                        REQUIRE_OK(kefir_asmcmp_context_instr_drop(context, next_instr_index));
                        next_instr_index = instr_index;
                    }
                }
            } break;

            default:
                break;
        }

        instr_index = next_instr_index;
    }
    return KEFIR_OK;
}

const struct kefir_asmcmp_pipeline_pass KefirAsmcmpAmd64PeepholePass = {.name = "amd64-peephole",
                                                                        .type = KEFIR_ASMCMP_PIPELINE_PASS_DEVIRTUAL,
                                                                        .apply = amd64_peephole_apply,
                                                                        .payload = NULL};
