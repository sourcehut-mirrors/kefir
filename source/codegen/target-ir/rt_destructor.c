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

#include "kefir/codegen/target-ir/rt_destructor.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/liveness.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

struct rt_destructor_state {
    struct kefir_mem *mem;
    const struct kefir_codegen_target_ir_code *code;
    struct kefir_asmcmp_context *asmcmp_ctx;
    const struct kefir_codegen_target_ir_code_constructor_metadata *constructor_metadata;
    const struct kefir_codegen_target_ir_round_trip_destructor_ops *parameter;

    struct kefir_codegen_target_ir_control_flow control_flow;
    struct kefir_codegen_target_ir_liveness liveness;
    struct kefir_hashtable blocks;
    struct kefir_hashtable value_vregs;
    struct kefir_hashtable native_labels;
    struct kefir_hashtable instruction_labels;
    struct kefir_list queue;
    struct kefir_hashset visited;
    struct kefir_hashtable implicit_read_vregs;
    struct kefir_hashtable implicit_write_vregs;
    struct kefir_hashset alive_vregs;
    kefir_uint32_t next_temp_virtual_block;
};

struct block_state {
    kefir_codegen_target_ir_block_ref_t block_ref;
    kefir_asmcmp_label_index_t asmcmp_label;
    kefir_asmcmp_label_index_t asmcmp_end_label;
};

static kefir_result_t map_block_labels(struct rt_destructor_state *state) {
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(state->code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(state->code, i);

        kefir_asmcmp_label_index_t asmcmp_label, asmcmp_end_label = KEFIR_ASMCMP_INDEX_NONE;
        REQUIRE_OK(kefir_asmcmp_context_new_label(state->mem, state->asmcmp_ctx, KEFIR_ASMCMP_INDEX_NONE, &asmcmp_label));

        const struct kefir_codegen_target_ir_block *block = kefir_codegen_target_ir_code_block_at(state->code, block_ref);
        REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve target IR block"));

        if (state->constructor_metadata != NULL || block->externally_visible) {
            REQUIRE_OK(kefir_asmcmp_context_label_mark_external_dependencies(state->mem, state->asmcmp_ctx, asmcmp_label));
        }
        if (state->constructor_metadata != NULL) {
            REQUIRE_OK(kefir_asmcmp_context_new_label(state->mem, state->asmcmp_ctx, KEFIR_ASMCMP_INDEX_NONE, &asmcmp_end_label));
            REQUIRE_OK(kefir_asmcmp_context_label_mark_external_dependencies(state->mem, state->asmcmp_ctx, asmcmp_end_label));
        }
        
        kefir_result_t res;
        struct kefir_hashtreeset_iterator iter;
        for (res = kefir_hashtreeset_iter(&block->public_labels, &iter); res == KEFIR_OK;
            res = kefir_hashtreeset_next(&iter)) {
            ASSIGN_DECL_CAST(const char *, public_label, iter.entry);
            REQUIRE_OK(kefir_asmcmp_context_label_add_public_name(state->mem, state->asmcmp_ctx, asmcmp_label, public_label));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        struct block_state *block_state = KEFIR_MALLOC(state->mem, sizeof(struct block_state));
        REQUIRE(block_state != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR block state"));

        block_state->block_ref = block_ref;
        block_state->asmcmp_label = asmcmp_label;
        block_state->asmcmp_end_label = asmcmp_end_label;
        res = kefir_hashtable_insert(state->mem, &state->blocks, (kefir_hashtable_key_t) block_ref, (kefir_hashtable_value_t) block_state);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(state->mem, block_state);
            return res;
        });
    }
    return KEFIR_OK;
}

static kefir_result_t apply_virtual_register_constraint(struct rt_destructor_state *state, kefir_asmcmp_virtual_register_index_t vreg_idx, const struct kefir_codegen_target_ir_allocation_constraint *constraint) {
    if (constraint != NULL) {
        switch (constraint->type) {
            case KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT:
                REQUIRE_OK(state->parameter->preallocation_requirement(state->mem, vreg_idx, constraint->physical_register, state->parameter->payload));
                break;

            case KEFIR_CODEGEN_TARGET_IR_ALLOCATION_HINT:
                REQUIRE_OK(state->parameter->preallocation_hint(state->mem, vreg_idx, constraint->physical_register, state->parameter->payload));
                break;

            case KEFIR_CODEGEN_TARGET_IR_ALLOCATION_SAME_AS:
            case KEFIR_CODEGEN_TARGET_IR_ALLOCATION_NO_CONSTRAINT:
                // Intentionally left blank
                break;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t allocate_value_virtual_register(struct rt_destructor_state *state, struct kefir_codegen_target_ir_value_ref value_ref,
    const struct kefir_codegen_target_ir_value_type *value_type, const struct kefir_codegen_target_ir_allocation_constraint *constraint, kefir_asmcmp_virtual_register_index_t *vreg_idx_ptr) {

    kefir_asmcmp_virtual_register_index_t vreg_idx = KEFIR_ASMCMP_INDEX_NONE;
    switch (value_type->kind) {
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_UNSPECIFIED:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(state->mem, state->asmcmp_ctx, KEFIR_ASMCMP_VIRTUAL_REGISTER_UNSPECIFIED, &vreg_idx));
            break;
            
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(state->mem, state->asmcmp_ctx, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &vreg_idx));
            break;
            
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(state->mem, state->asmcmp_ctx, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &vreg_idx));
            break;
            
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(state->mem, state->asmcmp_ctx, value_type->parameters.spill_space_allocation.length, value_type->parameters.spill_space_allocation.alignment, &vreg_idx));
            break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new_local_variable(state->mem, state->asmcmp_ctx, value_type->parameters.local_variable.identifier, value_type->parameters.local_variable.offset, &vreg_idx));
            break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_EXTERNAL_MEMORY:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new_memory_pointer(state->mem, state->asmcmp_ctx, value_type->parameters.memory.base_reg, value_type->parameters.memory.offset, &vreg_idx));
            break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLAGS:
            // Intentionally left blank
            break;
    }
    if (constraint != NULL) {
        REQUIRE_OK(apply_virtual_register_constraint(state, vreg_idx, constraint));
    }
    if (vreg_idx != KEFIR_ASMCMP_INDEX_NONE) {
        REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->value_vregs, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), (kefir_hashtable_value_t) vreg_idx));
    }
    ASSIGN_PTR(vreg_idx_ptr, vreg_idx);
    return KEFIR_OK;
}

static kefir_result_t resolve_value_virtual_register(struct rt_destructor_state *state, struct kefir_codegen_target_ir_value_ref value_ref, kefir_asmcmp_virtual_register_index_t *vreg_idx_ptr) {
    const struct kefir_codegen_target_ir_value_type *value_type;
    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->code, value_ref, &value_type));
    if (value_type->kind == KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE) {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new_local_variable(state->mem, state->asmcmp_ctx, value_type->parameters.local_variable.identifier, value_type->parameters.local_variable.offset, vreg_idx_ptr));
        return KEFIR_OK;
    }

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&state->value_vregs, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_PTR(vreg_idx_ptr, (kefir_asmcmp_virtual_register_index_t) table_value);
    } else {
        REQUIRE_OK(allocate_value_virtual_register(state, value_ref, value_type, &value_type->constraint, vreg_idx_ptr));
    }

    return KEFIR_OK;
}

