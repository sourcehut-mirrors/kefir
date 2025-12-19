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

#include "kefir/codegen/target-ir/amd64/destructor.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/liveness.h"
#include "kefir/codegen/target-ir/schedule.h"
#include "kefir/codegen/target-ir/amd64/regalloc.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

struct destructor_state {
    struct kefir_mem *mem;
    const struct kefir_codegen_target_ir_code *code;
    struct kefir_asmcmp_amd64 *asmcmp_ctx;
    const struct kefir_codegen_target_ir_stack_frame *stack_frame;
    const struct kefir_codegen_target_ir_interference *interference;
    const struct kefir_codegen_target_ir_regalloc *regalloc;
    const struct kefir_codegen_target_ir_code_constructor_metadata *constructor_metadata;
    const struct kefir_codegen_target_ir_round_trip_destructor_ops *destructor_ops;

    struct kefir_codegen_target_ir_control_flow control_flow;
    struct kefir_codegen_target_ir_liveness liveness;
    struct kefir_codegen_target_ir_code_schedule schedule;
    struct kefir_hashtable blocks;
    struct kefir_hashtable native_labels;

    struct {
        kefir_uint8_t *occupied_spill_slots;
        kefir_size_t occupied_spill_slots_length;
        struct kefir_hashset interfere_registers;
        struct kefir_hashset input_registers;
        struct kefir_hashset output_registers;
        struct kefir_hashset implicit_input_registers;
        struct kefir_hashtable scratch_registers;
        struct kefir_hashtable tmp_output_registers;
        struct kefir_hashtable tmp_output_spill;
    } current_instr;
};

struct block_state {
    kefir_codegen_target_ir_block_ref_t block_ref;
    kefir_asmcmp_label_index_t asmcmp_label;
    kefir_asmcmp_label_index_t asmcmp_end_label;
};

static kefir_result_t map_block_labels(struct destructor_state *state) {
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(state->code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(state->code, i);

        kefir_asmcmp_label_index_t asmcmp_label, asmcmp_end_label = KEFIR_ASMCMP_INDEX_NONE;
        REQUIRE_OK(kefir_asmcmp_context_new_label(state->mem, &state->asmcmp_ctx->context, KEFIR_ASMCMP_INDEX_NONE, &asmcmp_label));

        const struct kefir_codegen_target_ir_block *block = kefir_codegen_target_ir_code_block_at(state->code, block_ref);
        REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve target IR block"));

        if (state->constructor_metadata != NULL || block->externally_visible) {
            REQUIRE_OK(kefir_asmcmp_context_label_mark_external_dependencies(state->mem, &state->asmcmp_ctx->context, asmcmp_label));
        }
        if (state->constructor_metadata != NULL) {
            REQUIRE_OK(kefir_asmcmp_context_new_label(state->mem, &state->asmcmp_ctx->context, KEFIR_ASMCMP_INDEX_NONE, &asmcmp_end_label));
            REQUIRE_OK(kefir_asmcmp_context_label_mark_external_dependencies(state->mem, &state->asmcmp_ctx->context, asmcmp_end_label));
        }
        
        kefir_result_t res;
        struct kefir_hashtreeset_iterator iter;
        for (res = kefir_hashtreeset_iter(&block->public_labels, &iter); res == KEFIR_OK;
            res = kefir_hashtreeset_next(&iter)) {
            ASSIGN_DECL_CAST(const char *, public_label, iter.entry);
            REQUIRE_OK(kefir_asmcmp_context_label_add_public_name(state->mem, &state->asmcmp_ctx->context, asmcmp_label, public_label));
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

enum temporary_register_type {
    TEMPORARY_REGISTER_GP,
    TEMPORARY_REGISTER_SSE,
};

static kefir_result_t mark_spill_space_occupied(struct destructor_state *state, kefir_size_t index, kefir_size_t length) {
    if (state->current_instr.occupied_spill_slots_length < index + length) {
        kefir_size_t new_length = index + length;
        kefir_uint8_t *new_spill_slots = KEFIR_REALLOC(state->mem, state->current_instr.occupied_spill_slots, sizeof(kefir_uint8_t) * new_length);
        REQUIRE(new_spill_slots != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate spill space slots"));
        memset(&new_spill_slots[state->current_instr.occupied_spill_slots_length], 0, sizeof(kefir_uint8_t) * (new_length - state->current_instr.occupied_spill_slots_length));
        state->current_instr.occupied_spill_slots = new_spill_slots;
        state->current_instr.occupied_spill_slots_length = new_length;
    }

    memset(&state->current_instr.occupied_spill_slots[index], 1, sizeof(kefir_uint8_t) * length);
    return KEFIR_OK;
}

static kefir_result_t allocate_spill_space(struct destructor_state *state, kefir_size_t length, kefir_size_t alignment, kefir_size_t *index) {
    UNUSED(state);
    UNUSED(length);
    UNUSED(alignment);
    kefir_size_t i = 0;
    for (; i + length < state->current_instr.occupied_spill_slots_length; i += alignment) {
        kefir_bool_t available = true;
        for (kefir_size_t j = 0; available && j < length; j++) {
            if (state->current_instr.occupied_spill_slots[i + j]) {
                available = false;
            }
        }

        if (available) {
            REQUIRE_OK(mark_spill_space_occupied(state, i, length));
            *index = i;
            return KEFIR_OK;
        }
    }
    i = (state->current_instr.occupied_spill_slots_length + alignment - 1) / alignment * alignment;
    REQUIRE_OK(mark_spill_space_occupied(state, i, length));
    *index = i;
    REQUIRE_OK(state->stack_frame->use_spill_space(state->mem, i, length, state->stack_frame->payload));
    return KEFIR_OK;
}

static kefir_result_t allocate_scratch_register(struct destructor_state *state, kefir_asm_amd64_xasmgen_register_t *reg, enum temporary_register_type type,
    kefir_asmcmp_instruction_index_t *insert_idx) {
    kefir_size_t length;
    const kefir_asm_amd64_xasmgen_register_t *register_arr;
    kefir_size_t spill_qwords = 1;
    const struct kefir_codegen_target_ir_amd64_regalloc_class *regalloc_class = state->regalloc->klass->payload;
    switch (type) {
        case TEMPORARY_REGISTER_GP:
            length = regalloc_class->num_of_gp_registers;
            register_arr = regalloc_class->gp_registers;
            break;

        case TEMPORARY_REGISTER_SSE:
            spill_qwords = 2;
            length = regalloc_class->num_of_sse_registers;
            register_arr = regalloc_class->sse_registers;
            break;
    }

    for (kefir_size_t i = 0; i < length; i++) {
        const kefir_asm_amd64_xasmgen_register_t candidate = register_arr[i];

        if (!kefir_hashset_has(&state->current_instr.input_registers, (kefir_hashset_key_t) candidate) &&
            !kefir_hashset_has(&state->current_instr.output_registers, (kefir_hashset_key_t) candidate) &&
            !kefir_hashset_has(&state->current_instr.interfere_registers, (kefir_hashset_key_t) candidate) &&
            !kefir_hashtable_has(&state->current_instr.scratch_registers, (kefir_hashtable_key_t) candidate)) {
            REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->current_instr.scratch_registers, (kefir_hashtable_key_t) candidate, (kefir_hashtable_value_t) -1ll));
            REQUIRE_OK(state->stack_frame->use_register(state->mem, candidate, state->stack_frame->payload));
            *reg = candidate;
            return KEFIR_OK;
        }
    }

    for (kefir_size_t i = 0; i < length; i++) {
        const kefir_asm_amd64_xasmgen_register_t candidate = register_arr[i];

        if (!kefir_hashset_has(&state->current_instr.input_registers, (kefir_hashset_key_t) candidate) &&
            !kefir_hashset_has(&state->current_instr.output_registers, (kefir_hashset_key_t) candidate) &&
            !kefir_hashtable_has(&state->current_instr.scratch_registers, (kefir_hashtable_key_t) candidate)) {
            kefir_size_t spill_index;
            REQUIRE_OK(allocate_spill_space(state, spill_qwords, spill_qwords, &spill_index));
            kefir_asmcmp_instruction_index_t insert_at = insert_idx != NULL
                ? *insert_idx
                : kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context);
            switch (type) {
                case TEMPORARY_REGISTER_GP:
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(
                        state->mem, state->asmcmp_ctx, insert_at,
                        &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(spill_index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                        &KEFIR_ASMCMP_MAKE_PHREG(candidate), insert_idx));
                    break;

                case TEMPORARY_REGISTER_SSE:
                    REQUIRE_OK(kefir_asmcmp_amd64_movaps(
                        state->mem, state->asmcmp_ctx, insert_at,
                        &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(spill_index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
                        &KEFIR_ASMCMP_MAKE_PHREG(candidate), insert_idx));
                    break;
            }
            REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->current_instr.scratch_registers, (kefir_hashtable_key_t) candidate, (kefir_hashtable_value_t) spill_index));
            REQUIRE_OK(state->stack_frame->use_register(state->mem, candidate, state->stack_frame->payload));
            *reg = candidate;
            return KEFIR_OK;
        }
    }

    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find a temporary register for eviction");
}

static kefir_result_t acquire_scratch_register(struct destructor_state *state, kefir_asm_amd64_xasmgen_register_t reg) {
    REQUIRE(kefir_hashset_has(&state->current_instr.interfere_registers, (kefir_hashset_key_t) reg), KEFIR_OK);

    kefir_size_t spill_index;
    if (!kefir_asm_amd64_xasmgen_register_is_floating_point(reg)) {
        REQUIRE_OK(allocate_spill_space(state, 1, 1, &spill_index));
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(spill_index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
            &KEFIR_ASMCMP_MAKE_PHREG(reg), NULL));
    } else {
        REQUIRE_OK(allocate_spill_space(state, 2, 2, &spill_index));
        REQUIRE_OK(kefir_asmcmp_amd64_movaps(
            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(spill_index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
            &KEFIR_ASMCMP_MAKE_PHREG(reg), NULL));
    }
    REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->current_instr.scratch_registers, (kefir_hashtable_key_t) reg, (kefir_hashtable_value_t) spill_index));
    REQUIRE_OK(state->stack_frame->use_register(state->mem, reg, state->stack_frame->payload));
    return KEFIR_OK;
}

static kefir_result_t release_scratch_register(struct destructor_state *state, kefir_asm_amd64_xasmgen_register_t reg) {
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&state->current_instr.scratch_registers, (kefir_hashtable_key_t) reg, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to release temporary register");
    }
    REQUIRE_OK(res);

    if (table_value != (kefir_hashtable_value_t) -1ll) {
        ASSIGN_DECL_CAST(kefir_size_t, spill_index, table_value);
        if (kefir_asm_amd64_xasmgen_register_is_floating_point(reg)) {
            REQUIRE_OK(kefir_asmcmp_amd64_movaps(
                state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                &KEFIR_ASMCMP_MAKE_PHREG(reg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(spill_index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_128BIT), NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                &KEFIR_ASMCMP_MAKE_PHREG(reg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(spill_index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));
        }
    }
    REQUIRE_OK(kefir_hashtable_delete(state->mem, &state->current_instr.scratch_registers, (kefir_hashtable_key_t) reg));

    return KEFIR_OK;
}

static kefir_result_t match_physical_reg_variant(kefir_asm_amd64_xasmgen_register_t *reg,
                                                 kefir_asmcmp_operand_variant_t variant,
                                                kefir_bool_t high_half) {
    switch (variant) {
        case KEFIR_ASMCMP_OPERAND_VARIANT_8BIT:
            if (high_half) {
                REQUIRE_OK(kefir_asm_amd64_xasmgen_register8_high(*reg, reg));
            } else {
                REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(*reg, reg));
            }
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_16BIT:
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register16(*reg, reg));
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_32BIT:
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(*reg, reg));
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_64BIT:
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register64(*reg, reg));
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_80BIT:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected virtual register variant");

        case KEFIR_ASMCMP_OPERAND_VARIANT_128BIT:
            REQUIRE(kefir_asm_amd64_xasmgen_register_is_floating_point(*reg),
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected floating-point register"));
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(*reg, reg));
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT:
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(*reg, reg));
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE:
        case KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected register variant");
    }
    return KEFIR_OK;
}

