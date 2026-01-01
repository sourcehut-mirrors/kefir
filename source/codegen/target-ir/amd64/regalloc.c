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

#include "kefir/codegen/target-ir/amd64/regalloc.h"
#include "kefir/target/abi/amd64/function.h"
#include "kefir/core/sort.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

struct state_payload {
    kefir_uint8_t *register_conflicts;
    struct kefir_hashtreeset hints;
    kefir_uint8_t *spill_slots;
    kefir_size_t spill_slots_length;
};

static kefir_result_t ensure_spill_area(struct kefir_mem *mem, struct state_payload *state_payload, kefir_size_t length) {
    if (state_payload->spill_slots_length < length) {
        kefir_uint8_t *new_spill_slots = KEFIR_REALLOC(mem, state_payload->spill_slots, sizeof(kefir_uint8_t) * length);
        REQUIRE(new_spill_slots != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR register allocator spill state"));
        memset(&new_spill_slots[state_payload->spill_slots_length], 0, sizeof(kefir_uint8_t) * (length - state_payload->spill_slots_length));
        state_payload->spill_slots = new_spill_slots;
        state_payload->spill_slots_length = length;
    }
    return KEFIR_OK;
}

static kefir_result_t allocate_spill_area(struct kefir_mem *mem, struct state_payload *state_payload, kefir_size_t length, kefir_size_t alignment, kefir_size_t *index_ptr) {
    kefir_size_t index = 0;
    for (; index + length < state_payload->spill_slots_length; index += alignment) {
        kefir_bool_t available = true;
        for (kefir_size_t i = 0; available && i < length; i++) {
            if (state_payload->spill_slots[index + i]) {
                available = false;
            }
        }

        if (available) {
            memset(&state_payload->spill_slots[index], 1, sizeof(kefir_uint8_t) * length);
            *index_ptr = index;
            return KEFIR_OK;
        }
    }

    index = (state_payload->spill_slots_length + alignment - 1) / alignment * alignment;
    REQUIRE_OK(ensure_spill_area(mem, state_payload, index + length));
    memset(&state_payload->spill_slots[index], 1, sizeof(kefir_uint8_t) * length);
    *index_ptr = index;
    return KEFIR_OK;
}

static kefir_bool_t is_spill_available(struct state_payload *state_payload, kefir_size_t index, kefir_size_t length) {
    REQUIRE(index + length <= state_payload->spill_slots_length, false);

    for (kefir_size_t i = index; i < index + length; i++) {
        REQUIRE(!state_payload->spill_slots[i], false);
    }

    return true;
}

static kefir_result_t do_allocate(struct kefir_mem *mem,
    const struct kefir_codegen_target_ir_value_type *value_type,
    const struct kefir_codegen_target_ir_stack_frame *stack_frame,
    void *state,
    kefir_bool_t try_optimistic,
    kefir_codegen_target_ir_regalloc_allocation_t *allocation_ptr,
    void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(value_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR value type"));
    ASSIGN_DECL_CAST(struct state_payload *, state_payload,
        state);
    REQUIRE(allocation_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR register allocation"));
    ASSIGN_DECL_CAST(const struct kefir_codegen_target_ir_amd64_regalloc_class *, klass,
        payload);
    UNUSED(klass);

    union kefir_codegen_target_ir_amd64_regalloc_entry regalloc_entry = {0};
    switch (value_type->kind) {
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_UNSPECIFIED:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_EXTERNAL_MEMORY:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLAGS:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_INDIRECT:
        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE:
            *allocation_ptr = regalloc_entry.allocation;
            break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE: {
            if (value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
                kefir_asm_amd64_xasmgen_register_t reg = value_type->constraint.physical_register;
                REQUIRE(!state_payload->register_conflicts[reg],
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Conflict in target IR amd64 register allocation constraints"));
                regalloc_entry.reg.type = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP;
                regalloc_entry.reg.value = reg;
                *allocation_ptr = regalloc_entry.allocation;
                if (stack_frame != NULL) {
                    REQUIRE_OK(stack_frame->use_register(mem, reg, stack_frame->payload));
                }
                return KEFIR_OK;
            }

            kefir_result_t res;
            struct kefir_hashtreeset_iterator hint_iter;
            for (res = kefir_hashtreeset_iter(&state_payload->hints, &hint_iter);
                res == KEFIR_OK;
                res = kefir_hashtreeset_next(&hint_iter)) {
                union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                    .allocation = hint_iter.entry
                };

                if (entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP &&
                    !state_payload->register_conflicts[entry.reg.value]) {
                    regalloc_entry.reg.type = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP;
                    regalloc_entry.reg.value = entry.reg.value;
                    *allocation_ptr = regalloc_entry.allocation;
                    if (stack_frame != NULL) {
                        REQUIRE_OK(stack_frame->use_register(mem, entry.reg.value, stack_frame->payload));
                    }
                    return KEFIR_OK;
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }

            for (kefir_size_t i = 0; i < klass->num_of_gp_registers; i++) {
                kefir_asm_amd64_xasmgen_register_t candidate_reg = klass->gp_registers[i];
                if (!state_payload->register_conflicts[candidate_reg]) {
                    regalloc_entry.reg.type = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP;
                    regalloc_entry.reg.value = candidate_reg;
                    *allocation_ptr = regalloc_entry.allocation;
                    if (stack_frame != NULL) {
                        REQUIRE_OK(stack_frame->use_register(mem, candidate_reg, stack_frame->payload));
                    }
                    return KEFIR_OK;
                }
            }

            REQUIRE(!try_optimistic, KEFIR_SET_ERROR(KEFIR_OUT_OF_SPACE, "Unable to optimistically allocate general purpose register for target IR value"));
            for (res = kefir_hashtreeset_iter(&state_payload->hints, &hint_iter);
                res == KEFIR_OK;
                res = kefir_hashtreeset_next(&hint_iter)) {
                union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                    .allocation = hint_iter.entry
                };

                if (entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL &&
                    entry.spill_area.length == 1 &&
                    is_spill_available(state_payload, entry.spill_area.index, entry.spill_area.length)) {
                    regalloc_entry.spill_area.type = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL;
                    regalloc_entry.spill_area.index = entry.spill_area.index;
                    regalloc_entry.spill_area.length = 1;
                    *allocation_ptr = regalloc_entry.allocation;
                    if (stack_frame != NULL) {
                        REQUIRE_OK(stack_frame->use_spill_space(mem, regalloc_entry.spill_area.index, regalloc_entry.spill_area.length, stack_frame->payload));
                    }
                    return KEFIR_OK;
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }

            kefir_size_t spill_index = 0;
            REQUIRE_OK(allocate_spill_area(mem, state_payload, 1, 1, &spill_index));
            regalloc_entry.spill_area.type = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL;
            regalloc_entry.spill_area.index = spill_index;
            regalloc_entry.spill_area.length = 1;
            *allocation_ptr = regalloc_entry.allocation;
            if (stack_frame != NULL) {
                REQUIRE_OK(stack_frame->use_spill_space(mem, regalloc_entry.spill_area.index, regalloc_entry.spill_area.length, stack_frame->payload));
            }
        } break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT: {
            if (value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
                kefir_asm_amd64_xasmgen_register_t reg = value_type->constraint.physical_register;
                REQUIRE(!state_payload->register_conflicts[reg],
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Conflict in target IR amd64 register allocation constraints"));
                regalloc_entry.reg.type = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE;
                regalloc_entry.reg.value = reg;
                *allocation_ptr = regalloc_entry.allocation;
                if (stack_frame != NULL) {
                    REQUIRE_OK(stack_frame->use_register(mem, reg, stack_frame->payload));
                }
                return KEFIR_OK;
            }
            
            kefir_result_t res;
            struct kefir_hashtreeset_iterator hint_iter;
            for (res = kefir_hashtreeset_iter(&state_payload->hints, &hint_iter);
                res == KEFIR_OK;
                res = kefir_hashtreeset_next(&hint_iter)) {
                union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                    .allocation = hint_iter.entry
                };

                if (entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE &&
                    !state_payload->register_conflicts[entry.reg.value]) {
                    regalloc_entry.reg.type = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE;
                    regalloc_entry.reg.value = entry.reg.value;
                    *allocation_ptr = regalloc_entry.allocation;
                    if (stack_frame != NULL) {
                        REQUIRE_OK(stack_frame->use_register(mem, entry.reg.value, stack_frame->payload));
                    }
                    return KEFIR_OK;
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }

            for (kefir_size_t i = 0; i < klass->num_of_sse_registers; i++) {
                kefir_asm_amd64_xasmgen_register_t candidate_reg = klass->sse_registers[i];
                if (!state_payload->register_conflicts[candidate_reg]) {
                    regalloc_entry.reg.type = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE;
                    regalloc_entry.reg.value = candidate_reg;
                    *allocation_ptr = regalloc_entry.allocation;
                    if (stack_frame != NULL) {
                        REQUIRE_OK(stack_frame->use_register(mem, candidate_reg, stack_frame->payload));
                    }
                    return KEFIR_OK;
                }
            }
            REQUIRE(!try_optimistic, KEFIR_SET_ERROR(KEFIR_OUT_OF_SPACE, "Unable to optimistically allocate floating point register for target IR value"));
            for (res = kefir_hashtreeset_iter(&state_payload->hints, &hint_iter);
                res == KEFIR_OK;
                res = kefir_hashtreeset_next(&hint_iter)) {
                union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                    .allocation = hint_iter.entry
                };

                if (entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL &&
                    entry.spill_area.length == 2 &&
                    entry.spill_area.index % 2 == 0 &&
                    is_spill_available(state_payload, entry.spill_area.index, entry.spill_area.length)) {
                    regalloc_entry.spill_area.type = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL;
                    regalloc_entry.spill_area.index = entry.spill_area.index;
                    regalloc_entry.spill_area.length = 2;
                    *allocation_ptr = regalloc_entry.allocation;
                    if (stack_frame != NULL) {
                        REQUIRE_OK(stack_frame->use_spill_space(mem, regalloc_entry.spill_area.index, regalloc_entry.spill_area.length, stack_frame->payload));
                    }
                    return KEFIR_OK;
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }

            kefir_size_t spill_index = 0;
            REQUIRE_OK(allocate_spill_area(mem, state_payload, 2, 2, &spill_index));
            regalloc_entry.spill_area.type = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL;
            regalloc_entry.spill_area.index = spill_index;
            regalloc_entry.spill_area.length = 2;
            *allocation_ptr = regalloc_entry.allocation;
            if (stack_frame != NULL) {
                REQUIRE_OK(stack_frame->use_spill_space(mem, regalloc_entry.spill_area.index, regalloc_entry.spill_area.length, stack_frame->payload));
            }
        } break;

        case KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE: {
            kefir_result_t res;
            struct kefir_hashtreeset_iterator hint_iter;
            for (res = kefir_hashtreeset_iter(&state_payload->hints, &hint_iter);
                res == KEFIR_OK;
                res = kefir_hashtreeset_next(&hint_iter)) {
                union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
                    .allocation = hint_iter.entry
                };

                if (entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL &&
                    entry.spill_area.length == value_type->parameters.spill_space_allocation.length &&
                    entry.spill_area.index % value_type->parameters.spill_space_allocation.alignment == 0 &&
                    is_spill_available(state_payload, entry.spill_area.index, entry.spill_area.length)) {
                    
                    regalloc_entry.spill_area.type = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL;
                    regalloc_entry.spill_area.index = entry.spill_area.index;
                    regalloc_entry.spill_area.length = entry.spill_area.length;
                    *allocation_ptr = regalloc_entry.allocation;
                    if (stack_frame != NULL) {
                        REQUIRE_OK(stack_frame->use_spill_space(mem, regalloc_entry.spill_area.index, regalloc_entry.spill_area.length, stack_frame->payload));
                    }
                    return KEFIR_OK;
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }

            kefir_size_t spill_index = 0;
            REQUIRE_OK(allocate_spill_area(mem, state_payload, value_type->parameters.spill_space_allocation.length, value_type->parameters.spill_space_allocation.alignment, &spill_index));
            regalloc_entry.spill_area.type = KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL;
            regalloc_entry.spill_area.index = spill_index;
            regalloc_entry.spill_area.length = value_type->parameters.spill_space_allocation.length;
            *allocation_ptr = regalloc_entry.allocation;
            if (stack_frame != NULL) {
                REQUIRE_OK(stack_frame->use_spill_space(mem, regalloc_entry.spill_area.index, regalloc_entry.spill_area.length, stack_frame->payload));
            }
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t is_callee_preserved_reg(kefir_abi_amd64_variant_t variant, kefir_asm_amd64_xasmgen_register_t reg,
                                              kefir_bool_t *preserved) {
    const kefir_size_t num_of_regs = kefir_abi_amd64_num_of_callee_preserved_general_purpose_registers(variant);
    kefir_asm_amd64_xasmgen_register_t preserved_reg;
    for (kefir_size_t i = 0; i < num_of_regs; i++) {
        REQUIRE_OK(kefir_abi_amd64_get_callee_preserved_general_purpose_register(variant, i, &preserved_reg));
        if (preserved_reg == reg) {
            *preserved = true;
            return KEFIR_OK;
        }
    }

    *preserved = false;
    return KEFIR_OK;
}

static kefir_result_t abi_register_comparator(void *ptr1, void *ptr2, kefir_int_t *cmp, void *payload) {
    UNUSED(payload);
    REQUIRE(ptr1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid first register pointer"));
    REQUIRE(ptr2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid second register pointer"));
    REQUIRE(cmp != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to comparison result"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid comparator payload"));
    ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t *, reg1, ptr1);
    ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t *, reg2, ptr2);
    ASSIGN_DECL_CAST(const struct kefir_codegen_target_ir_amd64_regalloc_class *, klass,
        payload);

    kefir_bool_t preserved1, preserved2;
    REQUIRE_OK(is_callee_preserved_reg(klass->abi_variant, *reg1, &preserved1));
    REQUIRE_OK(is_callee_preserved_reg(klass->abi_variant, *reg2, &preserved2));

    if (!preserved1 && preserved2) {
        *cmp = -1;
    } else if (preserved1 && !preserved2) {
        *cmp = 1;
    } else if (*reg1 < *reg2) {
        *cmp = -1;
    } else if (*reg1 == *reg2) {
        *cmp = 0;
    } else {
        *cmp = 1;
    }
    return KEFIR_OK;
}

static kefir_result_t state_reset(struct kefir_mem *mem, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct state_payload *, state_payload,
        payload);
    REQUIRE(state_payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR register allocator state payload"));
    
    REQUIRE_OK(kefir_hashtreeset_clean(mem, &state_payload->hints));
    memset(state_payload->register_conflicts, 0, sizeof(kefir_uint8_t) * KEFIR_AMD64_XASMGEN_REGISTER_NUM_OF_ENTRIES);
    if (state_payload->spill_slots_length > 0) {
        memset(state_payload->spill_slots, 0, sizeof(kefir_uint8_t) * state_payload->spill_slots_length);
    }
    return KEFIR_OK;
}

static kefir_result_t state_add_conflict(struct kefir_mem *mem, kefir_codegen_target_ir_regalloc_allocation_t allocation, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct state_payload *, state_payload,
        payload);
    REQUIRE(state_payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR register allocator state payload"));
    
    union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
        .allocation = allocation
    };
    switch (entry.type) {
        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_NA:
            // Intentionally left blank
            break;

        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP:
        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE:
            state_payload->register_conflicts[entry.reg.value] = 1;
            break;

        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL: {
            REQUIRE_OK(ensure_spill_area(mem, state_payload, entry.spill_area.index + entry.spill_area.length));
            memset(&state_payload->spill_slots[entry.spill_area.index], 1, sizeof(kefir_uint8_t) * entry.spill_area.length);
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t state_free(struct kefir_mem *mem, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct state_payload *, state_payload,
        payload);
    REQUIRE(state_payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR register allocator state payload"));
    
    REQUIRE_OK(kefir_hashtreeset_free(mem, &state_payload->hints));
    KEFIR_FREE(mem, state_payload->register_conflicts);
    KEFIR_FREE(mem, state_payload->spill_slots);
    KEFIR_FREE(mem, state_payload);
    return KEFIR_OK;
}

static kefir_result_t add_allocation_hint(struct kefir_mem *mem, kefir_codegen_target_ir_regalloc_allocation_t allocation, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct state_payload *, state_payload,
        payload);
    REQUIRE(state_payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR register allocator state payload"));

    REQUIRE_OK(kefir_hashtreeset_add(mem, &state_payload->hints, (kefir_hashtreeset_entry_t) allocation));
    return KEFIR_OK;
}

static kefir_result_t new_state(struct kefir_mem *mem, struct kefir_codegen_target_ir_regalloc_state *state, void *payload) {
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR register allocator state"));

    struct state_payload *state_payload = KEFIR_MALLOC(mem, sizeof(struct state_payload));
    REQUIRE(state_payload != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR amd64 register allocator state"));
    state_payload->spill_slots = NULL;
    state_payload->spill_slots_length = 0;
    state_payload->register_conflicts = KEFIR_MALLOC(mem, sizeof(kefir_uint8_t) * KEFIR_AMD64_XASMGEN_REGISTER_NUM_OF_ENTRIES);
    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN_SET(&res, state_payload->register_conflicts != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR amd64 register allocator state"));
    REQUIRE_CHAIN(&res, kefir_hashtreeset_init(&state_payload->hints, &kefir_hashtree_uint_ops));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, state_payload->register_conflicts);
        KEFIR_FREE(mem, state_payload);
        return res;
    });
    memset(state_payload->register_conflicts, 0, sizeof(kefir_uint8_t) * KEFIR_AMD64_XASMGEN_REGISTER_NUM_OF_ENTRIES);

    state->reset = state_reset;
    state->add_conflict = state_add_conflict;
    state->add_allocation_hint = add_allocation_hint;
    state->free_state = state_free;
    state->payload = state_payload;
    return KEFIR_OK;
}

static kefir_result_t amd64_regalloc_is_evictable(kefir_codegen_target_ir_regalloc_allocation_t allocation, kefir_bool_t *evictable, void *payload) {
    UNUSED(payload);
    REQUIRE(evictable != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolea flag"));
    
    union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
        .allocation = allocation
    };
    *evictable = entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP ||
        entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE;
    return KEFIR_OK;
}

static kefir_result_t amd64_regalloc_register_allocation(kefir_codegen_target_ir_physical_register_t phreg, kefir_codegen_target_ir_regalloc_allocation_t *allocation_ptr, void *payload) {
    UNUSED(payload);
    REQUIRE(allocation_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR register allocation"));
    
    union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
        .reg = {
            .type = kefir_asm_amd64_xasmgen_register_is_floating_point(phreg)
                ? KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE
                :KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP,
            .value = phreg
        }
    };
    *allocation_ptr = entry.allocation;
    return KEFIR_OK;
}

static kefir_result_t amd64_regalloc_format_allocation(struct kefir_json_output *, kefir_codegen_target_ir_regalloc_allocation_t);

kefir_result_t format_allocation(struct kefir_json_output *json, kefir_codegen_target_ir_regalloc_allocation_t allocation, void *payload) {
    UNUSED(payload);
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid json output"));

    REQUIRE_OK(amd64_regalloc_format_allocation(json, allocation));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_regalloc_class_init(struct kefir_mem *mem, struct kefir_codegen_target_ir_amd64_regalloc_class *klass, kefir_abi_amd64_variant_t abi_variant) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(klass != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR amd64 register allocator class"));

    static const kefir_asm_amd64_xasmgen_register_t AMD64_GENERAL_PURPOSE_REGS[] = {
        KEFIR_AMD64_XASMGEN_REGISTER_RAX, KEFIR_AMD64_XASMGEN_REGISTER_RBX, KEFIR_AMD64_XASMGEN_REGISTER_RCX,
        KEFIR_AMD64_XASMGEN_REGISTER_RDX, KEFIR_AMD64_XASMGEN_REGISTER_RSI, KEFIR_AMD64_XASMGEN_REGISTER_RDI,
        KEFIR_AMD64_XASMGEN_REGISTER_R8,  KEFIR_AMD64_XASMGEN_REGISTER_R9,  KEFIR_AMD64_XASMGEN_REGISTER_R10,
        KEFIR_AMD64_XASMGEN_REGISTER_R11, KEFIR_AMD64_XASMGEN_REGISTER_R12, KEFIR_AMD64_XASMGEN_REGISTER_R13,
        KEFIR_AMD64_XASMGEN_REGISTER_R14, KEFIR_AMD64_XASMGEN_REGISTER_R15};

    static const kefir_size_t NUM_OF_AMD64_GENERAL_PURPOSE_REGS =
        sizeof(AMD64_GENERAL_PURPOSE_REGS) / sizeof(AMD64_GENERAL_PURPOSE_REGS[0]);

    static const kefir_asm_amd64_xasmgen_register_t AMD64_FLOATING_POINT_REGS[] = {
        KEFIR_AMD64_XASMGEN_REGISTER_XMM0,  KEFIR_AMD64_XASMGEN_REGISTER_XMM1,  KEFIR_AMD64_XASMGEN_REGISTER_XMM2,
        KEFIR_AMD64_XASMGEN_REGISTER_XMM3,  KEFIR_AMD64_XASMGEN_REGISTER_XMM4,  KEFIR_AMD64_XASMGEN_REGISTER_XMM5,
        KEFIR_AMD64_XASMGEN_REGISTER_XMM6,  KEFIR_AMD64_XASMGEN_REGISTER_XMM7,  KEFIR_AMD64_XASMGEN_REGISTER_XMM8,
        KEFIR_AMD64_XASMGEN_REGISTER_XMM9,  KEFIR_AMD64_XASMGEN_REGISTER_XMM10, KEFIR_AMD64_XASMGEN_REGISTER_XMM11,
        KEFIR_AMD64_XASMGEN_REGISTER_XMM12, KEFIR_AMD64_XASMGEN_REGISTER_XMM13, KEFIR_AMD64_XASMGEN_REGISTER_XMM14,
        KEFIR_AMD64_XASMGEN_REGISTER_XMM15};

    static const kefir_size_t NUM_OF_AMD64_FLOATING_POINT_REGS =
        sizeof(AMD64_FLOATING_POINT_REGS) / sizeof(AMD64_FLOATING_POINT_REGS[0]);

    _Static_assert(sizeof(AMD64_GENERAL_PURPOSE_REGS) / sizeof(AMD64_GENERAL_PURPOSE_REGS[0]) <= KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_CLASS_REGISTERS, "Mismatch betwee number of available amd64 registers");
    _Static_assert(sizeof(AMD64_FLOATING_POINT_REGS) / sizeof(AMD64_FLOATING_POINT_REGS[0]) <= KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_CLASS_REGISTERS, "Mismatch betwee number of available amd64 registers");

    klass->abi_variant = abi_variant;
    memcpy(klass->gp_registers, AMD64_GENERAL_PURPOSE_REGS, sizeof(kefir_asm_amd64_xasmgen_register_t)* NUM_OF_AMD64_GENERAL_PURPOSE_REGS);
    memcpy(klass->sse_registers, AMD64_FLOATING_POINT_REGS, sizeof(kefir_asm_amd64_xasmgen_register_t)* NUM_OF_AMD64_FLOATING_POINT_REGS);
    klass->num_of_gp_registers = NUM_OF_AMD64_GENERAL_PURPOSE_REGS;
    klass->num_of_sse_registers = NUM_OF_AMD64_FLOATING_POINT_REGS;

    REQUIRE_OK(kefir_mergesort(
        mem, klass->gp_registers, sizeof(kefir_asm_amd64_xasmgen_register_t),
        klass->num_of_gp_registers, abi_register_comparator, klass));
    REQUIRE_OK(kefir_mergesort(
        mem, klass->sse_registers, sizeof(kefir_asm_amd64_xasmgen_register_t),
        klass->num_of_sse_registers, abi_register_comparator, klass));

    static const struct kefir_codegen_target_ir_regalloc_transforms TRANSFORMS = {
        .hot_copy_locality = 4,
        .hot_copy_pass_max_regs = 64000
    };
    klass->klass.transforms = &TRANSFORMS;

    klass->split_profile.general_purpose_interference_threshold = klass->num_of_gp_registers + 6;
    klass->split_profile.floating_point_interference_threshold = klass->num_of_sse_registers + 8;
    klass->split_profile.max_splits_per_use_pct = 15;
    klass->split_profile.max_splits_baseline = 1;
    klass->split_profile.max_blocks = 8192;
    klass->split_profile.max_branching = 32;

    klass->klass.do_allocate = do_allocate;
    klass->klass.new_state = new_state;
    klass->klass.is_evictable = amd64_regalloc_is_evictable;
    klass->klass.register_allocation = amd64_regalloc_register_allocation;
    klass->klass.format_allocation = format_allocation;
    klass->klass.split_profile = &klass->split_profile;
    klass->klass.payload = klass;
    return KEFIR_OK;
}

static kefir_result_t amd64_regalloc_format_allocation(struct kefir_json_output *json, kefir_codegen_target_ir_regalloc_allocation_t allocation) {
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid json output"));

    union kefir_codegen_target_ir_amd64_regalloc_entry entry = {
        .allocation = allocation
    };
    switch (entry.type) {
        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_NA:
            REQUIRE_OK(kefir_json_output_null(json));
            break;

        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP:
        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE: {
            const char *symbolic_name = kefir_asm_amd64_xasmgen_register_symbolic_name(entry.reg.value);
            REQUIRE(symbolic_name != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve amd64 register name"));

            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "register"));
            REQUIRE_OK(kefir_json_output_object_key(json, "register"));
            REQUIRE_OK(kefir_json_output_string(json, symbolic_name));
            REQUIRE_OK(kefir_json_output_object_end(json));
        } break;

        case KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL: {
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            REQUIRE_OK(kefir_json_output_string(json, "spill_area"));
            REQUIRE_OK(kefir_json_output_object_key(json, "index"));
            REQUIRE_OK(kefir_json_output_uinteger(json, entry.spill_area.index));
            REQUIRE_OK(kefir_json_output_object_key(json, "length"));
            REQUIRE_OK(kefir_json_output_uinteger(json, entry.spill_area.length));
            REQUIRE_OK(kefir_json_output_object_end(json));
        } break;
    }
    return KEFIR_OK;
}