static kefir_result_t resolve_variant(kefir_codegen_target_ir_operand_variant_t variant, kefir_asmcmp_operand_variant_t *asmcmp_variant, kefir_bool_t *high_half) {
    ASSIGN_PTR(high_half, false);
    switch (variant) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT:
            *asmcmp_variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT;
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT:
            *asmcmp_variant = KEFIR_ASMCMP_OPERAND_VARIANT_8BIT;
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT_HIGHER:
            *asmcmp_variant = KEFIR_ASMCMP_OPERAND_VARIANT_8BIT;
            ASSIGN_PTR(high_half, true);
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT:
            *asmcmp_variant = KEFIR_ASMCMP_OPERAND_VARIANT_16BIT;
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT:
            *asmcmp_variant = KEFIR_ASMCMP_OPERAND_VARIANT_32BIT;
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT:
            *asmcmp_variant = KEFIR_ASMCMP_OPERAND_VARIANT_64BIT;
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_80BIT:
            *asmcmp_variant = KEFIR_ASMCMP_OPERAND_VARIANT_80BIT;
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_128BIT:
            *asmcmp_variant = KEFIR_ASMCMP_OPERAND_VARIANT_128BIT;
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_FP_SINGLE:
            *asmcmp_variant = KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE;
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_FP_DOUBLE:
            *asmcmp_variant = KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE;
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t resolve_relocation(kefir_codegen_target_ir_external_label_relocation_t relocation, kefir_asmcmp_external_label_relocation_t *asmcmp_reloc) {
    switch (relocation) {
        case KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_ABSOLUTE:
            *asmcmp_reloc = KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE;
            break;

        case KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_PLT:
            *asmcmp_reloc = KEFIR_ASMCMP_EXTERNAL_LABEL_PLT;
            break;

        case KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_GOTPCREL:
            *asmcmp_reloc = KEFIR_ASMCMP_EXTERNAL_LABEL_GOTPCREL;
            break;

        case KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_TPOFF:
            *asmcmp_reloc = KEFIR_ASMCMP_EXTERNAL_LABEL_TPOFF;
            break;

        case KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_GOTTPOFF:
            *asmcmp_reloc = KEFIR_ASMCMP_EXTERNAL_LABEL_GOTTPOFF;
            break;

        case KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_TLSGD:
            *asmcmp_reloc = KEFIR_ASMCMP_EXTERNAL_LABEL_TLSGD;
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t native_id_to_label(struct rt_destructor_state *state, kefir_codegen_target_ir_native_id_t native_id, kefir_asmcmp_label_index_t *asmcmp_label) {
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&state->native_labels, (kefir_hashtable_key_t) native_id, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *asmcmp_label = (kefir_asmcmp_label_index_t) table_value;
    } else {
        REQUIRE_OK(kefir_asmcmp_context_new_label(state->mem, state->asmcmp_ctx, KEFIR_ASMCMP_INDEX_NONE, asmcmp_label));
        REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->native_labels, (kefir_hashtable_key_t) native_id, *asmcmp_label));
        REQUIRE_OK(state->parameter->bind_native_id(state->mem, *asmcmp_label, native_id, state->parameter->payload));
    }
    return KEFIR_OK;
}

static kefir_result_t resolve_operand(struct rt_destructor_state *state, const struct kefir_codegen_target_ir_operand *operand, struct kefir_asmcmp_value *value, kefir_bool_t immediate) {
    REQUIRE(!immediate || operand->type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UINTEGER || operand->type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected immediate operand type"));
    switch (operand->type) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE:
            value->type = KEFIR_ASMCMP_VALUE_TYPE_NONE;
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UINTEGER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER:
            if (immediate) {
                value->type = KEFIR_ASMCMP_VALUE_TYPE_INTEGER;
                value->int_immediate = operand->immediate.int_immediate;
            } else {
                kefir_asmcmp_virtual_register_index_t vreg_idx;
                REQUIRE_OK(kefir_asmcmp_virtual_register_new_immediate_integer(state->mem, state->asmcmp_ctx, operand->immediate.int_immediate, &vreg_idx));
                value->type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER;
                value->vreg.index = vreg_idx;
                REQUIRE_OK(resolve_variant(operand->immediate.variant, &value->vreg.variant, &value->vreg.high_half));
            }
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_PHYSICAL_REGISTER:
            value->type = KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER;
            value->phreg = operand->phreg;
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF:
            value->type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER;
            REQUIRE_OK(resolve_value_virtual_register(state, operand->direct.value_ref, &value->vreg.index));
            REQUIRE_OK(resolve_variant(operand->direct.variant, &value->vreg.variant, &value->vreg.high_half));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT:
            value->type = KEFIR_ASMCMP_VALUE_TYPE_INDIRECT;
            switch (operand->indirect.type) {
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_PHYSICAL_BASIS:
                    value->indirect.type = KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS;
                    value->indirect.base.phreg = operand->indirect.base.phreg;
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS:
                    value->indirect.type = KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS;
                    REQUIRE_OK(resolve_value_virtual_register(state, operand->indirect.base.value_ref, &value->indirect.base.vreg));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_IMMEDIATE_BASIS: {
                    value->indirect.type = KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS;
                    REQUIRE_OK(kefir_asmcmp_virtual_register_new_immediate_integer(state->mem, state->asmcmp_ctx, operand->indirect.base.immediate, &value->indirect.base.vreg));
                } break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS: {
                    kefir_hashtable_value_t table_value;
                    REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) operand->block_ref, &table_value));
                    ASSIGN_DECL_CAST(struct block_state *, block_state,
                        table_value);
                    value->indirect.type = KEFIR_ASMCMP_INDIRECT_INTERNAL_LABEL_BASIS;
                    value->indirect.base.internal_label = block_state->asmcmp_label;
                } break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_NATIVE_LABEL_BASIS:
                    value->indirect.type = KEFIR_ASMCMP_INDIRECT_INTERNAL_LABEL_BASIS;
                    REQUIRE_OK(native_id_to_label(state, operand->indirect.base.native_id, &value->indirect.base.internal_label));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_EXTERNAL_LABEL_BASIS:
                    value->indirect.type = KEFIR_ASMCMP_INDIRECT_EXTERNAL_LABEL_BASIS;
                    value->indirect.base.external_label = operand->indirect.base.external_label;
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_LOCAL_AREA_BASIS:
                    value->indirect.type = KEFIR_ASMCMP_INDIRECT_LOCAL_AREA_BASIS;
                    value->indirect.base.local_variable_id = operand->indirect.base.local_variable_id;
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_SPILL_AREA_BASIS:
                    value->indirect.type = KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS;
                    value->indirect.base.spill_index = operand->indirect.base.spill_index;
                    break;
            }
            value->indirect.offset = operand->indirect.offset;
            REQUIRE_OK(resolve_variant(operand->indirect.variant, &value->indirect.variant, NULL));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF: {
            kefir_hashtable_value_t table_value;
            REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) operand->rip_indirection.block_ref, &table_value));
            ASSIGN_DECL_CAST(struct block_state *, block_state,
                table_value);
            value->type = KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL;
            value->rip_indirection.internal = block_state->asmcmp_label;
            REQUIRE_OK(resolve_variant(operand->rip_indirection.variant, &value->rip_indirection.variant, NULL));
            REQUIRE_OK(resolve_relocation(operand->rip_indirection.position, &value->rip_indirection.position));
        } break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_NATIVE:
            value->type = KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL;
            REQUIRE_OK(native_id_to_label(state, operand->rip_indirection.native_id, &value->rip_indirection.internal));
            REQUIRE_OK(resolve_variant(operand->rip_indirection.variant, &value->rip_indirection.variant, NULL));
            REQUIRE_OK(resolve_relocation(operand->rip_indirection.position, &value->rip_indirection.position));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_EXTERNAL:
            value->type = KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL;
            value->rip_indirection.external = operand->rip_indirection.external;
            REQUIRE_OK(resolve_variant(operand->rip_indirection.variant, &value->rip_indirection.variant, NULL));
            REQUIRE_OK(resolve_relocation(operand->rip_indirection.position, &value->rip_indirection.position));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF: {
            kefir_hashtable_value_t table_value;
            REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) operand->block_ref, &table_value));
            ASSIGN_DECL_CAST(struct block_state *, block_state,
                table_value);
            value->type = KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL;
            value->internal_label = block_state->asmcmp_label;
        } break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NATIVE_LABEL:
            value->type = KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL;
            REQUIRE_OK(native_id_to_label(state, operand->native_id, &value->internal_label));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_EXTERNAL_LABEL:
            value->type = KEFIR_ASMCMP_VALUE_TYPE_EXTERNAL_LABEL;
            value->external_label.symbolic = operand->external_label.symbolic;
            value->external_label.offset = operand->external_label.offset;
            REQUIRE_OK(resolve_relocation(operand->external_label.position, &value->external_label.position));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87:
            value->type = KEFIR_ASMCMP_VALUE_TYPE_X87;
            value->x87 = operand->x87;
            break;
    }
    if (operand->segment.present) {
        value->segment.present = true;
        value->segment.reg = operand->segment.reg;
    } else {
        value->segment.present = false;
    }
    return KEFIR_OK;
}

