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

#include "kefir/codegen/target-ir/constructor.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/core/hashtable.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define LINEAR_INDEX_NONE ((kefir_size_t) ~0ull)

struct code_block_state {
    kefir_codegen_target_ir_block_ref_t block_ref;
    struct kefir_hashtable block_inputs;
    struct kefir_hashtable virtual_register_refs;
};

struct asmcmp_instr_state {
    kefir_size_t linear_index;
};

struct asmcmp_vreg_state {
    struct {
        kefir_codegen_target_ir_block_ref_t block_ref;
    } dominator;
    struct kefir_hashtable block_lifetimes;
};

struct constructor_state {
    struct kefir_mem *mem;
    struct kefir_codegen_target_ir_code *code;
    const struct kefir_asmcmp_context *asmcmp_ctx;
    const struct kefir_codegen_target_ir_code_constructor_class *klass;
    struct kefir_codegen_target_ir_control_flow control_flow;
    kefir_size_t instr_linear_index;
    struct kefir_hashtree blocks;
    struct asmcmp_instr_state *asmcmp_instrs;
    struct asmcmp_vreg_state *asmcmp_vregs;
    struct kefir_hashtable block_head_instr;
    struct kefir_hashtable label_blocks;
};

static kefir_result_t mark_vreg_liveness(struct constructor_state *state, kefir_size_t linear_index, kefir_codegen_target_ir_block_ref_t block_ref, kefir_asmcmp_virtual_register_index_t vreg_idx) {
    struct asmcmp_vreg_state *vreg_state = &state->asmcmp_vregs[vreg_idx];
    kefir_hashtable_value_t *value_ptr = NULL;
    kefir_result_t res = kefir_hashtable_at_mut(&vreg_state->block_lifetimes, (kefir_hashtable_key_t) block_ref, &value_ptr);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        kefir_size_t current_linear_index_begin = ((kefir_uint64_t) *value_ptr) >> 32;
        kefir_size_t current_linear_index_end = (kefir_uint32_t) *value_ptr;
        *value_ptr = ((kefir_uint64_t) MIN(linear_index, current_linear_index_begin)) << 32 | ((kefir_uint32_t) MAX(linear_index, current_linear_index_end));
    } else {
        const kefir_uint64_t value = ((kefir_uint64_t) linear_index) << 32 | ((kefir_uint32_t) linear_index);
        REQUIRE_OK(kefir_hashtable_insert(state->mem, &vreg_state->block_lifetimes, (kefir_hashtable_key_t) block_ref, (kefir_hashtable_value_t) value));
    }
    return KEFIR_OK;
}

static kefir_result_t mark_value_liveness(struct constructor_state *state, kefir_size_t linear_index, kefir_codegen_target_ir_block_ref_t block_ref, const struct kefir_asmcmp_value *value) {
    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL:
        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL:
        case KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL:
        case KEFIR_ASMCMP_VALUE_TYPE_EXTERNAL_LABEL:
        case KEFIR_ASMCMP_VALUE_TYPE_X87:
        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER:
        case KEFIR_ASMCMP_VALUE_TYPE_INLINE_ASSEMBLY_INDEX:
            // Intentionally left blank
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
            REQUIRE_OK(mark_vreg_liveness(state, linear_index, block_ref, value->vreg.index));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                    REQUIRE_OK(mark_vreg_liveness(state, linear_index, block_ref, value->indirect.base.vreg));
                    break;

                case KEFIR_ASMCMP_INDIRECT_INTERNAL_LABEL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_EXTERNAL_LABEL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_LOCAL_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_STASH_INDEX: {
            kefir_asmcmp_virtual_register_index_t vreg_idx;
            REQUIRE_OK(kefir_asmcmp_register_stash_vreg(state->asmcmp_ctx, value->stash_idx, &vreg_idx));
            REQUIRE_OK(mark_vreg_liveness(state, linear_index, block_ref, vreg_idx));
        } break;
    }

    return KEFIR_OK;
}