static kefir_result_t match_physical_reg_to(kefir_asm_amd64_xasmgen_register_t *reg,
                                                 kefir_asm_amd64_xasmgen_register_t other) {
    if (kefir_asm_amd64_xasmgen_register_is_wide(other, 8)) {
        if (kefir_asm_amd64_xasmgen_register_is_high(other)) {
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8_high(*reg, reg));
        } else {
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(*reg, reg));
        }
    } else if (kefir_asm_amd64_xasmgen_register_is_wide(other, 16)) {
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register16(*reg, reg));
    } else if (kefir_asm_amd64_xasmgen_register_is_wide(other, 32)) {
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(*reg, reg));
    } else if (kefir_asm_amd64_xasmgen_register_is_wide(other, 64)) {
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register64(*reg, reg));
    } else if (kefir_asm_amd64_xasmgen_register_is_wide(other, 128)) {
        REQUIRE(kefir_asm_amd64_xasmgen_register_is_floating_point(*reg),
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected floating-point register"));
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(*reg, reg));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected physical register width");
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

static kefir_result_t native_id_to_label(struct destructor_state *state, kefir_codegen_target_ir_native_id_t native_id, kefir_asmcmp_label_index_t *asmcmp_label) {
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&state->native_labels, (kefir_hashtable_key_t) native_id, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *asmcmp_label = (kefir_asmcmp_label_index_t) table_value;
    } else {
        REQUIRE_OK(kefir_asmcmp_context_new_label(state->mem, &state->asmcmp_ctx->context, KEFIR_ASMCMP_INDEX_NONE, asmcmp_label));
        REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->native_labels, (kefir_hashtable_key_t) native_id, *asmcmp_label));
        REQUIRE_OK(state->destructor_ops->bind_native_id(state->mem, *asmcmp_label, native_id, state->destructor_ops->payload));
    }
    return KEFIR_OK;
}

static kefir_result_t resolve_value_ref(struct destructor_state *state, kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_operand_variant_t variant, struct kefir_asmcmp_value *value) {
    const struct kefir_codegen_target_ir_value_type *value_type;
    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->code, value_ref, &value_type));
    kefir_asmcmp_operand_variant_t asmcmp_variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT;
    kefir_bool_t asmcmp_variant_high_half = false;
    REQUIRE_OK(resolve_variant(variant, &asmcmp_variant, &asmcmp_variant_high_half));
    switch (value_type->kind) {
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_UNSPECIFIED:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected kind of target IR value reference");

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT: {
            kefir_codegen_target_ir_regalloc_allocation_t allocation;
            REQUIRE_OK(kefir_codegen_target_ir_regalloc_get(state->regalloc, value_ref, &allocation));
            union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                .allocation = allocation
            };
            switch (entry.type) {
                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_NA:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 target IR register allocation");
                    
                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP:
                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE: {
                    kefir_asm_amd64_xasmgen_register_t reg = entry.reg.value;
                    REQUIRE_OK(match_physical_reg_variant(&reg, asmcmp_variant, asmcmp_variant_high_half));
                    *value = KEFIR_ASMCMP_MAKE_PHREG(reg);
                } break;

                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL:
                    *value = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(entry.spill_area.index, 0, asmcmp_variant);
                    value->internal.write64_to_spill = value_type->kind == KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE &&
                                                    value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT;
                    break;
            }
        } break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE: {
            kefir_asm_amd64_xasmgen_register_t phreg;
            REQUIRE_OK(allocate_scratch_register(state, &phreg, TEMPORARY_REGISTER_GP, NULL));
            const kefir_int64_t offset = value_type->parameters.local_variable.offset;
            if (offset >= KEFIR_INT16_MIN && offset <= KEFIR_INT16_MAX) {
                struct kefir_asmcmp_value arg = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_AREA(
                    value_type->parameters.local_variable.identifier, offset, asmcmp_variant);
                REQUIRE_OK(kefir_asmcmp_amd64_lea(
                    state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                    &KEFIR_ASMCMP_MAKE_PHREG(phreg), &arg, NULL));
            } else if (offset >= KEFIR_INT32_MIN && offset <= KEFIR_INT32_MAX) {
                struct kefir_asmcmp_value arg = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_AREA(
                    value_type->parameters.local_variable.identifier, 0, asmcmp_variant);
                REQUIRE_OK(kefir_asmcmp_amd64_lea(
                    state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                    &KEFIR_ASMCMP_MAKE_PHREG(phreg), &arg, NULL));
                REQUIRE_OK(kefir_asmcmp_amd64_add(
                    state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                    &KEFIR_ASMCMP_MAKE_PHREG(phreg), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
            } else {
                kefir_asm_amd64_xasmgen_register_t phreg2;
                REQUIRE_OK(allocate_scratch_register(state, &phreg2, TEMPORARY_REGISTER_GP, NULL));

                struct kefir_asmcmp_value arg = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_AREA(
                    value_type->parameters.local_variable.identifier, 0, asmcmp_variant);
                REQUIRE_OK(kefir_asmcmp_amd64_lea(
                    state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                    &KEFIR_ASMCMP_MAKE_PHREG(phreg), &arg, NULL));
                REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                    state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                    &KEFIR_ASMCMP_MAKE_PHREG(phreg2), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
                REQUIRE_OK(kefir_asmcmp_amd64_add(
                    state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                    &KEFIR_ASMCMP_MAKE_PHREG(phreg), &KEFIR_ASMCMP_MAKE_PHREG(phreg2), NULL));
                REQUIRE_OK(release_scratch_register(state, phreg2));
            }

            *value = KEFIR_ASMCMP_MAKE_PHREG(phreg);
        } break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_EXTERNAL_MEMORY: {
            kefir_asm_amd64_xasmgen_register_t phreg;
            REQUIRE_OK(allocate_scratch_register(state, &phreg, TEMPORARY_REGISTER_GP, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_lea(
                state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                &KEFIR_ASMCMP_MAKE_PHREG(phreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(value_type->parameters.memory.base_reg,
                                                        value_type->parameters.memory.offset, value->vreg.variant),
                NULL));
            *value = KEFIR_ASMCMP_MAKE_PHREG(phreg);
        } break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE: {
            kefir_codegen_target_ir_regalloc_allocation_t allocation;
            REQUIRE_OK(kefir_codegen_target_ir_regalloc_get(state->regalloc, value_ref, &allocation));
            union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                .allocation = allocation
            };
            REQUIRE(entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected spill space allocation"));

            kefir_asm_amd64_xasmgen_register_t phreg;
            REQUIRE_OK(allocate_scratch_register(state, &phreg, TEMPORARY_REGISTER_GP, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_lea(
                state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                &KEFIR_ASMCMP_MAKE_PHREG(phreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(entry.spill_area.index, 0,
                                                    asmcmp_variant),
                NULL));
            *value = KEFIR_ASMCMP_MAKE_PHREG(phreg);
        } break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLAGS:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t resolve_operand(struct destructor_state *state, const struct kefir_codegen_target_ir_operand *operand, struct kefir_asmcmp_value *value) {
    switch (operand->type) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE:
            value->type = KEFIR_ASMCMP_VALUE_TYPE_NONE;
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UINTEGER:
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER:
            *value = KEFIR_ASMCMP_MAKE_INT(operand->immediate.int_immediate);
            REQUIRE_OK(resolve_variant(operand->immediate.variant, &value->immediate_variant, NULL));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_PHYSICAL_REGISTER:
            *value = KEFIR_ASMCMP_MAKE_PHREG(operand->phreg);
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF:
            REQUIRE_OK(resolve_value_ref(state, operand->direct.value_ref, operand->direct.variant, value));
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT: {
            kefir_asmcmp_operand_variant_t variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT;
            REQUIRE_OK(resolve_variant(operand->indirect.variant, &variant, NULL));
            switch (operand->indirect.type) {
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_PHYSICAL_BASIS:
                    *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(operand->indirect.base.phreg, operand->indirect.offset, variant);
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS: {
                    const struct kefir_codegen_target_ir_value_type *value_type;
                    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->code, operand->indirect.base.value_ref, &value_type));
                    switch (value_type->kind) {
                        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_UNSPECIFIED:
                        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE:
                        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLAGS:
                        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT:
                        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT: {
                            kefir_codegen_target_ir_regalloc_allocation_t allocation;
                            REQUIRE_OK(kefir_codegen_target_ir_regalloc_get(state->regalloc, operand->indirect.base.value_ref, &allocation));
                            union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                                .allocation = allocation
                            };
                            switch (entry.type) {
                                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_NA:
                                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 target IR register allocation");
                                    
                                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP:
                                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE:
                                    *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(entry.reg.value, operand->indirect.offset,
                                                                                variant);
                                    break;

                                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL: {
                                    kefir_asm_amd64_xasmgen_register_t phreg;
                                    REQUIRE_OK(allocate_scratch_register(state, &phreg, TEMPORARY_REGISTER_GP, NULL));
                                    REQUIRE_OK(kefir_asmcmp_amd64_mov(
                                        state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                                        &KEFIR_ASMCMP_MAKE_PHREG(phreg),
                                        &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(entry.spill_area.index, 0,
                                                                        KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                                        NULL));
                                    *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(phreg, operand->indirect.offset,
                                                                                variant);
                                } break;
                            }
                        } break;

                        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE: {
                            kefir_codegen_target_ir_regalloc_allocation_t allocation;
                            REQUIRE_OK(kefir_codegen_target_ir_regalloc_get(state->regalloc, operand->indirect.base.value_ref, &allocation));
                            union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                                .allocation = allocation
                            };
                            switch (entry.type) {
                                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_NA:
                                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP:
                                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE:
                                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 target IR spill space allocation");
                                    
                                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL:
                                    *value = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(entry.spill_area.index, operand->indirect.offset, variant);
                                    break;
                            }
                        } break;

                        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE: {
                            kefir_int64_t offset = value_type->parameters.local_variable.offset + operand->indirect.offset;
                            if (offset >= KEFIR_INT16_MIN && offset <= KEFIR_INT16_MAX) {
                                *value = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_AREA(
                                    value_type->parameters.local_variable.identifier, offset, variant);
                            } else if (offset >= KEFIR_INT32_MIN && offset <= KEFIR_INT32_MAX) {
                                kefir_asm_amd64_xasmgen_register_t phreg;
                                REQUIRE_OK(allocate_scratch_register(state, &phreg, TEMPORARY_REGISTER_GP, NULL));

                                struct kefir_asmcmp_value arg = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_AREA(
                                    value_type->parameters.local_variable.identifier, 0, variant);
                                REQUIRE_OK(kefir_asmcmp_amd64_lea(
                                    state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                                    &KEFIR_ASMCMP_MAKE_PHREG(phreg), &arg, NULL));
                                REQUIRE_OK(kefir_asmcmp_amd64_add(
                                    state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                                    &KEFIR_ASMCMP_MAKE_PHREG(phreg), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
                                *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(phreg, 0, variant);
                            } else {
                                kefir_asm_amd64_xasmgen_register_t phreg, phreg2;
                                REQUIRE_OK(allocate_scratch_register(state, &phreg, TEMPORARY_REGISTER_GP, NULL));
                                REQUIRE_OK(allocate_scratch_register(state, &phreg2, TEMPORARY_REGISTER_GP, NULL));

                                struct kefir_asmcmp_value arg = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_AREA(
                                    value_type->parameters.local_variable.identifier, 0, variant);
                                REQUIRE_OK(kefir_asmcmp_amd64_lea(
                                    state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                                    &KEFIR_ASMCMP_MAKE_PHREG(phreg), &arg, NULL));
                                REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                                    state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                                    &KEFIR_ASMCMP_MAKE_PHREG(phreg2), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
                                REQUIRE_OK(kefir_asmcmp_amd64_add(
                                    state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                                    &KEFIR_ASMCMP_MAKE_PHREG(phreg), &KEFIR_ASMCMP_MAKE_PHREG(phreg2), NULL));
                                REQUIRE_OK(release_scratch_register(state, phreg2));

                                *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(phreg, 0, variant);
                            }
                        } break;

                        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_EXTERNAL_MEMORY:
                            *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(value_type->parameters.memory.base_reg, value_type->parameters.memory.offset + operand->indirect.offset, variant);
                            break;
                    }
                } break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_IMMEDIATE_BASIS: {
                    kefir_asm_amd64_xasmgen_register_t phreg;
                    REQUIRE_OK(allocate_scratch_register(state, &phreg, TEMPORARY_REGISTER_GP, NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(
                        state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                        &KEFIR_ASMCMP_MAKE_PHREG(phreg), &KEFIR_ASMCMP_MAKE_INT(operand->indirect.base.immediate),
                        NULL));
                    *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(phreg, operand->indirect.offset,
                                                                    variant);
                } break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS: {
                    kefir_hashtable_value_t table_value;
                    REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) operand->block_ref, &table_value));
                    ASSIGN_DECL_CAST(struct block_state *, block_state,
                        table_value);
                    *value = KEFIR_ASMCMP_MAKE_INDIRECT_INTERNAL_LABEL(block_state->asmcmp_label, variant);
                } break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_NATIVE_LABEL_BASIS: {
                    kefir_asmcmp_label_index_t asmcmp_label;
                    REQUIRE_OK(native_id_to_label(state, operand->indirect.base.native_id, &asmcmp_label));
                    *value = KEFIR_ASMCMP_MAKE_INDIRECT_INTERNAL_LABEL(asmcmp_label, variant);
                } break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_EXTERNAL_LABEL_BASIS: {
                    kefir_asmcmp_external_label_relocation_t reloc = KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE;
                    REQUIRE_OK(resolve_relocation(operand->indirect.base.external_type, &reloc));
                    *value = KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(reloc, operand->indirect.base.external_label, operand->indirect.offset, variant);
                } break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_LOCAL_AREA_BASIS:
                    *value = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_AREA(operand->indirect.base.local_variable_id, operand->indirect.offset, variant);
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_SPILL_AREA_BASIS:
                    *value = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(operand->indirect.base.spill_index, operand->indirect.offset, variant);
                    break;
            }
        } break;

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

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UPSILON:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected target IR instruction operand");
    }
    if (operand->segment.present) {
        value->segment.present = true;
        value->segment.reg = operand->segment.reg;
    } else {
        value->segment.present = false;
    }
    return KEFIR_OK;
}

static kefir_result_t copy_spill_area(struct destructor_state *state,
                                      kefir_size_t from, kefir_size_t to, kefir_size_t length) {
    REQUIRE(from != to, KEFIR_OK);
    kefir_bool_t forward_direction = true;
    if (to >= from && to < from + length) {
        forward_direction = false;
    }

    kefir_asm_amd64_xasmgen_register_t tmp_reg;
    REQUIRE_OK(allocate_scratch_register(state, &tmp_reg, TEMPORARY_REGISTER_GP, NULL));
    for (kefir_size_t i = 0; i < length; i++) {
        const kefir_size_t index = forward_direction ? i : length - i - 1;
        kefir_size_t new_position;
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
            &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(from + index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), &new_position));
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(to + index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
            &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), NULL));
    }
    REQUIRE_OK(release_scratch_register(state, tmp_reg));
    return KEFIR_OK;
}

