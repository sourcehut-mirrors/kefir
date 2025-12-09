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
    const struct kefir_codegen_target_ir_interference *interference;
    const struct kefir_codegen_target_ir_regalloc *regalloc;
    const struct kefir_codegen_target_ir_code_constructor_metadata *constructor_metadata;
    const struct kefir_codegen_target_ir_round_trip_destructor_ops *parameter;

    struct kefir_codegen_target_ir_control_flow control_flow;
    struct kefir_codegen_target_ir_liveness liveness;
    struct kefir_codegen_target_ir_code_schedule schedule;
    struct kefir_hashtable blocks;
    struct kefir_hashtable native_labels;

    struct {
        struct kefir_hashset interfere_registers;
        struct kefir_hashset input_registers;
        struct kefir_hashtable scratch_registers;
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

static kefir_result_t allocate_spill_space(struct destructor_state *state, kefir_size_t length, kefir_size_t alignment, kefir_size_t *index) {
    UNUSED(state);
    UNUSED(length);
    UNUSED(alignment);
    *index = 0; // TODO KEFIR_NOT_IMPLEMENTED
    return KEFIR_OK;
}

static kefir_result_t obtain_temporary_register(struct destructor_state *state, kefir_asm_amd64_xasmgen_register_t *reg, enum temporary_register_type type) {
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
            !kefir_hashset_has(&state->current_instr.interfere_registers, (kefir_hashset_key_t) candidate) &&
            !kefir_hashtable_has(&state->current_instr.scratch_registers, (kefir_hashtable_key_t) candidate)) {
            REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->current_instr.scratch_registers, (kefir_hashtable_key_t) candidate, (kefir_hashtable_value_t) -1ll));
            *reg = candidate;
            return KEFIR_OK;
        }
    }

    for (kefir_size_t i = 0; i < length; i++) {
        const kefir_asm_amd64_xasmgen_register_t candidate = register_arr[i];

        if (!kefir_hashset_has(&state->current_instr.input_registers, (kefir_hashset_key_t) candidate) &&
            !kefir_hashtable_has(&state->current_instr.scratch_registers, (kefir_hashtable_key_t) candidate)) {
            kefir_size_t spill_index;
            REQUIRE_OK(allocate_spill_space(state, spill_qwords, spill_qwords, &spill_index));
            switch (type) {
                case TEMPORARY_REGISTER_GP:
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(
                        state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(spill_index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                        &KEFIR_ASMCMP_MAKE_PHREG(candidate), NULL));
                    break;

                case TEMPORARY_REGISTER_SSE:
                    REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
                        state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(spill_index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                        &KEFIR_ASMCMP_MAKE_PHREG(candidate), NULL));
                    break;
            }
            REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->current_instr.scratch_registers, (kefir_hashtable_key_t) candidate, (kefir_hashtable_value_t) spill_index));
            *reg = candidate;
            return KEFIR_OK;
        }
    }

    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find a temporary register for eviction");
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
        REQUIRE_OK(state->parameter->bind_native_id(state->mem, *asmcmp_label, native_id, state->parameter->payload));
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
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_PHYSICAL_REGISTER:
            *value = KEFIR_ASMCMP_MAKE_PHREG(operand->phreg);
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF: {
            const struct kefir_codegen_target_ir_value_type *value_type;
            REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->code, operand->direct.value_ref, &value_type));
            switch (value_type->kind) {
                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_UNSPECIFIED:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected kind of target IR value reference");

                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE:
                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT: {
                    kefir_codegen_target_ir_regalloc_allocation_t allocation;
                    REQUIRE_OK(kefir_codegen_target_ir_regalloc_get(state->regalloc, operand->direct.value_ref, &allocation));
                    union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                        .allocation = allocation
                    };
                    switch (entry.type) {
                        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_NA:
                            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 target IR register allocation");
                            
                        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP:
                        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE:
                            *value = KEFIR_ASMCMP_MAKE_PHREG(entry.reg.value);
                            break;

                        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL:
                            *value = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(entry.spill_area.index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT);
                            break;
                    }
                } break;

                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE: {
                    kefir_asm_amd64_xasmgen_register_t phreg;
                    REQUIRE_OK(obtain_temporary_register(state, &phreg, TEMPORARY_REGISTER_GP));
                    const kefir_int64_t offset = value_type->parameters.local_variable.offset;
                    if (offset >= KEFIR_INT16_MIN && offset <= KEFIR_INT16_MAX) {
                        struct kefir_asmcmp_value arg = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_AREA(
                            value_type->parameters.local_variable.identifier, offset, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT);
                        REQUIRE_OK(kefir_asmcmp_amd64_lea(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg), &arg, NULL));
                    } else if (offset >= KEFIR_INT32_MIN && offset <= KEFIR_INT32_MAX) {
                        struct kefir_asmcmp_value arg = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_AREA(
                            value_type->parameters.local_variable.identifier, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT);
                        REQUIRE_OK(kefir_asmcmp_amd64_lea(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg), &arg, NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_add(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
                    } else {
                        kefir_asm_amd64_xasmgen_register_t phreg2;
                        REQUIRE_OK(obtain_temporary_register(state, &phreg2, TEMPORARY_REGISTER_GP));

                        struct kefir_asmcmp_value arg = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_AREA(
                            value_type->parameters.local_variable.identifier, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT);
                        REQUIRE_OK(kefir_asmcmp_amd64_lea(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg), &arg, NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg2), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_add(
                            state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                            &KEFIR_ASMCMP_MAKE_PHREG(phreg), &KEFIR_ASMCMP_MAKE_PHREG(phreg2), NULL));
                    }

                    switch (value->vreg.variant) {
                        case KEFIR_ASMCMP_OPERAND_VARIANT_8BIT:
                            if (value->vreg.high_half) {
                                REQUIRE_OK(kefir_asm_amd64_xasmgen_register8_high(phreg, &phreg));
                            } else {
                                REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(phreg, &phreg));
                            }
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_16BIT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register16(phreg, &phreg));
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_32BIT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(phreg, &phreg));
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_64BIT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register64(phreg, &phreg));
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_80BIT:
                        case KEFIR_ASMCMP_OPERAND_VARIANT_128BIT:
                        case KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE:
                        case KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE:
                            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected virtual register variant");

                        case KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(phreg, &phreg));
                            break;
                    }
                    *value = KEFIR_ASMCMP_MAKE_PHREG(phreg);
                } break;

                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_EXTERNAL_MEMORY: {
                    kefir_asm_amd64_xasmgen_register_t phreg;
                    REQUIRE_OK(obtain_temporary_register(state, &phreg, TEMPORARY_REGISTER_GP));
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
                    REQUIRE_OK(kefir_codegen_target_ir_regalloc_get(state->regalloc, operand->direct.value_ref, &allocation));
                    union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                        .allocation = allocation
                    };
                    REQUIRE(entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected spill space allocation"));

                    kefir_asm_amd64_xasmgen_register_t phreg;
                    REQUIRE_OK(obtain_temporary_register(state, &phreg, TEMPORARY_REGISTER_GP));
                    REQUIRE_OK(kefir_asmcmp_amd64_lea(
                        state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                        &KEFIR_ASMCMP_MAKE_PHREG(phreg),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(entry.spill_area.index, 0,
                                                          KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                        NULL));
                    *value = KEFIR_ASMCMP_MAKE_PHREG(phreg);
                } break;

                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLAGS:
                case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT:
                    // Intentionally left blank
                    break;
            }
        } break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT: {
            kefir_asmcmp_operand_variant_t variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT;
            REQUIRE_OK(resolve_variant(operand->indirect.variant, &variant, NULL));
            switch (operand->indirect.type) {
                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_PHYSICAL_BASIS:
                    *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(operand->indirect.base.phreg, operand->indirect.offset, variant);
                    break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS: {
                    kefir_codegen_target_ir_regalloc_allocation_t allocation;
                    REQUIRE_OK(kefir_codegen_target_ir_regalloc_get(state->regalloc, operand->direct.value_ref, &allocation));
                    union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                        .allocation = allocation
                    };
                    switch (entry.type) {
                        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_NA:
                            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 target IR register allocation");
                            
                        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP:
                        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE:
                            *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(entry.reg.value, value->indirect.offset,
                                                                         value->indirect.variant);
                            break;

                        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL: {
                            kefir_asm_amd64_xasmgen_register_t phreg;
                            REQUIRE_OK(obtain_temporary_register(state, &phreg, TEMPORARY_REGISTER_GP));
                            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                                state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                                &KEFIR_ASMCMP_MAKE_PHREG(phreg),
                                &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(entry.spill_area.index, 0,
                                                                  KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                                NULL));
                            *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(phreg, value->indirect.offset,
                                                                         value->indirect.variant);
                        } break;
                    }
                } break;

                case KEFIR_CODEGEN_TARGET_IR_INDIRECT_IMMEDIATE_BASIS: {
                    kefir_asm_amd64_xasmgen_register_t phreg;
                    REQUIRE_OK(obtain_temporary_register(state, &phreg, TEMPORARY_REGISTER_GP));
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(
                        state->mem, state->asmcmp_ctx, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
                        &KEFIR_ASMCMP_MAKE_PHREG(phreg), &KEFIR_ASMCMP_MAKE_INT(operand->indirect.base.immediate),
                        NULL));
                    *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(phreg, value->indirect.offset,
                                                                    value->indirect.variant);
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
    }
    if (operand->segment.present) {
        value->segment.present = true;
        value->segment.reg = operand->segment.reg;
    } else {
        value->segment.present = false;
    }
    return KEFIR_OK;
}