static kefir_result_t link_virtual_registers(struct rt_destructor_state *state, kefir_asmcmp_instruction_index_t insert_after_idx, kefir_asmcmp_virtual_register_index_t vreg1_idx, kefir_asmcmp_virtual_register_index_t vreg2_idx, kefir_asmcmp_instruction_index_t *instr_idx_ptr) {
    struct kefir_asmcmp_instruction instr = {
        .opcode = state->parameter->link_virtual_registers_opcode,
        .args[0] = {
            .type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER,
            .vreg.index = vreg1_idx,
            .vreg.variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT
        },
        .args[1] = {
            .type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER,
            .vreg.index = vreg2_idx,
            .vreg.variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT
        },
        .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE
    };
    REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, state->asmcmp_ctx, insert_after_idx, &instr, instr_idx_ptr));
    return KEFIR_OK;
}

static kefir_result_t touch_virtual_register(struct rt_destructor_state *state, kefir_asmcmp_instruction_index_t insert_after_idx, kefir_asmcmp_virtual_register_index_t vreg1_idx, kefir_asmcmp_instruction_index_t *instr_idx_ptr) {
    struct kefir_asmcmp_instruction instr = {
        .opcode = state->parameter->touch_virtual_register_opcode,
        .args[0] = {
            .type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER,
            .vreg.index = vreg1_idx,
            .vreg.variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT
        },
        .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
        .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE
    };
    REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, state->asmcmp_ctx, insert_after_idx, &instr, instr_idx_ptr));
    return KEFIR_OK;
}

static kefir_result_t load_virtual_register(struct rt_destructor_state *state, kefir_asmcmp_virtual_register_index_t vreg_idx, const struct kefir_codegen_target_ir_operand *operand) {
    kefir_asmcmp_virtual_register_index_t input_vreg;
    REQUIRE(operand->type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected input operand to be value reference"));
    REQUIRE_OK(resolve_value_virtual_register(state, operand->direct.value_ref, &input_vreg));

    REQUIRE_OK(link_virtual_registers(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), vreg_idx, input_vreg, NULL));
    return KEFIR_OK;
}

static kefir_result_t map_phis_unconditional_impl(struct rt_destructor_state *state, struct block_state *block_state, kefir_codegen_target_ir_block_ref_t target_block_ref, struct kefir_hashtree *dst_phis) {
    kefir_result_t res;
    struct kefir_codegen_target_ir_value_phi_node_iterator phi_node_iter;
    kefir_codegen_target_ir_instruction_ref_t phi_ref;
    for (res = kefir_codegen_target_ir_code_phi_node_iter(state->code, &phi_node_iter, target_block_ref, &phi_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_phi_node_next(&phi_node_iter, &phi_ref)) {
        kefir_codegen_target_ir_value_ref_t link_value_ref;
        REQUIRE_OK(kefir_codegen_target_ir_code_phi_link_for(state->code, phi_ref, block_state->block_ref, &link_value_ref));

        const struct kefir_codegen_target_ir_value_type *value_type;
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->code, link_value_ref, &value_type));
        if (link_value_ref.aspect != KEFIR_CODEGEN_TARGET_IR_VALUE_FLAGS && value_type->kind != KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE) {
            const struct kefir_codegen_target_ir_instruction *linked_instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(state->code, link_value_ref.instr_ref, &linked_instr));
            if (linked_instr->block_ref == target_block_ref && linked_instr->operation.opcode == state->code->klass->phi_opcode && link_value_ref.instr_ref != phi_ref) {
                kefir_asmcmp_virtual_register_index_t src_vreg, dst_vreg;
                REQUIRE_OK(resolve_value_virtual_register(state, link_value_ref, &src_vreg));
                REQUIRE_OK(resolve_value_virtual_register(state, (kefir_codegen_target_ir_value_ref_t) {
                    .instr_ref = phi_ref,
                    .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
                }, &dst_vreg));
                kefir_asmcmp_virtual_register_index_t copy_vreg;
                REQUIRE_OK(kefir_asmcmp_new_virtual_register_of_type(state->mem, state->asmcmp_ctx, dst_vreg, &copy_vreg));
                REQUIRE_OK(link_virtual_registers(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), copy_vreg, src_vreg, NULL));
                REQUIRE_OK(kefir_hashtree_insert(state->mem, dst_phis, (kefir_hashtree_key_t) phi_ref, (kefir_hashtree_value_t) copy_vreg));
            }
        } 
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    
    for (res = kefir_codegen_target_ir_code_phi_node_iter(state->code, &phi_node_iter, target_block_ref, &phi_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_phi_node_next(&phi_node_iter, &phi_ref)) {
        kefir_codegen_target_ir_value_ref_t link_value_ref;
        REQUIRE_OK(kefir_codegen_target_ir_code_phi_link_for(state->code, phi_ref, block_state->block_ref, &link_value_ref));

        const struct kefir_codegen_target_ir_value_type *value_type;
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->code, link_value_ref, &value_type));
        if (link_value_ref.aspect != KEFIR_CODEGEN_TARGET_IR_VALUE_FLAGS && value_type->kind != KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE) {
            kefir_asmcmp_virtual_register_index_t src_vreg, dst_vreg;
            struct kefir_hashtree_node *node;
            res = kefir_hashtree_at(dst_phis, (kefir_hashtree_key_t) phi_ref, &node);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                src_vreg = (kefir_asmcmp_virtual_register_index_t) node->value;
            } else {
                REQUIRE_OK(resolve_value_virtual_register(state, link_value_ref, &src_vreg));
            }
            REQUIRE_OK(resolve_value_virtual_register(state, (kefir_codegen_target_ir_value_ref_t) {
                .instr_ref = phi_ref,
                .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
            }, &dst_vreg));

            REQUIRE_OK(link_virtual_registers(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), dst_vreg, src_vreg, NULL));
            REQUIRE_OK(kefir_hashset_add(state->mem, &state->alive_vregs, (kefir_hashset_key_t) dst_vreg));
        } 
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}
static kefir_result_t map_phis_unconditional(struct rt_destructor_state *state, struct block_state *block_state, kefir_codegen_target_ir_block_ref_t target_block_ref) {
    struct kefir_hashtree dst_phis;
    REQUIRE_OK(kefir_hashtree_init(&dst_phis, &kefir_hashtree_uint_ops));
    kefir_result_t res = map_phis_unconditional_impl(state, block_state, target_block_ref, &dst_phis);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(state->mem, &dst_phis);
        return res;
    });
    REQUIRE_OK(kefir_hashtree_free(state->mem, &dst_phis));
    return KEFIR_OK;
}

