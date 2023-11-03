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

#include "kefir/codegen/amd64/devirtualize.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/list.h"

struct devirtualize_state {
    struct kefir_asmcmp_amd64 *target;
    struct kefir_codegen_amd64_register_allocator *register_allocator;
    struct kefir_codegen_amd64_stack_frame *stack_frame;

    struct {
        struct kefir_hashtreeset physical_regs;
        struct kefir_hashtreeset virtual_regs;
        struct kefir_bitset spill_area;
    } alive;

    struct {
        kefir_bool_t active;
        kefir_asmcmp_stash_index_t idx;
        struct kefir_hashtreeset virtual_regs;
    } stash;

    struct kefir_hashtreeset current_instr_physical_regs;
    struct kefir_hashtree evicted_physical_regs;
};

static kefir_result_t remove_dead_virtual_regs(struct kefir_mem *mem, struct devirtualize_state *state,
                                               kefir_size_t linear_position) {
    REQUIRE_OK(kefir_hashtreeset_clean(mem, &state->current_instr_physical_regs));

    struct kefir_hashtreeset_iterator iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(&state->alive.virtual_regs, &iter); res == KEFIR_OK;) {

        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg_idx, iter.entry);
        const struct kefir_codegen_amd64_register_allocation *reg_alloc;
        REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(state->register_allocator, vreg_idx, &reg_alloc));

        if (reg_alloc->lifetime.end <= linear_position) {
            REQUIRE_OK(kefir_hashtreeset_delete(mem, &state->alive.virtual_regs, iter.entry));
            res = kefir_hashtreeset_iter(&state->alive.virtual_regs, &iter);
        } else {
            res = kefir_hashtreeset_next(&iter);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t update_live_virtual_regs(struct kefir_mem *mem, struct devirtualize_state *state,
                                               kefir_size_t linear_position, const struct kefir_asmcmp_value *value) {
    const struct kefir_codegen_amd64_register_allocation *reg_alloc;

    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER:
        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT:
        case KEFIR_ASMCMP_VALUE_TYPE_LABEL:
            // Intentionally left blank
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
            REQUIRE_OK(
                kefir_codegen_amd64_register_allocation_of(state->register_allocator, value->vreg.index, &reg_alloc));
            REQUIRE(reg_alloc->lifetime.begin <= linear_position && reg_alloc->lifetime.end >= linear_position,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 virtual register lifetime"));
            REQUIRE_OK(
                kefir_hashtreeset_add(mem, &state->alive.virtual_regs, (kefir_hashtreeset_entry_t) value->vreg.index));
            switch (reg_alloc->type) {
                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unallocated amd64 virtual register allocation");

                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER:
                    REQUIRE_OK(kefir_hashtreeset_add(mem, &state->current_instr_physical_regs,
                                                     (kefir_hashtreeset_entry_t) reg_alloc->direct_reg));
                    break;

                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SLOT:
                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SPACE:
                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                    REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(state->register_allocator,
                                                                          value->indirect.base.vreg, &reg_alloc));
                    REQUIRE(reg_alloc->lifetime.begin <= linear_position && reg_alloc->lifetime.end >= linear_position,
                            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 virtual register lifetime"));
                    REQUIRE_OK(kefir_hashtreeset_add(mem, &state->alive.virtual_regs,
                                                     (kefir_hashtreeset_entry_t) value->indirect.base.vreg));
                    switch (reg_alloc->type) {
                        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED:
                            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                                   "Unallocated amd64 virtual register allocation");

                        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER:
                            REQUIRE_OK(kefir_hashtreeset_add(mem, &state->current_instr_physical_regs,
                                                             (kefir_hashtreeset_entry_t) reg_alloc->direct_reg));
                            break;

                        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SLOT:
                        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SPACE:
                        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER:
                            // Intentionally left blank
                            break;
                    }
                    break;

                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_LABEL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_LOCAL_VAR_BASIS:
                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_TEMPORARY_AREA_BASIS:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_STASH_INDEX: {
            kefir_asmcmp_virtual_register_index_t vreg;
            REQUIRE_OK(kefir_asmcmp_register_stash_vreg(&state->target->context, value->stash_idx, &vreg));
            REQUIRE_OK(kefir_hashtreeset_add(mem, &state->alive.virtual_regs, (kefir_hashtreeset_entry_t) vreg));
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t rebuild_alive_physical_regs(struct kefir_mem *mem, struct devirtualize_state *state) {
    REQUIRE_OK(kefir_hashtreeset_clean(mem, &state->alive.physical_regs));
    kefir_size_t spill_area_length;
    REQUIRE_OK(kefir_bitset_length(&state->alive.spill_area, &spill_area_length));
    REQUIRE_OK(kefir_bitset_set_consecutive(&state->alive.spill_area, 0, spill_area_length, false));

    struct kefir_hashtreeset_iterator iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(&state->alive.virtual_regs, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg_idx, iter.entry);
        const struct kefir_codegen_amd64_register_allocation *reg_alloc;
        REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(state->register_allocator, vreg_idx, &reg_alloc));

        switch (reg_alloc->type) {
            case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unallocated amd64 virtual register allocation");

            case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER:
                REQUIRE_OK(kefir_hashtreeset_add(mem, &state->alive.physical_regs,
                                                 (kefir_hashtreeset_entry_t) reg_alloc->direct_reg));
                break;

            case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SLOT:
                REQUIRE_OK(kefir_bitset_set(&state->alive.spill_area, reg_alloc->spill_area_slot, true));
                break;

            case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SPACE:
                REQUIRE_OK(kefir_bitset_set_consecutive(&state->alive.spill_area, reg_alloc->spill_area_space.index,
                                                        reg_alloc->spill_area_space.length, true));
                break;

            case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER:
                // Intentionally left blank
                break;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t obtain_temporary_register(struct kefir_mem *mem, struct devirtualize_state *state,
                                                kefir_asmcmp_instruction_index_t position,
                                                kefir_asm_amd64_xasmgen_register_t *reg) {
    for (kefir_size_t i = 0; i < state->register_allocator->internal.num_of_gp_registers; i++) {
        const kefir_asm_amd64_xasmgen_register_t candidate =
            state->register_allocator->internal.gp_register_allocation_order[i];

        if (!kefir_hashtreeset_has(&state->current_instr_physical_regs, (kefir_hashtreeset_entry_t) candidate) &&
            !kefir_hashtreeset_has(&state->alive.physical_regs, (kefir_hashtreeset_entry_t) candidate) &&
            !kefir_hashtree_has(&state->evicted_physical_regs, (kefir_hashtree_key_t) candidate) &&
            kefir_hashtreeset_has(&state->register_allocator->used_registers, (kefir_hashtreeset_entry_t) candidate)) {
            REQUIRE_OK(kefir_hashtree_insert(mem, &state->evicted_physical_regs, (kefir_hashtree_key_t) candidate,
                                             (kefir_hashtree_value_t) (kefir_size_t) -1));
            *reg = candidate;
            return KEFIR_OK;
        }
    }
    for (kefir_size_t i = 0; i < state->register_allocator->internal.num_of_gp_registers; i++) {
        const kefir_asm_amd64_xasmgen_register_t candidate =
            state->register_allocator->internal.gp_register_allocation_order[i];

        if (!kefir_hashtreeset_has(&state->current_instr_physical_regs, (kefir_hashtreeset_entry_t) candidate) &&
            !kefir_hashtree_has(&state->evicted_physical_regs, (kefir_hashtree_key_t) candidate)) {

            kefir_size_t temp_spill_slot;
            kefir_result_t res = kefir_bitset_find(&state->alive.spill_area, false, 0, &temp_spill_slot);
            if (res == KEFIR_NOT_FOUND) {
                kefir_size_t num_of_slots;
                REQUIRE_OK(kefir_bitset_length(&state->alive.spill_area, &num_of_slots));
                REQUIRE_OK(kefir_bitset_resize(mem, &state->alive.spill_area, ++num_of_slots));
                REQUIRE_OK(kefir_codegen_amd64_stack_frame_ensure_spill_area(state->stack_frame, num_of_slots));
                REQUIRE_OK(kefir_bitset_find(&state->alive.spill_area, false, 0, &temp_spill_slot));
            } else {
                REQUIRE_OK(res);
            }
            REQUIRE_OK(kefir_bitset_set(&state->alive.spill_area, temp_spill_slot, true));

            REQUIRE_OK(kefir_hashtree_insert(mem, &state->evicted_physical_regs, (kefir_hashtree_key_t) candidate,
                                             (kefir_hashtree_value_t) temp_spill_slot));

            kefir_asmcmp_instruction_index_t new_position;
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, state->target, kefir_asmcmp_context_instr_prev(&state->target->context, position),
                &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(temp_spill_slot, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                &KEFIR_ASMCMP_MAKE_PHREG(candidate), &new_position));
            REQUIRE_OK(kefir_asmcmp_context_move_labels(mem, &state->target->context, new_position, position));
            *reg = candidate;
            return KEFIR_OK;
        }
    }

    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find a temporary register for eviction");
}

static kefir_result_t devirtualize_value(struct kefir_mem *mem, struct devirtualize_state *state,
                                         kefir_asmcmp_instruction_index_t position, struct kefir_asmcmp_value *value) {
    UNUSED(mem);
    const struct kefir_codegen_amd64_register_allocation *reg_alloc;
    const struct kefir_asmcmp_virtual_register *vreg;
    kefir_asm_amd64_xasmgen_register_t phreg;
    kefir_asmcmp_instruction_index_t new_position;

    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER:
        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT:
        case KEFIR_ASMCMP_VALUE_TYPE_LABEL:
        case KEFIR_ASMCMP_VALUE_TYPE_STASH_INDEX:
            // Intentionally left blank
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
            REQUIRE_OK(
                kefir_codegen_amd64_register_allocation_of(state->register_allocator, value->vreg.index, &reg_alloc));

            switch (reg_alloc->type) {
                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unallocated amd64 virtual register allocation");

                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER:
                    switch (value->vreg.variant) {
                        case KEFIR_ASMCMP_OPERAND_VARIANT_8BIT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(reg_alloc->direct_reg, &phreg));
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_16BIT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register16(reg_alloc->direct_reg, &phreg));
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_32BIT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(reg_alloc->direct_reg, &phreg));
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_64BIT:
                        case KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register64(reg_alloc->direct_reg, &phreg));
                            break;
                    }

                    *value = KEFIR_ASMCMP_MAKE_PHREG(phreg);
                    break;

                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SLOT:
                    *value = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc->spill_area_slot, 0, value->vreg.variant);
                    break;

                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SPACE:
                    REQUIRE_OK(obtain_temporary_register(mem, state, position, &phreg));
                    REQUIRE_OK(kefir_asmcmp_amd64_lea(
                        mem, state->target, kefir_asmcmp_context_instr_prev(&state->target->context, position),
                        &KEFIR_ASMCMP_MAKE_PHREG(phreg),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc->spill_area_space.index, 0,
                                                          KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                        &new_position));
                    REQUIRE_OK(kefir_asmcmp_context_move_labels(mem, &state->target->context, new_position, position));
                    *value = KEFIR_ASMCMP_MAKE_PHREG(phreg);
                    break;

                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER:
                    REQUIRE_OK(kefir_asmcmp_virtual_register_get(&state->target->context, value->vreg.index, &vreg));
                    *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(vreg->parameters.memory.base_reg,
                                                                 vreg->parameters.memory.offset,
                                                                 KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT);
                    break;
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_LABEL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_LOCAL_VAR_BASIS:
                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_TEMPORARY_AREA_BASIS:
                    // Intentionally left blank
                    break;

                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                    REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(state->register_allocator,
                                                                          value->indirect.base.vreg, &reg_alloc));

                    switch (reg_alloc->type) {
                        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED:
                            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                                   "Unallocated amd64 virtual register allocation");

                        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER:
                            *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(reg_alloc->direct_reg, value->indirect.offset,
                                                                         value->indirect.variant);
                            break;

                        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SLOT:
                            REQUIRE_OK(obtain_temporary_register(mem, state, position, &phreg));
                            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                                mem, state->target, kefir_asmcmp_context_instr_prev(&state->target->context, position),
                                &KEFIR_ASMCMP_MAKE_PHREG(phreg),
                                &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc->spill_area_slot, 0,
                                                                  KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                                &new_position));
                            REQUIRE_OK(
                                kefir_asmcmp_context_move_labels(mem, &state->target->context, new_position, position));
                            *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(phreg, value->indirect.offset,
                                                                         value->indirect.variant);
                            break;

                        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SPACE:
                            *value = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc->spill_area_space.index,
                                                                      value->indirect.offset, value->indirect.variant);
                            break;

                        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER:
                            REQUIRE_OK(
                                kefir_asmcmp_virtual_register_get(&state->target->context, value->vreg.index, &vreg));
                            REQUIRE_OK(obtain_temporary_register(mem, state, position, &phreg));
                            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                                mem, state->target, kefir_asmcmp_context_instr_prev(&state->target->context, position),
                                &KEFIR_ASMCMP_MAKE_PHREG(phreg),
                                &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(vreg->parameters.memory.base_reg,
                                                                     vreg->parameters.memory.offset,
                                                                     KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                                &new_position));
                            REQUIRE_OK(
                                kefir_asmcmp_context_move_labels(mem, &state->target->context, new_position, position));
                            *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(phreg, value->indirect.offset,
                                                                         value->indirect.variant);
                            break;
                    }
                    break;
            }
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t match_physical_reg_variant(kefir_asm_amd64_xasmgen_register_t *reg,
                                                 kefir_asmcmp_operand_variant_t variant) {
    switch (variant) {
        case KEFIR_ASMCMP_OPERAND_VARIANT_8BIT:
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(*reg, reg));
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_16BIT:
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register16(*reg, reg));
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_32BIT:
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(*reg, reg));
            break;

        case KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT:
        case KEFIR_ASMCMP_OPERAND_VARIANT_64BIT:
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register64(*reg, reg));
            break;
    }
    return KEFIR_OK;
}