// static kefir_result_t copy_spill_area(struct kefir_mem *mem, struct devirtualize_state *state, kefir_size_t instr_idx,
//                                       kefir_size_t from, kefir_size_t to, kefir_size_t length) {
//     REQUIRE(from != to, KEFIR_OK);
//     kefir_bool_t forward_direction = true;
//     if (to >= from && to < from + length) {
//         forward_direction = false;
//     }

//     kefir_asm_amd64_xasmgen_register_t tmp_reg;
//     REQUIRE_OK(obtain_temporary_register(mem, state, instr_idx, &tmp_reg, TEMPORARY_REGISTER_GP));
//     for (kefir_size_t i = 0; i < length; i++) {
//         const kefir_size_t index = forward_direction ? i : length - i - 1;
//         kefir_size_t new_position;
//         REQUIRE_OK(kefir_asmcmp_amd64_mov(
//             mem, state->target, kefir_asmcmp_context_instr_prev(&state->target->context, instr_idx),
//             &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg),
//             &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(from + index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), &new_position));
//         REQUIRE_OK(kefir_asmcmp_amd64_mov(
//             mem, state->target, kefir_asmcmp_context_instr_prev(&state->target->context, instr_idx),
//             &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(to + index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
//             &KEFIR_ASMCMP_MAKE_PHREG(tmp_reg), NULL));
//         REQUIRE_OK(kefir_asmcmp_context_move_labels(mem, &state->target->context, new_position, instr_idx));
//     }
//     return KEFIR_OK;
// }