static kefir_result_t translate_instruction(struct rt_destructor_state *state, struct block_state *block_state, kefir_codegen_target_ir_instruction_ref_t instr_ref) {
    UNUSED(block_state);
    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(state->code, instr_ref, &instr));

    if (instr->operation.opcode == state->code->klass->placeholder_opcode) {
        return KEFIR_OK;
    }

    if (instr->operation.opcode == state->code->klass->phi_opcode) {
        kefir_asmcmp_virtual_register_index_t vreg_idx;
        kefir_result_t res = resolve_value_virtual_register(state, (kefir_codegen_target_ir_value_ref_t) {
            .instr_ref = instr_ref,
            .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
        }, &vreg_idx);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            if (vreg_idx != KEFIR_ASMCMP_INDEX_NONE) {
                REQUIRE_OK(touch_virtual_register(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), (kefir_asmcmp_virtual_register_index_t) vreg_idx, NULL));
            }
        }
        return KEFIR_OK;
    }
    kefir_result_t res;
    kefir_codegen_target_ir_native_id_t attribute;
    struct kefir_codegen_target_ir_code_attribute_iterator attr_iter;
    for (res = kefir_codegen_target_ir_code_instruction_attribute_iter(state->code, &attr_iter, instr_ref, &attribute);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_instruction_attribute_next(&attr_iter, &attribute)) {
        REQUIRE_OK(state->parameter->materialize_attribute(state->mem, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), attribute, NULL, state->parameter->payload));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_codegen_target_ir_target_ir_instruction_destructor_classification classification;
    REQUIRE_OK(state->parameter->classify_instruction(state->code, instr_ref, &classification, state->parameter->payload));

    kefir_codegen_target_ir_block_ref_t next_block_ref = KEFIR_ID_NONE;
    struct kefir_codegen_target_ir_block_terminator_props terminator_props;
    REQUIRE_OK(state->code->klass->is_block_terminator(state->code, instr, &terminator_props, state->code->klass->payload));
    if (terminator_props.block_terminator && !terminator_props.function_terminator) {
        for (kefir_size_t live_out_idx = 0; live_out_idx < state->liveness.blocks[block_state->block_ref].live_out.length; live_out_idx++) {                        
            struct kefir_codegen_target_ir_value_ref value_ref = state->liveness.blocks[block_state->block_ref].live_out.content[live_out_idx];
            if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(value_ref.aspect)) {
                kefir_asmcmp_virtual_register_index_t vreg_idx;
                REQUIRE_OK(resolve_value_virtual_register(state, value_ref, &vreg_idx));
                if (vreg_idx != KEFIR_ASMCMP_INDEX_NONE) {
                    REQUIRE_OK(touch_virtual_register(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), (kefir_asmcmp_virtual_register_index_t) vreg_idx, NULL));
                }
            }
        }

        if (terminator_props.branch) {
            struct kefir_asmcmp_instruction asmcmp_instrs[2] = {0};
            REQUIRE_OK(state->parameter->split_branch_instruction(state->mem, instr, asmcmp_instrs, state->parameter->payload));

            kefir_asmcmp_label_index_t trampoline_label;
            REQUIRE_OK(kefir_asmcmp_context_new_label(state->mem, state->asmcmp_ctx, KEFIR_ASMCMP_INDEX_NONE, &trampoline_label));

            kefir_hashtable_value_t table_value;
            REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) terminator_props.target_block_refs[0], &table_value));
            ASSIGN_DECL_CAST(struct block_state *, target_block_state,
                table_value);
            REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) terminator_props.target_block_refs[1], &table_value));
            ASSIGN_DECL_CAST(struct block_state *, alternative_block_state,
                table_value);

            asmcmp_instrs[0].args[0].type = KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL;
            asmcmp_instrs[0].args[0].internal_label = trampoline_label;
            asmcmp_instrs[0].args[0].segment.present = false;
            REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx),
                &asmcmp_instrs[0], NULL));

            {
                REQUIRE_OK(map_phis_unconditional(state, block_state, terminator_props.target_block_refs[1]));
                asmcmp_instrs[1].args[0].type = KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL;
                asmcmp_instrs[1].args[0].internal_label = alternative_block_state->asmcmp_label;
                asmcmp_instrs[1].args[0].segment.present = false;
                REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx),
                    &asmcmp_instrs[1], NULL));

                kefir_hashset_key_t alive_entry;
                struct kefir_hashset_iterator alive_iter;
                for (res = kefir_hashset_iter(&state->alive_vregs, &alive_iter, &alive_entry); res == KEFIR_OK;
                    res = kefir_hashset_next(&alive_iter, &alive_entry)) {
                    ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg_idx, alive_entry);
                    REQUIRE_OK(touch_virtual_register(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), (kefir_asmcmp_virtual_register_index_t) vreg_idx, NULL));
                }
                if (res != KEFIR_ITERATOR_END) {
                    REQUIRE_OK(res);
                }
                REQUIRE_OK(kefir_hashset_clear(state->mem, &state->alive_vregs));

                for (kefir_size_t live_in_idx = 0; live_in_idx < state->liveness.blocks[alternative_block_state->block_ref].live_in.length; live_in_idx++) {
                    kefir_codegen_target_ir_value_ref_t value_ref = state->liveness.blocks[alternative_block_state->block_ref].live_in.content[live_in_idx];
                    if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(value_ref.aspect)) {
                        kefir_asmcmp_virtual_register_index_t vreg_idx;
                        REQUIRE_OK(resolve_value_virtual_register(state, value_ref, &vreg_idx));
                        if (vreg_idx != KEFIR_ASMCMP_INDEX_NONE) {
                            REQUIRE_OK(touch_virtual_register(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), (kefir_asmcmp_virtual_register_index_t) vreg_idx, NULL));
                        }
                    }
                }
            }

            struct kefir_asmcmp_instruction virtual_block_end_instr = {
                .opcode = state->parameter->virtual_block_end_opcode,
                .args[0] = {
                    .type = KEFIR_ASMCMP_VALUE_TYPE_UINTEGER,
                    .uint_immediate = block_state->block_ref
                },
                .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
                .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE
            };
            REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), &virtual_block_end_instr, NULL));
            
            REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(state->mem, state->asmcmp_ctx, trampoline_label));
            {
                if (terminator_props.target_block_refs[0] != block_state->block_ref) {
                    struct kefir_asmcmp_instruction virtual_block_begin_instr = {
                        .opcode = state->parameter->virtual_block_begin_opcode,
                        .args[0] = {
                            .type = KEFIR_ASMCMP_VALUE_TYPE_UINTEGER,
                            .uint_immediate = state->next_temp_virtual_block++
                        },
                        .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
                        .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE
                    };
                    REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), &virtual_block_begin_instr, NULL));
                }

                for (kefir_size_t live_in_idx = 0; live_in_idx < state->liveness.blocks[target_block_state->block_ref].live_in.length; live_in_idx++) {
                    kefir_codegen_target_ir_value_ref_t value_ref = state->liveness.blocks[target_block_state->block_ref].live_in.content[live_in_idx];
                    if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(value_ref.aspect)) {
                        kefir_asmcmp_virtual_register_index_t vreg_idx;
                        REQUIRE_OK(resolve_value_virtual_register(state, value_ref, &vreg_idx));
                        if (vreg_idx != KEFIR_ASMCMP_INDEX_NONE) {
                            REQUIRE_OK(touch_virtual_register(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), (kefir_asmcmp_virtual_register_index_t) vreg_idx, NULL));
                        }
                    }
                }

                struct kefir_codegen_target_ir_value_phi_node_iterator phi_node_iter;
                kefir_codegen_target_ir_instruction_ref_t phi_ref;
                for (res = kefir_codegen_target_ir_code_phi_node_iter(state->code, &phi_node_iter, terminator_props.target_block_refs[0], &phi_ref);
                    res == KEFIR_OK;
                    res = kefir_codegen_target_ir_code_phi_node_next(&phi_node_iter, &phi_ref)) {
                    kefir_codegen_target_ir_value_ref_t link_value_ref;
                    REQUIRE_OK(kefir_codegen_target_ir_code_phi_link_for(state->code, phi_ref, block_state->block_ref, &link_value_ref));

                    const struct kefir_codegen_target_ir_value_type *value_type;
                    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->code, link_value_ref, &value_type));
                    if (link_value_ref.aspect != KEFIR_CODEGEN_TARGET_IR_VALUE_FLAGS && value_type->kind != KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE) {
                        kefir_asmcmp_virtual_register_index_t src_vreg;
                        REQUIRE_OK(resolve_value_virtual_register(state, link_value_ref, &src_vreg));
                        REQUIRE_OK(touch_virtual_register(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), src_vreg, NULL));
                    } 
                }
                if (res != KEFIR_ITERATOR_END) {
                    REQUIRE_OK(res);
                }

                REQUIRE_OK(map_phis_unconditional(state, block_state, terminator_props.target_block_refs[0]));
                asmcmp_instrs[1].args[0].type = KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL;
                asmcmp_instrs[1].args[0].internal_label = target_block_state->asmcmp_label;
                asmcmp_instrs[1].args[0].segment.present = false;
                REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx),
                    &asmcmp_instrs[1], NULL));

                kefir_hashset_key_t alive_entry;
                struct kefir_hashset_iterator alive_iter;
                for (res = kefir_hashset_iter(&state->alive_vregs, &alive_iter, &alive_entry); res == KEFIR_OK;
                    res = kefir_hashset_next(&alive_iter, &alive_entry)) {
                    ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg_idx, alive_entry);
                    REQUIRE_OK(touch_virtual_register(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), (kefir_asmcmp_virtual_register_index_t) vreg_idx, NULL));
                }
                if (res != KEFIR_ITERATOR_END) {
                    REQUIRE_OK(res);
                }
                REQUIRE_OK(kefir_hashset_clear(state->mem, &state->alive_vregs));

                for (kefir_size_t live_in_idx = 0; live_in_idx < state->liveness.blocks[target_block_state->block_ref].live_in.length; live_in_idx++) {
                    kefir_codegen_target_ir_value_ref_t value_ref = state->liveness.blocks[target_block_state->block_ref].live_in.content[live_in_idx];
                    if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(value_ref.aspect)) {
                        kefir_asmcmp_virtual_register_index_t vreg_idx;
                        REQUIRE_OK(resolve_value_virtual_register(state, value_ref, &vreg_idx));
                        if (vreg_idx != KEFIR_ASMCMP_INDEX_NONE) {
                            REQUIRE_OK(touch_virtual_register(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), (kefir_asmcmp_virtual_register_index_t) vreg_idx, NULL));
                        }
                    }
                }

                if (terminator_props.target_block_refs[0] != block_state->block_ref) {
                    struct kefir_asmcmp_instruction virtual_block_end_instr = {
                        .opcode = state->parameter->virtual_block_end_opcode,
                        .args[0] = {
                            .type = KEFIR_ASMCMP_VALUE_TYPE_UINTEGER,
                            .uint_immediate = state->next_temp_virtual_block - 1
                        },
                        .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
                        .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE
                    };
                    REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), &virtual_block_end_instr, NULL));
                }
            }
            return KEFIR_OK;
        } else if (!terminator_props.undefined_target) {
            REQUIRE_OK(map_phis_unconditional(state, block_state, terminator_props.target_block_refs[0]));
            next_block_ref = terminator_props.target_block_refs[0];
        } else {
            REQUIRE_OK(map_phis_unconditional(state, block_state, state->control_flow.code->indirect_jump_gate_block));
            next_block_ref = state->control_flow.code->indirect_jump_gate_block;
        }
    }

    if (instr->operation.opcode == state->code->klass->inline_asm_opcode) {
        kefir_asmcmp_inline_assembly_index_t inline_asm;
        REQUIRE_OK(kefir_asmcmp_inline_assembly_new(state->mem, state->asmcmp_ctx, "", &inline_asm));
        for (const struct kefir_list_entry *iter = kefir_list_head(&instr->operation.inline_asm_node.fragments);
            iter != NULL;
            kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(const struct kefir_codegen_target_ir_inline_assembly_fragment *, fragment,
                iter->value);
            switch (fragment->type) {
                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_TEXT:
                    REQUIRE_OK(kefir_asmcmp_inline_assembly_add_text(state->mem, state->asmcmp_ctx, inline_asm, "%s", fragment->text));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_OPERAND: {
                    struct kefir_asmcmp_value value;
                    REQUIRE_OK(resolve_operand(state, &fragment->operand, &value, false));
                    REQUIRE_OK(kefir_asmcmp_inline_assembly_add_value(state->mem, state->asmcmp_ctx, inline_asm, &value));
                } break;
            }
        }

        REQUIRE_OK(state->parameter->new_inline_asm(state->mem, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx),
                                                    inline_asm, NULL, state->parameter->payload));

        if (instr->operation.inline_asm_node.target_block_ref != KEFIR_ID_NONE) {
            kefir_hashtable_value_t table_value;
            REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) instr->operation.inline_asm_node.target_block_ref, &table_value));
            ASSIGN_DECL_CAST(struct block_state *, block_state,
                table_value);

            struct kefir_asmcmp_instruction jump_instr = {
                .opcode = state->parameter->jump_opcode,
                .args[0] = {
                    .type = KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL,
                    .uint_immediate = block_state->asmcmp_label
                },
                .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
                .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE
            };
            REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), &jump_instr, NULL));
        }
    } else {
        struct kefir_asmcmp_instruction asmcmp_instruction = {
            .opcode = classification.opcode,
            .args[0].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
            .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
            .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE
        };

        kefir_size_t input_index = 0, output_index = 0;
        struct kefir_codegen_target_ir_value_ref output_value_ref;
        const struct kefir_codegen_target_ir_value_type *output_value_type;
        REQUIRE_OK(kefir_hashtable_clear(&state->implicit_read_vregs));
        REQUIRE_OK(kefir_hashtable_clear(&state->implicit_write_vregs));
        for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
            switch (classification.operands[i].class) {
                case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_NONE:
                    // Intentionally left blank
                    break;

                case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ:
                    if (!classification.operands[i].implicit) {
                        REQUIRE_OK(resolve_operand(state, &instr->operation.parameters[input_index++], &asmcmp_instruction.args[i], classification.operands[i].immediate));
                    } else {
                        kefir_asmcmp_virtual_register_index_t read_vreg = KEFIR_ASMCMP_INDEX_NONE;
                        if (instr->operation.parameters[input_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF) {
                            const struct kefir_codegen_target_ir_value_type *value_type;
                            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->code, instr->operation.parameters[input_index].direct.value_ref, &value_type));
                            if (value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT && value_type->constraint.physical_register == classification.operands[i].implicit_params.phreg) {
                                REQUIRE_OK(resolve_value_virtual_register(state, instr->operation.parameters[input_index].direct.value_ref, &read_vreg));
                            }
                        }
                        if (read_vreg == KEFIR_ASMCMP_INDEX_NONE) {
                            REQUIRE_OK(kefir_asmcmp_virtual_register_new(state->mem, state->asmcmp_ctx, classification.operands[i].implicit_params.vreg_type, &read_vreg));
                            REQUIRE_OK(state->parameter->preallocation_requirement(state->mem, read_vreg, classification.operands[i].implicit_params.phreg, state->parameter->payload));
                            REQUIRE_OK(load_virtual_register(state, read_vreg, &instr->operation.parameters[input_index]));
                        }
                        REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->implicit_read_vregs, (kefir_hashtable_key_t) read_vreg, (kefir_hashtable_value_t) 0));
                        input_index++;
                    }
                    break;

                case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE:
                    REQUIRE_OK(kefir_codegen_target_ir_code_instruction_output(state->code, instr_ref, output_index++, &output_value_ref, &output_value_type));
                    if (!classification.operands[i].implicit) {
                        if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_INDIRECT_OUTPUT(output_value_ref.aspect)) {
                            REQUIRE_OK(resolve_operand(state, &instr->operation.parameters[input_index++], &asmcmp_instruction.args[i], classification.operands[i].immediate));
                        } else {
                            REQUIRE(KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(output_value_ref.aspect), KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected either output register or indirect output value"));
                            REQUIRE_OK(resolve_value_virtual_register(state, output_value_ref, &asmcmp_instruction.args[i].vreg.index));
                            REQUIRE_OK(resolve_variant(output_value_type->variant, &asmcmp_instruction.args[i].vreg.variant, &asmcmp_instruction.args[i].vreg.high_half));
                            asmcmp_instruction.args[i].type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER;
                        }
                    } else {
                        kefir_asmcmp_virtual_register_index_t implicit_vreg;
                        REQUIRE_OK(kefir_asmcmp_virtual_register_new(state->mem, state->asmcmp_ctx, classification.operands[i].implicit_params.vreg_type, &implicit_vreg));
                        REQUIRE_OK(state->parameter->preallocation_requirement(state->mem, implicit_vreg, classification.operands[i].implicit_params.phreg, state->parameter->payload));
                        REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->implicit_write_vregs, (kefir_hashtable_key_t) implicit_vreg, (kefir_hashtable_value_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&output_value_ref)));
                    }
                    break;

                case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE:
                    REQUIRE_OK(kefir_codegen_target_ir_code_instruction_output(state->code, instr_ref, output_index++, &output_value_ref, &output_value_type));
                    if (!classification.operands[i].implicit) {
                        if (!classification.operands[i].implicit) {
                            if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_INDIRECT_OUTPUT(output_value_ref.aspect)) {
                                REQUIRE_OK(resolve_operand(state, &instr->operation.parameters[input_index++], &asmcmp_instruction.args[i], classification.operands[i].immediate));
                            } else {
                                REQUIRE(KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(output_value_ref.aspect), KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected either output register or indirect output value"));
                                REQUIRE_OK(resolve_value_virtual_register(state, output_value_ref, &asmcmp_instruction.args[i].vreg.index));
                                REQUIRE_OK(load_virtual_register(state, asmcmp_instruction.args[i].vreg.index, &instr->operation.parameters[input_index++]));
                                REQUIRE_OK(resolve_variant(output_value_type->variant, &asmcmp_instruction.args[i].vreg.variant, &asmcmp_instruction.args[i].vreg.high_half));
                                asmcmp_instruction.args[i].type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER;
                            }
                        }
                    } else {
                        kefir_asmcmp_virtual_register_index_t implicit_vreg;
                        REQUIRE_OK(kefir_asmcmp_virtual_register_new(state->mem, state->asmcmp_ctx, classification.operands[i].implicit_params.vreg_type, &implicit_vreg));
                        REQUIRE_OK(state->parameter->preallocation_requirement(state->mem, implicit_vreg, classification.operands[i].implicit_params.phreg, state->parameter->payload));
                        REQUIRE_OK(load_virtual_register(state, implicit_vreg, &instr->operation.parameters[input_index++]));
                        REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->implicit_read_vregs, (kefir_hashtable_key_t) implicit_vreg, (kefir_hashtable_value_t) 0));
                        REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->implicit_write_vregs, (kefir_hashtable_key_t) implicit_vreg, (kefir_hashtable_value_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&output_value_ref)));
                    }
                    break;
            }
        }

        kefir_hashtable_key_t table_key;
        kefir_hashtable_value_t table_value;
        struct kefir_hashtable_iterator implicit_iter;
        for (res = kefir_hashtable_iter(&state->implicit_read_vregs, &implicit_iter, &table_key, NULL);
            res == KEFIR_OK;
            res = kefir_hashtable_next(&implicit_iter, &table_key, NULL)) {
            REQUIRE_OK(touch_virtual_register(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), (kefir_asmcmp_virtual_register_index_t) table_key, NULL));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx),
            &asmcmp_instruction, NULL));

        for (res = kefir_hashtable_iter(&state->implicit_write_vregs, &implicit_iter, &table_key, &table_value);
            res == KEFIR_OK;
            res = kefir_hashtable_next(&implicit_iter, &table_key, &table_value)) {
            kefir_codegen_target_ir_value_ref_t output_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(table_value);
            kefir_asmcmp_virtual_register_index_t output_vreg;
            REQUIRE_OK(resolve_value_virtual_register(state, output_ref, &output_vreg));
            REQUIRE_OK(link_virtual_registers(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), output_vreg, (kefir_asmcmp_virtual_register_index_t) table_key, NULL));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    if (next_block_ref != KEFIR_ID_NONE) {
        kefir_result_t res;
        kefir_hashset_key_t alive_entry;
        struct kefir_hashset_iterator alive_iter;
        for (res = kefir_hashset_iter(&state->alive_vregs, &alive_iter, &alive_entry); res == KEFIR_OK;
            res = kefir_hashset_next(&alive_iter, &alive_entry)) {
            ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg_idx, alive_entry);
            REQUIRE_OK(touch_virtual_register(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), (kefir_asmcmp_virtual_register_index_t) vreg_idx, NULL));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(kefir_hashset_clear(state->mem, &state->alive_vregs));

        for (kefir_size_t live_in_idx = 0; live_in_idx < state->liveness.blocks[next_block_ref].live_in.length; live_in_idx++) {
            kefir_codegen_target_ir_value_ref_t value_ref = state->liveness.blocks[next_block_ref].live_in.content[live_in_idx];
            if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(value_ref.aspect)) {
                kefir_asmcmp_virtual_register_index_t vreg_idx;
                REQUIRE_OK(resolve_value_virtual_register(state, value_ref, &vreg_idx));
                if (vreg_idx != KEFIR_ASMCMP_INDEX_NONE) {
                    REQUIRE_OK(touch_virtual_register(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), (kefir_asmcmp_virtual_register_index_t) vreg_idx, NULL));
                }
            }
        }
    }

    if (next_block_ref != KEFIR_ID_NONE || instr_ref == kefir_codegen_target_ir_code_block_control_tail(state->code, block_state->block_ref)) {
        struct kefir_asmcmp_instruction virtual_block_end_instr = {
            .opcode = state->parameter->virtual_block_end_opcode,
            .args[0] = {
                .type = KEFIR_ASMCMP_VALUE_TYPE_UINTEGER,
                .uint_immediate = block_state->block_ref
            },
            .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
            .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE
        };
        REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), &virtual_block_end_instr, NULL));
    }

    return KEFIR_OK;
}

