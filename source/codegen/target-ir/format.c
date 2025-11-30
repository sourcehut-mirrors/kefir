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

static kefir_result_t aspect_format(struct kefir_json_output *json, kefir_uint32_t aspect) {
    if (aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_NONE) {
        REQUIRE_OK(kefir_json_output_null(json));
    } else if (aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_FLAGS) {
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "type"));
        REQUIRE_OK(kefir_json_output_string(json, "flags"));
        REQUIRE_OK(kefir_json_output_object_end(json));
    } else if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(aspect)) {
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "type"));
        REQUIRE_OK(kefir_json_output_string(json, "direct_output"));
        REQUIRE_OK(kefir_json_output_object_key(json, "index"));
        REQUIRE_OK(kefir_json_output_uinteger(json, KEFIR_CODEGEN_TARGET_IR_VALUE_GET_OUTPUT_INDEX(aspect)));
        REQUIRE_OK(kefir_json_output_object_end(json));
    } else if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_INDIRECT_OUTPUT(aspect)) {
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "type"));
        REQUIRE_OK(kefir_json_output_string(json, "indirect_ouput"));
        REQUIRE_OK(kefir_json_output_object_key(json, "index"));
        REQUIRE_OK(kefir_json_output_uinteger(json, KEFIR_CODEGEN_TARGET_IR_VALUE_GET_OUTPUT_INDEX(aspect)));
        REQUIRE_OK(kefir_json_output_object_end(json));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected target IR value aspect");
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
            REQUIRE_OK(kefir_json_output_integer(json, operand->immediate.int_immediate));
            REQUIRE_OK(kefir_json_output_object_key(json, "variant"));
            REQUIRE_OK(variant_format(json, operand->immediate.variant));
            REQUIRE_OK(kefir_json_output_object_end(json));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UINTEGER:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "uinteger"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_uinteger(json, operand->immediate.uint_immediate));
            REQUIRE_OK(kefir_json_output_object_key(json, "variant"));
            REQUIRE_OK(variant_format(json, operand->immediate.variant));
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
            REQUIRE_OK(aspect_format(json, operand->direct.value_ref.aspect));
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
                    REQUIRE_OK(aspect_format(json, operand->indirect.base.value_ref.aspect));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_IMMEDIATE_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "immediate"));
                    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                    REQUIRE_OK(kefir_json_output_uinteger(json, operand->indirect.base.immediate));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "block_ref"));
                    REQUIRE_OK(kefir_json_output_object_key(json, "block_ref"));
                    REQUIRE_OK(id_format(json, operand->indirect.base.block_ref));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_NATIVE_LABEL_BASIS:
                    REQUIRE_OK(kefir_json_output_object_key(json, "basis"));
                    REQUIRE_OK(kefir_json_output_string(json, "native"));
                    REQUIRE_OK(kefir_json_output_object_key(json, "block_ref"));
                    REQUIRE_OK(id_format(json, operand->indirect.base.native_id));
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

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_NATIVE:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "rip_indirect_native"));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(label_type_format(json, operand->rip_indirection.position));
            REQUIRE_OK(kefir_json_output_object_key(json, "base"));
            REQUIRE_OK(kefir_json_output_uinteger(json, operand->rip_indirection.native_id));
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

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NATIVE_LABEL:
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "native"));
            REQUIRE_OK(kefir_json_output_object_key(json, "label"));
            REQUIRE_OK(kefir_json_output_uinteger(json, operand->native_id));
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
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_format(const struct kefir_codegen_target_ir_code *code, struct kefir_json_output *json) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid json output"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "entry_point"));
    REQUIRE_OK(id_format(json, code->entry_block));
    if (code->indirect_jump_gate_block != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_json_output_object_key(json, "indirect_jump_gate"));
        REQUIRE_OK(id_format(json, code->indirect_jump_gate_block));
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "blocks"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
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

            kefir_result_t res;
            if (instr->operation.opcode == code->klass->phi_opcode) {
                REQUIRE_OK(kefir_json_output_object_key(json, "phi_links"));
                REQUIRE_OK(kefir_json_output_array_begin(json));
                struct kefir_codegen_target_ir_value_phi_link_iterator iter;
                kefir_codegen_target_ir_block_ref_t link_block_ref;
                struct kefir_codegen_target_ir_value_ref link_value_ref;
                for (res = kefir_codegen_target_ir_code_phi_link_iter(code, &iter, instr_ref, &link_block_ref, &link_value_ref);
                    res == KEFIR_OK;
                    res = kefir_codegen_target_ir_code_phi_link_next(&iter, &link_block_ref, &link_value_ref)) {
                    REQUIRE_OK(kefir_json_output_object_begin(json));
                    REQUIRE_OK(kefir_json_output_object_key(json, "block_ref"));
                    REQUIRE_OK(id_format(json, link_block_ref));
                    REQUIRE_OK(kefir_json_output_object_key(json, "value_ref"));
                    REQUIRE_OK(kefir_json_output_object_begin(json));
                    REQUIRE_OK(kefir_json_output_object_key(json, "instr_ref"));
                    REQUIRE_OK(id_format(json, link_value_ref.instr_ref));
                    REQUIRE_OK(kefir_json_output_object_key(json, "aspect"));
                    REQUIRE_OK(aspect_format(json, link_value_ref.aspect));
                    REQUIRE_OK(kefir_json_output_object_end(json));
                    REQUIRE_OK(kefir_json_output_object_end(json));
                }
                if (res != KEFIR_ITERATOR_END) {
                    REQUIRE_OK(res);
                }
                REQUIRE_OK(kefir_json_output_array_end(json));
            } else if (instr->operation.opcode == code->klass->inline_asm_opcode) {
                REQUIRE_OK(kefir_json_output_object_key(json, "target_block_ref"));
                REQUIRE_OK(id_format(json, instr->operation.inline_asm_node.target_block_ref));
                REQUIRE_OK(kefir_json_output_object_key(json, "fragments"));
                REQUIRE_OK(kefir_json_output_array_begin(json));
                struct kefir_codegen_target_ir_code_inline_assembly_fragment_iterator iter;
                const struct kefir_codegen_target_ir_inline_assembly_fragment *fragment;
                for (res = kefir_codegen_target_ir_code_inline_assembly_fragment_iter(code, &iter, instr_ref, &fragment);
                    res == KEFIR_OK;
                    res = kefir_codegen_target_ir_code_inline_assembly_fragment_next(&iter, &fragment)) {
                    REQUIRE_OK(kefir_json_output_object_begin(json));
                    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
                    switch (fragment->type) {
                        case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_TEXT:
                            REQUIRE_OK(kefir_json_output_string(json, "text"));
                            REQUIRE_OK(kefir_json_output_object_key(json, "text"));
                            REQUIRE_OK(kefir_json_output_string(json, fragment->text));
                            break;

                        case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_OPERAND:
                            REQUIRE_OK(kefir_json_output_string(json, "operand"));
                            REQUIRE_OK(kefir_json_output_object_key(json, "operand"));
                            REQUIRE_OK(operand_format(json, code, &fragment->operand));
                            break;
                    }
                    REQUIRE_OK(kefir_json_output_object_end(json));
                }
                if (res != KEFIR_ITERATOR_END) {
                    REQUIRE_OK(res);
                }
                REQUIRE_OK(kefir_json_output_array_end(json));
            } else {
                REQUIRE_OK(kefir_json_output_object_key(json, "parameters"));
                REQUIRE_OK(kefir_json_output_array_begin(json));
                for (kefir_size_t j = 0; j < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; j++) {
                    REQUIRE_OK(operand_format(json, code, &instr->operation.parameters[j]));
                }
                REQUIRE_OK(kefir_json_output_array_end(json));
            }

            REQUIRE_OK(kefir_json_output_object_key(json, "values"));
            REQUIRE_OK(kefir_json_output_array_begin(json));
            struct kefir_codegen_target_ir_value_iterator value_iter;
            struct kefir_codegen_target_ir_value_ref value_ref;
            const struct kefir_codegen_target_ir_value_type *value_type;
            for (res = kefir_codegen_target_ir_code_value_iter(code, &value_iter, instr_ref, &value_ref, &value_type);
                res == KEFIR_OK;
                res = kefir_codegen_target_ir_code_value_next(&value_iter, &value_ref, &value_type)) {
                REQUIRE_OK(kefir_json_output_object_begin(json));
                REQUIRE_OK(kefir_json_output_object_key(json, "aspect"));
                REQUIRE_OK(aspect_format(json, value_ref.aspect));
                REQUIRE_OK(kefir_json_output_object_key(json, "type"));
                REQUIRE_OK(kefir_json_output_object_begin(json));
                REQUIRE_OK(kefir_json_output_object_key(json, "variant"));
                REQUIRE_OK(variant_format(json, value_type->variant));
                REQUIRE_OK(kefir_json_output_object_key(json, "kind"));
                switch (value_type->kind) {
                    case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_UNSPECIFIED:
                        REQUIRE_OK(kefir_json_output_string(json, "unspecified"));
                        break;

                    case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE:
                        REQUIRE_OK(kefir_json_output_string(json, "general_purpose"));
                        break;

                    case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLAGS:
                        REQUIRE_OK(kefir_json_output_string(json, "flags"));
                        break;

                    case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT:
                        REQUIRE_OK(kefir_json_output_string(json, "indirect"));
                        break;

                    case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT:
                        REQUIRE_OK(kefir_json_output_string(json, "floating_point"));
                        break;

                    case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE:
                        REQUIRE_OK(kefir_json_output_string(json, "spill_space"));
                        REQUIRE_OK(kefir_json_output_object_key(json, "alignment"));
                        REQUIRE_OK(kefir_json_output_uinteger(json, value_type->parameters.spill_space_allocation.alignment));
                        REQUIRE_OK(kefir_json_output_object_key(json, "length"));
                        REQUIRE_OK(kefir_json_output_uinteger(json, value_type->parameters.spill_space_allocation.length));
                        break;

                    case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE:
                        REQUIRE_OK(kefir_json_output_string(json, "local_variable"));
                        REQUIRE_OK(kefir_json_output_object_key(json, "identifier"));
                        REQUIRE_OK(id_format(json, value_type->parameters.local_variable.identifier));
                        REQUIRE_OK(kefir_json_output_object_key(json, "offset"));
                        REQUIRE_OK(kefir_json_output_uinteger(json, value_type->parameters.local_variable.offset));
                        break;

                    case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_EXTERNAL_MEMORY: {
                        const char *mnemonic;
                        REQUIRE_OK(code->klass->register_mnemonic(value_type->parameters.memory.base_reg, &mnemonic, code->klass->payload));
                        REQUIRE_OK(kefir_json_output_string(json, "external_memory"));
                        REQUIRE_OK(kefir_json_output_object_key(json, "base_reg"));
                        REQUIRE_OK(kefir_json_output_string(json, mnemonic));
                        REQUIRE_OK(kefir_json_output_object_key(json, "offset"));
                        REQUIRE_OK(kefir_json_output_uinteger(json, value_type->parameters.memory.offset));
                    } break;
                }
                REQUIRE_OK(kefir_json_output_object_end(json));

                if (value_type->constraint.type != KEFIR_CODEGEN_TARGET_IR_ALLOCATION_NO_CONSTRAINT) {
                    REQUIRE_OK(kefir_json_output_object_key(json, "constraint"));
                    REQUIRE_OK(kefir_json_output_object_begin(json));
                    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
                    switch (value_type->constraint.type) {
                        case KEFIR_CODEGEN_TARGET_IR_ALLOCATION_NO_CONSTRAINT:
                            // Intentionally left blank
                            break;

                        case KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT: {
                            REQUIRE_OK(kefir_json_output_string(json, "requirement"));
                            const char *mnemonic;
                            REQUIRE_OK(code->klass->register_mnemonic(value_type->constraint.physical_register, &mnemonic, code->klass->payload));
                            REQUIRE_OK(kefir_json_output_object_key(json, "register"));
                            REQUIRE_OK(kefir_json_output_string(json, mnemonic));
                        } break;

                        case KEFIR_CODEGEN_TARGET_IR_ALLOCATION_HINT: {
                            REQUIRE_OK(kefir_json_output_string(json, "hint"));
                            const char *mnemonic;
                            REQUIRE_OK(code->klass->register_mnemonic(value_type->constraint.physical_register, &mnemonic, code->klass->payload));
                            REQUIRE_OK(kefir_json_output_object_key(json, "register"));
                            REQUIRE_OK(kefir_json_output_string(json, mnemonic));
                        } break;

                        case KEFIR_CODEGEN_TARGET_IR_ALLOCATION_SAME_AS:
                            REQUIRE_OK(kefir_json_output_string(json, "same_as"));
                            // TODO KEFIR_NOT_IMPLEMENTED
                            break;
                    }
                    REQUIRE_OK(kefir_json_output_object_end(json));
                }
                REQUIRE_OK(kefir_json_output_object_end(json));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
            REQUIRE_OK(kefir_json_output_array_end(json));

            kefir_bool_t has_attributes = false;
            kefir_codegen_target_ir_native_id_t attribute;
            struct kefir_codegen_target_ir_code_attribute_iterator attr_iter;
            for (res = kefir_codegen_target_ir_code_instruction_attribute_iter(code, &attr_iter, instr_ref, &attribute);
                res == KEFIR_OK;
                res = kefir_codegen_target_ir_code_instruction_attribute_next(&attr_iter, &attribute)) {
                if (!has_attributes) {
                    REQUIRE_OK(kefir_json_output_object_key(json, "attributes"));
                    REQUIRE_OK(kefir_json_output_array_begin(json));
                    has_attributes = true;
                }

                const char *mnemonic;
                REQUIRE_OK(code->klass->attribute_mnemonic(attribute, &mnemonic, code->klass->payload));
                REQUIRE_OK(kefir_json_output_string(json, mnemonic));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
            if (has_attributes) {
                REQUIRE_OK(kefir_json_output_array_end(json));
            }

            REQUIRE_OK(kefir_json_output_object_key(json, "metadata"));
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "source_location"));
            if (instr->metadata.source_location.source != NULL) {
                REQUIRE_OK(kefir_json_output_object_begin(json));
                REQUIRE_OK(kefir_json_output_object_key(json, "source"));
                REQUIRE_OK(kefir_json_output_string(json, instr->metadata.source_location.source));
                REQUIRE_OK(kefir_json_output_object_key(json, "line"));
                REQUIRE_OK(kefir_json_output_uinteger(json, instr->metadata.source_location.line));
                REQUIRE_OK(kefir_json_output_object_key(json, "column"));
                REQUIRE_OK(kefir_json_output_uinteger(json, instr->metadata.source_location.column));
                REQUIRE_OK(kefir_json_output_object_end(json));
            } else {
                REQUIRE_OK(kefir_json_output_null(json));
            }
            REQUIRE_OK(kefir_json_output_object_key(json, "code_ref"));
            if (instr->metadata.code_ref != KEFIR_CODEGEN_TARGET_IR_METADATA_CODE_REF_NONE) {
                REQUIRE_OK(kefir_json_output_uinteger(json, instr->metadata.code_ref));
            } else {
                REQUIRE_OK(kefir_json_output_null(json));
            }
            REQUIRE_OK(kefir_json_output_object_end(json));
            REQUIRE_OK(kefir_json_output_object_end(json));
        }
        REQUIRE_OK(kefir_json_output_array_end(json));
        const struct kefir_codegen_target_ir_block *block = kefir_codegen_target_ir_code_block_at(code, block_ref);
        REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve target IR block"));
        REQUIRE_OK(kefir_json_output_object_key(json, "externally_visible"));
        REQUIRE_OK(kefir_json_output_boolean(json, block->externally_visible));
        REQUIRE_OK(kefir_json_output_object_key(json, "public_labels"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        struct kefir_hashtreeset_iterator iter;
        kefir_result_t res;
        for (res = kefir_hashtreeset_iter(&block->public_labels, &iter); res == KEFIR_OK;
            res = kefir_hashtreeset_next(&iter)) {
            ASSIGN_DECL_CAST(const char *, public_label, iter.entry);
            REQUIRE_OK(kefir_json_output_string(json, public_label));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(kefir_json_output_array_end(json));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}