#define DEVIRT_NONE (0ull)
#define DEVIRT_ARG1 (1ull)
#define DEVIRT_ARG2 (1ull << 1)
#define DEVIRT_ARG_READ (1ull << 2)
#define DEVIRT_ARG_WRITE (1ull << 3)
#define DEVIRT_ARG_ALWAYS_DIRECT (1ull << 4)

#define DEVIRT_HAS_FLAG(_flags, _flag) (((_flags) & (_flag)) != 0)

#define DEVIRT_FLAGS_FOR_None (DEVIRT_NONE)
#define DEVIRT_FLAGS_FOR_Virtual (DEVIRT_NONE)
#define DEVIRT_FLAGS_FOR_Jump (DEVIRT_NONE)
#define DEVIRT_FLAGS_FOR_Repeat (DEVIRT_NONE)
#define DEVIRT_FLAGS_FOR_RegR (DEVIRT_ARG1 | DEVIRT_ARG_ALWAYS_DIRECT | DEVIRT_ARG_READ)
#define DEVIRT_FLAGS_FOR_RegW (DEVIRT_ARG1 | DEVIRT_ARG_ALWAYS_DIRECT | DEVIRT_ARG_WRITE)
#define DEVIRT_FLAGS_FOR_RegMemW_RegMemR (DEVIRT_ARG1 | DEVIRT_ARG_WRITE)
#define DEVIRT_FLAGS_FOR_RegMemRW_RegMemR (DEVIRT_ARG1 | DEVIRT_ARG_READ | DEVIRT_ARG_WRITE)
#define DEVIRT_FLAGS_FOR_RegW_RegMemR (DEVIRT_ARG1 | DEVIRT_ARG_ALWAYS_DIRECT | DEVIRT_ARG_WRITE)
#define DEVIRT_FLAGS_FOR_RegRW_RegMemR (DEVIRT_ARG1 | DEVIRT_ARG_ALWAYS_DIRECT | DEVIRT_ARG_READ | DEVIRT_ARG_WRITE)
#define DEVIRT_FLAGS_FOR_RegW_Mem (DEVIRT_ARG1 | DEVIRT_ARG_ALWAYS_DIRECT | DEVIRT_ARG_WRITE)
#define DEVIRT_FLAGS_FOR_RegMemRW (DEVIRT_NONE)
#define DEVIRT_FLAGS_FOR_RegMemW (DEVIRT_NONE)
#define DEVIRT_FLAGS_FOR_RegMemRW_RegR (DEVIRT_ARG2 | DEVIRT_ARG_ALWAYS_DIRECT | DEVIRT_ARG_READ)
#define DEVIRT_FLAGS_FOR_RegMemR_RegR (DEVIRT_ARG2 | DEVIRT_ARG_ALWAYS_DIRECT | DEVIRT_ARG_READ)