static kefir_result_t translate_block(struct rt_destructor_state *state, kefir_codegen_target_ir_block_ref_t block_ref) {
    kefir_hashtable_value_t table_value;
    REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) block_ref, &table_value));
    ASSIGN_DECL_CAST(struct block_state *, block_state,
        table_value);

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(state->mem, state->asmcmp_ctx, block_state->asmcmp_label));

    struct kefir_asmcmp_instruction virtual_block_begin_instr = {
        .opcode = state->parameter->virtual_block_begin_opcode,
        .args[0] = {
            .type = KEFIR_ASMCMP_VALUE_TYPE_UINTEGER,
            .uint_immediate = block_ref
        },
        .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
        .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE
    };
    REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), &virtual_block_begin_instr, NULL));

    kefir_result_t res;
    struct kefir_codegen_target_ir_value_phi_node_iterator phi_node_iter;
    kefir_codegen_target_ir_instruction_ref_t phi_ref;
    for (res = kefir_codegen_target_ir_code_phi_node_iter(state->code, &phi_node_iter, block_ref, &phi_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_phi_node_next(&phi_node_iter, &phi_ref)) {
        res = kefir_codegen_target_ir_code_value_props(state->code, (kefir_codegen_target_ir_value_ref_t) {
            .instr_ref = phi_ref,
            .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
        }, NULL);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            kefir_asmcmp_virtual_register_index_t vreg_idx;
            REQUIRE_OK(resolve_value_virtual_register(state, (kefir_codegen_target_ir_value_ref_t) {
                .instr_ref = phi_ref,
                .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0)
            }, &vreg_idx));
            if (vreg_idx != KEFIR_ASMCMP_INDEX_NONE) {
                REQUIRE_OK(touch_virtual_register(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), (kefir_asmcmp_virtual_register_index_t) vreg_idx, NULL));
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (kefir_size_t live_in_idx = 0; live_in_idx < state->liveness.blocks[block_ref].live_in.length; live_in_idx++) {
        kefir_codegen_target_ir_value_ref_t value_ref = state->liveness.blocks[block_ref].live_in.content[live_in_idx];
        if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(value_ref.aspect)) {
            kefir_asmcmp_virtual_register_index_t vreg_idx;
            REQUIRE_OK(resolve_value_virtual_register(state, value_ref, &vreg_idx));
            if (vreg_idx != KEFIR_ASMCMP_INDEX_NONE) {
                REQUIRE_OK(touch_virtual_register(state, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), (kefir_asmcmp_virtual_register_index_t) vreg_idx, NULL));
            }
        }
    }

    const struct kefir_source_location *current_source_location = NULL;
    kefir_codegen_target_ir_metadata_code_ref_t current_code_ref = KEFIR_CODEGEN_TARGET_IR_METADATA_CODE_REF_NONE;
    kefir_asmcmp_instruction_index_t current_source_location_start = KEFIR_ASMCMP_INDEX_NONE;
    kefir_asmcmp_instruction_index_t current_code_ref_start = kefir_asmcmp_context_instr_tail(state->asmcmp_ctx);