static kefir_result_t init_code_blocks(struct constructor_state *state) {
    for (kefir_size_t i = 0; i < state->asmcmp_ctx->code_length; i++) {
        state->asmcmp_instrs[i].linear_index = LINEAR_INDEX_NONE;
    }

    struct code_block_state *block_state = NULL;
    for (kefir_asmcmp_instruction_index_t instr_idx = kefir_asmcmp_context_instr_head(state->asmcmp_ctx);
        instr_idx != KEFIR_ASMCMP_INDEX_NONE;
        instr_idx = kefir_asmcmp_context_instr_next(state->asmcmp_ctx, instr_idx)) {

        struct kefir_asmcmp_instruction *asmcmp_instr;
        REQUIRE_OK(kefir_asmcmp_context_instr_at(state->asmcmp_ctx, instr_idx, &asmcmp_instr));

        kefir_bool_t is_jump;
        REQUIRE_OK(state->klass->is_jump(asmcmp_instr->opcode, &is_jump, state->klass->payload));
        kefir_asmcmp_label_index_t label_idx = kefir_asmcmp_context_instr_label_head(state->asmcmp_ctx, instr_idx);
        if (block_state == NULL || label_idx != KEFIR_ASMCMP_INDEX_NONE) {
            kefir_codegen_target_ir_block_ref_t block_ref;
            REQUIRE_OK(kefir_codegen_target_ir_code_new_block(state->mem, state->code, &block_ref));
            
            block_state = KEFIR_MALLOC(state->mem, sizeof(struct code_block_state));
            REQUIRE(block_state != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate code block state"));
            block_state->block_ref = block_ref;
            kefir_result_t res = kefir_hashtable_init(&block_state->block_inputs, &kefir_hashtable_uint_ops);
            REQUIRE_CHAIN(&res, kefir_hashtable_init(&block_state->virtual_register_refs, &kefir_hashtable_uint_ops));
            REQUIRE_CHAIN(&res, kefir_hashtree_insert(state->mem, &state->blocks, (kefir_hashtree_key_t) block_ref, (kefir_hashtree_value_t) block_state));
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_FREE(state->mem, block_state);
                return res;
            });
            REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->block_head_instr, (kefir_hashtable_key_t) instr_idx, (kefir_hashtable_value_t) block_ref));
        }
        for (; label_idx != KEFIR_ASMCMP_INDEX_NONE; label_idx = kefir_asmcmp_context_instr_label_next(state->asmcmp_ctx, label_idx)) {
            REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->label_blocks, (kefir_hashtable_key_t) label_idx, (kefir_hashtable_value_t) block_state->block_ref));
        }

        state->asmcmp_instrs[instr_idx].linear_index = state->instr_linear_index++;
        for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
            REQUIRE_OK(mark_value_liveness(state, state->asmcmp_instrs[instr_idx].linear_index, block_state->block_ref, &asmcmp_instr->args[i]));
        }

        if (is_jump) {
            block_state = NULL;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t init_relocation(kefir_asmcmp_external_label_relocation_t asmcmp_reloc, kefir_codegen_target_ir_external_label_relocation_t *relocation) {
    switch (asmcmp_reloc) {
        case KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE:
            *relocation = KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_ABSOLUTE;
            break;

        case KEFIR_ASMCMP_EXTERNAL_LABEL_PLT:
            *relocation = KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_PLT;
            break;

        case KEFIR_ASMCMP_EXTERNAL_LABEL_GOTPCREL:
            *relocation = KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_GOTPCREL;
            break;

        case KEFIR_ASMCMP_EXTERNAL_LABEL_TPOFF:
            *relocation = KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_TPOFF;
            break;

        case KEFIR_ASMCMP_EXTERNAL_LABEL_GOTTPOFF:
            *relocation = KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_GOTTPOFF;
            break;

        case KEFIR_ASMCMP_EXTERNAL_LABEL_TLSGD:
            *relocation = KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_TLSGD;
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t init_variant(kefir_asmcmp_operand_variant_t asmcmp_variant, kefir_bool_t high_half, kefir_codegen_target_ir_operand_variant_t *variant) {
    switch (asmcmp_variant) {
        case KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT:
            *variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT;
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_8BIT:
            *variant = high_half ? KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT_HIGHER : KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT;
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_16BIT:
            *variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT;
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_32BIT:
            *variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT;
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_64BIT:
            *variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT;
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_80BIT:
            *variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_80BIT;
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_128BIT:
            *variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_128BIT;
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE:
            *variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_FP_SINGLE;
            break;
            
        case KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE:
            *variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_FP_DOUBLE;
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t store_virtual_reigster_output(struct constructor_state *state, struct code_block_state *block_state, kefir_asmcmp_virtual_register_index_t vreg, struct kefir_codegen_target_ir_value_ref value_ref) {
    kefir_hashtable_value_t value_ref_encoded = (((kefir_uint64_t) value_ref.instr_ref) << 32) | (kefir_uint32_t) value_ref.aspect;
    kefir_hashtable_value_t *value;
    kefir_result_t res = kefir_hashtable_at_mut(&block_state->virtual_register_refs, (kefir_hashtable_key_t) vreg, &value);
    if (res != KEFIR_NOT_FOUND) {
        *value = value_ref_encoded;
    } else {
        REQUIRE_OK(kefir_hashtable_insert(state->mem, &block_state->virtual_register_refs, (kefir_hashtable_key_t) vreg, value_ref_encoded));
    }
    return KEFIR_OK;
}

static kefir_result_t resolve_input_virtual_register(struct constructor_state *state, struct code_block_state *block_state, kefir_asmcmp_virtual_register_index_t vreg_idx, struct kefir_codegen_target_ir_value_ref *value_ref) {
    kefir_hashtable_value_t value;
    kefir_result_t res = kefir_hashtable_at(&block_state->virtual_register_refs, (kefir_hashtable_key_t) vreg_idx, &value);
    if (res != KEFIR_NOT_FOUND) {
        value_ref->instr_ref = ((kefir_uint64_t) value) >> 32;
        value_ref->aspect = (kefir_uint32_t) value;
    } else {
        struct kefir_codegen_target_ir_operation operation = {
            .opcode = state->code->klass->phi_opcode,
            .parameters[0].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE,
            .parameters[1].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE,
            .parameters[2].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE
        };
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(state->mem, state->code, block_state->block_ref,
            KEFIR_ID_NONE,
            &operation, &value_ref->instr_ref));
        value_ref->aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_PHI;
        REQUIRE_OK(store_virtual_reigster_output(state, block_state, vreg_idx, *value_ref));
        REQUIRE_OK(kefir_hashtable_insert(state->mem, &block_state->block_inputs, (kefir_hashtable_key_t) vreg_idx, (kefir_hashtable_value_t) value_ref->instr_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t init_operand(struct constructor_state *state, struct code_block_state *block_state, struct kefir_codegen_target_ir_operand *operand, const struct kefir_asmcmp_instruction *instr, const struct kefir_codegen_target_ir_asmcmp_operand_classification *classification, kefir_asmcmp_virtual_register_index_t *output_vreg) {
    UNUSED(state);
    
    operand->type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE;
    *output_vreg = KEFIR_ASMCMP_INDEX_NONE;
    REQUIRE(classification->class != KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_NONE, KEFIR_OK);
    const struct kefir_asmcmp_value *value = &instr->args[classification->index];
    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected asmcmp value of none type");

        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
            REQUIRE(classification->class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected integral constant to have read operand class"));
            operand->type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER;
            operand->int_immediate = value->int_immediate;
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
            REQUIRE(classification->class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected integral constant to have read operand class"));
            operand->type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UINTEGER;
            operand->uint_immediate = value->uint_immediate;
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER:
            operand->type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_PHYSICAL_REGISTER;
            operand->phreg = value->phreg;
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER: {
            const struct kefir_asmcmp_virtual_register *vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_get(state->asmcmp_ctx, value->vreg.index, &vreg));
            if (vreg->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_IMMEDIATE_INTEGER) {
                operand->type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER;
                operand->int_immediate = vreg->parameters.immediate_int;
            } else {
                if (classification->class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ || classification->class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE) {
                    operand->type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF;
                    REQUIRE_OK(resolve_input_virtual_register(state, block_state, value->vreg.index, &operand->direct.value_ref));
                    REQUIRE_OK(init_variant(value->vreg.variant, value->vreg.high_half, &operand->direct.variant));
                }
                if (classification->class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE || classification->class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE) {
                    *output_vreg = value->vreg.index;
                }
            }
        } break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            operand->type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT;
            operand->indirect.offset = value->indirect.offset;
            REQUIRE_OK(init_variant(value->indirect.variant, false, &operand->indirect.variant));
            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                    operand->indirect.type = KEFIR_CODEGEN_TARGET_IR_INDIRECT_PHYSICAL_BASIS;
                    operand->indirect.base.phreg = value->indirect.base.phreg;
                    break;

                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                    operand->indirect.type = KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS;
                    REQUIRE_OK(resolve_input_virtual_register(state, block_state, value->indirect.base.vreg, &operand->indirect.base.value_ref));
                    break;

                case KEFIR_ASMCMP_INDIRECT_INTERNAL_LABEL_BASIS: {
                    kefir_hashtable_value_t table_value;
                    kefir_result_t res = kefir_hashtable_at(&state->label_blocks, (kefir_hashtable_key_t) value->indirect.base.internal_label, &table_value);
                    if (res != KEFIR_NOT_FOUND) {
                        REQUIRE_OK(res);
                        operand->indirect.type = KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS;
                        operand->indirect.base.block_ref = (kefir_codegen_target_ir_block_ref_t) table_value;
                    } else {
                        operand->indirect.type = KEFIR_CODEGEN_TARGET_IR_INDIRECT_ASMCMP_LABEL_BASIS;
                        operand->indirect.base.asmcmp_label = value->indirect.base.internal_label;
                    }
                } break;
                
                case KEFIR_ASMCMP_INDIRECT_EXTERNAL_LABEL_BASIS:
                    operand->indirect.type = KEFIR_CODEGEN_TARGET_IR_INDIRECT_EXTERNAL_LABEL_BASIS;
                    operand->indirect.base.external_label = value->indirect.base.external_label;
                    REQUIRE_OK(init_relocation(value->indirect.base.external_type, &operand->indirect.base.external_type));
                    break;

                case KEFIR_ASMCMP_INDIRECT_LOCAL_AREA_BASIS:
                    operand->indirect.type = KEFIR_CODEGEN_TARGET_IR_INDIRECT_LOCAL_AREA_BASIS;
                    operand->indirect.base.local_variable_id = value->indirect.base.local_variable_id;
                    break;

                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                    operand->indirect.type = KEFIR_CODEGEN_TARGET_IR_INDIRECT_SPILL_AREA_BASIS;
                    operand->indirect.base.spill_index = value->indirect.base.spill_index;
                    break;
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL: {
            kefir_hashtable_value_t table_value;
            kefir_result_t res = kefir_hashtable_at(&state->label_blocks, (kefir_hashtable_key_t) value->rip_indirection.internal, &table_value);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                operand->type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF;
                operand->rip_indirection.block_ref = (kefir_codegen_target_ir_block_ref_t) table_value;
            } else {
                operand->type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_ASMCMP;
                operand->rip_indirection.asmcmp_label = value->rip_indirection.internal;
            }

            REQUIRE_OK(init_relocation(value->rip_indirection.position, &operand->rip_indirection.position));
            REQUIRE_OK(init_variant(value->rip_indirection.variant, false, &operand->rip_indirection.variant));
        } break;

        case KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL: {
            kefir_hashtable_value_t table_value;
            kefir_result_t res = kefir_hashtable_at(&state->label_blocks, (kefir_hashtable_key_t) value->internal_label, &table_value);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                operand->type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF;
                operand->block_ref = (kefir_codegen_target_ir_block_ref_t) table_value;
            } else {
                operand->type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_ASMCMP_LABEL;
                operand->asmcmp_label = value->internal_label;
            }
        } break;

        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL:
            operand->type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_EXTERNAL;
            operand->rip_indirection.external = value->rip_indirection.external;
            REQUIRE_OK(init_relocation(value->rip_indirection.position, &operand->rip_indirection.position));
            REQUIRE_OK(init_variant(value->rip_indirection.variant, false, &operand->rip_indirection.variant));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_EXTERNAL_LABEL:
            operand->type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_EXTERNAL_LABEL;
            operand->external_label.symbolic = value->external_label.symbolic;
            operand->external_label.offset = value->external_label.offset;
            REQUIRE_OK(init_relocation(value->external_label.position, &operand->external_label.position));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_X87:
            operand->type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87;
            operand->x87 = value->x87;
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_STASH_INDEX:
            operand->type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_STASH_INDEX;
            operand->stash_idx = value->stash_idx;
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INLINE_ASSEMBLY_INDEX:
            operand->type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INLINE_ASSEMBLY_INDEX;
            operand->inline_asm_idx = value->inline_asm_idx;
            break;
    }

    return KEFIR_OK;
}

static kefir_result_t terminate_current_block(struct constructor_state *state, struct code_block_state *current_block_state, struct code_block_state *next_block_state) {
    kefir_codegen_target_ir_instruction_ref_t current_block_tail_ref = kefir_codegen_target_ir_code_block_control_tail(state->code, current_block_state->block_ref);
    if (current_block_tail_ref == KEFIR_ID_NONE) {
        struct kefir_codegen_target_ir_operation operation;
        REQUIRE_OK(state->code->klass->make_unconditional_jump(next_block_state->block_ref, &operation, state->code->klass->payload));
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(state->mem, state->code, current_block_state->block_ref,
            kefir_codegen_target_ir_code_block_control_tail(state->code, current_block_state->block_ref),
            &operation, NULL));
        return KEFIR_OK;
    }

    const struct kefir_codegen_target_ir_instruction *current_block_tail = NULL;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(state->code, current_block_tail_ref, &current_block_tail));

    struct kefir_codegen_target_ir_block_terminator_props terminator_props;
    REQUIRE_OK(state->code->klass->is_block_terminator(current_block_tail, &terminator_props, state->code->klass->payload));

    if (!terminator_props.block_terminator) {
        struct kefir_codegen_target_ir_operation operation;
        REQUIRE_OK(state->code->klass->make_unconditional_jump(next_block_state->block_ref, &operation, state->code->klass->payload));
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(state->mem, state->code, current_block_state->block_ref,
            kefir_codegen_target_ir_code_block_control_tail(state->code, current_block_state->block_ref),
            &operation, NULL));
    } else if (terminator_props.fallthrough) {
        struct kefir_codegen_target_ir_operation operation;
        REQUIRE_OK(state->code->klass->finalize_conditional_jump(&current_block_tail->operation, next_block_state->block_ref, &operation, state->code->klass->payload));
        REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(state->mem, state->code, current_block_tail_ref));
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(state->mem, state->code, current_block_state->block_ref,
            kefir_codegen_target_ir_code_block_control_tail(state->code, current_block_state->block_ref),
            &operation, NULL));
    }
    return KEFIR_OK;
}

static kefir_result_t scan_instructions(struct constructor_state *state) {
    struct code_block_state *current_block_state = NULL;
    for (kefir_asmcmp_instruction_index_t instr_idx = kefir_asmcmp_context_instr_head(state->asmcmp_ctx);
        instr_idx != KEFIR_ASMCMP_INDEX_NONE;
        instr_idx = kefir_asmcmp_context_instr_next(state->asmcmp_ctx, instr_idx)) {
        kefir_hashtable_value_t value;
        kefir_result_t res = kefir_hashtable_at(&state->block_head_instr, (kefir_hashtable_key_t) instr_idx, &value);
        if (res != KEFIR_NOT_FOUND) {
            struct kefir_hashtree_node *node;
            REQUIRE_OK(kefir_hashtree_at(&state->blocks, (kefir_hashtree_key_t) value, &node));
            if (current_block_state != NULL) {
                REQUIRE_OK(terminate_current_block(state, current_block_state, (struct code_block_state *) node->value));
            }
            current_block_state = (struct code_block_state *) node->value;
        }

        REQUIRE(current_block_state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected non-NULL target IR code block state"));
        struct kefir_asmcmp_instruction *asmcmp_instr;
        REQUIRE_OK(kefir_asmcmp_context_instr_at(state->asmcmp_ctx, instr_idx, &asmcmp_instr));

        struct kefir_codegen_target_ir_asmcmp_instruction_classification classification;
        REQUIRE_OK(state->klass->classify_instruction(asmcmp_instr, &classification, state->klass->payload));

        if (classification.special == KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_SKIP) {
            continue;
        } else if (classification.special == KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_VIRTUAL_REGISTER_LINK) {
            struct kefir_codegen_target_ir_value_ref value_ref;
            REQUIRE_OK(resolve_input_virtual_register(state, current_block_state, asmcmp_instr->args[1].vreg.index, &value_ref));
            REQUIRE_OK(store_virtual_reigster_output(state, current_block_state, asmcmp_instr->args[0].vreg.index, value_ref));
            continue;
        }

        struct kefir_codegen_target_ir_operation operation = {
            .opcode = classification.opcode
        };
        _Static_assert(KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS >= KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS,
            "Expected number of target IR instruction parameters to exceed or be equal to the number of asmcmp instruction operands");
        kefir_size_t input_index = 0, output_index = 0;
        kefir_asmcmp_virtual_register_index_t output_vregs[KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS];
        for (kefir_size_t i = 0; i <  KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
            kefir_asmcmp_virtual_register_index_t output_vreg = KEFIR_ASMCMP_INDEX_NONE;
            REQUIRE_OK(init_operand(state, current_block_state, &operation.parameters[input_index], asmcmp_instr, &classification.operands[i], &output_vreg));
            if (operation.parameters[input_index].type != KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE) {
                input_index++;
            }
            if (output_vreg != KEFIR_ASMCMP_INDEX_NONE) {
                output_vregs[output_index++] = output_vreg;
            }
        }
        kefir_codegen_target_ir_instruction_ref_t instr_ref;
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(state->mem, state->code, current_block_state->block_ref, kefir_codegen_target_ir_code_block_control_tail(state->code, current_block_state->block_ref), &operation, &instr_ref));

        for (kefir_size_t i = 0; i < output_index; i++) {
            kefir_asmcmp_virtual_register_index_t output_vreg = output_vregs[i];
            REQUIRE_OK(store_virtual_reigster_output(state, current_block_state, output_vreg, (struct kefir_codegen_target_ir_value_ref) {
                .instr_ref = instr_ref,
                .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_OUTPUT_REGISTER(i)
            }));
        }
    }

    return KEFIR_OK;
}

static kefir_result_t link_phi(struct constructor_state *, struct code_block_state *, kefir_asmcmp_virtual_register_index_t, kefir_codegen_target_ir_instruction_ref_t);

static kefir_result_t resolve_phi(struct constructor_state *state, struct code_block_state *block_state, kefir_asmcmp_virtual_register_index_t vreg_idx, struct kefir_codegen_target_ir_value_ref *value_ref) {
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&block_state->virtual_register_refs, (kefir_hashtable_key_t) vreg_idx, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        value_ref->instr_ref = ((kefir_uint64_t) table_value) >> 32;
        value_ref->aspect = (kefir_uint32_t) table_value;
        return KEFIR_OK;
    }

    struct asmcmp_vreg_state *vreg_state = &state->asmcmp_vregs[vreg_idx];
    if (vreg_state->dominator.block_ref == KEFIR_ID_NONE) {
        kefir_hashtable_key_t key;
        struct kefir_hashtable_iterator iter;
        for (res = kefir_hashtable_iter(&vreg_state->block_lifetimes, &iter, &key, NULL);
            res == KEFIR_OK;
            res = kefir_hashtable_next(&iter, &key, NULL)) {
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, use_block_ref,
                key);
            if (use_block_ref != state->code->entry_block && state->control_flow.blocks[use_block_ref].immediate_dominator == KEFIR_ID_NONE) {
                continue;
            }
            
            REQUIRE_OK(kefir_codegen_target_ir_control_flow_find_closest_common_dominator(&state->control_flow, use_block_ref, vreg_state->dominator.block_ref, &vreg_state->dominator.block_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        REQUIRE(vreg_state->dominator.block_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to compute block dominating virtual register uses"));
    }

    kefir_bool_t use_dominator = true;
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_is_dominator(&state->control_flow, vreg_state->dominator.block_ref, block_state->block_ref, &use_dominator));

    if (use_dominator) {
        struct kefir_codegen_target_ir_operation operation = {
            .opcode = state->code->klass->placeholder_opcode,
            .parameters[0].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE,
            .parameters[1].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE,
            .parameters[2].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE
        };
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(state->mem, state->code, block_state->block_ref,
            KEFIR_ID_NONE,
            &operation, &value_ref->instr_ref));
        value_ref->aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_PHI;
        REQUIRE_OK(store_virtual_reigster_output(state, block_state, vreg_idx, *value_ref));
    } else {
        struct kefir_hashtreeset_iterator iter;
        kefir_result_t res = kefir_hashtreeset_iter(&state->control_flow.blocks[block_state->block_ref].predecessors, &iter);
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, predecessor_block_ref,
                iter.entry);
            res = kefir_hashtreeset_next(&iter);
            if (res == KEFIR_ITERATOR_END) {
                struct kefir_hashtree_node *node;
                REQUIRE_OK(kefir_hashtree_at(&state->blocks, (kefir_hashtree_key_t) predecessor_block_ref, &node));
                ASSIGN_DECL_CAST(struct code_block_state *, predecessor_block_state,
                    node->value);
                return resolve_phi(state, predecessor_block_state, vreg_idx, value_ref);
            } else {
                REQUIRE_OK(res);
            }
        }

        kefir_codegen_target_ir_instruction_ref_t own_phi_instr_ref;
        struct kefir_codegen_target_ir_operation operation = {
            .opcode = state->code->klass->phi_opcode,
            .parameters[0].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE,
            .parameters[1].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE,
            .parameters[2].type = KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE
        };
        REQUIRE_OK(kefir_codegen_target_ir_code_new_instruction(state->mem, state->code, block_state->block_ref,
            KEFIR_ID_NONE,
            &operation, &own_phi_instr_ref));
        REQUIRE_OK(store_virtual_reigster_output(state, block_state, vreg_idx, (struct kefir_codegen_target_ir_value_ref) {
            .instr_ref = own_phi_instr_ref,
            .aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_PHI
        }));
        REQUIRE_OK(kefir_hashtable_insert(state->mem, &block_state->block_inputs, (kefir_hashtable_key_t) vreg_idx, (kefir_hashtable_value_t) own_phi_instr_ref));

        REQUIRE_OK(link_phi(state, block_state, vreg_idx, own_phi_instr_ref));

        value_ref->instr_ref = own_phi_instr_ref;
        value_ref->aspect = KEFIR_CODEGEN_TARGET_IR_VALUE_PHI;
    }

    return KEFIR_OK;
}