static kefir_result_t link_values(struct destructor_state *state, kefir_codegen_target_ir_value_ref_t dst_ref, kefir_codegen_target_ir_value_ref_t src_ref) {
    const struct kefir_codegen_target_ir_value_type *dst_value_type, *src_value_type;
    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->code, dst_ref, &dst_value_type));
    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->code, src_ref, &src_value_type));

    switch (dst_value_type->kind) {
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_UNSPECIFIED:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLAGS:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_EXTERNAL_MEMORY:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected target IR value type");

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT: {
            kefir_codegen_target_ir_regalloc_allocation_t dst_alloc;
            REQUIRE_OK(kefir_codegen_target_ir_regalloc_get(state->regalloc, dst_ref, &dst_alloc));
            union kefir_codegen_target_ir_amd64_regalloc_entry dst_alloc_entry = {
                .allocation = dst_alloc
            };

            switch (src_value_type->kind) {
                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_UNSPECIFIED:
                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLAGS:
                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected target IR value type");

                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE:
                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT: {
                    kefir_codegen_target_ir_regalloc_allocation_t src_alloc;
                    REQUIRE_OK(kefir_codegen_target_ir_regalloc_get(state->regalloc, src_ref, &src_alloc));
                    union kefir_codegen_target_ir_amd64_regalloc_entry src_alloc_entry = {
                        .allocation = src_alloc
                    };

                    if (dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP &&
                        src_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP) {
                        if (dst_alloc_entry.reg.value != src_alloc_entry.reg.value) {
                            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                                state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                                &KEFIR_ASMCMP_MAKE_PHREG(dst_alloc_entry.reg.value),
                                &KEFIR_ASMCMP_MAKE_PHREG(src_alloc_entry.reg.value), NULL));
                        }
                    } else if (dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP &&
                        src_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL) {
                        REQUIRE(src_alloc_entry.spill_area.length >= 1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected spill area size"));
                        REQUIRE_OK(kefir_asmcmp_amd64_mov(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(dst_alloc_entry.reg.value),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(src_alloc_entry.spill_area.index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
                    } else if (dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL &&
                        src_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP) {
                        REQUIRE(dst_alloc_entry.spill_area.length >= 1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected spill area size"));
                        REQUIRE_OK(kefir_asmcmp_amd64_mov(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(dst_alloc_entry.spill_area.index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                            &KEFIR_ASMCMP_MAKE_PHREG(src_alloc_entry.reg.value), NULL));
                    } else if (dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE &&
                        src_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE) {
                        if (dst_alloc_entry.reg.value != src_alloc_entry.reg.value) {
                            REQUIRE_OK(kefir_asmcmp_amd64_movaps(
                                state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                                &KEFIR_ASMCMP_MAKE_PHREG(dst_alloc_entry.reg.value),
                                &KEFIR_ASMCMP_MAKE_PHREG(src_alloc_entry.reg.value), NULL));
                        }
                    } else if (dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE &&
                        src_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL) {
                        if (src_alloc_entry.spill_area.length >= 2) {
                            REQUIRE_OK(kefir_asmcmp_amd64_movaps(
                                state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                                &KEFIR_ASMCMP_MAKE_PHREG(dst_alloc_entry.reg.value),
                                &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(src_alloc_entry.spill_area.index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_128BIT), NULL));
                        } else {
                            REQUIRE_OK(kefir_asmcmp_amd64_movq(
                                state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                                &KEFIR_ASMCMP_MAKE_PHREG(dst_alloc_entry.reg.value),
                                &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(src_alloc_entry.spill_area.index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_128BIT), NULL));
                        }
                    } else if (dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL &&
                        src_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE) {
                        if (dst_alloc_entry.spill_area.length >= 2) {
                            REQUIRE_OK(kefir_asmcmp_amd64_movaps(
                                state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                                &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(dst_alloc_entry.spill_area.index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
                                &KEFIR_ASMCMP_MAKE_PHREG(src_alloc_entry.reg.value), NULL));
                        } else {
                            REQUIRE_OK(kefir_asmcmp_amd64_movq(
                                state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                                &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(dst_alloc_entry.spill_area.index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
                                &KEFIR_ASMCMP_MAKE_PHREG(src_alloc_entry.reg.value), NULL));
                        }
                    } else if ((dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE &&
                        src_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP) ||
                        (dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP &&
                        src_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE)) {
                        REQUIRE_OK(kefir_asmcmp_amd64_movq(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(dst_alloc_entry.reg.value),
                            &KEFIR_ASMCMP_MAKE_PHREG(src_alloc_entry.reg.value), NULL));
                    } else if (dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL &&
                        src_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL) {
                        REQUIRE_OK(copy_spill_area(state, src_alloc_entry.spill_area.index, dst_alloc_entry.spill_area.index, MIN(src_alloc_entry.spill_area.length, dst_alloc_entry.spill_area.length)));
                    }
                } break;

                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE: {
                    kefir_codegen_target_ir_regalloc_allocation_t src_alloc;
                    REQUIRE_OK(kefir_codegen_target_ir_regalloc_get(state->regalloc, src_ref, &src_alloc));
                    union kefir_codegen_target_ir_amd64_regalloc_entry src_alloc_entry = {
                        .allocation = src_alloc
                    };

                    REQUIRE(src_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected spill space allocation"));
                        
                    if (dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP) {
                        REQUIRE_OK(kefir_asmcmp_amd64_lea(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(dst_alloc_entry.reg.value),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(src_alloc_entry.spill_area.index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
                    } else if (dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE) {
                        kefir_asm_amd64_xasmgen_register_t phreg;
                        REQUIRE_OK(allocate_scratch_register(state, &phreg, TEMPORARY_REGISTER_GP, NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_lea(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(src_alloc_entry.spill_area.index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_movq(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(dst_alloc_entry.reg.value),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg), NULL));
                        REQUIRE_OK(release_scratch_register(state, phreg));
                    } else if (dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL) {
                        kefir_asm_amd64_xasmgen_register_t phreg;
                        REQUIRE_OK(allocate_scratch_register(state, &phreg, TEMPORARY_REGISTER_GP, NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_lea(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(src_alloc_entry.spill_area.index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_mov(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(dst_alloc_entry.spill_area.index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg), NULL));
                        REQUIRE_OK(release_scratch_register(state, phreg));
                    } else {
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected target IR register allocation");
                    }
                } break;

                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE: {
                    kefir_asm_amd64_xasmgen_register_t phreg;
                    if (dst_alloc_entry.type != KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP) {
                        REQUIRE_OK(allocate_scratch_register(state, &phreg, TEMPORARY_REGISTER_GP, NULL));
                    } else {
                        phreg = dst_alloc_entry.reg.value;
                    }
                    const kefir_int64_t offset = src_value_type->parameters.local_variable.offset;
                    if (offset >= KEFIR_INT16_MIN && offset <= KEFIR_INT16_MAX) {
                        struct kefir_asmcmp_value arg = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_AREA(
                            src_value_type->parameters.local_variable.identifier, offset, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT);
                        REQUIRE_OK(kefir_asmcmp_amd64_lea(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg), &arg, NULL));
                    } else if (offset >= KEFIR_INT32_MIN && offset <= KEFIR_INT32_MAX) {
                        struct kefir_asmcmp_value arg = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_AREA(
                            src_value_type->parameters.local_variable.identifier, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT);
                        REQUIRE_OK(kefir_asmcmp_amd64_lea(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg), &arg, NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_add(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
                    } else {
                        kefir_asm_amd64_xasmgen_register_t phreg2;
                        REQUIRE_OK(allocate_scratch_register(state, &phreg2, TEMPORARY_REGISTER_GP, NULL));

                        struct kefir_asmcmp_value arg = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_AREA(
                            src_value_type->parameters.local_variable.identifier, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT);
                        REQUIRE_OK(kefir_asmcmp_amd64_lea(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg), &arg, NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg2), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_add(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg), &KEFIR_ASMCMP_MAKE_PHREG(phreg2), NULL));
                        REQUIRE_OK(release_scratch_register(state, phreg2));
                    }

                    if (dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE) {
                        REQUIRE_OK(kefir_asmcmp_amd64_movq(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(dst_alloc_entry.reg.value), &KEFIR_ASMCMP_MAKE_PHREG(phreg), NULL));
                        REQUIRE_OK(release_scratch_register(state, phreg));
                    } else if (dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL) {
                        REQUIRE_OK(kefir_asmcmp_amd64_mov(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(dst_alloc_entry.spill_area.index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), &KEFIR_ASMCMP_MAKE_PHREG(phreg), NULL));
                        REQUIRE_OK(release_scratch_register(state, phreg));
                    }
                } break;

                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_EXTERNAL_MEMORY:
                    REQUIRE_OK(kefir_asmcmp_amd64_lea(
                        state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context), &KEFIR_ASMCMP_MAKE_PHREG(dst_alloc_entry.reg.value),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(src_value_type->parameters.memory.base_reg,
                                                                src_value_type->parameters.memory.offset,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                        NULL));
                    break;
            }
        } break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE: {
            kefir_codegen_target_ir_regalloc_allocation_t dst_alloc;
            REQUIRE_OK(kefir_codegen_target_ir_regalloc_get(state->regalloc, dst_ref, &dst_alloc));
            union kefir_codegen_target_ir_amd64_regalloc_entry dst_alloc_entry = {
                .allocation = dst_alloc
            };
            REQUIRE(dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected spill space allocation"));

            switch (src_value_type->kind) {
                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_UNSPECIFIED:
                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE:
                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT:
                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLAGS:
                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT:
                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE:
                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_EXTERNAL_MEMORY:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected target IR value type");

                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE: {
                    kefir_codegen_target_ir_regalloc_allocation_t src_alloc;
                    REQUIRE_OK(kefir_codegen_target_ir_regalloc_get(state->regalloc, src_ref, &src_alloc));
                    union kefir_codegen_target_ir_amd64_regalloc_entry src_alloc_entry = {
                        .allocation = src_alloc
                    };
                    REQUIRE(src_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected spill space allocation"));
                    REQUIRE(src_alloc_entry.spill_area.length == dst_alloc_entry.spill_area.length, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected spill space allocation"));

                    REQUIRE_OK(copy_spill_area(state, src_alloc_entry.spill_area.index, dst_alloc_entry.spill_area.index, dst_alloc_entry.spill_area.length));
                } break;
            }
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t assign_immediate(struct destructor_state *state, kefir_codegen_target_ir_value_ref_t dst_ref, kefir_int64_t value) {
    const struct kefir_codegen_target_ir_value_type *dst_value_type;
    REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->code, dst_ref, &dst_value_type));

    switch (dst_value_type->kind) {
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_UNSPECIFIED:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLAGS:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_EXTERNAL_MEMORY:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected target IR value type");

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE: {
            kefir_codegen_target_ir_regalloc_allocation_t dst_alloc;
            REQUIRE_OK(kefir_codegen_target_ir_regalloc_get(state->regalloc, dst_ref, &dst_alloc));
            union kefir_codegen_target_ir_amd64_regalloc_entry dst_alloc_entry = {
                .allocation = dst_alloc
            };
            if (dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP) {
                if (value >= KEFIR_INT32_MIN && value <= KEFIR_INT32_MAX) {
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(
                        state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                        &KEFIR_ASMCMP_MAKE_PHREG(dst_alloc_entry.reg.value),
                        &KEFIR_ASMCMP_MAKE_INT(value), NULL));
                } else {
                    REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                        state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                        &KEFIR_ASMCMP_MAKE_PHREG(dst_alloc_entry.reg.value),
                        &KEFIR_ASMCMP_MAKE_INT(value), NULL));
                }
            } else if (dst_alloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL) {
                if (value >= KEFIR_INT32_MIN && value <= KEFIR_INT32_MAX) {
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(
                        state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(dst_alloc_entry.spill_area.index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                        &KEFIR_ASMCMP_MAKE_INT(value), NULL));
                } else {
                    kefir_asm_amd64_xasmgen_register_t phreg;
                    REQUIRE_OK(allocate_scratch_register(state, &phreg, TEMPORARY_REGISTER_GP, NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                        state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                        &KEFIR_ASMCMP_MAKE_PHREG(phreg),
                        &KEFIR_ASMCMP_MAKE_INT(value), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(
                        state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(dst_alloc_entry.spill_area.index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                        &KEFIR_ASMCMP_MAKE_PHREG(phreg), NULL));
                    REQUIRE_OK(release_scratch_register(state, phreg));
                }
            } else {
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected target IR register allocation");
            }
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t build_current_instr_state(struct destructor_state *state, kefir_codegen_target_ir_instruction_ref_t instr_ref, const struct kefir_codegen_target_ir_target_ir_instruction_destructor_classification *classification) {
    REQUIRE_OK(kefir_hashset_clear(state->mem, &state->current_instr.interfere_registers));
    REQUIRE_OK(kefir_hashset_clear(state->mem, &state->current_instr.input_registers));
    REQUIRE_OK(kefir_hashset_clear(state->mem, &state->current_instr.implicit_input_registers));
    REQUIRE_OK(kefir_hashset_clear(state->mem, &state->current_instr.output_registers));
    REQUIRE_OK(kefir_hashtable_clear(&state->current_instr.scratch_registers));
    REQUIRE_OK(kefir_hashtable_clear(&state->current_instr.tmp_output_registers));
    REQUIRE_OK(kefir_hashtable_clear(&state->current_instr.tmp_output_spill));
    if (state->current_instr.occupied_spill_slots_length > 0) {
        memset(state->current_instr.occupied_spill_slots, 0, sizeof(kefir_uint8_t) * state->current_instr.occupied_spill_slots_length);
    }

    kefir_result_t res;
    struct kefir_codegen_target_ir_value_iterator value_iter;
    kefir_codegen_target_ir_value_ref_t value_ref;
    for (res = kefir_codegen_target_ir_code_value_iter(state->code, &value_iter, instr_ref, &value_ref, NULL);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_value_next(&value_iter, &value_ref, NULL)) {
        struct kefir_graph_edge_iterator iter;
        kefir_graph_vertex_id_t interfere_vertex_id;
        for (res = kefir_graph_edge_iter(&state->interference->interference_graph, &iter, (kefir_graph_vertex_id_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &interfere_vertex_id);
            res == KEFIR_OK;
            res = kefir_graph_edge_next(&iter, &interfere_vertex_id)) {
            kefir_codegen_target_ir_value_ref_t interfere_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(interfere_vertex_id);

            kefir_codegen_target_ir_regalloc_allocation_t allocation;
            res = kefir_codegen_target_ir_regalloc_get(state->regalloc, interfere_value_ref, &allocation);
            if (res == KEFIR_NOT_FOUND) {
                continue;
            }
            REQUIRE_OK(res);

            union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                .allocation = allocation
            };
            switch (entry.type) {
                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_NA:
                    // Intentionally left blank
                    break;

                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL:
                    REQUIRE_OK(mark_spill_space_occupied(state, entry.spill_area.index, entry.spill_area.length));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP:
                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE:
                    REQUIRE_OK(kefir_hashset_add(state->mem, &state->current_instr.interfere_registers, (kefir_hashset_key_t) entry.reg.value));
                    break;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(state->code, instr_ref, &instr));
    if (instr->operation.opcode == state->code->klass->phi_opcode ||
        instr->operation.opcode == state->code->klass->inline_asm_opcode) {
        // Intentionally left blank
    } else {
        for (kefir_size_t i = 0; i < KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS; i++) {
            kefir_codegen_target_ir_value_ref_t value_ref = {
                .instr_ref = KEFIR_ID_NONE,
                .aspect = 0
            };
            switch (instr->operation.parameters[i].type) {
                case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF:
                    value_ref = instr->operation.parameters[i].direct.value_ref;
                    break;

                case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT:
                    switch (instr->operation.parameters[i].indirect.type) {
                        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS:
                            value_ref = instr->operation.parameters[i].indirect.base.value_ref;
                            break;

                        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_PHYSICAL_BASIS:
                        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_IMMEDIATE_BASIS:
                        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS:
                        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_NATIVE_LABEL_BASIS:
                        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_EXTERNAL_LABEL_BASIS:
                        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_LOCAL_AREA_BASIS:
                        case KEFIR_CODEGEN_TARGET_IR_INDIRECT_SPILL_AREA_BASIS:
                            // Intentionally left blank
                            break;
                    }
                    break;

                case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE:
                case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER:
                case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UINTEGER:
                case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_PHYSICAL_REGISTER:
                case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF:
                case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_NATIVE:
                case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_EXTERNAL:
                case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF:
                case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NATIVE_LABEL:
                case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_EXTERNAL_LABEL:
                case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87:
                case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UPSILON:
                    // Intentionally left blank
                    break;
            }

            if (value_ref.instr_ref == KEFIR_ID_NONE) {
                continue;
            }

            kefir_codegen_target_ir_regalloc_allocation_t allocation;
            res = kefir_codegen_target_ir_regalloc_get(state->regalloc, value_ref, &allocation);
            if (res == KEFIR_NOT_FOUND) {
                continue;
            }
            REQUIRE_OK(res);

            union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                .allocation = allocation
            };
            switch (entry.type) {
                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_NA:
                    // Intentionally left blank
                    break;

                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL:
                    REQUIRE_OK(mark_spill_space_occupied(state, entry.spill_area.index, entry.spill_area.length));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP:
                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE:
                    REQUIRE_OK(kefir_hashset_add(state->mem, &state->current_instr.input_registers, (kefir_hashset_key_t) entry.reg.value));
                    break;
            }
        }
    }

    struct kefir_codegen_target_ir_value_iterator output_value_iter;
    struct kefir_codegen_target_ir_value_ref output_value_ref;
    for (res = kefir_codegen_target_ir_code_value_iter(state->code, &output_value_iter, instr_ref, &output_value_ref, NULL);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_value_next(&output_value_iter, &output_value_ref, NULL)) {
        
        kefir_codegen_target_ir_regalloc_allocation_t allocation;
        res = kefir_codegen_target_ir_regalloc_get(state->regalloc, value_ref, &allocation);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
            .allocation = allocation
        };
        switch (entry.type) {
            case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_NA:
                // Intentionally left blank
                break;

            case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL:
                REQUIRE_OK(mark_spill_space_occupied(state, entry.spill_area.index, entry.spill_area.length));
                break;

            case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP:
            case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE:
                REQUIRE_OK(kefir_hashset_add(state->mem, &state->current_instr.output_registers, (kefir_hashset_key_t) entry.reg.value));
                break;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    if (instr->operation.opcode == state->code->klass->upsilon_opcode) {
        kefir_codegen_target_ir_regalloc_allocation_t allocation;
        res = kefir_codegen_target_ir_regalloc_get(state->regalloc, instr->operation.parameters[0].upsilon_ref, &allocation);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);

            union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                .allocation = allocation
            };
            switch (entry.type) {
                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_NA:
                    // Intentionally left blank
                    break;

                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL:
                    REQUIRE_OK(mark_spill_space_occupied(state, entry.spill_area.index, entry.spill_area.length));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP:
                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE:
                    REQUIRE_OK(kefir_hashset_add(state->mem, &state->current_instr.output_registers, (kefir_hashset_key_t) entry.reg.value));
                    break;
            }
        }

        struct kefir_graph_edge_iterator iter;
        kefir_graph_vertex_id_t interfere_vertex_id;
        for (res = kefir_graph_edge_iter(&state->interference->interference_graph, &iter, (kefir_graph_vertex_id_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&instr->operation.parameters[0].upsilon_ref), &interfere_vertex_id);
            res == KEFIR_OK;
            res = kefir_graph_edge_next(&iter, &interfere_vertex_id)) {
            kefir_codegen_target_ir_value_ref_t interfere_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(interfere_vertex_id);

            kefir_codegen_target_ir_regalloc_allocation_t allocation;
            res = kefir_codegen_target_ir_regalloc_get(state->regalloc, interfere_value_ref, &allocation);
            if (res == KEFIR_NOT_FOUND) {
                continue;
            }
            REQUIRE_OK(res);

            union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                .allocation = allocation
            };
            switch (entry.type) {
                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_NA:
                    // Intentionally left blank
                    break;

                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL:
                    REQUIRE_OK(mark_spill_space_occupied(state, entry.spill_area.index, entry.spill_area.length));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP:
                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE:
                    REQUIRE_OK(kefir_hashset_add(state->mem, &state->current_instr.interfere_registers, (kefir_hashset_key_t) entry.reg.value));
                    break;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        if ((classification->operands[i].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ ||
            classification->operands[i].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE) &&
            classification->operands[i].implicit) {
            REQUIRE_OK(kefir_hashset_add(state->mem, &state->current_instr.implicit_input_registers, (kefir_hashset_key_t) classification->operands[i].implicit_params.phreg));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t resolve_implicit_conflict(struct destructor_state *state, struct kefir_asmcmp_value *value) {
    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL:
        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL:
        case KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL:
        case KEFIR_ASMCMP_VALUE_TYPE_EXTERNAL_LABEL:
        case KEFIR_ASMCMP_VALUE_TYPE_X87:
        case KEFIR_ASMCMP_VALUE_TYPE_INLINE_ASSEMBLY_INDEX:
            // Intentionally left blank
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER: {
            kefir_asm_amd64_xasmgen_register_t widest;
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(value->phreg, &widest));
            if (kefir_hashset_has(&state->current_instr.implicit_input_registers, (kefir_hashset_key_t) widest)) {
                kefir_asm_amd64_xasmgen_register_t phreg;
                if (kefir_asm_amd64_xasmgen_register_is_floating_point(value->phreg)) {
                    REQUIRE_OK(allocate_scratch_register(state, &phreg, TEMPORARY_REGISTER_SSE, NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_movaps(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                        &KEFIR_ASMCMP_MAKE_PHREG(phreg), &KEFIR_ASMCMP_MAKE_PHREG(widest), NULL));
                } else {
                    REQUIRE_OK(allocate_scratch_register(state, &phreg, TEMPORARY_REGISTER_GP, NULL));   
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                        &KEFIR_ASMCMP_MAKE_PHREG(phreg), &KEFIR_ASMCMP_MAKE_PHREG(widest), NULL));
                }
                REQUIRE_OK(match_physical_reg_to(&phreg, value->phreg));
                *value = KEFIR_ASMCMP_MAKE_PHREG(phreg);
            }
        } break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS: {
                    kefir_asm_amd64_xasmgen_register_t widest;
                    REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(value->indirect.base.phreg, &widest));
                    if (kefir_hashset_has(&state->current_instr.implicit_input_registers, (kefir_hashset_key_t) widest)) {
                        kefir_asm_amd64_xasmgen_register_t phreg;
                        REQUIRE_OK(allocate_scratch_register(state, &phreg, TEMPORARY_REGISTER_GP, NULL));   
                        REQUIRE_OK(kefir_asmcmp_amd64_mov(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg), &KEFIR_ASMCMP_MAKE_PHREG(widest), NULL));
                        *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(phreg, value->indirect.offset, value->indirect.variant);
                    }
                } break;

                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_INTERNAL_LABEL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_EXTERNAL_LABEL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_LOCAL_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                    // Intentionally left blank
                    break;
            }
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t materialize_attributes(struct destructor_state *state, kefir_codegen_target_ir_instruction_ref_t instr_ref) {
    kefir_result_t res;
    kefir_codegen_target_ir_native_id_t attribute;
    struct kefir_codegen_target_ir_code_attribute_iterator attr_iter;
    for (res = kefir_codegen_target_ir_code_instruction_attribute_iter(state->code, &attr_iter, instr_ref, &attribute);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_instruction_attribute_next(&attr_iter, &attribute)) {
        REQUIRE_OK(state->destructor_ops->materialize_attribute(state->mem, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context), attribute, NULL, state->destructor_ops->payload));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t mov_to_reg(struct destructor_state *state, kefir_asm_amd64_xasmgen_register_t phreg, const struct kefir_asmcmp_value *value) {
    if (kefir_asm_amd64_xasmgen_register_is_floating_point(phreg)) {
        REQUIRE_OK(kefir_asmcmp_amd64_movaps(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
            &KEFIR_ASMCMP_MAKE_PHREG(phreg), value, NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
            &KEFIR_ASMCMP_MAKE_PHREG(phreg), value, NULL));
    }
    return KEFIR_OK;
}

static kefir_result_t is_spill_range_available(struct destructor_state *state, kefir_size_t index, kefir_size_t length, kefir_bool_t *available) {
    *available = true;
    for (kefir_size_t i = 0; *available && i < length; i++) {
        if (index + i >= state->current_instr.occupied_spill_slots_length ||
            state->current_instr.occupied_spill_slots[index + i]) {
            *available = false;
        }
    }
    return KEFIR_OK;
}

#define DEVIRT_HAS_FLAG(_flags, _flag) (((_flags) & (_flag)) != 0)
static kefir_result_t devirtualize_instr_arg(struct destructor_state *state,
                                             struct kefir_asmcmp_instruction *instr,
                                             kefir_size_t arg_idx,
                                             kefir_uint64_t op_flags,
                                             kefir_bool_t no_memory_arg,
                                            kefir_asmcmp_instruction_index_t *insert_idx) {
    kefir_asm_amd64_xasmgen_register_t tmp_reg;
    switch (instr->args[arg_idx].type) {
        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
            if (!DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_IMMEDIATE)) {
                REQUIRE_OK(allocate_scratch_register(state, &tmp_reg, TEMPORARY_REGISTER_GP, insert_idx));
                REQUIRE_OK(match_physical_reg_variant(&tmp_reg, instr->args[arg_idx].immediate_variant, false));
                struct kefir_asmcmp_value original = instr->args[arg_idx];
                instr->args[arg_idx] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
                REQUIRE_OK(kefir_asmcmp_amd64_mov(state->mem, state->asmcmp_ctx,
                                                  *insert_idx,
                                                  &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), &original, insert_idx));
            } else if (instr->args[arg_idx].type == KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER) {
                if (instr->args[arg_idx].vreg.variant == KEFIR_ASMCMP_OPERAND_VARIANT_64BIT) {
                    // Intentionally left blank
                } else if (instr->args[arg_idx].vreg.variant == KEFIR_ASMCMP_OPERAND_VARIANT_32BIT) {
                    instr->args[arg_idx].int_immediate = (kefir_int32_t) instr->args[arg_idx].int_immediate;
                } else if (instr->args[arg_idx].vreg.variant == KEFIR_ASMCMP_OPERAND_VARIANT_16BIT) {
                    instr->args[arg_idx].int_immediate = (kefir_int16_t) instr->args[arg_idx].int_immediate;
                } else if (instr->args[arg_idx].vreg.variant == KEFIR_ASMCMP_OPERAND_VARIANT_8BIT) {
                    instr->args[arg_idx].int_immediate = (kefir_int8_t) instr->args[arg_idx].int_immediate;
                }
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL:
        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL:
            REQUIRE(DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_ANY_MEMORY) && !no_memory_arg,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to devirtualize instruction"));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL:
        case KEFIR_ASMCMP_VALUE_TYPE_EXTERNAL_LABEL:
            REQUIRE(DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_LABEL),
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to devirtualize instruction"));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_X87:
            REQUIRE(DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_FPU_STACK),
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to devirtualize instruction"));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER:
            if (!kefir_asm_amd64_xasmgen_register_is_floating_point(instr->args[arg_idx].phreg)) {
                if (!DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_GP_REGISTER)) {
                    REQUIRE(DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_XMM_REGISTER_ANY),
                            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to devirtualize instruction"));
                    REQUIRE_OK(allocate_scratch_register(state, &tmp_reg, TEMPORARY_REGISTER_SSE, insert_idx));
                    struct kefir_asmcmp_value original = instr->args[arg_idx];
                    instr->args[arg_idx] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
                    if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_READ)) {
                        if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_XMM_REGISTER_SINGLE)) {
                            kefir_asm_amd64_xasmgen_register_t reg32;
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(original.phreg, &reg32));
                            REQUIRE_OK(kefir_asmcmp_amd64_movd(
                                state->mem, state->asmcmp_ctx, *insert_idx,
                                &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), &KEFIR_ASMCMP_MAKE_PHREG(reg32), insert_idx));
                        } else if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_XMM_REGISTER_DOUBLE)) {
                            REQUIRE_OK(kefir_asmcmp_amd64_movq(
                                state->mem, state->asmcmp_ctx, *insert_idx,
                                &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), &original, insert_idx));
                        } else {
                            REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
                                state->mem, state->asmcmp_ctx, *insert_idx,
                                &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), &original, insert_idx));
                        }
                    }

                    if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_WRITE)) {
                        if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_XMM_REGISTER_SINGLE)) {
                            kefir_asm_amd64_xasmgen_register_t reg32;
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(original.phreg, &reg32));
                            REQUIRE_OK(kefir_asmcmp_amd64_movd(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                                                               &KEFIR_ASMCMP_MAKE_PHREG(reg32),
                                                               &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), NULL));
                        } else if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_XMM_REGISTER_SINGLE)) {
                            REQUIRE_OK(kefir_asmcmp_amd64_movq(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context), &original,
                                                               &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), NULL));
                        } else {
                            REQUIRE_OK(kefir_asmcmp_amd64_movdqu(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context), &original,
                                                                 &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), NULL));
                        }
                    }
                }
            } else {
                if (!DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_XMM_REGISTER_ANY)) {
                    REQUIRE(DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_GP_REGISTER),
                            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to devirtualize instruction"));
                    REQUIRE_OK(allocate_scratch_register(state, &tmp_reg, TEMPORARY_REGISTER_GP, insert_idx));
                    struct kefir_asmcmp_value original = instr->args[arg_idx];
                    instr->args[arg_idx] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
                    if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_READ)) {
                        REQUIRE_OK(kefir_asmcmp_amd64_movq(
                            state->mem, state->asmcmp_ctx, *insert_idx,
                            &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), &original, insert_idx));
                    }

                    if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_WRITE)) {
                        REQUIRE_OK(kefir_asmcmp_amd64_movq(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context), &original,
                                                           &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), NULL));
                    }
                }
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            if (!DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_ANY_MEMORY) || no_memory_arg ||
                instr->args[arg_idx].internal.write64_to_spill) {
                if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_GP_REGISTER)) {
                    REQUIRE_OK(allocate_scratch_register(state, &tmp_reg, TEMPORARY_REGISTER_GP, insert_idx));
                    struct kefir_asmcmp_value original = instr->args[arg_idx];
                    kefir_asm_amd64_xasmgen_register_t tmp_reg_variant = tmp_reg;
                    REQUIRE_OK(match_physical_reg_variant(&tmp_reg_variant, original.indirect.variant, false));
                    instr->args[arg_idx] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg_variant);
                    if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_READ)) {
                        REQUIRE_OK(kefir_asmcmp_amd64_mov(
                            state->mem, state->asmcmp_ctx, *insert_idx,
                            &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg_variant), &original, insert_idx));
                    }

                    if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_WRITE)) {
                        if (original.internal.write64_to_spill) {
                            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                                state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                                &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(original.indirect.base.spill_index,
                                                                  original.indirect.offset,
                                                                  KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                                &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), NULL));
                        } else {
                            REQUIRE_OK(kefir_asmcmp_amd64_mov(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context), &original,
                                                              &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg_variant),
                                                              NULL));
                        }
                    }
                } else {
                    REQUIRE(DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_XMM_REGISTER_ANY),
                            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to devirtualize instruction"));
                    REQUIRE_OK(allocate_scratch_register(state, &tmp_reg, TEMPORARY_REGISTER_SSE, insert_idx));
                    struct kefir_asmcmp_value original = instr->args[arg_idx];
                    instr->args[arg_idx] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
                    if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_READ)) {
                        if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_XMM_REGISTER_SINGLE)) {
                            REQUIRE_OK(kefir_asmcmp_amd64_movd(
                                state->mem, state->asmcmp_ctx, *insert_idx,
                                &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), &original, insert_idx));
                        } else if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_XMM_REGISTER_DOUBLE)) {
                            REQUIRE_OK(kefir_asmcmp_amd64_movq(
                                state->mem, state->asmcmp_ctx, *insert_idx,
                                &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), &original, insert_idx));
                        } else {
                            REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
                                state->mem, state->asmcmp_ctx, *insert_idx,
                                &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), &original, insert_idx));
                        }
                    }

                    if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_WRITE)) {
                        if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_XMM_REGISTER_SINGLE)) {
                            REQUIRE_OK(kefir_asmcmp_amd64_movd(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context), &original,
                                                               &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), NULL));
                        } else if (DEVIRT_HAS_FLAG(op_flags, KEFIR_AMD64_INSTRDB_XMM_REGISTER_DOUBLE)) {
                            REQUIRE_OK(kefir_asmcmp_amd64_movq(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context), &original,
                                                               &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), NULL));
                        } else {
                            REQUIRE_OK(kefir_asmcmp_amd64_movdqu(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context), &original,
                                                                 &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), NULL));
                        }
                    }
                }
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
        case KEFIR_ASMCMP_VALUE_TYPE_INLINE_ASSEMBLY_INDEX:
        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected instruction argument type");
    }
    return KEFIR_OK;
}