#define WRITE_SOURCE_LOCATION \
    do { \
        const kefir_asmcmp_instruction_index_t end_idx = kefir_asmcmp_context_instr_length(state->asmcmp_ctx); \
        REQUIRE_OK(kefir_asmcmp_debug_info_source_map_add_location(state->mem, &state->asmcmp_ctx->debug_info.source_map, \
                                                        &state->asmcmp_ctx->strings, current_source_location_start, end_idx, \
                                                        current_source_location)); \
        current_source_location = NULL; \
        current_source_location_start = KEFIR_ASMCMP_INDEX_NONE; \
    } while (0)

#define WRITE_CODE_REF \
    do { \
        if (current_code_ref != KEFIR_CODEGEN_TARGET_IR_METADATA_CODE_REF_NONE) { \
            kefir_asmcmp_instruction_index_t current_code_ref_end = kefir_asmcmp_context_instr_tail(state->asmcmp_ctx); \
            if (current_code_ref_start != current_code_ref_end) { \
                kefir_asmcmp_label_index_t begin_label, end_label; \
                REQUIRE_OK(kefir_asmcmp_context_new_label(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_next(state->asmcmp_ctx, current_code_ref_start), &begin_label)); \
                REQUIRE_OK(kefir_asmcmp_context_new_label(state->mem, state->asmcmp_ctx, KEFIR_ASMCMP_INDEX_NONE, &end_label)); \
                REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(state->mem, state->asmcmp_ctx, end_label)); \
                REQUIRE_OK(kefir_asmcmp_context_label_mark_external_dependencies(state->mem, state->asmcmp_ctx, begin_label)); \
                REQUIRE_OK(kefir_asmcmp_context_label_mark_external_dependencies(state->mem, state->asmcmp_ctx, end_label)); \
                REQUIRE_OK(state->parameter->new_code_fragment(state->mem, current_code_ref, begin_label, end_label, state->parameter->payload)); \
            } \
        } \
    } while (0)

    for (kefir_codegen_target_ir_instruction_ref_t instr_ref =  kefir_codegen_target_ir_code_block_control_head(state->code, block_ref);
        instr_ref != KEFIR_ID_NONE;
        instr_ref = kefir_codegen_target_ir_code_control_next(state->code, instr_ref)) {
        const struct kefir_codegen_target_ir_instruction *instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(state->code, instr_ref, &instr));
        if (instr->metadata.source_location.source != NULL) {
            if (current_source_location != NULL && current_source_location->source != NULL &&
                (instr->metadata.source_location.source == NULL ||
                strcmp(instr->metadata.source_location.source, current_source_location->source) != 0 ||
                instr->metadata.source_location.line != current_source_location->line ||
                instr->metadata.source_location.column != current_source_location->column)) {
                WRITE_SOURCE_LOCATION;
            }
            if (current_source_location == NULL) {
                current_source_location = &instr->metadata.source_location;
                current_source_location_start = kefir_asmcmp_context_instr_length(state->asmcmp_ctx);
            }
        } else if (current_source_location != NULL) {
            WRITE_SOURCE_LOCATION;
        }

        if (instr->metadata.code_ref != current_code_ref) {
            WRITE_CODE_REF;
            current_code_ref = instr->metadata.code_ref;
            current_code_ref_start = kefir_asmcmp_context_instr_tail(state->asmcmp_ctx);
        }

        kefir_asmcmp_label_index_t begin_label, end_label;
        if (state->constructor_metadata != NULL) {
            REQUIRE_OK(kefir_asmcmp_context_new_label(state->mem, state->asmcmp_ctx, KEFIR_ASMCMP_INDEX_NONE, &begin_label));
            REQUIRE_OK(kefir_asmcmp_context_new_label(state->mem, state->asmcmp_ctx, KEFIR_ASMCMP_INDEX_NONE, &end_label));
            REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(state->mem, state->asmcmp_ctx, begin_label));
        }
        REQUIRE_OK(translate_instruction(state, block_state, instr_ref));
        if (state->constructor_metadata != NULL) {
            REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(state->mem, state->asmcmp_ctx, end_label));

            REQUIRE_OK(kefir_asmcmp_context_label_mark_external_dependencies(state->mem, state->asmcmp_ctx, begin_label));
            REQUIRE_OK(kefir_asmcmp_context_label_mark_external_dependencies(state->mem, state->asmcmp_ctx, end_label));

            REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->instruction_labels, (kefir_hashtable_key_t) instr_ref,
                (kefir_hashtable_value_t) ((((kefir_uint64_t) begin_label) << 32) | (kefir_uint32_t) end_label)));
        }
    }

    if (current_source_location != NULL) {
        WRITE_SOURCE_LOCATION;
    }
    WRITE_CODE_REF;