// static kefir_result_t link_virtual_registers(struct destructor_state *state, kefir_codegen_target_ir_value_ref_t dst_ref, kefir_codegen_target_ir_value_ref_t src_ref, kefir_size_t *tail_idx) {
//     kefir_bool_t do_link = true;
//     kefir_codegen_target_ir_regalloc_allocation_t dst_allo, src_alloc;
//     REQUIRE_OK(kefir_codegen_target_ir_regalloc_get(state->regalloc, dst_ref, &alloc1));
//     REQUIRE_OK(kefir_codegen_target_ir_regalloc_get(state->regalloc, src_ref, &alloc2));
//     const struct kefir_codegen_amd64_register_allocation *reg_alloc1, *reg_alloc2;
//     REQUIRE_OK(kefir_codegen_amd64_xregalloc_allocation_of(state->xregalloc, instr->args[0].vreg.index, &reg_alloc1));
//     REQUIRE_OK(kefir_codegen_amd64_xregalloc_allocation_of(state->xregalloc, instr->args[1].vreg.index, &reg_alloc2));

//     switch (reg_alloc1->type) {
//         case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED:
//             return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected allocated virtual register");

//         case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_IMMEDIATE_INTEGER:
//             return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to link immediate value virtual register");

//         case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER:
//             return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to link memory pointer virtual register");

//         case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_LOCAL_VARIABLE:
//             return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to link local variable virtual register");

//         case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER:
//             switch (reg_alloc2->type) {
//                 case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_IMMEDIATE_INTEGER:
//                 case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_INDIRECT:
//                     // Intentionally left blank
//                     break;

//                 case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER:
//                     if (kefir_asm_amd64_xasmgen_register_is_floating_point(reg_alloc1->direct_reg) &&
//                         kefir_asm_amd64_xasmgen_register_is_floating_point(reg_alloc2->direct_reg)) {
//                         if (reg_alloc1->direct_reg != reg_alloc2->direct_reg) {
//                             REQUIRE_OK(kefir_asmcmp_amd64_movaps(
//                                 mem, state->target, *tail_idx, &KEFIR_ASMCMP_MAKE_PHREG(reg_alloc1->direct_reg),
//                                 &KEFIR_ASMCMP_MAKE_PHREG(reg_alloc2->direct_reg), tail_idx));
//                         }
//                         do_link = false;
//                     }
//                     break;

//                 case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT:
//                     if (kefir_asm_amd64_xasmgen_register_is_floating_point(reg_alloc1->direct_reg) &&
//                         reg_alloc2->spill_area.length >= 2) {
//                         REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
//                             mem, state->target, *tail_idx, &KEFIR_ASMCMP_MAKE_PHREG(reg_alloc1->direct_reg),
//                             &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc2->spill_area.index, 0,
//                                                               KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
//                             tail_idx));
//                         do_link = false;
//                     }
//                     break;

//                 case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER: {
//                     const struct kefir_asmcmp_virtual_register *vreg;
//                     REQUIRE_OK(
//                         kefir_asmcmp_virtual_register_get(&state->target->context, instr->args[1].vreg.index, &vreg));
//                     REQUIRE_OK(kefir_asmcmp_amd64_lea(
//                         mem, state->target, *tail_idx, &KEFIR_ASMCMP_MAKE_PHREG(reg_alloc1->direct_reg),
//                         &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(vreg->parameters.memory.base_reg,
//                                                              vreg->parameters.memory.offset,
//                                                              KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
//                         tail_idx));
//                     do_link = false;
//                 } break;

//                 case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_LOCAL_VARIABLE: {
//                     const struct kefir_asmcmp_virtual_register *vreg;
//                     REQUIRE_OK(
//                         kefir_asmcmp_virtual_register_get(&state->target->context, instr->args[1].vreg.index, &vreg));
//                     const kefir_int64_t offset = vreg->parameters.local_variable.offset;
//                     if (offset >= KEFIR_INT16_MIN && offset <= KEFIR_INT16_MAX) {
//                         struct kefir_asmcmp_value arg = KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_AREA(
//                             vreg->parameters.local_variable.identifier, offset, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT);
//                         REQUIRE_OK(devirtualize_value(mem, state, instr_idx, &arg));
//                         REQUIRE_OK(kefir_asmcmp_amd64_lea(mem, state->target, *tail_idx,
//                                                           &KEFIR_ASMCMP_MAKE_PHREG(reg_alloc1->direct_reg), &arg,
//                                                           tail_idx));
//                         do_link = false;
//                     }
//                 } break;

//                 case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED:
//                     return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected allocated virtual register");

//                 case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_PAIR:
//                     return KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
//                                            "Unable to link a pair of virtual register into a single register");
//             }
//             break;

//         case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT:
//             switch (reg_alloc2->type) {
//                 case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_IMMEDIATE_INTEGER:
//                 case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_INDIRECT:
//                 case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_LOCAL_VARIABLE:
//                 case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER:
//                     // Intentionally left blank
//                     break;

//                 case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER:
//                     if (kefir_asm_amd64_xasmgen_register_is_floating_point(reg_alloc2->direct_reg) &&
//                         reg_alloc1->spill_area.length >= 2) {
//                         REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
//                             mem, state->target, *tail_idx,
//                             &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc1->spill_area.index, 0,
//                                                               KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
//                             &KEFIR_ASMCMP_MAKE_PHREG(reg_alloc2->direct_reg), tail_idx));
//                         do_link = false;
//                     }
//                     break;