static kefir_result_t link_phi(struct constructor_state *state, struct code_block_state *block_state, kefir_asmcmp_virtual_register_index_t vreg_idx, kefir_codegen_target_ir_instruction_ref_t phi_instr_ref) {

    kefir_result_t res;
    struct kefir_hashtreeset_iterator iter;
    for (res = kefir_hashtreeset_iter(&state->control_flow.blocks[block_state->block_ref].predecessors, &iter); res == KEFIR_OK;
        res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, predecessor_block_ref, (kefir_uptr_t) iter.entry);

        struct kefir_hashtree_node *node;
        REQUIRE_OK(kefir_hashtree_at(&state->blocks, (kefir_hashtree_key_t) predecessor_block_ref, &node));
        ASSIGN_DECL_CAST(struct code_block_state *, predecessor_block_state,
            node->value);
        
        struct kefir_codegen_target_ir_value_ref value_ref = {0};
        REQUIRE_OK(resolve_phi(state, predecessor_block_state, vreg_idx, &value_ref));
        REQUIRE_OK(kefir_codegen_target_ir_code_phi_attach(state->mem, state->code, phi_instr_ref, predecessor_block_ref, value_ref));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t link_phis(struct constructor_state *state) {
    struct kefir_hashtree_node *block_node;
    REQUIRE_OK(kefir_hashtree_min(&state->blocks, &block_node));
    for (; block_node != NULL; block_node = kefir_hashtree_next_node(&state->blocks, block_node)) {
        ASSIGN_DECL_CAST(struct code_block_state *, block_state,
            block_node->value);
        if (block_state->block_ref != state->code->entry_block && state->control_flow.blocks[block_state->block_ref].immediate_dominator == KEFIR_ID_NONE) {
            continue;
        }
        
        kefir_hashtable_key_t key;
        kefir_hashtable_value_t value;
        struct kefir_hashtable_iterator iter;
        kefir_result_t res;
        for (res = kefir_hashtable_iter(&block_state->block_inputs, &iter, &key, &value);
            res == KEFIR_OK;
            res = kefir_hashtable_next(&iter, &key, &value)) {
            ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg_idx,
                key);
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_instruction_ref_t, phi_instr_ref,
                value);
            REQUIRE_OK(link_phi(state, block_state, vreg_idx, phi_instr_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t code_construct(struct constructor_state *state) {
    REQUIRE_OK(init_code_blocks(state));
    REQUIRE_OK(scan_instructions(state));
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_build(state->mem, &state->control_flow));
    REQUIRE_OK(link_phis(state));
    return KEFIR_OK;
}

static kefir_result_t free_code_block_state(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct code_block_state *, block_state,
        value);
    REQUIRE(block_state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid code block state"));

    REQUIRE_OK(kefir_hashtable_free(mem, &block_state->block_inputs));
    REQUIRE_OK(kefir_hashtable_free(mem, &block_state->virtual_register_refs));
    KEFIR_FREE(mem, block_state);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_construct(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, const struct kefir_asmcmp_context *asmcmp_ctx, const struct kefir_codegen_target_ir_code_constructor_class *klass) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(asmcmp_ctx != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp context"));
    REQUIRE(klass != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code constructor class"));

    struct constructor_state state = {
        .mem = mem,
        .code = code,
        .asmcmp_ctx = asmcmp_ctx,
        .klass = klass,
        .instr_linear_index = 0,
        .asmcmp_instrs = NULL
    };
    REQUIRE_OK(kefir_hashtree_init(&state.blocks, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&state.blocks, free_code_block_state, NULL));
    REQUIRE_OK(kefir_hashtable_init(&state.block_head_instr, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&state.label_blocks, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_init(&state.control_flow, code));

    kefir_result_t res = KEFIR_OK;
    state.asmcmp_instrs = KEFIR_MALLOC(mem, sizeof(struct asmcmp_instr_state) * asmcmp_ctx->code_length);
    REQUIRE_CHAIN_SET(&res, state.asmcmp_instrs != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate asmcmp instruction state"));
    state.asmcmp_vregs = KEFIR_MALLOC(mem, sizeof(struct asmcmp_vreg_state) * asmcmp_ctx->virtual_register_length);
    REQUIRE_CHAIN_SET(&res, state.asmcmp_vregs != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate asmcmp virtual register state"));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, state.asmcmp_instrs);
        KEFIR_FREE(mem, state.asmcmp_vregs);
        return res;
    });

    for (kefir_size_t i = 0; i < asmcmp_ctx->virtual_register_length; i++) {
        res = kefir_hashtable_init(&state.asmcmp_vregs[i].block_lifetimes, &kefir_hashtable_uint_ops);
        REQUIRE_ELSE(res == KEFIR_OK, {
            for (kefir_size_t j = 0; j < i; j++) {
                kefir_hashtable_free(mem, &state.asmcmp_vregs[j].block_lifetimes);
            }
            return res;
        });

        state.asmcmp_vregs[i].dominator.block_ref = KEFIR_ID_NONE;
    }

    REQUIRE_CHAIN(&res, code_construct(&state));
    for (kefir_size_t i = 0; i < asmcmp_ctx->virtual_register_length; i++) {
        REQUIRE_CHAIN(&res, kefir_hashtable_free(mem, &state.asmcmp_vregs[i].block_lifetimes));
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
            KEFIR_FREE(mem, state.asmcmp_instrs);
            KEFIR_FREE(mem, state.asmcmp_vregs);
            kefir_hashtree_free(mem, &state.blocks);
            kefir_hashtable_free(mem, &state.block_head_instr);
            kefir_hashtable_free(mem, &state.label_blocks);
            return res;
        }); 
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
        KEFIR_FREE(mem, state.asmcmp_instrs);
        KEFIR_FREE(mem, state.asmcmp_vregs);
        kefir_hashtree_free(mem, &state.blocks);
        kefir_hashtable_free(mem, &state.block_head_instr);
        kefir_hashtable_free(mem, &state.label_blocks);
        return res;
    });
    res = kefir_codegen_target_ir_control_flow_free(mem, &state.control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, state.asmcmp_instrs);
        KEFIR_FREE(mem, state.asmcmp_vregs);
        kefir_hashtree_free(mem, &state.blocks);
        kefir_hashtable_free(mem, &state.block_head_instr);
        kefir_hashtable_free(mem, &state.label_blocks);
        return res;
    });
    res = kefir_hashtree_free(mem, &state.blocks);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, state.asmcmp_instrs);
        KEFIR_FREE(mem, state.asmcmp_vregs);
        kefir_hashtable_free(mem, &state.block_head_instr);
        kefir_hashtable_free(mem, &state.label_blocks);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.block_head_instr);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, state.asmcmp_instrs);
        KEFIR_FREE(mem, state.asmcmp_vregs);
        kefir_hashtable_free(mem, &state.label_blocks);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.label_blocks);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, state.asmcmp_instrs);
        KEFIR_FREE(mem, state.asmcmp_vregs);
        return res;
    });
    KEFIR_FREE(mem, state.asmcmp_instrs);
    KEFIR_FREE(mem, state.asmcmp_vregs);
    return KEFIR_OK;
}
