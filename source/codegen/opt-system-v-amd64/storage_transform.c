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

#include "kefir/codegen/opt-system-v-amd64/storage_transform.h"
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_CODE_INTERNAL
#include "kefir/codegen/opt-system-v-amd64/code.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include <string.h>

typedef struct kefir_codegen_opt_amd64_sysv_storage_transform_entry {
    struct kefir_codegen_opt_amd64_sysv_storage_transform_location source;
    struct kefir_codegen_opt_amd64_sysv_storage_transform_location destination;

    struct {
        kefir_size_t stack_temporary_index;
    } mapping;
} kefir_codegen_opt_amd64_sysv_storage_transform_entry_t;

static kefir_hashtree_hash_t transform_location_hash(kefir_hashtree_key_t key, void *data) {
    UNUSED(data);
    ASSIGN_DECL_CAST(const struct kefir_codegen_opt_amd64_sysv_storage_transform_location *, loc, key);
    REQUIRE(loc != NULL, 0);

    switch (loc->type) {
        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_REGISTER:
            return (kefir_uint64_t) loc->reg;

        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_MEMORY:
            return ((((kefir_uint64_t) loc->memory.base_reg) + 1) << 32) | ((kefir_uint32_t) loc->memory.offset);
    }
    return 0;
}

static bool transform_location_compare(kefir_hashtree_key_t key1, kefir_hashtree_key_t key2, void *data) {
    UNUSED(data);
    ASSIGN_DECL_CAST(const struct kefir_codegen_opt_amd64_sysv_storage_transform_location *, loc1, key1);
    ASSIGN_DECL_CAST(const struct kefir_codegen_opt_amd64_sysv_storage_transform_location *, loc2, key2);
    if (loc1 == NULL && loc2 == NULL) {
        return true;
    } else if (loc1 == NULL || loc2 == NULL) {
        return false;
    }

    switch (loc1->type) {
        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_REGISTER:
            return loc2->type == KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_REGISTER && loc1->reg == loc2->reg;

        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_MEMORY:
            return loc2->type == KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_MEMORY &&
                   loc1->memory.base_reg == loc2->memory.base_reg && loc1->memory.offset == loc2->memory.offset;
    }
    return false;
}

const struct kefir_hashtree_ops kefir_hashtree_transform_location_ops = {
    .hash = transform_location_hash, .compare_keys = transform_location_compare, .data = NULL};