static kefir_result_t devirtualize_instr(struct kefir_mem *mem, struct devirtualize_state *state,
                                         kefir_asmcmp_instruction_index_t instr_idx,
                                         struct kefir_asmcmp_instruction *instr,
                                         kefir_asmcmp_instruction_index_t *tail_instr_idx, kefir_uint64_t flags) {
    REQUIRE_OK(devirtualize_value(mem, state, instr_idx, &instr->args[0]));
    REQUIRE_OK(devirtualize_value(mem, state, instr_idx, &instr->args[1]));
    REQUIRE_OK(devirtualize_value(mem, state, instr_idx, &instr->args[2]));

    REQUIRE(DEVIRT_HAS_FLAG(flags, DEVIRT_ARG1) || DEVIRT_HAS_FLAG(flags, DEVIRT_ARG2), KEFIR_OK);
    REQUIRE(!DEVIRT_HAS_FLAG(flags, DEVIRT_ARG1) || !DEVIRT_HAS_FLAG(flags, DEVIRT_ARG2),
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Invalid combination of devirtualization flags"));

    const kefir_size_t primary_index = DEVIRT_HAS_FLAG(flags, DEVIRT_ARG1) ? 0 : 1;
    const kefir_size_t secondary_index = DEVIRT_HAS_FLAG(flags, DEVIRT_ARG1) ? 1 : 0;

    if (instr->args[primary_index].type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT &&
        (DEVIRT_HAS_FLAG(flags, DEVIRT_ARG_ALWAYS_DIRECT) ||
         instr->args[secondary_index].type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT)) {
        kefir_asm_amd64_xasmgen_register_t phreg;
        struct kefir_asmcmp_value original = instr->args[primary_index];
        REQUIRE_OK(obtain_temporary_register(mem, state, instr_idx, &phreg));
        REQUIRE_OK(match_physical_reg_variant(&phreg, original.indirect.variant));
        if (DEVIRT_HAS_FLAG(flags, DEVIRT_ARG_READ)) {
            kefir_asmcmp_instruction_index_t head_instr_idx;
            REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, state->target,
                                              kefir_asmcmp_context_instr_prev(&state->target->context, instr_idx),
                                              &KEFIR_ASMCMP_MAKE_PHREG(phreg), &original, &head_instr_idx));
            REQUIRE_OK(kefir_asmcmp_context_move_labels(mem, &state->target->context, head_instr_idx, instr_idx));
        }

        instr->args[primary_index] = KEFIR_ASMCMP_MAKE_PHREG(phreg);
        if (DEVIRT_HAS_FLAG(flags, DEVIRT_ARG_WRITE)) {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, state->target, instr_idx, &original, &KEFIR_ASMCMP_MAKE_PHREG(phreg),
                                              tail_instr_idx));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t activate_stash(struct kefir_mem *mem, struct devirtualize_state *state,
                                     kefir_asmcmp_instruction_index_t instr_idx,
                                     struct kefir_asmcmp_instruction *instr) {
    REQUIRE(!state->stash.active, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Another stash is already active"));

    kefir_asmcmp_stash_index_t stash_idx = instr->args[0].stash_idx;
    kefir_asmcmp_virtual_register_index_t stash_vreg;
    kefir_asmcmp_instruction_index_t liveness_idx;
    REQUIRE_OK(kefir_asmcmp_register_stash_vreg(&state->target->context, stash_idx, &stash_vreg));
    REQUIRE_OK(kefir_asmcmp_register_stash_liveness_index(&state->target->context, stash_idx, &liveness_idx));

    const struct kefir_codegen_amd64_register_allocation *stash_alloc;
    REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(state->register_allocator, stash_vreg, &stash_alloc));
    REQUIRE(stash_alloc->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SPACE,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected spill area space allocation for stash"));

    struct kefir_hashtreeset_iterator iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(&state->alive.virtual_regs, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg_idx, iter.entry);
        const struct kefir_codegen_amd64_register_allocation *reg_alloc;
        REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(state->register_allocator, vreg_idx, &reg_alloc));

        if (reg_alloc->type != KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER) {
            continue;
        }

        kefir_size_t spill_area_slot;
        res = kefir_asmcmp_register_stash_for(&state->target->context, stash_idx, reg_alloc->direct_reg,
                                              &spill_area_slot);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        if (liveness_idx != KEFIR_ASMCMP_INDEX_NONE &&
            !(reg_alloc->lifetime.begin <= liveness_idx && reg_alloc->lifetime.end > liveness_idx)) {
            continue;
        }
        REQUIRE(spill_area_slot < stash_alloc->spill_area_space.length,
                KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Stash spill area slot is out of backing storage space"));

        kefir_asmcmp_instruction_index_t new_position;
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, state->target, kefir_asmcmp_context_instr_prev(&state->target->context, instr_idx),
            &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(stash_alloc->spill_area_space.index + spill_area_slot, 0,
                                              KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
            &KEFIR_ASMCMP_MAKE_PHREG(reg_alloc->direct_reg), &new_position));
        REQUIRE_OK(kefir_asmcmp_context_move_labels(mem, &state->target->context, new_position, instr_idx));

        REQUIRE_OK(kefir_hashtreeset_add(mem, &state->stash.virtual_regs, (kefir_hashtreeset_entry_t) vreg_idx));
    }

    state->stash.active = true;
    state->stash.idx = stash_idx;

    instr->opcode = KEFIR_ASMCMP_AMD64_OPCODE(noop);
    return KEFIR_OK;
}