static kefir_result_t devirtualize_instr1(struct destructor_state *state,
                                          struct kefir_asmcmp_instruction *instr,
                                          kefir_uint64_t op1_flags,
                                            kefir_asmcmp_instruction_index_t *insert_idx) {
    REQUIRE_OK(devirtualize_instr_arg(state, instr, 0, op1_flags, false, insert_idx));

    return KEFIR_OK;
}

static kefir_result_t devirtualize_instr2(struct destructor_state *state,
                                          struct kefir_asmcmp_instruction *instr,
                                          kefir_uint64_t op1_flags, kefir_uint64_t op2_flags,
                                            kefir_asmcmp_instruction_index_t *insert_idx) {
    REQUIRE_OK(devirtualize_instr_arg(state, instr, 0, op1_flags,
                                      (instr->args[1].type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT ||
                                           instr->args[1].type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL ||
                                           instr->args[1].type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL), insert_idx));
    REQUIRE_OK(devirtualize_instr_arg(state, instr, 1, op2_flags, false, insert_idx));

    return KEFIR_OK;
}

static kefir_result_t devirtualize_instr3(struct destructor_state *state,
                                          struct kefir_asmcmp_instruction *instr,
                                          kefir_uint64_t op1_flags, kefir_uint64_t op2_flags,
                                          kefir_uint64_t op3_flags,
                                            kefir_asmcmp_instruction_index_t *insert_idx) {
    REQUIRE_OK(devirtualize_instr_arg(state, instr, 0, op1_flags,
                                      (instr->args[1].type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT ||
                                           instr->args[1].type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL ||
                                           instr->args[1].type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL), insert_idx));
    REQUIRE_OK(devirtualize_instr_arg(state, instr, 1, op2_flags, false, insert_idx));
    REQUIRE_OK(devirtualize_instr_arg(state, instr, 2, op3_flags, false, insert_idx));

    return KEFIR_OK;
}
#undef DEVIRT_HAS_FLAG

