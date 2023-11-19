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

#define KEFIR_CODEGEN_ASMCMP_PIPELINE_INTERNAL
#include "kefir/codegen/asmcmp/pipeline.h"
#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define IS_INT(_op, _value) ((_op)->type == KEFIR_ASMCMP_VALUE_TYPE_INTEGER && (_op)->int_immediate == (_value))
#define IS_UINT(_op, _value) ((_op)->type == KEFIR_ASMCMP_VALUE_TYPE_UINTEGER && (_op)->uint_immediate == (_value))

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
            case KEFIR_ASMCMP_AMD64_OPCODE(mov):
                if (instr->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER &&
                    (IS_INT(&instr->args[1], 0) || IS_UINT(&instr->args[1], 0))) {
                    instr->opcode = KEFIR_ASMCMP_AMD64_OPCODE(xor);
                    instr->args[1] = KEFIR_ASMCMP_MAKE_PHREG(instr->args[0].phreg);
                }
                break;

            case KEFIR_ASMCMP_AMD64_OPCODE(add):
            case KEFIR_ASMCMP_AMD64_OPCODE(sub):
                if (IS_INT(&instr->args[1], 0) || IS_UINT(&instr->args[1], 0)) {
                    REQUIRE_OK(kefir_asmcmp_context_move_labels(mem, context, next_instr_index, instr_index));
                    REQUIRE_OK(kefir_asmcmp_context_instr_drop(context, instr_index));
                }
                break;

            case KEFIR_ASMCMP_AMD64_OPCODE(imul3):
                if (IS_INT(&instr->args[2], 1) || IS_UINT(&instr->args[2], 1)) {
                    instr->opcode = KEFIR_ASMCMP_AMD64_OPCODE(mov);
                }
                break;

            case KEFIR_ASMCMP_AMD64_OPCODE(lea):
                if (instr->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER &&
                    instr->args[1].type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT &&
                    instr->args[1].indirect.variant == KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT) {
                    ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t, reg, instr->args[0].phreg);

                    struct kefir_asmcmp_instruction *next_instr;
                    REQUIRE_OK(kefir_asmcmp_context_instr_at(context, next_instr_index, &next_instr));
                    if (next_instr->opcode == KEFIR_ASMCMP_AMD64_OPCODE(mov) &&
                        next_instr->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER &&
                        next_instr->args[0].phreg == reg &&
                        next_instr->args[1].type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT &&
                        next_instr->args[1].indirect.type == KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS &&
                        next_instr->args[1].indirect.base.phreg == reg) {

                        kefir_asmcmp_operand_variant_t variant = next_instr->args[1].indirect.variant;
                        kefir_int64_t offset = next_instr->args[1].indirect.offset;
                        next_instr->args[1] = instr->args[1];
                        next_instr->args[1].indirect.variant = variant;
                        next_instr->args[1].indirect.offset += offset;
                        REQUIRE_OK(kefir_asmcmp_context_move_labels(mem, context, next_instr_index, instr_index));
                        REQUIRE_OK(kefir_asmcmp_context_instr_drop(context, instr_index));
                        next_instr_index = kefir_asmcmp_context_instr_next(context, next_instr_index);
                    }
                }
                break;

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