//                 case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT:
//                     if (reg_alloc1->spill_area.index != reg_alloc2->spill_area.index) {
//                         const kefir_size_t length = MIN(reg_alloc1->spill_area.length, reg_alloc2->spill_area.length);
//                         REQUIRE_OK(copy_spill_area(mem, state, instr_idx, reg_alloc2->spill_area.index,
//                                                    reg_alloc1->spill_area.index, length));
//                     }
//                     do_link = false;
//                     break;

//                 case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_PAIR:
//                     return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected virtual register pair");

//                 case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED:
//                     return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected allocated virtual register");
//             }
//             break;

//         case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_INDIRECT:
//             REQUIRE(reg_alloc2->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_INDIRECT &&
//                         reg_alloc1->spill_area.length == reg_alloc2->spill_area.length,
//                     KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Indirect spill area virtual register can only be linked to "
//                                                          "another such virtual register of equal length"));
//             if (reg_alloc1->spill_area.index != reg_alloc2->spill_area.index) {
//                 REQUIRE_OK(copy_spill_area(mem, state, instr_idx, reg_alloc2->spill_area.index,
//                                            reg_alloc1->spill_area.index, reg_alloc1->spill_area.length));
//             }
//             do_link = false;
//             break;

//         case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_PAIR:
//             return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected virtual register pair");
//     }

//     if (do_link) {
//         REQUIRE_OK(devirtualize_instr2(mem, state, instr_idx, instr, original_instr, tail_idx, KEFIR_AMD64_INSTRDB_NONE,
//                                        KEFIR_AMD64_INSTRDB_WRITE | KEFIR_AMD64_INSTRDB_GP_REGISTER_MEMORY |
//                                            KEFIR_AMD64_INSTRDB_XMM_REGISTER_MEMORY_FULL,
//                                        KEFIR_AMD64_INSTRDB_READ | KEFIR_AMD64_INSTRDB_GP_REGISTER_MEMORY |
//                                            KEFIR_AMD64_INSTRDB_XMM_REGISTER_MEMORY_FULL |
//                                            KEFIR_AMD64_INSTRDB_IMMEDIATE));
//     } else {
//         instr->opcode = KEFIR_ASMCMP_AMD64_OPCODE(noop);
//     }
//     return KEFIR_OK;
// }

// static kefir_result_t do_link_virtual_registers(struct kefir_mem *mem, struct devirtualize_state *state,
//                                                 kefir_size_t instr_idx, struct kefir_asmcmp_instruction *instr,
//                                                 struct kefir_asmcmp_instruction *original_instr,
//                                                 kefir_size_t *tail_idx) {
//     REQUIRE(instr->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER &&
//                 instr->args[1].type == KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER,
//             KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected virtual register link arguments"));

//     const struct kefir_asmcmp_virtual_register *vreg, *vreg2;
//     REQUIRE_OK(kefir_asmcmp_virtual_register_get(&state->target->context, instr->args[0].vreg.index, &vreg));
//     REQUIRE_OK(kefir_asmcmp_virtual_register_get(&state->target->context, instr->args[1].vreg.index, &vreg2));

//     const struct kefir_codegen_amd64_register_allocation *reg_alloc1, *reg_alloc2;
//     REQUIRE_OK(kefir_codegen_amd64_xregalloc_allocation_of(state->xregalloc, instr->args[0].vreg.index, &reg_alloc1));
//     REQUIRE_OK(kefir_codegen_amd64_xregalloc_allocation_of(state->xregalloc, instr->args[1].vreg.index, &reg_alloc2));

//     if (reg_alloc1->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_PAIR &&
//         reg_alloc2->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT) {
//         struct kefir_asmcmp_instruction link_part_instr[3];
//         kefir_size_t num_of_links = 2;
//         kefir_uint64_t op_flags = KEFIR_AMD64_INSTRDB_WRITE | KEFIR_AMD64_INSTRDB_GP_REGISTER_MEMORY | KEFIR_AMD64_INSTRDB_XMM_REGISTER_MEMORY_FULL;

//         const struct kefir_codegen_amd64_register_allocation *vreg_part1_alloc;
//         REQUIRE_OK(kefir_codegen_amd64_xregalloc_allocation_of(state->xregalloc, vreg->parameters.pair.virtual_registers[1], &vreg_part1_alloc));
//         const kefir_bool_t alias = vreg_part1_alloc->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT &&
//                 vreg_part1_alloc->spill_area.index == reg_alloc2->spill_area.index;
//         switch (vreg->parameters.pair.type) {
//             case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERIC:
//                 return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to link generic pair of virtual registers");

//             case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_SINGLE:
//                 if (alias) {
//                     kefir_asm_amd64_xasmgen_register_t tmp_reg;
//                     REQUIRE_OK(obtain_temporary_register(mem, state, *tail_idx, &tmp_reg, TEMPORARY_REGISTER_SSE));

//                     link_part_instr[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movd);
//                     link_part_instr[0].args[0] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
//                     link_part_instr[0].args[1] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(
//                         reg_alloc2->spill_area.index, KEFIR_AMD64_ABI_QWORD / 2, KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE);
//                     link_part_instr[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movd);
//                     link_part_instr[1].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[0]);
//                     link_part_instr[1].args[1] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc2->spill_area.index, 0,
//                                                                                 KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE);
//                     link_part_instr[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[2].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movd);
//                     link_part_instr[2].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[1]);
//                     link_part_instr[2].args[1] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
//                     link_part_instr[2].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     num_of_links = 3;
//                 } else {
//                     link_part_instr[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movd);
//                     link_part_instr[0].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[1]);
//                     link_part_instr[0].args[1] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(
//                         reg_alloc2->spill_area.index, KEFIR_AMD64_ABI_QWORD / 2, KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE);
//                     link_part_instr[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movd);
//                     link_part_instr[1].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[0]);
//                     link_part_instr[1].args[1] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc2->spill_area.index, 0,
//                                                                                 KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE);
//                     link_part_instr[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;
//                 }
//                 break;