static kefir_result_t link_into_phreg(struct destructor_state *state, kefir_bool_t acquire, kefir_asm_amd64_xasmgen_register_t phreg, const struct kefir_asmcmp_value *value) {
    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
        case KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL:
        case KEFIR_ASMCMP_VALUE_TYPE_EXTERNAL_LABEL:
        case KEFIR_ASMCMP_VALUE_TYPE_X87:
        case KEFIR_ASMCMP_VALUE_TYPE_INLINE_ASSEMBLY_INDEX:
            if (acquire) {
                REQUIRE_OK(acquire_scratch_register(state, phreg));
            }
            REQUIRE_OK(mov_to_reg(state, phreg, value));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER: {
            kefir_asm_amd64_xasmgen_register_t widest, widest_phreg;
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(value->phreg, &widest));
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(phreg, &widest_phreg));
            if (widest != widest_phreg) {
                if (acquire) {
                    REQUIRE_OK(acquire_scratch_register(state, phreg));
                }
                REQUIRE_OK(mov_to_reg(state, widest_phreg, &KEFIR_ASMCMP_MAKE_PHREG(widest)));
            }
        } break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT: {
            if (acquire) {
                REQUIRE_OK(acquire_scratch_register(state, phreg));
            }
            kefir_asm_amd64_xasmgen_register_t match_phreg = phreg;
            REQUIRE_OK(match_physical_reg_variant(&match_phreg, value->indirect.variant, false));
            REQUIRE_OK(mov_to_reg(state, match_phreg, value));
        } break;

        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL:
        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL: {
            if (acquire) {
                REQUIRE_OK(acquire_scratch_register(state, phreg));
            }
            kefir_asm_amd64_xasmgen_register_t match_phreg = phreg;
            REQUIRE_OK(match_physical_reg_variant(&match_phreg, value->rip_indirection.variant, false));
            REQUIRE_OK(mov_to_reg(state, match_phreg, value));
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t link_into_spill(struct destructor_state *state, kefir_size_t index, kefir_size_t length, kefir_asmcmp_operand_variant_t variant, const struct kefir_asmcmp_value *value) {
    struct kefir_asmcmp_instruction instr;
    kefir_uint64_t op1_flags = 0, op2_flags = 0;
    if (length == 1) {
        instr = (struct kefir_asmcmp_instruction) {
            .opcode = KEFIR_ASMCMP_AMD64_OPCODE(mov),
            .args[0] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(index, 0, variant),
            .args[1] = *value
        };
        op1_flags = KEFIR_AMD64_INSTRDB_WRITE | KEFIR_AMD64_INSTRDB_GP_REGISTER_MEMORY;
        op2_flags = KEFIR_AMD64_INSTRDB_READ | KEFIR_AMD64_INSTRDB_GP_REGISTER_MEMORY | KEFIR_AMD64_INSTRDB_IMMEDIATE;
    } else if (length == 2) {
        instr = (struct kefir_asmcmp_instruction) {
            .opcode = KEFIR_ASMCMP_AMD64_OPCODE(movdqu),
            .args[0] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(index, 0, variant),
            .args[1] = *value
        };
        op1_flags = KEFIR_AMD64_INSTRDB_WRITE | KEFIR_AMD64_INSTRDB_XMM_REGISTER_MEMORY_FULL;
        op2_flags = KEFIR_AMD64_INSTRDB_READ | KEFIR_AMD64_INSTRDB_XMM_REGISTER_MEMORY_FULL;
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected spill space length");
    }

    kefir_asmcmp_instruction_index_t insert_at = kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context);
    REQUIRE_OK(devirtualize_instr2(state, &instr, op1_flags, op2_flags, &insert_at));
    REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, &state->asmcmp_ctx->context, insert_at,
        &instr, NULL));
    return KEFIR_OK;
}