static kefir_result_t deactivate_stash(struct kefir_mem *mem, struct devirtualize_state *state,
                                       kefir_asmcmp_instruction_index_t instr_idx,
                                       struct kefir_asmcmp_instruction *instr) {
    kefir_asmcmp_stash_index_t stash_idx = instr->args[0].stash_idx;
    kefir_asmcmp_virtual_register_index_t stash_vreg;
    REQUIRE_OK(kefir_asmcmp_register_stash_vreg(&state->target->context, stash_idx, &stash_vreg));

    REQUIRE(state->stash.active && state->stash.idx == stash_idx,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Stash activation mismatch"));

    const struct kefir_codegen_amd64_register_allocation *stash_alloc;
    REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(state->register_allocator, stash_vreg, &stash_alloc));
    REQUIRE(stash_alloc->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SPACE,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected spill area space allocation for stash"));

    struct kefir_hashtreeset_iterator iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(&state->alive.virtual_regs, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg_idx, iter.entry);
        const struct kefir_codegen_amd64_register_allocation *reg_alloc;
        REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(state->register_allocator, vreg_idx, &reg_alloc));

        if (reg_alloc->type != KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER ||
            !kefir_hashtreeset_has(&state->stash.virtual_regs, (kefir_hashtreeset_entry_t) vreg_idx)) {
            continue;
        }

        kefir_size_t spill_area_slot;
        res = kefir_asmcmp_register_stash_for(&state->target->context, stash_idx, reg_alloc->direct_reg,
                                              &spill_area_slot);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        REQUIRE(spill_area_slot < stash_alloc->spill_area_space.length,
                KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Stash spill area slot is out of backing storage space"));

        kefir_asmcmp_instruction_index_t new_position;
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, state->target, kefir_asmcmp_context_instr_prev(&state->target->context, instr_idx),
            &KEFIR_ASMCMP_MAKE_PHREG(reg_alloc->direct_reg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(stash_alloc->spill_area_space.index + spill_area_slot, 0,
                                              KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
            &new_position));
        REQUIRE_OK(kefir_asmcmp_context_move_labels(mem, &state->target->context, new_position, instr_idx));
    }

    REQUIRE_OK(kefir_hashtreeset_clean(mem, &state->stash.virtual_regs));
    state->stash.active = false;

    instr->opcode = KEFIR_ASMCMP_AMD64_OPCODE(noop);
    return KEFIR_OK;
}