//             case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_DOUBLE:
//                 if (alias) {
//                     kefir_asm_amd64_xasmgen_register_t tmp_reg;
//                     REQUIRE_OK(obtain_temporary_register(mem, state, *tail_idx, &tmp_reg, TEMPORARY_REGISTER_SSE));

//                     link_part_instr[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movq);
//                     link_part_instr[0].args[0] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
//                     link_part_instr[0].args[1] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(
//                         reg_alloc2->spill_area.index, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE);
//                     link_part_instr[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movq);
//                     link_part_instr[1].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[0]);
//                     link_part_instr[1].args[1] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc2->spill_area.index, 0,
//                                                                                 KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE);
//                     link_part_instr[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[2].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movq);
//                     link_part_instr[2].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[1]);
//                     link_part_instr[2].args[1] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
//                     link_part_instr[2].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     num_of_links = 3;
//                 } else {
//                     link_part_instr[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movq);
//                     link_part_instr[0].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[1]);
//                     link_part_instr[0].args[1] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(
//                         reg_alloc2->spill_area.index, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE);
//                     link_part_instr[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movq);
//                     link_part_instr[1].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[0]);
//                     link_part_instr[1].args[1] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc2->spill_area.index, 0,
//                                                                                 KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE);
//                     link_part_instr[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;
//                 }
//                 break;

//             case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE:
//                 if (alias) {
//                     kefir_asm_amd64_xasmgen_register_t tmp_reg;
//                     REQUIRE_OK(obtain_temporary_register(mem, state, *tail_idx, &tmp_reg, TEMPORARY_REGISTER_GP));

//                     link_part_instr[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(mov);
//                     link_part_instr[0].args[0] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
//                     link_part_instr[0].args[1] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(
//                         reg_alloc2->spill_area.index, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT);
//                     link_part_instr[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(mov);
//                     link_part_instr[1].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[0]);
//                     link_part_instr[1].args[1] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc2->spill_area.index, 0,
//                                                                                 KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT);
//                     link_part_instr[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[2].opcode = KEFIR_ASMCMP_AMD64_OPCODE(mov);
//                     link_part_instr[2].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[1]);
//                     link_part_instr[2].args[1] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
//                     link_part_instr[2].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;
//                 } else {
//                     link_part_instr[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(mov);
//                     link_part_instr[0].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[1]);
//                     link_part_instr[0].args[1] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(
//                         reg_alloc2->spill_area.index, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT);
//                     link_part_instr[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(mov);
//                     link_part_instr[1].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[0]);
//                     link_part_instr[1].args[1] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc2->spill_area.index, 0,
//                                                                                 KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT);
//                     link_part_instr[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;
//                 }
//                 op_flags = KEFIR_AMD64_INSTRDB_WRITE | KEFIR_AMD64_INSTRDB_GP_REGISTER_MEMORY;
//                 break;
//         }
//         for (kefir_size_t i = 0; i < num_of_links; i++) {
//             REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(mem, &state->target->context, *tail_idx, &link_part_instr[i],
//                                                             tail_idx));
//             kefir_asmcmp_instruction_index_t virtual_instr_idx = *tail_idx;
//             REQUIRE_OK(devirtualize_instr2(
//                 mem, state, *tail_idx, &link_part_instr[i], original_instr, tail_idx, KEFIR_AMD64_INSTRDB_NONE,
//                 op_flags,
//                 op_flags | KEFIR_AMD64_INSTRDB_IMMEDIATE));
//             REQUIRE_OK(kefir_asmcmp_context_instr_replace(&state->target->context, virtual_instr_idx, &link_part_instr[i]));
//         }
//         instr->opcode = KEFIR_ASMCMP_AMD64_OPCODE(noop);
//     } else if (reg_alloc1->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT &&
//                reg_alloc2->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_PAIR) {
//         struct kefir_asmcmp_instruction link_part_instr[3];
//         kefir_size_t num_of_links = 2;
//         kefir_uint64_t op_flags = KEFIR_AMD64_INSTRDB_WRITE | KEFIR_AMD64_INSTRDB_GP_REGISTER_MEMORY | KEFIR_AMD64_INSTRDB_XMM_REGISTER_MEMORY_FULL;

//         const struct kefir_codegen_amd64_register_allocation *vreg2_part1_alloc;
//         REQUIRE_OK(kefir_codegen_amd64_xregalloc_allocation_of(state->xregalloc, vreg2->parameters.pair.virtual_registers[1], &vreg2_part1_alloc));
//         const kefir_bool_t alias = vreg2_part1_alloc->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT &&
//                 vreg2_part1_alloc->spill_area.index == reg_alloc1->spill_area.index;
//         switch (vreg->parameters.pair.type) {
//             case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERIC:
//                 return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to link generic pair of virtual registers");

//             case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_SINGLE:
//                 if (alias) {
//                     kefir_asm_amd64_xasmgen_register_t tmp_reg;
//                     REQUIRE_OK(obtain_temporary_register(mem, state, *tail_idx, &tmp_reg, TEMPORARY_REGISTER_SSE));