static kefir_result_t translate_instruction(struct destructor_state *state, kefir_codegen_target_ir_instruction_ref_t instr_ref) {
    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(state->code, instr_ref, &instr));

    if (instr->operation.opcode == state->code->klass->placeholder_opcode ||
        instr->operation.opcode == state->code->klass->phi_opcode ||
        instr->operation.opcode == state->code->klass->touch_opcode) {
        return KEFIR_OK;
    }

    kefir_result_t res;
    struct kefir_codegen_target_ir_target_ir_instruction_destructor_classification classification;
    REQUIRE_OK(state->destructor_ops->classify_instruction(state->code, instr_ref, &classification, state->destructor_ops->payload));


    REQUIRE_OK(materialize_attributes(state, instr_ref));
    REQUIRE_OK(build_current_instr_state(state, instr_ref, &classification));

    struct kefir_codegen_target_ir_block_terminator_props terminator_props;
    REQUIRE_OK(state->code->klass->is_block_terminator(state->code, instr, &terminator_props, state->code->klass->payload));
    if (terminator_props.block_terminator && !terminator_props.function_terminator) {
        if (terminator_props.branch) {
            struct kefir_asmcmp_instruction asmcmp_instrs[2] = {0};

            kefir_hashtable_value_t table_value;
            REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) terminator_props.target_block_refs[0], &table_value));
            ASSIGN_DECL_CAST(struct block_state *, target_block_state,
                table_value);
            REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) terminator_props.target_block_refs[1], &table_value));
            ASSIGN_DECL_CAST(struct block_state *, alternative_block_state,
                table_value);

            REQUIRE(!kefir_codegen_target_ir_control_flow_is_critical_edge(&state->control_flow, instr->block_ref, terminator_props.target_block_refs[0]),
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected critical edges to be split before target IR destruction"));
            REQUIRE(!kefir_codegen_target_ir_control_flow_is_critical_edge(&state->control_flow, instr->block_ref, terminator_props.target_block_refs[1]),
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected critical edges to be split before target IR destruction"));

            asmcmp_instrs[0].opcode = classification.opcode;
            asmcmp_instrs[0].args[0].type = KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL;
            asmcmp_instrs[0].args[0].internal_label = target_block_state->asmcmp_label;
            asmcmp_instrs[0].args[0].segment.present = false;
            REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, &state->asmcmp_ctx->context, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                &asmcmp_instrs[0], NULL));

            const struct kefir_codegen_target_ir_block_schedule *current_block_schedule, *next_block_schedule;
            REQUIRE_OK(kefir_codegen_target_ir_code_schedule_of_block(&state->schedule, instr->block_ref, &current_block_schedule));
            REQUIRE_OK(kefir_codegen_target_ir_code_schedule_of_block(&state->schedule, alternative_block_state->block_ref, &next_block_schedule));

            if (current_block_schedule->linear_position + 1 != next_block_schedule->linear_position) {
                REQUIRE_OK(kefir_asmcmp_amd64_jmp(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                    &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(alternative_block_state->asmcmp_label), NULL));
            }
            return KEFIR_OK;
        } else if (!terminator_props.undefined_target) {
            const struct kefir_codegen_target_ir_block_schedule *current_block_schedule, *next_block_schedule;
            REQUIRE_OK(kefir_codegen_target_ir_code_schedule_of_block(&state->schedule, instr->block_ref, &current_block_schedule));
            REQUIRE_OK(kefir_codegen_target_ir_code_schedule_of_block(&state->schedule, terminator_props.target_block_refs[0], &next_block_schedule));
            if (current_block_schedule->linear_position + 1 != next_block_schedule->linear_position) {
                kefir_hashtable_value_t table_value;
                REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) terminator_props.target_block_refs[0], &table_value));
                ASSIGN_DECL_CAST(struct block_state *, next_block_state,
                    table_value);
                REQUIRE_OK(kefir_asmcmp_amd64_jmp(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                    &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(next_block_state->asmcmp_label), NULL));
            }
            return KEFIR_OK;
        } else if (terminator_props.inline_assembly) {
            // Intentionally left blank
        } else {
            struct kefir_asmcmp_value value;
            REQUIRE_OK(resolve_operand(state, &instr->operation.parameters[0], &value));
            REQUIRE_OK(kefir_asmcmp_amd64_jmp(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                &value, NULL));
            return KEFIR_OK;
        }
    }

    if (instr->operation.opcode == state->code->klass->inline_asm_opcode) {
        kefir_asmcmp_inline_assembly_index_t inline_asm;
        REQUIRE_OK(kefir_asmcmp_inline_assembly_new(state->mem, &state->asmcmp_ctx->context, "", &inline_asm));
        for (const struct kefir_list_entry *iter = kefir_list_head(&instr->operation.inline_asm_node.fragments);
            iter != NULL;
            kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(const struct kefir_codegen_target_ir_inline_assembly_fragment *, fragment,
                iter->value);
            switch (fragment->type) {
                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_TEXT:
                    REQUIRE_OK(kefir_asmcmp_inline_assembly_add_text(state->mem, &state->asmcmp_ctx->context, inline_asm, "%s", fragment->text));
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INLINE_ASSEMBLY_FRAGMENT_OPERAND: {
                    struct kefir_asmcmp_value value;
                    REQUIRE_OK(resolve_operand(state, &fragment->operand, &value));
                    REQUIRE_OK(kefir_asmcmp_inline_assembly_add_value(state->mem, &state->asmcmp_ctx->context, inline_asm, &value));
                } break;
            }
        }

        REQUIRE_OK(state->destructor_ops->new_inline_asm(state->mem, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                                                    inline_asm, NULL, state->destructor_ops->payload));

        if (instr->operation.inline_asm_node.target_block_ref != KEFIR_ID_NONE) {
            kefir_hashtable_value_t table_value;
            REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) instr->operation.inline_asm_node.target_block_ref, &table_value));
            ASSIGN_DECL_CAST(struct block_state *, block_state,
                table_value);

            REQUIRE_OK(kefir_asmcmp_amd64_jmp(state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(block_state->asmcmp_label), NULL));
        }
        return KEFIR_OK;
    } else if (instr->operation.opcode == state->code->klass->assign_opcode) {
        kefir_codegen_target_ir_value_ref_t dst_ref;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction_output(state->code, instr_ref, 0, &dst_ref, NULL));
        if (instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER) {
            REQUIRE_OK(assign_immediate(state, dst_ref, instr->operation.parameters[0].immediate.int_immediate));
        } else {
            REQUIRE(instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected target IR value reference"));
            REQUIRE_OK(link_values(state, dst_ref, instr->operation.parameters[0].direct.value_ref));
        }
        return KEFIR_OK;
    } else if (instr->operation.opcode == state->code->klass->upsilon_opcode) {
        if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(instr->operation.parameters[0].upsilon_ref.aspect)) {
            if (instr->operation.parameters[1].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER) {
                REQUIRE_OK(assign_immediate(state, instr->operation.parameters[0].upsilon_ref, instr->operation.parameters[1].immediate.int_immediate));
            } else {
                REQUIRE(instr->operation.parameters[1].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected target IR value reference"));
                REQUIRE_OK(link_values(state, instr->operation.parameters[0].upsilon_ref, instr->operation.parameters[1].direct.value_ref));
            }
        }
        return KEFIR_OK;
    }

    struct kefir_asmcmp_instruction asmcmp_instruction = {
        .opcode = classification.opcode,
        .args[0].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
        .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
        .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE
    };
    kefir_size_t output_index = 0, parameter_idx = 0;
    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        switch (classification.operands[i].class) {
            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_NONE:
                // Intentionally left blank
                break;

            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ:
                if (!classification.operands[i].implicit) {
                    struct kefir_asmcmp_value value;
                    REQUIRE_OK(resolve_operand(state, &instr->operation.parameters[parameter_idx++], &value));
                    REQUIRE_OK(resolve_implicit_conflict(state, &value));
                    asmcmp_instruction.args[i] = value;
                } else {
                    parameter_idx++;
                }
                break;

            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE:
                if (!classification.operands[i].implicit) {
                    kefir_codegen_target_ir_value_ref_t output_value_ref;
                    const struct kefir_codegen_target_ir_value_type *output_value_type;
                    REQUIRE_OK(kefir_codegen_target_ir_code_instruction_output(state->code, instr_ref, output_index++, &output_value_ref, &output_value_type));
                    if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_INDIRECT_OUTPUT(output_value_ref.aspect)) {
                        struct kefir_asmcmp_value value;
                        REQUIRE_OK(resolve_operand(state, &instr->operation.parameters[parameter_idx++], &value));
                        REQUIRE_OK(resolve_implicit_conflict(state, &value));
                        asmcmp_instruction.args[i] = value;
                    } else {
                        REQUIRE_OK(resolve_value_ref(state, output_value_ref, output_value_type->variant, &asmcmp_instruction.args[i]));
                    }
                } else {
                    output_index++;
                }
                break;

            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE:
                if (!classification.operands[i].implicit) {
                    kefir_codegen_target_ir_value_ref_t output_value_ref;
                    const struct kefir_codegen_target_ir_value_type *output_value_type;
                    REQUIRE_OK(kefir_codegen_target_ir_code_instruction_output(state->code, instr_ref, output_index++, &output_value_ref, &output_value_type));
                    if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_INDIRECT_OUTPUT(output_value_ref.aspect)) {
                        struct kefir_asmcmp_value value;
                        REQUIRE_OK(resolve_operand(state, &instr->operation.parameters[parameter_idx++], &value));
                        REQUIRE_OK(resolve_implicit_conflict(state, &value));
                        asmcmp_instruction.args[i] = value;
                    } else {
                        REQUIRE(KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(output_value_ref.aspect), KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected either output register or indirect output value"));
                        struct kefir_asmcmp_value output_value, input_value;
                        REQUIRE_OK(resolve_operand(state, &instr->operation.parameters[parameter_idx++], &input_value));
                        REQUIRE_OK(resolve_value_ref(state, output_value_ref, output_value_type->variant, &output_value));
                        kefir_codegen_target_ir_regalloc_allocation_t allocation;
                        res = kefir_codegen_target_ir_regalloc_get(state->regalloc, output_value_ref, &allocation);
                        if (res == KEFIR_NOT_FOUND) {
                            continue;
                        }
                        REQUIRE_OK(res);

                        switch (output_value.type) {
                            case KEFIR_ASMCMP_VALUE_TYPE_NONE:
                            case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
                            case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
                            case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
                            case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL:
                            case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL:
                            case KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL:
                            case KEFIR_ASMCMP_VALUE_TYPE_EXTERNAL_LABEL:
                            case KEFIR_ASMCMP_VALUE_TYPE_X87:
                            case KEFIR_ASMCMP_VALUE_TYPE_INLINE_ASSEMBLY_INDEX:
                                // Intentionally left blank
                                break;

                            case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER: {
                                kefir_asm_amd64_xasmgen_register_t widest;
                                REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(output_value.phreg, &widest));
                                if (input_value.type == KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER && input_value.phreg == output_value.phreg) {
                                    // Intentionally left blank
                                } else if (kefir_hashset_has(&state->current_instr.input_registers, (kefir_hashset_key_t) widest) ||
                                    kefir_hashset_has(&state->current_instr.implicit_input_registers, (kefir_hashset_key_t) widest)) {
                                    kefir_asm_amd64_xasmgen_register_t phreg;
                                    if (kefir_asm_amd64_xasmgen_register_is_floating_point(output_value.phreg)) {
                                        REQUIRE_OK(allocate_scratch_register(state, &phreg, TEMPORARY_REGISTER_SSE, NULL));
                                    } else {
                                        REQUIRE_OK(allocate_scratch_register(state, &phreg, TEMPORARY_REGISTER_GP, NULL));
                                    }
                                    kefir_asmcmp_operand_variant_t output_variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT;
                                    kefir_bool_t output_variant_high_half = false;
                                    REQUIRE_OK(resolve_variant(output_value_type->variant, &output_variant, &output_variant_high_half));
                                    REQUIRE_OK(match_physical_reg_variant(&phreg, output_variant, output_variant_high_half));
                                    REQUIRE_OK(link_into_phreg(state, false, phreg, &input_value));
                                    REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->current_instr.tmp_output_registers, (kefir_hashtable_key_t) widest, (kefir_hashtable_value_t) phreg));
                                    output_value = KEFIR_ASMCMP_MAKE_PHREG(phreg);
                                } else {
                                    REQUIRE_OK(link_into_phreg(state, false, output_value.phreg, &input_value));
                                }
                            } break;

                            case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
                                switch (output_value.indirect.type) {
                                    case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                                    case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                                    case KEFIR_ASMCMP_INDIRECT_INTERNAL_LABEL_BASIS:
                                    case KEFIR_ASMCMP_INDIRECT_EXTERNAL_LABEL_BASIS:
                                    case KEFIR_ASMCMP_INDIRECT_LOCAL_AREA_BASIS:
                                        // Intentionally left blank
                                        break;

                                    case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS: {
                                        kefir_bool_t available = false;
                                        switch (output_value_type->kind) {
                                            case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE:
                                                REQUIRE_OK(is_spill_range_available(state, output_value.indirect.base.spill_index, 1, &available));
                                                if (!available) {
                                                    kefir_size_t index = 0;
                                                    REQUIRE_OK(allocate_spill_space(state, 1, 1, &index));
                                                    REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->current_instr.tmp_output_spill, (kefir_hashtable_key_t) output_value.indirect.base.spill_index, (kefir_hashtable_value_t) (((kefir_uint64_t) index) << 32) | 1));
                                                    output_value.indirect.base.spill_index = index;
                                                    REQUIRE_OK(link_into_spill(state, index, 1, output_value.indirect.variant, &input_value));
                                                } else if ((input_value.type != KEFIR_ASMCMP_VALUE_TYPE_INDIRECT && input_value.indirect.type != KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS) ||
                                                    input_value.indirect.base.spill_index != output_value.indirect.base.spill_index) {
                                                    REQUIRE_OK(link_into_spill(state, output_value.indirect.base.spill_index, 1, output_value.indirect.variant, &input_value));
                                                }
                                                break;

                                            case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT:
                                                REQUIRE_OK(is_spill_range_available(state, output_value.indirect.base.spill_index, 2, &available));
                                                if (!available) {
                                                    kefir_size_t index = 0;
                                                    REQUIRE_OK(allocate_spill_space(state, 2, 2, &index));
                                                    REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->current_instr.tmp_output_spill, (kefir_hashtable_key_t) output_value.indirect.base.spill_index, (kefir_hashtable_value_t) (((kefir_uint64_t) index) << 32) | 2));
                                                    output_value.indirect.base.spill_index = index;
                                                    REQUIRE_OK(link_into_spill(state, index, 2, output_value.indirect.variant, &input_value));
                                                } else if ((input_value.type != KEFIR_ASMCMP_VALUE_TYPE_INDIRECT && input_value.indirect.type != KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS) ||
                                                    input_value.indirect.base.spill_index != output_value.indirect.base.spill_index) {
                                                    REQUIRE_OK(link_into_spill(state, output_value.indirect.base.spill_index, 2, output_value.indirect.variant, &input_value));
                                                }
                                                break;

                                            case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_UNSPECIFIED:
                                            case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLAGS:
                                            case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT:
                                            case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE:
                                            case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_EXTERNAL_MEMORY:
                                            case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE:
                                                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected target IR value type");
                                        }
                                    } break;
                                }
                                break;
                        }

                        asmcmp_instruction.args[i] = output_value;
                    }
                } else {
                    parameter_idx++;
                    output_index++;
                }
                break;
        }
    }

    parameter_idx = 0;
    output_index = 0;
    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        switch (classification.operands[i].class) {
            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_NONE:
                // Intentionally left blank
                break;

            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ:
                if (classification.operands[i].implicit) {
                    struct kefir_asmcmp_value value;
                    REQUIRE_OK(resolve_operand(state, &instr->operation.parameters[parameter_idx++], &value));
                    REQUIRE_OK(link_into_phreg(state, true, classification.operands[i].implicit_params.phreg, &value));
                } else {
                    parameter_idx++;
                }
                break;

            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE: {
                kefir_codegen_target_ir_value_ref_t output_value_ref;
                const struct kefir_codegen_target_ir_value_type *output_value_type;
                REQUIRE_OK(kefir_codegen_target_ir_code_instruction_output(state->code, instr_ref, output_index++, &output_value_ref, &output_value_type));
                if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_INDIRECT_OUTPUT(output_value_ref.aspect)) {
                    parameter_idx++;
                }
            } break;

            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE:
                if (classification.operands[i].implicit) {
                    kefir_codegen_target_ir_value_ref_t output_value_ref;
                    REQUIRE_OK(kefir_codegen_target_ir_code_instruction_output(state->code, instr_ref, output_index++, &output_value_ref, NULL));
                    REQUIRE(KEFIR_CODEGEN_TARGET_IR_VALUE_IS_DIRECT_OUTPUT(output_value_ref.aspect), KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected either output register or indirect output value"));

                    struct kefir_asmcmp_value input_value;
                    REQUIRE_OK(resolve_operand(state, &instr->operation.parameters[parameter_idx++], &input_value));
                    kefir_codegen_target_ir_regalloc_allocation_t allocation;
                    res = kefir_codegen_target_ir_regalloc_get(state->regalloc, output_value_ref, &allocation);
                    if (res == KEFIR_NOT_FOUND) {
                        continue;
                    }
                    REQUIRE_OK(res);

                    REQUIRE_OK(link_into_phreg(state, true, classification.operands[i].implicit_params.phreg, &input_value));

                    union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                        .allocation = allocation
                    };
                    switch (entry.type) {
                        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_NA:
                        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL:
                            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected register allocation");

                        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP:
                        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE:
                            if (classification.operands[i].implicit_params.phreg != entry.reg.value) {
                                REQUIRE_OK(acquire_scratch_register(state, classification.operands[i].implicit_params.phreg));
                                REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->current_instr.tmp_output_registers, (kefir_hashtable_key_t) entry.reg.value, (kefir_hashtable_value_t) classification.operands[i].implicit_params.phreg));
                            }
                            break;
                    }
                } else {
                    parameter_idx++;
                    output_index++;
                }
                break;
        }
    }

    kefir_asmcmp_instruction_index_t insert_idx = kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context);
    switch (asmcmp_instruction.opcode) {
#define DEF_OPCODE0(_opcode, _mnemonic, _variant, _flags)
#define DEF_OPCODE1(_opcode, _mnemonic, _variant, _flags, _op1)                                                        \
        case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                                                                           \
            REQUIRE_OK(                                                                                                    \
                devirtualize_instr1(state, &asmcmp_instruction, (_op1), &insert_idx)); \
            break;
#define DEF_OPCODE2(_opcode, _mnemonic, _variant, _flags, _op1, _op2)                                               \
        case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                                                                        \
            REQUIRE_OK(devirtualize_instr2(state, &asmcmp_instruction, \
                                            (_op1), (_op2), &insert_idx));                                                            \
            break;
#define DEF_OPCODE3(_opcode, _mnemonic, _variant, _flags, _op1, _op2, _op3)                                         \
        case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                                                                        \
            REQUIRE_OK(devirtualize_instr3(state, &asmcmp_instruction, \
                                            (_op1), (_op2), (_op3), &insert_idx));                                                    \
            break;
        KEFIR_AMD64_INSTRUCTION_DATABASE(DEF_OPCODE0, DEF_OPCODE1, DEF_OPCODE2, DEF_OPCODE3, )
#undef DEF_OPCODE0
#undef DEF_OPCODE1
#undef DEF_OPCODE2
#undef DEF_OPCODE3

        case KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link):
        case KEFIR_ASMCMP_AMD64_OPCODE(inline_assembly):
        case KEFIR_ASMCMP_AMD64_OPCODE(virtual_block_begin):
        case KEFIR_ASMCMP_AMD64_OPCODE(virtual_block_end):
        case KEFIR_ASMCMP_AMD64_OPCODE(tail_call):
        case KEFIR_ASMCMP_AMD64_OPCODE(produce_virtual_register):
        case KEFIR_ASMCMP_AMD64_OPCODE(touch_virtual_register):
        case KEFIR_ASMCMP_AMD64_OPCODE(weak_touch_virtual_register):
        case KEFIR_ASMCMP_AMD64_OPCODE(function_prologue):
        case KEFIR_ASMCMP_AMD64_OPCODE(function_epilogue):
        case KEFIR_ASMCMP_AMD64_OPCODE(noop):
        case KEFIR_ASMCMP_AMD64_OPCODE(data_word):
            // Intentionally left blank
            break;
    }

    REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, &state->asmcmp_ctx->context, insert_idx,
        &asmcmp_instruction, NULL));

    struct kefir_hashtable_iterator tmp_output_iter;
    kefir_hashtable_key_t tmp_output_key;
    kefir_hashtable_value_t tmp_output_value;
    for (res = kefir_hashtable_iter(&state->current_instr.tmp_output_registers, &tmp_output_iter, &tmp_output_key, &tmp_output_value);
        res == KEFIR_OK;
        res = kefir_hashtable_next(&tmp_output_iter, &tmp_output_key, &tmp_output_value)) {
        REQUIRE_OK(link_into_phreg(state, false, tmp_output_key, &KEFIR_ASMCMP_MAKE_PHREG(tmp_output_value)));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (res = kefir_hashtable_iter(&state->current_instr.tmp_output_spill, &tmp_output_iter, &tmp_output_key, &tmp_output_value);
        res == KEFIR_OK;
        res = kefir_hashtable_next(&tmp_output_iter, &tmp_output_key, &tmp_output_value)) {
        ASSIGN_DECL_CAST(kefir_size_t, dst_index, tmp_output_key);
        kefir_uint32_t src_index = ((kefir_uint64_t) tmp_output_value) >> 32;
        kefir_uint32_t length = (kefir_uint32_t) tmp_output_value;
        REQUIRE_OK(copy_spill_area(state, src_index, dst_index, length));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_hashtable_iterator scratch_iter;
    kefir_hashtable_key_t scratch_key;
    for (res = kefir_hashtable_iter(&state->current_instr.scratch_registers, &scratch_iter, &scratch_key, NULL);
        res == KEFIR_OK;
        res = kefir_hashtable_iter(&state->current_instr.scratch_registers, &scratch_iter, &scratch_key, NULL)) {
        REQUIRE_OK(release_scratch_register(state, (kefir_asm_amd64_xasmgen_register_t) scratch_key));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t translate_block(struct destructor_state *state, kefir_codegen_target_ir_block_ref_t block_ref) {
    kefir_hashtable_value_t table_value;
    REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) block_ref, &table_value));
    ASSIGN_DECL_CAST(struct block_state *, block_state,
        table_value);

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(state->mem, &state->asmcmp_ctx->context, block_state->asmcmp_label));
    struct kefir_asmcmp_instruction instr = {
        .opcode = state->destructor_ops->noop_opcode,
        .args[0].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
        .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
        .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE
    };
    REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, &state->asmcmp_ctx->context, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context), &instr, NULL));

    for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(state->code, block_ref);
        instr_ref != KEFIR_ID_NONE;
        instr_ref = kefir_codegen_target_ir_code_control_next(state->code, instr_ref)) {
        REQUIRE_OK(translate_instruction(state, instr_ref));
    }

    if (state->constructor_metadata != NULL) {
        REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(state->mem, &state->asmcmp_ctx->context, block_state->asmcmp_end_label));
    }

    return KEFIR_OK;
}

