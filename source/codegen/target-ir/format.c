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

#include "kefir/codegen/target-ir/format.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t id_format(struct kefir_json_output *json, kefir_id_t id) {
    if (id == KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_json_output_null(json));
    } else {
        REQUIRE_OK(kefir_json_output_uinteger(json, id));
    }
    return KEFIR_OK;
}
static kefir_result_t variant_format(struct kefir_json_output *json, kefir_codegen_target_ir_operand_variant_t variant) {
    switch (variant) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT:
            REQUIRE_OK(kefir_json_output_string(json, "default"));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT:
            REQUIRE_OK(kefir_json_output_string(json, "8bit"));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT_HIGHER:
            REQUIRE_OK(kefir_json_output_string(json, "8bit_high"));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT:
            REQUIRE_OK(kefir_json_output_string(json, "16bit"));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT:
            REQUIRE_OK(kefir_json_output_string(json, "32bit"));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT:
            REQUIRE_OK(kefir_json_output_string(json, "64bit"));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_80BIT:
            REQUIRE_OK(kefir_json_output_string(json, "80bit"));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_128BIT:
            REQUIRE_OK(kefir_json_output_string(json, "128bit"));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_FP_SINGLE:
            REQUIRE_OK(kefir_json_output_string(json, "fp_single"));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_FP_DOUBLE:
            REQUIRE_OK(kefir_json_output_string(json, "fp_double"));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t label_type_format(struct kefir_json_output *json, kefir_codegen_target_ir_external_label_relocation_t type) {
    switch (type) {
        case KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_ABSOLUTE:
            REQUIRE_OK(kefir_json_output_string(json, "absolute"));
            break;

        case KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_PLT:
            REQUIRE_OK(kefir_json_output_string(json, "plt"));
            break;

        case KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_GOTPCREL:
            REQUIRE_OK(kefir_json_output_string(json, "gotpcrel"));
            break;

        case KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_TPOFF:
            REQUIRE_OK(kefir_json_output_string(json, "tpoff"));
            break;

        case KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_GOTTPOFF:
            REQUIRE_OK(kefir_json_output_string(json, "gottpoff"));
            break;

        case KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_TLSGD:
            REQUIRE_OK(kefir_json_output_string(json, "tlsgd"));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t operand_format(struct kefir_json_output *json, const struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_operand *operand) {
    switch (operand->type) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE:
            // Intentionally left blank
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "integer"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_integer(json, operand->int_immediate));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UINTEGER:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "uinteger"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_uinteger(json, operand->uint_immediate));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_PHYSICAL_REGISTER: {
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "physical_register"));
            REQUIRE_OK(kefir_json_output_object_key(json, "reg"));
            const char *mnemonic;
            REQUIRE_OK(code->klass->register_mnemonic(operand->phreg, &mnemonic, code->klass->payload));
            REQUIRE_OK(kefir_json_output_string(json, mnemonic));
            REQUIRE_OK(kefir_json_output_object_end(json));
        } break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "value_ref"));
            REQUIRE_OK(kefir_json_output_object_key(json, "instr_ref"));
            REQUIRE_OK(id_format(json, operand->direct.value_ref.instr_ref));
            REQUIRE_OK(kefir_json_output_object_key(json, "aspect"));
            REQUIRE_OK(kefir_json_output_uinteger(json, operand->direct.value_ref.aspect));
            REQUIRE_OK(kefir_json_output_object_key(json, "variant"));
            REQUIRE_OK(variant_format(json, operand->direct.variant));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "indirect"));
            switch (operand->indirect.type) {
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_PHYSICAL_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "physical_register"));
                    REQUIRE_OK(kefir_json_output_object_key(json, "reg"));
                    const char *mnemonic;
                    REQUIRE_OK(
                        code->klass->register_mnemonic(operand->indirect.base.phreg, &mnemonic, code->klass->payload));
                    REQUIRE_OK(kefir_json_output_string(json, mnemonic));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "value_ref"));
                    REQUIRE_OK(kefir_json_output_object_key(json, "instr_ref"));
                    REQUIRE_OK(kefir_json_output_uinteger(json, operand->indirect.base.value_ref.instr_ref));
                    REQUIRE_OK(kefir_json_output_object_key(json, "aspect"));
                    REQUIRE_OK(kefir_json_output_uinteger(json, operand->indirect.base.value_ref.aspect));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "block_ref"));
                    REQUIRE_OK(kefir_json_output_object_key(json, "block_ref"));
                    REQUIRE_OK(id_format(json, operand->indirect.base.block_ref));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_ASMCMP_LABEL_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "asmcmp_label"));
                    REQUIRE_OK(kefir_json_output_object_key(json, "block_ref"));
                    REQUIRE_OK(id_format(json, operand->indirect.base.asmcmp_label));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_EXTERNAL_LABEL_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "external_label"));
                    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
                    REQUIRE_OK(label_type_format(json, operand->indirect.base.external_type));
                    REQUIRE_OK(kefir_json_output_object_key(json, "label"));
                    REQUIRE_OK(kefir_json_output_string(json, operand->indirect.base.external_label));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_LOCAL_AREA_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "local_var"));
                    REQUIRE_OK(kefir_json_output_object_key(json, "identifier"));
                    REQUIRE_OK(kefir_json_output_integer(json, operand->indirect.base.local_variable_id));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_SPILL_AREA_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "spill_area"));
                    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
                    REQUIRE_OK(kefir_json_output_uinteger(json, operand->indirect.base.spill_index));
                    break;
            }
            REQUIRE_OK(kefir_json_output_object_key(json, "offset"));
            REQUIRE_OK(kefir_json_output_integer(json, operand->indirect.offset));
            REQUIRE_OK(kefir_json_output_object_key(json, "variant"));
            REQUIRE_OK(variant_format(json, operand->indirect.variant));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "rip_indirect_block_ref"));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(label_type_format(json, operand->rip_indirection.position));
            REQUIRE_OK(kefir_json_output_object_key(json, "base"));
            REQUIRE_OK(id_format(json, operand->rip_indirection.block_ref));
            REQUIRE_OK(kefir_json_output_object_key(json, "variant"));
            REQUIRE_OK(variant_format(json, operand->rip_indirection.variant));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_ASMCMP:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "rip_indirect_asmcmp"));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(label_type_format(json, operand->rip_indirection.position));
            REQUIRE_OK(kefir_json_output_object_key(json, "base"));
            REQUIRE_OK(kefir_json_output_uinteger(json, operand->rip_indirection.asmcmp_label));
            REQUIRE_OK(kefir_json_output_object_key(json, "variant"));
            REQUIRE_OK(variant_format(json, operand->rip_indirection.variant));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_EXTERNAL:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "rip_indirect_external"));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(label_type_format(json, operand->rip_indirection.position));
            REQUIRE_OK(kefir_json_output_object_key(json, "base"));
            REQUIRE_OK(kefir_json_output_string(json, operand->rip_indirection.external));
            REQUIRE_OK(kefir_json_output_object_key(json, "variant"));
            REQUIRE_OK(variant_format(json, operand->rip_indirection.variant));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "block_ref"));
            REQUIRE_OK(kefir_json_output_object_key(json, "block_ref"));
            REQUIRE_OK(kefir_json_output_uinteger(json, operand->block_ref));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_ASMCMP_LABEL:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "asmcmp_label"));
            REQUIRE_OK(kefir_json_output_object_key(json, "label"));
            REQUIRE_OK(kefir_json_output_uinteger(json, operand->asmcmp_label));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_EXTERNAL_LABEL:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "external_label"));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(label_type_format(json, operand->external_label.position));
            REQUIRE_OK(kefir_json_output_object_key(json, "label"));
            REQUIRE_OK(kefir_json_output_string(json, operand->external_label.symbolic));
            REQUIRE_OK(kefir_json_output_object_key(json, "offset"));
            REQUIRE_OK(kefir_json_output_integer(json, operand->external_label.offset));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "x87"));
            REQUIRE_OK(kefir_json_output_object_key(json, "index"));
            REQUIRE_OK(kefir_json_output_uinteger(json, operand->x87));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_STASH_INDEX:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "stash_index"));
            REQUIRE_OK(kefir_json_output_object_key(json, "index"));
            REQUIRE_OK(kefir_json_output_uinteger(json, operand->stash_idx));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INLINE_ASSEMBLY_INDEX:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "inline_asm_index"));
            REQUIRE_OK(kefir_json_output_object_key(json, "index"));
            REQUIRE_OK(kefir_json_output_uinteger(json, operand->inline_asm_idx));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_format(const struct kefir_codegen_target_ir_code *code, struct kefir_json_output *json) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid json output"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "entry_point"));
    REQUIRE_OK(id_format(json, code->entry_block));
    REQUIRE_OK(kefir_json_output_object_key(json, "blocks"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_at(code, i);
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "block_ref"));
        REQUIRE_OK(id_format(json, block_ref));
        REQUIRE_OK(kefir_json_output_object_key(json, "code"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(code, block_ref);
            instr_ref != KEFIR_ID_NONE;
            instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref)) {
            REQUIRE_OK(kefir_json_output_object_begin(json));        
            REQUIRE_OK(kefir_json_output_object_key(json, "instr_ref"));
            REQUIRE_OK(id_format(json, instr_ref));
            const struct kefir_codegen_target_ir_instruction *instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));
            const char *mnemonic;
            REQUIRE_OK(code->klass->opcode_mnemonic(instr->operation.opcode, &mnemonic, code->klass->payload));
            REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
            REQUIRE_OK(kefir_json_output_string(json, mnemonic));
            REQUIRE_OK(kefir_json_output_object_key(json, "parameters"));
            REQUIRE_OK(kefir_json_output_array_begin(json));
            for (kefir_size_t j = 0; j < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; j++) {
                REQUIRE_OK(operand_format(json, code, &instr->operation.parameters[j]));
            }
            REQUIRE_OK(kefir_json_output_array_end(json));
            REQUIRE_OK(kefir_json_output_object_end(json));
        }
        REQUIRE_OK(kefir_json_output_array_end(json));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}