//                     link_part_instr[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movd);
//                     link_part_instr[0].args[0] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
//                     link_part_instr[0].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[0]);
//                     link_part_instr[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movd);
//                     link_part_instr[1].args[0] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(
//                         reg_alloc1->spill_area.index, KEFIR_AMD64_ABI_QWORD / 2, KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE);
//                     link_part_instr[1].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[1]);
//                     link_part_instr[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[2].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movd);
//                     link_part_instr[2].args[0] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc1->spill_area.index, 0,
//                                                                                 KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE);
//                     link_part_instr[2].args[1] = KEFIR_ASMCMP_MAKE_VREG(tmp_reg);
//                     link_part_instr[2].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     num_of_links = 3;
//                 } else {
//                     link_part_instr[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movd);
//                     link_part_instr[0].args[0] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc1->spill_area.index, 0,
//                                                                                 KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE);
//                     link_part_instr[0].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[0]);
//                     link_part_instr[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movd);
//                     link_part_instr[1].args[0] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(
//                         reg_alloc1->spill_area.index, KEFIR_AMD64_ABI_QWORD / 2, KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE);
//                     link_part_instr[1].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[1]);
//                     link_part_instr[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     num_of_links = 2;
//                 }
//                 break;

//             case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_DOUBLE:
//                 if (alias) {
//                     kefir_asm_amd64_xasmgen_register_t tmp_reg;
//                     REQUIRE_OK(obtain_temporary_register(mem, state, *tail_idx, &tmp_reg, TEMPORARY_REGISTER_SSE));
                    
//                     link_part_instr[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movq);
//                     link_part_instr[0].args[0] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
//                     link_part_instr[0].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[0]);
//                     link_part_instr[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movq);
//                     link_part_instr[1].args[0] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(
//                         reg_alloc1->spill_area.index, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE);
//                     link_part_instr[1].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[1]);
//                     link_part_instr[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[2].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movq);
//                     link_part_instr[2].args[0] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc1->spill_area.index, 0,
//                                                                                 KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE);
//                     link_part_instr[2].args[1] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
//                     link_part_instr[2].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     num_of_links = 3;
//                 } else {
//                     link_part_instr[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movq);
//                     link_part_instr[0].args[0] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc1->spill_area.index, 0,
//                                                                                 KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE);
//                     link_part_instr[0].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[0]);
//                     link_part_instr[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movq);
//                     link_part_instr[1].args[0] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(
//                         reg_alloc1->spill_area.index, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE);
//                     link_part_instr[1].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[1]);
//                     link_part_instr[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     num_of_links = 2;
//                 }

//                 op_flags = KEFIR_AMD64_INSTRDB_WRITE | KEFIR_AMD64_INSTRDB_GP_REGISTER_MEMORY;
//                 break;

//             case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE:
//                 if (alias) {
//                     kefir_asm_amd64_xasmgen_register_t tmp_reg;
//                     REQUIRE_OK(obtain_temporary_register(mem, state, *tail_idx, &tmp_reg, TEMPORARY_REGISTER_GP));

//                     link_part_instr[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(mov);
//                     link_part_instr[0].args[0] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
//                     link_part_instr[0].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[0]);
//                     link_part_instr[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(mov);
//                     link_part_instr[1].args[0] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(
//                         reg_alloc1->spill_area.index, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT);
//                     link_part_instr[1].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[1]);
//                     link_part_instr[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[2].opcode = KEFIR_ASMCMP_AMD64_OPCODE(mov);
//                     link_part_instr[2].args[0] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc1->spill_area.index, 0,
//                                                                                 KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT);
//                     link_part_instr[2].args[1] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
//                     link_part_instr[2].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     num_of_links = 3;
//                 } else {
//                     link_part_instr[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(mov);
//                     link_part_instr[0].args[0] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc1->spill_area.index, 0,
//                                                                                 KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT);
//                     link_part_instr[0].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[0]);
//                     link_part_instr[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     link_part_instr[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(mov);
//                     link_part_instr[1].args[0] = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(
//                         reg_alloc1->spill_area.index, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT);
//                     link_part_instr[1].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[1]);
//                     link_part_instr[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     num_of_links = 2;
//                 }
//                 break;
//         }

//         for (kefir_size_t i = 0; i < num_of_links; i++) {
//             REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(mem, &state->target->context, *tail_idx, &link_part_instr[i],
//                                                             tail_idx));
//             kefir_asmcmp_instruction_index_t virtual_instr_idx = *tail_idx;
//             REQUIRE_OK(devirtualize_instr2(
//                 mem, state, *tail_idx, &link_part_instr[i], original_instr, tail_idx, KEFIR_AMD64_INSTRDB_NONE,
//                 op_flags,
//             op_flags | KEFIR_AMD64_INSTRDB_IMMEDIATE));
//             REQUIRE_OK(kefir_asmcmp_context_instr_replace(&state->target->context, virtual_instr_idx, &link_part_instr[i]));
//         }
//         instr->opcode = KEFIR_ASMCMP_AMD64_OPCODE(noop);
//     } else if (reg_alloc1->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_PAIR &&
//                reg_alloc2->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_PAIR) {
//         struct kefir_asmcmp_instruction link_part_instr[3];
//         kefir_asm_amd64_xasmgen_register_t tmp_reg;
//         kefir_size_t num_of_links = 3;
//         kefir_uint64_t op_flags = KEFIR_AMD64_INSTRDB_WRITE | KEFIR_AMD64_INSTRDB_GP_REGISTER_MEMORY | KEFIR_AMD64_INSTRDB_XMM_REGISTER_MEMORY_FULL;