static kefir_result_t translate_blocks(struct destructor_state *state) {
    for (kefir_size_t i = 0; i < state->schedule.schedule_length; i++) {
        REQUIRE_OK(translate_block(state, state->schedule.block_schedule[i].block_ref));
    }

    kefir_result_t res;
    struct kefir_hashtreeset_iterator iter;
    for (res = kefir_hashtreeset_iter(&state->control_flow.indirect_jump_targets, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref, iter.entry);
        if (!kefir_codegen_target_ir_control_flow_is_reachable(&state->control_flow, block_ref)) {
            kefir_hashtable_value_t table_value;
            REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) block_ref, &table_value));
            ASSIGN_DECL_CAST(struct block_state *, block_state,
                table_value);
            REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(state->mem, &state->asmcmp_ctx->context, block_state->asmcmp_label));
            struct kefir_asmcmp_instruction instr = {
                .opcode = state->destructor_ops->unreachable_opcode,
                .args[0].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
                .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
                .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE
            };
            REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, &state->asmcmp_ctx->context, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context), &instr, NULL));
            break;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_asmcmp_instruction instr = {
        .opcode = state->destructor_ops->noop_opcode,
        .args[0].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
        .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
        .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE
    };
    REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, &state->asmcmp_ctx->context, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context), &instr, NULL));

    return KEFIR_OK;
}