static kefir_result_t free_transform_entry(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                           kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_opt_amd64_sysv_storage_transform_entry *, entry, value);
    REQUIRE(entry != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage transform location"));

    memset(entry, 0, sizeof(struct kefir_codegen_opt_amd64_sysv_storage_transform_entry));
    KEFIR_FREE(mem, entry);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_transform_init(
    struct kefir_codegen_opt_amd64_sysv_storage_transform *transform) {
    REQUIRE(transform != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer codegen storage transform"));

    REQUIRE_OK(kefir_hashtree_init(&transform->map, &kefir_hashtree_transform_location_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&transform->map, free_transform_entry, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_transform_free(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64_sysv_storage_transform *transform) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(transform != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage transform"));

    REQUIRE_OK(kefir_hashtree_free(mem, &transform->map));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_transform_location_from_reg_allocation(
    struct kefir_codegen_opt_amd64_sysv_storage_transform_location *location,
    const struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation) {
    REQUIRE(location != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to optimizer codegen storage transform location"));
    REQUIRE(stack_frame_map != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer codegen stack frame map"));
    REQUIRE(reg_allocation != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                                    "Expected valid pointer to optimizer codegen register allocation"));

    switch (reg_allocation->result.type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unexpected register allocation type");

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER:
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER:
            location->type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_REGISTER;
            location->reg = reg_allocation->result.reg;
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
            location->type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_MEMORY;
            location->memory.base_reg = KEFIR_AMD64_XASMGEN_REGISTER_RBP;
            location->memory.offset =
                stack_frame_map->offset.spill_area + reg_allocation->result.spill.index * KEFIR_AMD64_SYSV_ABI_QWORD;
            break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT:
            location->type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_MEMORY;
            location->memory.base_reg = reg_allocation->result.indirect.base_register;
            location->memory.offset = reg_allocation->result.indirect.offset;
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_transform_insert(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64_sysv_storage_transform *transform,
    const struct kefir_codegen_opt_amd64_sysv_storage_transform_location *destination,
    const struct kefir_codegen_opt_amd64_sysv_storage_transform_location *source) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(transform != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage transform"));
    REQUIRE(destination != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid optimizer codegen storage transform destination location"));
    REQUIRE(source != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                            "Expected valid optimizer codegen storage transform source location"));

    struct kefir_codegen_opt_amd64_sysv_storage_transform_entry *entry =
        KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_opt_amd64_sysv_storage_transform_entry));
    REQUIRE(entry != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer codegen storage transform entry"));
    entry->source = *source;
    entry->destination = *destination;

    kefir_result_t res = kefir_hashtree_insert(mem, &transform->map, (kefir_hashtree_key_t) &entry->destination,
                                               (kefir_hashtree_value_t) entry);
    if (res == KEFIR_ALREADY_EXISTS) {
        res = KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Transform with the same destination already exists");
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, entry);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_transform_operations(
    const struct kefir_codegen_opt_amd64_sysv_storage_transform *transform, kefir_size_t *num_of_ops) {
    REQUIRE(transform != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage transform"));
    REQUIRE(num_of_ops != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to transformation counter"));

    *num_of_ops = 0;
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&transform->map, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(const struct kefir_codegen_opt_amd64_sysv_storage_transform_entry *, entry, node->value);

        if (transform_location_hash((kefir_hashtree_key_t) &entry->destination, NULL) !=
            transform_location_hash((kefir_hashtree_key_t) &entry->source, NULL)) {
            (*num_of_ops)++;
        }
    }
    return KEFIR_OK;
}

static const struct kefir_asm_amd64_xasmgen_operand *location_operand(
    struct kefir_asm_amd64_xasmgen_operand *operand,
    const struct kefir_codegen_opt_amd64_sysv_storage_transform_location *location, kefir_size_t num_of_temporaries) {
    if (location->type == KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_REGISTER) {
        return kefir_asm_amd64_xasmgen_operand_reg(location->reg);
    } else if (location->memory.base_reg != KEFIR_AMD64_XASMGEN_REGISTER_RSP) {
        return kefir_asm_amd64_xasmgen_operand_indirect(
            operand, kefir_asm_amd64_xasmgen_operand_reg(location->memory.base_reg), location->memory.offset);
    } else {
        return kefir_asm_amd64_xasmgen_operand_indirect(
            operand, kefir_asm_amd64_xasmgen_operand_reg(location->memory.base_reg),
            location->memory.offset + num_of_temporaries * KEFIR_AMD64_SYSV_ABI_QWORD);
    }
}

static kefir_result_t push_temporary(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                     struct kefir_opt_sysv_amd64_function *codegen_func,
                                     const struct kefir_codegen_opt_amd64_sysv_storage_transform_location *location,
                                     kefir_size_t num_of_temporaries) {
    switch (location->type) {
        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_REGISTER:
            if (!kefir_asm_amd64_xasmgen_register_is_floating_point(location->reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen,
                                                          kefir_asm_amd64_xasmgen_operand_reg(location->reg)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                        KEFIR_AMD64_SYSV_ABI_QWORD)));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0),
                    kefir_asm_amd64_xasmgen_operand_reg(location->reg)));
            }
            break;

        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_MEMORY: {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                    KEFIR_AMD64_SYSV_ABI_QWORD)));

            struct kefir_codegen_opt_sysv_amd64_translate_temporary_register tmp_reg;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
                mem, codegen, NULL, codegen_func, &tmp_reg, NULL, NULL));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                                     kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg),
                                                     location_operand(&codegen->xasmgen_helpers.operands[0], location,
                                                                      num_of_temporaries + (tmp_reg.evicted ? 1 : 0))));

            if (tmp_reg.evicted) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                        KEFIR_AMD64_SYSV_ABI_QWORD),
                    kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0),
                    kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg)));
            }

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &tmp_reg));
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t load_temporary(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                     struct kefir_opt_sysv_amd64_function *codegen_func,
                                     const struct kefir_codegen_opt_amd64_sysv_storage_transform_location *location,
                                     kefir_size_t temporary_position, kefir_size_t total_tmp_positions) {
    switch (location->type) {
        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_REGISTER:
            if (!kefir_asm_amd64_xasmgen_register_is_floating_point(location->reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(location->reg),
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                        KEFIR_AMD64_SYSV_ABI_QWORD * (total_tmp_positions - temporary_position - 1))));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(location->reg),
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                        KEFIR_AMD64_SYSV_ABI_QWORD * (total_tmp_positions - temporary_position - 1))));
            }
            break;

        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_MEMORY: {
            struct kefir_codegen_opt_sysv_amd64_translate_temporary_register tmp_reg;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
                mem, codegen, NULL, codegen_func, &tmp_reg, NULL, NULL));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    KEFIR_AMD64_SYSV_ABI_QWORD *
                        (total_tmp_positions - temporary_position - (tmp_reg.evicted ? 0 : 1)))));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                                     location_operand(&codegen->xasmgen_helpers.operands[0], location,
                                                                      total_tmp_positions + (tmp_reg.evicted ? 1 : 0)),
                                                     kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg)));

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &tmp_reg));
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t load_from_location(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen, struct kefir_opt_sysv_amd64_function *codegen_func,
    const struct kefir_codegen_opt_amd64_sysv_storage_transform_location *destination,
    const struct kefir_codegen_opt_amd64_sysv_storage_transform_location *source, kefir_size_t num_of_temporaries) {
    switch (destination->type) {
        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_REGISTER:
            if (!kefir_asm_amd64_xasmgen_register_is_floating_point(destination->reg)) {
                if (source->type != KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_REGISTER ||
                    !kefir_asm_amd64_xasmgen_register_is_floating_point(source->reg)) {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(destination->reg),
                        location_operand(&codegen->xasmgen_helpers.operands[0], source, num_of_temporaries)));

                } else {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(destination->reg),
                        location_operand(&codegen->xasmgen_helpers.operands[0], source, num_of_temporaries)));
                }
            } else {
                if (source->type != KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_REGISTER ||
                    !kefir_asm_amd64_xasmgen_register_is_floating_point(source->reg)) {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(destination->reg),
                        location_operand(&codegen->xasmgen_helpers.operands[0], source, num_of_temporaries)));

                } else {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVDQU(
                        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(destination->reg),
                        location_operand(&codegen->xasmgen_helpers.operands[0], source, num_of_temporaries)));
                }
            }
            break;

        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_MEMORY:
            switch (source->type) {
                case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_REGISTER:
                    if (!kefir_asm_amd64_xasmgen_register_is_floating_point(source->reg)) {
                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &codegen->xasmgen,
                            location_operand(&codegen->xasmgen_helpers.operands[0], destination, num_of_temporaries),
                            kefir_asm_amd64_xasmgen_operand_reg(source->reg)));
                    } else {
                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                            &codegen->xasmgen,
                            location_operand(&codegen->xasmgen_helpers.operands[0], destination, num_of_temporaries),
                            kefir_asm_amd64_xasmgen_operand_reg(source->reg)));
                    }
                    break;

                case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_MEMORY: {
                    struct kefir_codegen_opt_sysv_amd64_translate_temporary_register tmp_reg;
                    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
                        mem, codegen, NULL, codegen_func, &tmp_reg, NULL, NULL));

                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg),
                        location_operand(&codegen->xasmgen_helpers.operands[0], source,
                                         num_of_temporaries + (tmp_reg.evicted ? 1 : 0))));

                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                        &codegen->xasmgen,
                        location_operand(&codegen->xasmgen_helpers.operands[0], destination,
                                         num_of_temporaries + (tmp_reg.evicted ? 1 : 0)),
                        kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg)));

                    REQUIRE_OK(
                        kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &tmp_reg));
                } break;
            }
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_transform_perform(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen, struct kefir_opt_sysv_amd64_function *codegen_func,
    const struct kefir_codegen_opt_amd64_sysv_storage_transform *transform) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen"));
    REQUIRE(codegen_func != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen function"));
    REQUIRE(transform != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage transform"));

    kefir_size_t num_of_stack_temporaries = 0;

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&transform->map, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(struct kefir_codegen_opt_amd64_sysv_storage_transform_entry *, entry, node->value);

        kefir_size_t hashcode_source = transform_location_hash((kefir_hashtree_key_t) &entry->source, NULL);
        kefir_size_t hashcode_destination = transform_location_hash((kefir_hashtree_key_t) &entry->destination, NULL);

        if (hashcode_source < hashcode_destination &&
            kefir_hashtree_has(&transform->map, (kefir_hashtree_key_t) &entry->source)) {
            entry->mapping.stack_temporary_index = num_of_stack_temporaries++;
            REQUIRE_OK(push_temporary(mem, codegen, codegen_func, &entry->source, num_of_stack_temporaries));
        }
    }

    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&transform->map, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(struct kefir_codegen_opt_amd64_sysv_storage_transform_entry *, entry, node->value);

        kefir_size_t hashcode_source = transform_location_hash((kefir_hashtree_key_t) &entry->source, NULL);
        kefir_size_t hashcode_destination = transform_location_hash((kefir_hashtree_key_t) &entry->destination, NULL);

        if (hashcode_source < hashcode_destination &&
            kefir_hashtree_has(&transform->map, (kefir_hashtree_key_t) &entry->source)) {
            REQUIRE_OK(load_temporary(mem, codegen, codegen_func, &entry->destination,
                                      entry->mapping.stack_temporary_index, num_of_stack_temporaries));
        } else if (!transform_location_compare((kefir_hashtree_key_t) &entry->source,
                                               (kefir_hashtree_key_t) &entry->destination, NULL)) {
            REQUIRE_OK(load_from_location(mem, codegen, codegen_func, &entry->destination, &entry->source,
                                          num_of_stack_temporaries));
        }
    }

    if (num_of_stack_temporaries > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                num_of_stack_temporaries * KEFIR_AMD64_SYSV_ABI_QWORD)));
    }
    return KEFIR_OK;
}