#undef WRITE_CODE_REF
#undef WRITE_SOURCE_LOCATION

    if (state->constructor_metadata != NULL) {
        REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(state->mem, state->asmcmp_ctx, block_state->asmcmp_end_label));
    }

    return KEFIR_OK;
}

static kefir_result_t translate_blocks(struct rt_destructor_state *state) {
    REQUIRE_OK(kefir_list_clear(state->mem, &state->queue));
    REQUIRE_OK(kefir_hashset_clear(state->mem, &state->visited));

    REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, kefir_list_tail(&state->queue), (void *) (kefir_uptr_t) state->code->entry_block));
    for (struct kefir_list_entry *iter = kefir_list_head(&state->queue);
        iter != NULL;
        iter = kefir_list_head(&state->queue)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref,
            (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(state->mem, &state->queue, iter));
        if (kefir_hashset_has(&state->visited, (kefir_hashset_key_t) block_ref)) {
            continue;
        }
        REQUIRE_OK(kefir_hashset_add(state->mem, &state->visited, (kefir_hashset_key_t) block_ref));
        REQUIRE_OK(translate_block(state, block_ref));

        kefir_result_t res;
        struct kefir_hashtreeset_iterator iter;
        for (res = kefir_hashtreeset_iter(&state->control_flow.blocks[block_ref].successors, &iter); res == KEFIR_OK;
            res = kefir_hashtreeset_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, successor_block_ref, iter.entry);
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, kefir_list_tail(&state->queue), (void *) (kefir_uptr_t) successor_block_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    kefir_result_t res;
    struct kefir_hashtreeset_iterator iter;
    kefir_bool_t has_dead_indirect_targets = false;
    for (res = kefir_hashtreeset_iter(&state->control_flow.indirect_jump_targets, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref, iter.entry);
        if (state->control_flow.blocks[block_ref].immediate_dominator == KEFIR_ID_NONE && block_ref != state->code->entry_block) {
            kefir_hashtable_value_t table_value;
            REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) block_ref, &table_value));
            ASSIGN_DECL_CAST(struct block_state *, block_state,
                table_value);
            REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(state->mem, state->asmcmp_ctx, block_state->asmcmp_label));
            has_dead_indirect_targets = true;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    if (has_dead_indirect_targets) {
        struct kefir_asmcmp_instruction instr = {
            .opcode = state->parameter->unreachable_opcode,
            .args[0].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
            .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
            .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE
        };
        REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), &instr, NULL));
    }

    struct kefir_asmcmp_instruction instr = {
        .opcode = state->parameter->noop_opcode,
        .args[0].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
        .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
        .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE
    };
    REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(state->asmcmp_ctx), &instr, NULL));

    return KEFIR_OK;
}

