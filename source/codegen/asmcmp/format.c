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

#include "kefir/codegen/asmcmp/format.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t variant_format(struct kefir_json_output *json, kefir_asmcmp_operand_variant_t variant) {
    switch (variant) {
        case KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT:
            REQUIRE_OK(kefir_json_output_string(json, "default"));
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_8BIT:
            REQUIRE_OK(kefir_json_output_string(json, "8bit"));
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_16BIT:
            REQUIRE_OK(kefir_json_output_string(json, "16bit"));
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_32BIT:
            REQUIRE_OK(kefir_json_output_string(json, "32bit"));
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_64BIT:
            REQUIRE_OK(kefir_json_output_string(json, "64bit"));
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_80BIT:
            REQUIRE_OK(kefir_json_output_string(json, "80bit"));
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_128BIT:
            REQUIRE_OK(kefir_json_output_string(json, "128bit"));
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE:
            REQUIRE_OK(kefir_json_output_string(json, "fp_single"));
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE:
            REQUIRE_OK(kefir_json_output_string(json, "fp_double"));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t value_format(struct kefir_json_output *json, const struct kefir_asmcmp_context *context,
                                   const struct kefir_asmcmp_value *value) {
    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
            REQUIRE_OK(kefir_json_output_null(json));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "integer"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_integer(json, value->int_immediate));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "uinteger"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_uinteger(json, value->uint_immediate));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER: {
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "physical_register"));
            REQUIRE_OK(kefir_json_output_object_key(json, "reg"));
            const char *mnemonic;
            REQUIRE_OK(context->klass->register_mnemonic(value->phreg, &mnemonic, context->payload));
            REQUIRE_OK(kefir_json_output_string(json, mnemonic));
            REQUIRE_OK(kefir_json_output_object_end(json));
        } break;

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "virtual_register"));
            REQUIRE_OK(kefir_json_output_object_key(json, "index"));
            REQUIRE_OK(kefir_json_output_uinteger(json, value->vreg.index));
            REQUIRE_OK(kefir_json_output_object_key(json, "variant"));
            REQUIRE_OK(variant_format(json, value->vreg.variant));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "indirect"));
            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "physical_register"));
                    REQUIRE_OK(kefir_json_output_object_key(json, "reg"));
                    const char *mnemonic;
                    REQUIRE_OK(
                        context->klass->register_mnemonic(value->indirect.base.phreg, &mnemonic, context->payload));
                    REQUIRE_OK(kefir_json_output_string(json, mnemonic));
                    break;

                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "virtual_register"));
                    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
                    REQUIRE_OK(kefir_json_output_uinteger(json, value->indirect.base.vreg));
                    break;

                case KEFIR_ASMCMP_INDIRECT_LABEL_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "label"));
                    REQUIRE_OK(kefir_json_output_object_key(json, "label"));
                    REQUIRE_OK(kefir_json_output_string(json, value->indirect.base.label));
                    break;

                case KEFIR_ASMCMP_INDIRECT_LOCAL_VAR_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "local_var"));
                    break;

                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "spill_area"));
                    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
                    REQUIRE_OK(kefir_json_output_uinteger(json, value->indirect.base.spill_index));
                    break;

                case KEFIR_ASMCMP_INDIRECT_TEMPORARY_AREA_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "temporary_area"));
                    break;

                case KEFIR_ASMCMP_INDIRECT_VARARG_SAVE_AREA_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "vararg_save_area"));
                    break;
            }
            REQUIRE_OK(kefir_json_output_object_key(json, "offset"));
            REQUIRE_OK(kefir_json_output_integer(json, value->indirect.offset));
            REQUIRE_OK(kefir_json_output_object_key(json, "variant"));
            REQUIRE_OK(variant_format(json, value->indirect.variant));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "rip_indirect"));
            REQUIRE_OK(kefir_json_output_object_key(json, "base"));
            REQUIRE_OK(kefir_json_output_string(json, value->rip_indirection.base));
            REQUIRE_OK(kefir_json_output_object_key(json, "variant"));
            REQUIRE_OK(variant_format(json, value->rip_indirection.variant));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_LABEL:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "label"));
            REQUIRE_OK(kefir_json_output_object_key(json, "label"));
            REQUIRE_OK(kefir_json_output_string(json, value->label.symbolic));
            REQUIRE_OK(kefir_json_output_object_key(json, "offset"));
            REQUIRE_OK(kefir_json_output_integer(json, value->label.offset));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_X87:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "x87"));
            REQUIRE_OK(kefir_json_output_object_key(json, "index"));
            REQUIRE_OK(kefir_json_output_uinteger(json, value->x87));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_STASH_INDEX:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "stash_index"));
            REQUIRE_OK(kefir_json_output_object_key(json, "index"));
            REQUIRE_OK(kefir_json_output_uinteger(json, value->stash_idx));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INLINE_ASSEMBLY_INDEX:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "inline_asm_index"));
            REQUIRE_OK(kefir_json_output_object_key(json, "index"));
            REQUIRE_OK(kefir_json_output_uinteger(json, value->inline_asm_idx));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_context_format(struct kefir_json_output *json, const struct kefir_asmcmp_context *context) {
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid json output"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp context"));

    REQUIRE_OK(kefir_json_output_object_begin(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "code"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (kefir_asmcmp_instruction_index_t instr_idx = kefir_asmcmp_context_instr_head(context);
         instr_idx != KEFIR_ASMCMP_INDEX_NONE; instr_idx = kefir_asmcmp_context_instr_next(context, instr_idx)) {
        struct kefir_asmcmp_instruction *instr;
        REQUIRE_OK(kefir_asmcmp_context_instr_at(context, instr_idx, &instr));

        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
        const char *opcode;
        REQUIRE_OK(context->klass->opcode_mnemonic(instr->opcode, &opcode, context->payload));
        REQUIRE_OK(kefir_json_output_string(json, opcode));

        REQUIRE_OK(kefir_json_output_object_key(json, "args"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        if (instr->args[0].type != KEFIR_ASMCMP_VALUE_TYPE_NONE ||
            instr->args[1].type != KEFIR_ASMCMP_VALUE_TYPE_NONE ||
            instr->args[2].type != KEFIR_ASMCMP_VALUE_TYPE_NONE) {
            REQUIRE_OK(value_format(json, context, &instr->args[0]));
        }
        if (instr->args[1].type != KEFIR_ASMCMP_VALUE_TYPE_NONE ||
            instr->args[2].type != KEFIR_ASMCMP_VALUE_TYPE_NONE) {
            REQUIRE_OK(value_format(json, context, &instr->args[1]));
        }
        if (instr->args[2].type != KEFIR_ASMCMP_VALUE_TYPE_NONE) {
            REQUIRE_OK(value_format(json, context, &instr->args[2]));
        }
        REQUIRE_OK(kefir_json_output_array_end(json));

        REQUIRE_OK(kefir_json_output_object_key(json, "labels"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        for (kefir_asmcmp_label_index_t label_idx = kefir_asmcmp_context_instr_label_head(context, instr_idx);
             label_idx != KEFIR_ASMCMP_INDEX_NONE;
             label_idx = kefir_asmcmp_context_instr_label_next(context, label_idx)) {
            REQUIRE_OK(kefir_json_output_uinteger(json, label_idx));
        }
        REQUIRE_OK(kefir_json_output_array_end(json));

        REQUIRE_OK(kefir_json_output_object_end(json));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}