static kefir_result_t devirtualize_impl(struct kefir_mem *mem, struct devirtualize_state *state) {
    UNUSED(mem);
    UNUSED(state);

    kefir_size_t num_of_spill_slots;
    REQUIRE_OK(kefir_bitset_length(&state->register_allocator->internal.spill_area, &num_of_spill_slots));
    REQUIRE_OK(kefir_bitset_resize(mem, &state->alive.spill_area, num_of_spill_slots));

    for (kefir_asmcmp_instruction_index_t idx = kefir_asmcmp_context_instr_head(&state->target->context);
         idx != KEFIR_ASMCMP_INDEX_NONE; idx = kefir_asmcmp_context_instr_next(&state->target->context, idx)) {

        const struct kefir_asmcmp_instruction *instr;
        REQUIRE_OK(kefir_asmcmp_context_instr_at(&state->target->context, idx, &instr));

        kefir_size_t instr_linear_position;
        kefir_result_t res = kefir_codegen_amd64_register_allocator_linear_position_of(state->register_allocator, idx,
                                                                                       &instr_linear_position);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        } else {
            REQUIRE_OK(res);
        }

        REQUIRE_OK(remove_dead_virtual_regs(mem, state, instr_linear_position));
        REQUIRE_OK(update_live_virtual_regs(mem, state, instr_linear_position, &instr->args[0]));
        REQUIRE_OK(update_live_virtual_regs(mem, state, instr_linear_position, &instr->args[1]));
        REQUIRE_OK(update_live_virtual_regs(mem, state, instr_linear_position, &instr->args[2]));
        REQUIRE_OK(rebuild_alive_physical_regs(mem, state));

        struct kefir_asmcmp_instruction devirtualized_instr = *instr;

        kefir_asmcmp_instruction_index_t tail_idx = idx;
        switch (devirtualized_instr.opcode) {
#define CASE_IMPL_Virtual(_opcode, _argclass)
#define CASE_IMPL_Normal(_opcode, _argclass)                                                                     \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                                                                     \
        REQUIRE_OK(                                                                                              \
            devirtualize_instr(mem, state, idx, &devirtualized_instr, &tail_idx, DEVIRT_FLAGS_FOR_##_argclass)); \
        break;
#define CASE_IMPL_None(_opcode, _argclass) CASE_IMPL_Normal(_opcode, _argclass)
#define CASE_IMPL_Jump(_opcode, _argclass) CASE_IMPL_Normal(_opcode, _argclass)
#define CASE_IMPL_RegR(_opcode, _argclass) CASE_IMPL_Normal(_opcode, _argclass)
#define CASE_IMPL_RegW(_opcode, _argclass) CASE_IMPL_Normal(_opcode, _argclass)
#define CASE_IMPL_Repeat(_opcode, _argclass) CASE_IMPL_Normal(_opcode, _argclass)
#define CASE_IMPL_RegMemW_RegMemR(_opcode, _argclass) CASE_IMPL_Normal(_opcode, _argclass)
#define CASE_IMPL_RegW_RegMemR(_opcode, _argclass) CASE_IMPL_Normal(_opcode, _argclass)
#define CASE_IMPL_RegRW_RegMemR(_opcode, _argclass) CASE_IMPL_Normal(_opcode, _argclass)
#define CASE_IMPL_RegW_Mem(_opcode, _argclass) CASE_IMPL_Normal(_opcode, _argclass)
#define CASE_IMPL_RegMemRW_RegMemR(_opcode, _argclass) CASE_IMPL_Normal(_opcode, _argclass)
#define CASE_IMPL_RegMemRW(_opcode, _argclass) CASE_IMPL_Normal(_opcode, _argclass)
#define CASE_IMPL_RegMemW(_opcode, _argclass) CASE_IMPL_Normal(_opcode, _argclass)
#define CASE_IMPL_RegMemRW_RegR(_opcode, _argclass) CASE_IMPL_Normal(_opcode, _argclass)
#define CASE_IMPL_RegMemR_RegR(_opcode, _argclass) CASE_IMPL_Normal(_opcode, _argclass)
#define CASE_IMPL(_opcode, _xasmgen, _argclass) CASE_IMPL_##_argclass(_opcode, _argclass)
            KEFIR_ASMCMP_AMD64_OPCODES(CASE_IMPL, )
#undef CASE_IMPL

            case KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link):
                REQUIRE_OK(devirtualize_instr(mem, state, idx, &devirtualized_instr, &tail_idx,
                                              DEVIRT_FLAGS_FOR_RegMemW_RegMemR));
                break;

            case KEFIR_ASMCMP_AMD64_OPCODE(stash_activate):
                REQUIRE_OK(activate_stash(mem, state, idx, &devirtualized_instr));
                break;

            case KEFIR_ASMCMP_AMD64_OPCODE(stash_deactivate):
                REQUIRE_OK(deactivate_stash(mem, state, idx, &devirtualized_instr));
                break;

            case KEFIR_ASMCMP_AMD64_OPCODE(touch_virtual_register):
            case KEFIR_ASMCMP_AMD64_OPCODE(function_prologue):
            case KEFIR_ASMCMP_AMD64_OPCODE(function_epilogue):
            case KEFIR_ASMCMP_AMD64_OPCODE(noop):
                // Intentionally left blank
                break;
        }

        REQUIRE_OK(kefir_asmcmp_context_instr_replace(&state->target->context, idx, &devirtualized_instr));

        struct kefir_hashtree_node_iterator iter;
        for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&state->evicted_physical_regs, &iter);
             node != NULL; node = kefir_hashtree_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t, reg, node->key);
            ASSIGN_DECL_CAST(kefir_size_t, spill_index, node->value);
            if (spill_index != (kefir_size_t) -1) {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, state->target, tail_idx, &KEFIR_ASMCMP_MAKE_PHREG(reg),
                    &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(spill_index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));
                REQUIRE_OK(kefir_bitset_set(&state->alive.spill_area, spill_index, false));
            }
        }
        REQUIRE_OK(kefir_hashtree_clean(mem, &state->evicted_physical_regs));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_devirtualize(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                                struct kefir_codegen_amd64_register_allocator *register_allocator,
                                                struct kefir_codegen_amd64_stack_frame *stack_frame) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 asmcmp target"));
    REQUIRE(register_allocator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator"));
    REQUIRE(stack_frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 stack frame"));

    struct devirtualize_state state = {.target = target,
                                       .register_allocator = register_allocator,
                                       .stack_frame = stack_frame,
                                       .stash = {.active = false}};
    REQUIRE_OK(kefir_hashtreeset_init(&state.alive.physical_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&state.alive.virtual_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&state.current_instr_physical_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&state.stash.virtual_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&state.evicted_physical_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_bitset_init(&state.alive.spill_area));

    kefir_result_t res = devirtualize_impl(mem, &state);
    kefir_bitset_free(mem, &state.alive.spill_area);
    kefir_hashtree_free(mem, &state.evicted_physical_regs);
    kefir_hashtreeset_free(mem, &state.stash.virtual_regs);
    kefir_hashtreeset_free(mem, &state.alive.physical_regs);
    kefir_hashtreeset_free(mem, &state.alive.virtual_regs);
    kefir_hashtreeset_free(mem, &state.current_instr_physical_regs);
    return res;
}