//         const struct kefir_codegen_amd64_register_allocation *vreg_part0_alloc, *vreg2_part1_alloc;
//         REQUIRE_OK(kefir_codegen_amd64_xregalloc_allocation_of(state->xregalloc, vreg->parameters.pair.virtual_registers[0], &vreg_part0_alloc));
//         REQUIRE_OK(kefir_codegen_amd64_xregalloc_allocation_of(state->xregalloc, vreg2->parameters.pair.virtual_registers[1], &vreg2_part1_alloc));
//         kefir_bool_t alias = vreg_part0_alloc->type == vreg2_part1_alloc->type;
//         if (vreg_part0_alloc->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER &&
//             vreg2_part1_alloc->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER &&
//             vreg_part0_alloc->direct_reg != vreg2_part1_alloc->direct_reg) {
//             alias = false;
//         } else if (vreg_part0_alloc->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT &&
//                 vreg2_part1_alloc->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT &&
//                 vreg_part0_alloc->spill_area.index != vreg2_part1_alloc->spill_area.index) {
//             alias = false;
//         }

//         switch (vreg->parameters.pair.type) {
//             case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERIC:
//                 return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected virtual register pair type");

//             case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_SINGLE:
//             case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_DOUBLE: {
//                 if (alias) {
//                     REQUIRE_OK(obtain_temporary_register(mem, state, *tail_idx, &tmp_reg, TEMPORARY_REGISTER_SSE));
//                     link_part_instr[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movdqu);
//                     link_part_instr[0].args[0] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
//                     link_part_instr[0].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[0]);
//                     link_part_instr[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;
//                     link_part_instr[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movdqu);
//                     link_part_instr[1].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[1]);
//                     link_part_instr[1].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[1]);
//                     link_part_instr[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;
//                     link_part_instr[2].opcode = KEFIR_ASMCMP_AMD64_OPCODE(movdqu);
//                     link_part_instr[2].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[0]);
//                     link_part_instr[2].args[1] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
//                     link_part_instr[2].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     num_of_links = 3;
//                 } else {
//                     link_part_instr[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link);
//                     link_part_instr[0].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[0]);
//                     link_part_instr[0].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[0]);
//                     link_part_instr[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;
//                     link_part_instr[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link);
//                     link_part_instr[1].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[1]);
//                     link_part_instr[1].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[1]);
//                     link_part_instr[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     num_of_links = 2;
//                 }
//                 op_flags = KEFIR_AMD64_INSTRDB_WRITE | KEFIR_AMD64_INSTRDB_GP_REGISTER_MEMORY | KEFIR_AMD64_INSTRDB_XMM_REGISTER_MEMORY_FULL;
//             } break;

//             case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE: {
//                 if (alias) {
//                     REQUIRE_OK(obtain_temporary_register(mem, state, *tail_idx, &tmp_reg, TEMPORARY_REGISTER_GP));
//                     link_part_instr[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(mov);
//                     link_part_instr[0].args[0] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
//                     link_part_instr[0].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[0]);
//                     link_part_instr[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;
//                     link_part_instr[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link);
//                     link_part_instr[1].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[1]);
//                     link_part_instr[1].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[1]);
//                     link_part_instr[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;
//                     link_part_instr[2].opcode = KEFIR_ASMCMP_AMD64_OPCODE(mov);
//                     link_part_instr[2].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[0]);
//                     link_part_instr[2].args[1] = KEFIR_ASMCMP_MAKE_PHREG(tmp_reg);
//                     link_part_instr[2].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     num_of_links = 3;
//                 } else {
//                     link_part_instr[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link);
//                     link_part_instr[0].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[0]);
//                     link_part_instr[0].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[0]);
//                     link_part_instr[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;
//                     link_part_instr[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link);
//                     link_part_instr[1].args[0] = KEFIR_ASMCMP_MAKE_VREG(vreg->parameters.pair.virtual_registers[1]);
//                     link_part_instr[1].args[1] = KEFIR_ASMCMP_MAKE_VREG(vreg2->parameters.pair.virtual_registers[1]);
//                     link_part_instr[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE;

//                     num_of_links = 2;
//                 }

//                 op_flags = KEFIR_AMD64_INSTRDB_WRITE | KEFIR_AMD64_INSTRDB_GP_REGISTER_MEMORY;
//             } break;
//         }

//         for (kefir_size_t i = 0; i < num_of_links; i++) {
//             REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(mem, &state->target->context, *tail_idx, &link_part_instr[i],
//                                                             tail_idx));
//             kefir_asmcmp_instruction_index_t virtual_instr_idx = *tail_idx;
//             REQUIRE_OK(devirtualize_instr2(
//                 mem, state, *tail_idx, &link_part_instr[i], original_instr, tail_idx, KEFIR_AMD64_INSTRDB_NONE,
//                 op_flags,
//                 op_flags | KEFIR_AMD64_INSTRDB_IMMEDIATE));
//             REQUIRE_OK(kefir_asmcmp_context_instr_replace(&state->target->context, virtual_instr_idx, &link_part_instr[i]));
//         }

//         instr->opcode = KEFIR_ASMCMP_AMD64_OPCODE(noop);
//     } else {
//         REQUIRE(reg_alloc1->type != KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_PAIR,
//                 KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
//                                 "Unexpected virtual register linking into a pair of virtual registers"));
//         REQUIRE(reg_alloc2->type != KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_PAIR,
//                 KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
//                                 "Unexpected pair of virtual registers linking into a virtual register"));
//         REQUIRE_OK(link_virtual_registers(mem, state, instr_idx, instr, original_instr, tail_idx));
//     }

//     return KEFIR_OK;
// }