static kefir_result_t destruct_impl(struct destructor_state *state) {
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_build(state->mem, &state->control_flow));
    REQUIRE_OK(kefir_codegen_target_ir_liveness_build(state->mem, &state->control_flow, &state->liveness));
    REQUIRE_OK(state->destructor_ops->schedule_code(state->mem, &state->control_flow, &state->schedule, state->destructor_ops->payload));
    REQUIRE_OK(map_block_labels(state));
    REQUIRE_OK(translate_blocks(state));
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

kefir_result_t kefir_codegen_target_ir_amd64_destruct(struct kefir_mem *mem,
    const struct kefir_codegen_target_ir_code *code,
    struct kefir_asmcmp_amd64 *asmcmp_ctx,
    const struct kefir_codegen_target_ir_stack_frame *stack_frame,
    const struct kefir_codegen_target_ir_interference *interference,
    const struct kefir_codegen_target_ir_regalloc *regalloc,
    const struct kefir_codegen_target_ir_code_constructor_metadata *constructor_metadata,
    const struct kefir_codegen_target_ir_round_trip_destructor_ops *parameter) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(asmcmp_ctx != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp context"));
    REQUIRE(stack_frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR stack frame"));
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference"));
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));
    REQUIRE(parameter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR round trip destructor parameter"));

    struct destructor_state state = {
        .mem = mem,
        .code = code,
        .asmcmp_ctx = asmcmp_ctx,
        .stack_frame = stack_frame,
        .interference = interference,
        .regalloc = regalloc,
        .destructor_ops = parameter,
        .constructor_metadata = constructor_metadata
    };
    REQUIRE_OK(kefir_hashtable_init(&state.blocks, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&state.blocks, free_block_state, NULL));
    REQUIRE_OK(kefir_hashtable_init(&state.native_labels, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_init(&state.control_flow, state.code));
    REQUIRE_OK(kefir_codegen_target_ir_liveness_init(&state.liveness));
    REQUIRE_OK(kefir_codegen_target_ir_code_schedule_init(&state.schedule, state.code));
    REQUIRE_OK(kefir_hashset_init(&state.current_instr.interfere_registers, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashset_init(&state.current_instr.input_registers, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashset_init(&state.current_instr.implicit_input_registers, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashset_init(&state.current_instr.output_registers, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&state.current_instr.scratch_registers, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&state.current_instr.tmp_output_registers, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&state.current_instr.tmp_output_spill, &kefir_hashtable_uint_ops));

    kefir_result_t res = destruct_impl(&state);
    KEFIR_FREE(mem, state.current_instr.occupied_spill_slots);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.current_instr.tmp_output_spill);
        kefir_hashtable_free(mem, &state.current_instr.tmp_output_registers);
        kefir_hashtable_free(mem, &state.current_instr.scratch_registers);
        kefir_hashset_free(mem, &state.current_instr.interfere_registers);
        kefir_hashset_free(mem, &state.current_instr.input_registers);
        kefir_hashset_free(mem, &state.current_instr.implicit_input_registers);
        kefir_hashset_free(mem, &state.current_instr.output_registers);
        kefir_codegen_target_ir_code_schedule_free(mem, &state.schedule);
        kefir_codegen_target_ir_liveness_free(mem, &state.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
        kefir_hashtable_free(mem, &state.blocks);
        kefir_hashtable_free(mem, &state.native_labels);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.current_instr.tmp_output_spill);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.current_instr.tmp_output_registers);
        kefir_hashtable_free(mem, &state.current_instr.scratch_registers);
        kefir_hashset_free(mem, &state.current_instr.interfere_registers);
        kefir_hashset_free(mem, &state.current_instr.input_registers);
        kefir_hashset_free(mem, &state.current_instr.implicit_input_registers);
        kefir_hashset_free(mem, &state.current_instr.output_registers);
        kefir_codegen_target_ir_code_schedule_free(mem, &state.schedule);
        kefir_codegen_target_ir_liveness_free(mem, &state.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
        kefir_hashtable_free(mem, &state.blocks);
        kefir_hashtable_free(mem, &state.native_labels);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.current_instr.tmp_output_registers);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.current_instr.scratch_registers);
        kefir_hashset_free(mem, &state.current_instr.interfere_registers);
        kefir_hashset_free(mem, &state.current_instr.input_registers);
        kefir_hashset_free(mem, &state.current_instr.implicit_input_registers);
        kefir_hashset_free(mem, &state.current_instr.output_registers);
        kefir_codegen_target_ir_code_schedule_free(mem, &state.schedule);
        kefir_codegen_target_ir_liveness_free(mem, &state.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
        kefir_hashtable_free(mem, &state.blocks);
        kefir_hashtable_free(mem, &state.native_labels);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.current_instr.scratch_registers);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.current_instr.interfere_registers);
        kefir_hashset_free(mem, &state.current_instr.input_registers);
        kefir_hashset_free(mem, &state.current_instr.implicit_input_registers);
        kefir_hashset_free(mem, &state.current_instr.output_registers);
        kefir_codegen_target_ir_code_schedule_free(mem, &state.schedule);
        kefir_codegen_target_ir_liveness_free(mem, &state.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
        kefir_hashtable_free(mem, &state.blocks);
        kefir_hashtable_free(mem, &state.native_labels);
        return res;
    });
    res = kefir_hashset_free(mem, &state.current_instr.interfere_registers);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.current_instr.input_registers);
        kefir_hashset_free(mem, &state.current_instr.implicit_input_registers);
        kefir_hashset_free(mem, &state.current_instr.output_registers);
        kefir_codegen_target_ir_code_schedule_free(mem, &state.schedule);
        kefir_codegen_target_ir_liveness_free(mem, &state.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
        kefir_hashtable_free(mem, &state.blocks);
        kefir_hashtable_free(mem, &state.native_labels);
        return res;
    });
    res = kefir_hashset_free(mem, &state.current_instr.input_registers);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.current_instr.implicit_input_registers);
        kefir_hashset_free(mem, &state.current_instr.output_registers);
        kefir_codegen_target_ir_code_schedule_free(mem, &state.schedule);
        kefir_codegen_target_ir_liveness_free(mem, &state.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
        kefir_hashtable_free(mem, &state.blocks);
        kefir_hashtable_free(mem, &state.native_labels);
        return res;
    });
    res = kefir_hashset_free(mem, &state.current_instr.implicit_input_registers);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.current_instr.output_registers);
        kefir_codegen_target_ir_code_schedule_free(mem, &state.schedule);
        kefir_codegen_target_ir_liveness_free(mem, &state.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
        kefir_hashtable_free(mem, &state.blocks);
        kefir_hashtable_free(mem, &state.native_labels);
        return res;
    });
    kefir_hashset_free(mem, &state.current_instr.output_registers);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_code_schedule_free(mem, &state.schedule);
        kefir_codegen_target_ir_liveness_free(mem, &state.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
        kefir_hashtable_free(mem, &state.blocks);
        kefir_hashtable_free(mem, &state.native_labels);
        return res;
    });
    res = kefir_codegen_target_ir_code_schedule_free(mem, &state.schedule);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_liveness_free(mem, &state.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
        kefir_hashtable_free(mem, &state.blocks);
        kefir_hashtable_free(mem, &state.native_labels);
        return res;
    });
    res = kefir_codegen_target_ir_liveness_free(mem, &state.liveness);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
        kefir_hashtable_free(mem, &state.blocks);
        kefir_hashtable_free(mem, &state.native_labels);
        return res;
    });
    res = kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.blocks);
        kefir_hashtable_free(mem, &state.native_labels);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.blocks);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.native_labels);
        return res;
    });
    REQUIRE_OK(kefir_hashtable_free(mem, &state.native_labels));
    return KEFIR_OK;
}