static kefir_result_t generate_metadata(struct rt_destructor_state *state) {
    REQUIRE(state->constructor_metadata != NULL, KEFIR_OK);

    kefir_result_t res;
    kefir_hashtable_key_t table_key;
    kefir_hashtable_value_t table_value;
    struct kefir_hashtable_iterator iter;
    for (res = kefir_hashtable_iter(&state->value_vregs, &iter, &table_key, &table_value);
        res == KEFIR_OK;
        res = kefir_hashtable_next(&iter, &table_key, &table_value)) {
        kefir_codegen_target_ir_value_ref_t value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(table_key);
        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg_idx, table_value);

        const struct kefir_codegen_target_ir_value_type *value_type;
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->code, value_ref, &value_type));
        if (value_type->metadata.value_ref == KEFIR_CODEGEN_TARGET_IR_METADATA_VALUE_REF_NONE) {
            continue;
        }

        struct kefir_codegen_target_ir_value_liveness_iterator liveness_iter;
        kefir_codegen_target_ir_block_ref_t block_ref;
        kefir_codegen_target_ir_instruction_ref_t begin_ref, end_ref;
        for (res = kefir_codegen_target_ir_value_liveness_iter(&state->liveness, &liveness_iter, value_ref, &block_ref, &begin_ref, &end_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_value_liveness_next(&liveness_iter, &block_ref, &begin_ref, &end_ref)) {

            if (state->control_flow.blocks[block_ref].immediate_dominator == KEFIR_ID_NONE && block_ref != state->control_flow.code->entry_block) {
                continue;
            }

            kefir_hashtable_value_t table_value;
            REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) block_ref, &table_value));
            ASSIGN_DECL_CAST(struct block_state *, block_state,
                table_value);

            kefir_asmcmp_label_index_t begin_label, end_label;
            if (begin_ref == KEFIR_ID_NONE) {
                begin_label = block_state->asmcmp_label;
            } else {
                res = kefir_hashtable_at(&state->instruction_labels, (kefir_hashtable_key_t) begin_ref, &table_value);
                if (res == KEFIR_NOT_FOUND) {
                    continue;
                }
                REQUIRE_OK(res);
                begin_label = (kefir_uint32_t) table_value;
            }
            if (end_ref == KEFIR_ID_NONE) {
                end_label = block_state->asmcmp_end_label;
            } else {
                res = kefir_hashtable_at(&state->instruction_labels, (kefir_hashtable_key_t) end_ref, &table_value);
                if (res == KEFIR_NOT_FOUND) {
                    continue;
                }
                REQUIRE_OK(res);
                end_label = ((kefir_uint64_t) table_value) >> 32;
            }

            struct kefir_codegen_target_ir_code_constructor_metadata_value_ref_iterator value_ref_iter;
            kefir_codegen_target_ir_metadata_value_ref_t metadata_value_ref;
            for (res = kefir_codegen_target_ir_code_constructor_metadata_value_ref_iter(state->constructor_metadata, &value_ref_iter, value_type->metadata.value_ref, &metadata_value_ref);
                res == KEFIR_OK;
                res = kefir_codegen_target_ir_code_constructor_metadata_value_ref_next(&value_ref_iter, &metadata_value_ref)) {
                REQUIRE_OK(state->parameter->new_value_fragment(state->mem, metadata_value_ref, vreg_idx, begin_label, end_label, state->parameter->payload));
            }
            if (res == KEFIR_NOT_FOUND) {
                continue;
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t rt_destruct_impl(struct rt_destructor_state *state) {
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_build(state->mem, &state->control_flow));
    REQUIRE_OK(kefir_codegen_target_ir_liveness_build(state->mem, &state->control_flow, &state->liveness));
    REQUIRE_OK(map_block_labels(state));
    REQUIRE_OK(translate_blocks(state));
    REQUIRE_OK(generate_metadata(state));
    return KEFIR_OK;
}

static kefir_result_t free_block_state(struct kefir_mem *mem, struct kefir_hashtable *table,
                                                          kefir_hashtable_key_t key, kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct block_state *, block_state,
        value);
    REQUIRE(block_state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR block state"));

    KEFIR_FREE(mem, block_state);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_round_trip_destruct(struct kefir_mem *mem,
    const struct kefir_codegen_target_ir_code *code,
    struct kefir_asmcmp_context *asmcmp_ctx,
    const struct kefir_codegen_target_ir_code_constructor_metadata *constructor_metadata,
    const struct kefir_codegen_target_ir_round_trip_destructor_ops *parameter) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(asmcmp_ctx != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp context"));
    REQUIRE(parameter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR round trip destructor parameter"));

    struct rt_destructor_state state = {
        .mem = mem,
        .code = code,
        .asmcmp_ctx = asmcmp_ctx,
        .parameter = parameter,
        .constructor_metadata = constructor_metadata,
        .next_temp_virtual_block = kefir_codegen_target_ir_code_block_count(code)
    };
    REQUIRE_OK(kefir_hashtable_init(&state.blocks, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&state.blocks, free_block_state, NULL));
    REQUIRE_OK(kefir_hashtable_init(&state.value_vregs, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&state.native_labels, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&state.instruction_labels, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_list_init(&state.queue));
    REQUIRE_OK(kefir_hashset_init(&state.visited, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&state.implicit_read_vregs, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&state.implicit_write_vregs, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashset_init(&state.alive_vregs, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_init(&state.control_flow, state.code));
    REQUIRE_OK(kefir_codegen_target_ir_liveness_init(&state.liveness));

    kefir_result_t res = rt_destruct_impl(&state);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.alive_vregs);
        kefir_codegen_target_ir_liveness_free(mem, &state.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
        kefir_hashtable_free(mem, &state.implicit_read_vregs);
        kefir_hashtable_free(mem, &state.implicit_write_vregs);
        kefir_hashset_free(mem, &state.visited);
        kefir_list_free(mem, &state.queue);
        kefir_hashtable_free(mem, &state.instruction_labels);
        kefir_hashtable_free(mem, &state.native_labels);
        kefir_hashtable_free(mem, &state.value_vregs);
        kefir_hashtable_free(mem, &state.blocks);
        return res;
    });
    res = kefir_hashset_free(mem, &state.alive_vregs);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_liveness_free(mem, &state.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
        kefir_hashtable_free(mem, &state.implicit_read_vregs);
        kefir_hashtable_free(mem, &state.implicit_write_vregs);
        kefir_hashset_free(mem, &state.visited);
        kefir_list_free(mem, &state.queue);
        kefir_hashtable_free(mem, &state.instruction_labels);
        kefir_hashtable_free(mem, &state.native_labels);
        kefir_hashtable_free(mem, &state.value_vregs);
        kefir_hashtable_free(mem, &state.blocks);
        return res;
    });
    res = kefir_codegen_target_ir_liveness_free(mem, &state.liveness);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
        kefir_hashtable_free(mem, &state.implicit_read_vregs);
        kefir_hashtable_free(mem, &state.implicit_write_vregs);
        kefir_hashset_free(mem, &state.visited);
        kefir_list_free(mem, &state.queue);
        kefir_hashtable_free(mem, &state.instruction_labels);
        kefir_hashtable_free(mem, &state.native_labels);
        kefir_hashtable_free(mem, &state.value_vregs);
        kefir_hashtable_free(mem, &state.blocks);
        return res;
    });
    res = kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.implicit_read_vregs);
        kefir_hashtable_free(mem, &state.implicit_write_vregs);
        kefir_hashset_free(mem, &state.visited);
        kefir_list_free(mem, &state.queue);
        kefir_hashtable_free(mem, &state.instruction_labels);
        kefir_hashtable_free(mem, &state.native_labels);
        kefir_hashtable_free(mem, &state.value_vregs);
        kefir_hashtable_free(mem, &state.blocks);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.implicit_read_vregs);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.implicit_write_vregs);
        kefir_hashset_free(mem, &state.visited);
        kefir_list_free(mem, &state.queue);
        kefir_hashtable_free(mem, &state.instruction_labels);
        kefir_hashtable_free(mem, &state.native_labels);
        kefir_hashtable_free(mem, &state.value_vregs);
        kefir_hashtable_free(mem, &state.blocks);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.implicit_write_vregs);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.visited);
        kefir_list_free(mem, &state.queue);
        kefir_hashtable_free(mem, &state.instruction_labels);
        kefir_hashtable_free(mem, &state.native_labels);
        kefir_hashtable_free(mem, &state.value_vregs);
        kefir_hashtable_free(mem, &state.blocks);
        return res;
    });
    res = kefir_hashset_free(mem, &state.visited);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.queue);
        kefir_hashtable_free(mem, &state.instruction_labels);
        kefir_hashtable_free(mem, &state.native_labels);
        kefir_hashtable_free(mem, &state.value_vregs);
        kefir_hashtable_free(mem, &state.blocks);
        return res;
    });
    res = kefir_list_free(mem, &state.queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.instruction_labels);
        kefir_hashtable_free(mem, &state.native_labels);
        kefir_hashtable_free(mem, &state.value_vregs);
        kefir_hashtable_free(mem, &state.blocks);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.instruction_labels);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.native_labels);
        kefir_hashtable_free(mem, &state.value_vregs);
        kefir_hashtable_free(mem, &state.blocks);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.native_labels);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.value_vregs);
        kefir_hashtable_free(mem, &state.blocks);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.value_vregs);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.blocks);
        return res;
    });
    REQUIRE_OK(kefir_hashtable_free(mem, &state.blocks));
    return KEFIR_OK;
}