static kefir_result_t build_current_instr_state(struct destructor_state *state, kefir_codegen_target_ir_instruction_ref_t instr_ref) {
    REQUIRE_OK(kefir_hashset_clear(state->mem, &state->current_instr.interfere_registers));
    REQUIRE_OK(kefir_hashset_clear(state->mem, &state->current_instr.input_registers));
    REQUIRE_OK(kefir_hashtable_clear(&state->current_instr.scratch_registers));

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
                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL:
                    // Intentionally left blank
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
                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL:
                    // Intentionally left blank
                    break;

                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP:
                case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE:
                    REQUIRE_OK(kefir_hashset_add(state->mem, &state->current_instr.input_registers, (kefir_hashset_key_t) entry.reg.value));
                    break;
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_instruction(struct destructor_state *state, kefir_codegen_target_ir_instruction_ref_t instr_ref) {
    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(state->code, instr_ref, &instr));

    if (instr->operation.opcode == state->code->klass->placeholder_opcode ||
        instr->operation.opcode == state->code->klass->phi_opcode ||
        instr->operation.opcode == state->code->klass->touch_opcode ||
        instr->operation.opcode == state->code->klass->assign_opcode) {
        return KEFIR_OK;
    }

    kefir_result_t res;
    kefir_codegen_target_ir_native_id_t attribute;
    struct kefir_codegen_target_ir_code_attribute_iterator attr_iter;
    for (res = kefir_codegen_target_ir_code_instruction_attribute_iter(state->code, &attr_iter, instr_ref, &attribute);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_instruction_attribute_next(&attr_iter, &attribute)) {
        REQUIRE_OK(state->parameter->materialize_attribute(state->mem, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context), attribute, NULL, state->parameter->payload));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_codegen_target_ir_target_ir_instruction_destructor_classification classification;
    REQUIRE_OK(state->parameter->classify_instruction(state->code, instr_ref, &classification, state->parameter->payload));

    struct kefir_codegen_target_ir_block_terminator_props terminator_props;
    REQUIRE_OK(state->code->klass->is_block_terminator(state->code, instr, &terminator_props, state->code->klass->payload));
    if (terminator_props.block_terminator && !terminator_props.function_terminator) {
        return KEFIR_OK; // TODO KEFIR_NOT_IMPLEMENTED
    }

    if (instr->operation.opcode == state->code->klass->inline_asm_opcode) {
        return KEFIR_OK; // TODO KEFIR_NOT_IMPLEMENTED
    }

    REQUIRE_OK(build_current_instr_state(state, instr_ref));

    struct kefir_asmcmp_instruction asmcmp_instruction = {
        .opcode = classification.opcode,
        .args[0].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
        .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
        .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE
    };
    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        switch (classification.operands[i].class) {
            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_NONE:
                // Intentionally left blank
                break;

            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ:
                if (!classification.operands[i].implicit) {
                    REQUIRE_OK(resolve_operand(state, &instr->operation.parameters[i], &asmcmp_instruction.args[i]));
                } else {
                    asmcmp_instruction.args[i].type = KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER;
                }
                break;

            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE:
                if (!classification.operands[i].implicit) {
                    REQUIRE_OK(resolve_operand(state, &instr->operation.parameters[i], &asmcmp_instruction.args[i]));
                } else {
                    asmcmp_instruction.args[i].type = KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER;
                }
                break;

            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE:
                if (!classification.operands[i].implicit) {
                    REQUIRE_OK(resolve_operand(state, &instr->operation.parameters[i], &asmcmp_instruction.args[i]));
                } else {
                    asmcmp_instruction.args[i].type = KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER;
                }
                break;
        }
    }
    REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(state->mem, &state->asmcmp_ctx->context, kefir_asmcmp_context_instr_tail(&state->asmcmp_ctx->context),
        &asmcmp_instruction, NULL));
    return KEFIR_OK;
}

static kefir_result_t translate_block(struct destructor_state *state, kefir_codegen_target_ir_block_ref_t block_ref) {
    kefir_hashtable_value_t table_value;
    REQUIRE_OK(kefir_hashtable_at(&state->blocks, (kefir_hashtable_key_t) block_ref, &table_value));
    ASSIGN_DECL_CAST(struct block_state *, block_state,
        table_value);

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(state->mem, &state->asmcmp_ctx->context, block_state->asmcmp_label));

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
                .opcode = state->parameter->unreachable_opcode,
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
        .opcode = state->parameter->noop_opcode,
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
    REQUIRE_OK(state->parameter->schedule_code(state->mem, &state->control_flow, &state->schedule, state->parameter->payload));
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
    const struct kefir_codegen_target_ir_interference *interference,
    const struct kefir_codegen_target_ir_regalloc *regalloc,
    const struct kefir_codegen_target_ir_code_constructor_metadata *constructor_metadata,
    const struct kefir_codegen_target_ir_round_trip_destructor_ops *parameter) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(asmcmp_ctx != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp context"));
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference"));
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));
    REQUIRE(parameter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR round trip destructor parameter"));

    UNUSED(obtain_temporary_register);

    struct destructor_state state = {
        .mem = mem,
        .code = code,
        .asmcmp_ctx = asmcmp_ctx,
        .interference = interference,
        .regalloc = regalloc,
        .parameter = parameter,
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
    REQUIRE_OK(kefir_hashtable_init(&state.current_instr.scratch_registers, &kefir_hashtable_uint_ops));

    kefir_result_t res = destruct_impl(&state);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.current_instr.scratch_registers);
        kefir_hashset_free(mem, &state.current_instr.interfere_registers);
        kefir_hashset_free(mem, &state.current_instr.input_registers);
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
        kefir_codegen_target_ir_code_schedule_free(mem, &state.schedule);
        kefir_codegen_target_ir_liveness_free(mem, &state.liveness);
        kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
        kefir_hashtable_free(mem, &state.blocks);
        kefir_hashtable_free(mem, &state.native_labels);
        return res;
    });
    res = kefir_hashset_free(mem, &state.current_instr.input_registers);
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
