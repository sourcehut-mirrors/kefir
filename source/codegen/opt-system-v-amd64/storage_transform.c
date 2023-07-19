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
    struct kefir_codegen_opt_amd64_sysv_storage_location source;
    struct kefir_codegen_opt_amd64_sysv_storage_location destination;

    struct {
        kefir_size_t source;
        kefir_bool_t source_preserved;
        kefir_size_t destination_base_reg;
        kefir_bool_t destination_base_reg_preserved;
    } temporaries;
} kefir_codegen_opt_amd64_sysv_storage_transform_entry_t;

static kefir_hashtree_hash_t transform_location_hash(const struct kefir_codegen_opt_amd64_sysv_storage_location *loc) {
    switch (loc->type) {
        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER:
            return (kefir_uint64_t) loc->reg;

        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY:
            return ((((kefir_uint64_t) loc->memory.base_reg) + 1) << 32) | ((kefir_uint32_t) loc->memory.offset);
    }
    return 0;
}

static kefir_hashtree_hash_t transform_location_ops_hash(kefir_hashtree_key_t key, void *data) {
    UNUSED(data);
    ASSIGN_DECL_CAST(const struct kefir_codegen_opt_amd64_sysv_storage_location *, loc, key);
    REQUIRE(loc != NULL, 0);

    return transform_location_hash(loc);
}

static kefir_int_t transform_location_compare(const struct kefir_codegen_opt_amd64_sysv_storage_location *loc1,
                                              const struct kefir_codegen_opt_amd64_sysv_storage_location *loc2) {
    if (loc1 == NULL && loc2 == NULL) {
        return 0;
    } else if (loc1 == NULL) {
        return -1;
    } else if (loc2 == NULL) {
        return 1;
    }

    if (loc1->type == KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER &&
        loc2->type == KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY) {
        return -1;
    } else if (loc1->type == KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY &&
               loc2->type == KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER) {
        return 1;
    }

    switch (loc1->type) {
        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER:
            if ((kefir_int64_t) loc1->reg < (kefir_int64_t) loc2->reg) {
                return -1;
            } else if (loc1->reg != loc2->reg) {
                return 1;
            }
            break;

        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY:
            if (loc1->memory.base_reg == loc2->memory.base_reg) {
                if (loc1->memory.offset < loc2->memory.offset) {
                    return -1;
                } else if (loc1->memory.offset > loc2->memory.offset) {
                    return 1;
                }
            } else if ((kefir_int64_t) loc1->memory.base_reg < (kefir_int64_t) loc2->memory.base_reg) {
                return -1;
            } else {
                return 1;
            }
            break;
    }

    return 0;
}

static kefir_int_t transform_location_ops_compare(kefir_hashtree_key_t key1, kefir_hashtree_key_t key2, void *data) {
    UNUSED(data);
    ASSIGN_DECL_CAST(const struct kefir_codegen_opt_amd64_sysv_storage_location *, loc1, key1);
    ASSIGN_DECL_CAST(const struct kefir_codegen_opt_amd64_sysv_storage_location *, loc2, key2);
    return transform_location_compare(loc1, loc2);
}

const struct kefir_hashtree_ops kefir_hashtree_transform_location_ops = {
    .hash = transform_location_ops_hash, .compare = transform_location_ops_compare, .data = NULL};

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
    REQUIRE_OK(kefir_hashtreeset_init(&transform->active_regs, &kefir_hashtree_uint_ops));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_transform_free(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64_sysv_storage_transform *transform) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(transform != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage transform"));

    REQUIRE_OK(kefir_hashtreeset_free(mem, &transform->active_regs));
    REQUIRE_OK(kefir_hashtree_free(mem, &transform->map));
    return KEFIR_OK;
}

static kefir_result_t update_active_regs(struct kefir_mem *mem,
                                         struct kefir_codegen_opt_amd64_sysv_storage_transform *transform,
                                         const struct kefir_codegen_opt_amd64_sysv_storage_location *location) {
    switch (location->type) {
        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER:
            REQUIRE_OK(kefir_hashtreeset_add(mem, &transform->active_regs, (kefir_hashtreeset_entry_t) location->reg));
            break;

        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY:
            REQUIRE_OK(kefir_hashtreeset_add(mem, &transform->active_regs,
                                             (kefir_hashtreeset_entry_t) location->memory.base_reg));
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_transform_insert(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64_sysv_storage_transform *transform,
    const struct kefir_codegen_opt_amd64_sysv_storage_location *destination,
    const struct kefir_codegen_opt_amd64_sysv_storage_location *source) {
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

    REQUIRE_OK(update_active_regs(mem, transform, destination));
    REQUIRE_OK(update_active_regs(mem, transform, source));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_transform_reset(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64_sysv_storage_transform *transform) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(transform != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage transform"));

    REQUIRE_OK(kefir_hashtreeset_clean(mem, &transform->active_regs));
    REQUIRE_OK(kefir_hashtree_clean(mem, &transform->map));
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

        if (transform_location_compare(&entry->destination, &entry->source) != 0) {
            (*num_of_ops)++;
        }
    }
    return KEFIR_OK;
}

static const struct kefir_asm_amd64_xasmgen_operand *location_operand(
    struct kefir_asm_amd64_xasmgen_operand *operand,
    const struct kefir_codegen_opt_amd64_sysv_storage_location *location, kefir_size_t num_of_temporaries) {
    if (location->type == KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER) {
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

static kefir_result_t obtain_temporary(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                       struct kefir_codegen_opt_sysv_amd64_storage *storage,
                                       const struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map,
                                       const struct kefir_codegen_opt_amd64_sysv_storage_transform *transform,
                                       struct kefir_codegen_opt_amd64_sysv_storage_handle *handle,
                                       kefir_size_t *num_of_stack_temporaries) {
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, storage, stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER, NULL, handle, NULL, NULL));

    kefir_bool_t self_evicted = false;
    if (!KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_HANDLE_IS_REG_EVICTED(handle) &&
        kefir_hashtreeset_has(&transform->active_regs, (kefir_hashtreeset_entry_t) handle->location.reg)) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen,
                                                  kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                                      &codegen->xasmgen_helpers.operands[0], &handle->location)));
        self_evicted = true;
    }
    if (KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_HANDLE_IS_REG_EVICTED(handle) || self_evicted) {
        (*num_of_stack_temporaries)++;
    }
    return KEFIR_OK;
}

static kefir_result_t free_temporary(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                     struct kefir_codegen_opt_sysv_amd64_storage *storage,
                                     const struct kefir_codegen_opt_amd64_sysv_storage_transform *transform,
                                     struct kefir_codegen_opt_amd64_sysv_storage_handle *handle,
                                     kefir_size_t *num_of_stack_temporaries) {
    kefir_bool_t self_evicted = false;
    if (!KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_HANDLE_IS_REG_EVICTED(handle) &&
        kefir_hashtreeset_has(&transform->active_regs, (kefir_hashtreeset_entry_t) handle->location.reg)) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&codegen->xasmgen,
                                                 kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                                     &codegen->xasmgen_helpers.operands[0], &handle->location)));
        self_evicted = true;
    }
    if (self_evicted || KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_HANDLE_IS_REG_EVICTED(handle)) {
        (*num_of_stack_temporaries)--;
    }
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, storage, handle));
    return KEFIR_OK;
}

static kefir_result_t push_temporary(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                     struct kefir_codegen_opt_sysv_amd64_storage *storage,
                                     const struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map,
                                     const struct kefir_codegen_opt_amd64_sysv_storage_transform *transform,
                                     const struct kefir_codegen_opt_amd64_sysv_storage_location *location,
                                     kefir_size_t num_of_temporaries) {
    switch (location->type) {
        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER:
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

        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY: {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                    KEFIR_AMD64_SYSV_ABI_QWORD)));

            struct kefir_codegen_opt_amd64_sysv_storage_handle tmp_handle;
            REQUIRE_OK(
                obtain_temporary(mem, codegen, storage, stack_frame_map, transform, &tmp_handle, &num_of_temporaries));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen,
                kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[0],
                                                                      &tmp_handle.location),
                location_operand(&codegen->xasmgen_helpers.operands[1], location, num_of_temporaries)));

            if (KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_HANDLE_IS_REG_EVICTED(&tmp_handle)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                        KEFIR_AMD64_SYSV_ABI_QWORD),
                    kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[1],
                                                                          &tmp_handle.location)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0),
                    kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[1],
                                                                          &tmp_handle.location)));
            }

            REQUIRE_OK(free_temporary(mem, codegen, storage, transform, &tmp_handle, &num_of_temporaries));
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t load_temporary(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                     struct kefir_codegen_opt_sysv_amd64_storage *storage,
                                     const struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map,
                                     const struct kefir_codegen_opt_amd64_sysv_storage_transform *transform,
                                     const struct kefir_codegen_opt_amd64_sysv_storage_transform_entry *entry,
                                     kefir_size_t num_of_stack_temporaries) {
    switch (entry->destination.type) {
        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER:
            if (!kefir_asm_amd64_xasmgen_register_is_floating_point(entry->destination.reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(entry->destination.reg),
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                        KEFIR_AMD64_SYSV_ABI_QWORD * (num_of_stack_temporaries - entry->temporaries.source - 1))));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(entry->destination.reg),
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                        KEFIR_AMD64_SYSV_ABI_QWORD * (num_of_stack_temporaries - entry->temporaries.source - 1))));
            }
            break;

        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY: {
            struct kefir_codegen_opt_amd64_sysv_storage_handle source_tmp_handle;
            REQUIRE_OK(obtain_temporary(mem, codegen, storage, stack_frame_map, transform, &source_tmp_handle,
                                        &num_of_stack_temporaries));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen,
                kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[0],
                                                                      &source_tmp_handle.location),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &codegen->xasmgen_helpers.operands[1],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    KEFIR_AMD64_SYSV_ABI_QWORD * (num_of_stack_temporaries - entry->temporaries.source - 1))));

            if (entry->temporaries.destination_base_reg_preserved) {
                struct kefir_codegen_opt_amd64_sysv_storage_handle destination_base_tmp_handle;
                REQUIRE_OK(obtain_temporary(mem, codegen, storage, stack_frame_map, transform,
                                            &destination_base_tmp_handle, &num_of_stack_temporaries));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen,
                    kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[0],
                                                                          &destination_base_tmp_handle.location),
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[1],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                        KEFIR_AMD64_SYSV_ABI_QWORD *
                            (num_of_stack_temporaries - entry->temporaries.destination_base_reg - 1))));

                struct kefir_codegen_opt_amd64_sysv_storage_location dest_location = {
                    .type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY,
                    .memory.base_reg = destination_base_tmp_handle.location.reg,
                    .memory.offset = entry->destination.memory.offset};

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen,
                    location_operand(&codegen->xasmgen_helpers.operands[0], &dest_location, num_of_stack_temporaries),
                    kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[1],
                                                                          &source_tmp_handle.location)));

                REQUIRE_OK(free_temporary(mem, codegen, storage, transform, &destination_base_tmp_handle,
                                          &num_of_stack_temporaries));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen,
                    location_operand(&codegen->xasmgen_helpers.operands[0], &entry->destination,
                                     num_of_stack_temporaries),
                    kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[1],
                                                                          &source_tmp_handle.location)));
            }

            REQUIRE_OK(free_temporary(mem, codegen, storage, transform, &source_tmp_handle, &num_of_stack_temporaries));
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t load_general_purpose_register(
    struct kefir_codegen_opt_amd64 *codegen, const struct kefir_codegen_opt_amd64_sysv_storage_transform_entry *entry,
    kefir_size_t num_of_stack_temporaries) {
    switch (entry->source.type) {
        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER:
            if (!kefir_asm_amd64_xasmgen_register_is_floating_point(entry->source.reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                                         kefir_asm_amd64_xasmgen_operand_reg(entry->destination.reg),
                                                         kefir_asm_amd64_xasmgen_operand_reg(entry->source.reg)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(&codegen->xasmgen,
                                                          kefir_asm_amd64_xasmgen_operand_reg(entry->destination.reg),
                                                          kefir_asm_amd64_xasmgen_operand_reg(entry->source.reg)));
            }
            break;

        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(entry->destination.reg),
                location_operand(&codegen->xasmgen_helpers.operands[0], &entry->source, num_of_stack_temporaries)));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t load_floating_point_register(
    struct kefir_codegen_opt_amd64 *codegen, const struct kefir_codegen_opt_amd64_sysv_storage_transform_entry *entry,
    kefir_size_t num_of_stack_temporaries) {
    switch (entry->source.type) {
        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER:
            if (!kefir_asm_amd64_xasmgen_register_is_floating_point(entry->source.reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(&codegen->xasmgen,
                                                          kefir_asm_amd64_xasmgen_operand_reg(entry->destination.reg),
                                                          kefir_asm_amd64_xasmgen_operand_reg(entry->source.reg)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVDQU(&codegen->xasmgen,
                                                            kefir_asm_amd64_xasmgen_operand_reg(entry->destination.reg),
                                                            kefir_asm_amd64_xasmgen_operand_reg(entry->source.reg)));
            }
            break;

        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(entry->destination.reg),
                location_operand(&codegen->xasmgen_helpers.operands[0], &entry->source, num_of_stack_temporaries)));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t load_from_location(struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
                                         struct kefir_codegen_opt_sysv_amd64_storage *storage,
                                         const struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map,
                                         const struct kefir_codegen_opt_amd64_sysv_storage_transform *transform,
                                         const struct kefir_codegen_opt_amd64_sysv_storage_transform_entry *entry,
                                         kefir_size_t num_of_stack_temporaries) {
    switch (entry->destination.type) {
        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER:
            if (!kefir_asm_amd64_xasmgen_register_is_floating_point(entry->destination.reg)) {
                REQUIRE_OK(load_general_purpose_register(codegen, entry, num_of_stack_temporaries));
            } else {
                REQUIRE_OK(load_floating_point_register(codegen, entry, num_of_stack_temporaries));
            }
            break;

        case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY:
            switch (entry->source.type) {
                case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER:
                    if (!kefir_asm_amd64_xasmgen_register_is_floating_point(entry->source.reg)) {
                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &codegen->xasmgen,
                            location_operand(&codegen->xasmgen_helpers.operands[0], &entry->destination,
                                             num_of_stack_temporaries),
                            kefir_asm_amd64_xasmgen_operand_reg(entry->source.reg)));
                    } else {
                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                            &codegen->xasmgen,
                            location_operand(&codegen->xasmgen_helpers.operands[0], &entry->destination,
                                             num_of_stack_temporaries),
                            kefir_asm_amd64_xasmgen_operand_reg(entry->source.reg)));
                    }
                    break;

                case KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY: {
                    struct kefir_codegen_opt_amd64_sysv_storage_handle source_tmp_handle;
                    REQUIRE_OK(obtain_temporary(mem, codegen, storage, stack_frame_map, transform, &source_tmp_handle,
                                                &num_of_stack_temporaries));

                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                        &codegen->xasmgen,
                        kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[0],
                                                                              &source_tmp_handle.location),
                        location_operand(&codegen->xasmgen_helpers.operands[1], &entry->source,
                                         num_of_stack_temporaries)));

                    if (entry->temporaries.destination_base_reg_preserved) {
                        struct kefir_codegen_opt_amd64_sysv_storage_handle destination_base_tmp_handle;
                        REQUIRE_OK(obtain_temporary(mem, codegen, storage, stack_frame_map, transform,
                                                    &destination_base_tmp_handle, &num_of_stack_temporaries));

                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &codegen->xasmgen,
                            kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                &codegen->xasmgen_helpers.operands[0], &destination_base_tmp_handle.location),
                            kefir_asm_amd64_xasmgen_operand_indirect(
                                &codegen->xasmgen_helpers.operands[1],
                                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                KEFIR_AMD64_SYSV_ABI_QWORD *
                                    (num_of_stack_temporaries - entry->temporaries.destination_base_reg - 1))));

                        struct kefir_codegen_opt_amd64_sysv_storage_location dest_location = {
                            .type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY,
                            .memory.base_reg = destination_base_tmp_handle.location.reg,
                            .memory.offset = entry->destination.memory.offset};

                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &codegen->xasmgen,
                            location_operand(&codegen->xasmgen_helpers.operands[0], &dest_location,
                                             num_of_stack_temporaries),
                            kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[1],
                                                                                  &source_tmp_handle.location)));

                        REQUIRE_OK(free_temporary(mem, codegen, storage, transform, &destination_base_tmp_handle,
                                                  &num_of_stack_temporaries));
                    } else {
                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &codegen->xasmgen,
                            location_operand(&codegen->xasmgen_helpers.operands[0], &entry->destination,
                                             num_of_stack_temporaries),
                            kefir_codegen_opt_amd64_sysv_storage_location_operand(&codegen->xasmgen_helpers.operands[1],
                                                                                  &source_tmp_handle.location)));
                    }

                    REQUIRE_OK(free_temporary(mem, codegen, storage, transform, &source_tmp_handle,
                                              &num_of_stack_temporaries));
                } break;
            }
            break;
    }
    return KEFIR_OK;
}

#define PRESERVE_SOURCE(_transform, _entry)                                                          \
    (transform_location_hash(&(_entry)->source) < transform_location_hash(&(_entry)->destination) && \
     kefir_hashtree_has(&(_transform)->map, (kefir_hashtree_key_t) & (_entry)->source))

#define REG_TO_LOCATION(_reg)                                                                                      \
    ((struct kefir_codegen_opt_amd64_sysv_storage_location){.type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER, \
                                                            .reg = (_reg)})

#define PRESERVE_MEMORY_BASE(_transform, _location)                                                                    \
    ((_location)->type == KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_MEMORY &&                                               \
     transform_location_hash(&REG_TO_LOCATION((_location)->memory.base_reg)) < transform_location_hash((_location)) && \
     kefir_hashtree_has(&transform->map, (kefir_hashtree_key_t) &REG_TO_LOCATION((_location)->memory.base_reg)))

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_transform_perform(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
    struct kefir_codegen_opt_sysv_amd64_storage *storage,
    const struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map,
    const struct kefir_codegen_opt_amd64_sysv_storage_transform *transform) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(stack_frame_map != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen stack frame map"));
    REQUIRE(transform != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage transform"));

    kefir_size_t num_of_stack_temporaries = 0;

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&transform->map, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(struct kefir_codegen_opt_amd64_sysv_storage_transform_entry *, entry, node->value);

        entry->temporaries.source_preserved = false;
        entry->temporaries.destination_base_reg_preserved = false;

        if (PRESERVE_SOURCE(transform, entry) || PRESERVE_MEMORY_BASE(transform, &entry->source)) {
            entry->temporaries.source = num_of_stack_temporaries++;
            entry->temporaries.source_preserved = true;
            REQUIRE_OK(push_temporary(mem, codegen, storage, stack_frame_map, transform, &entry->source,
                                      num_of_stack_temporaries));
        }

        if (PRESERVE_MEMORY_BASE(transform, &entry->destination)) {
            entry->temporaries.destination_base_reg = num_of_stack_temporaries++;
            entry->temporaries.destination_base_reg_preserved = true;
            REQUIRE_OK(push_temporary(mem, codegen, storage, stack_frame_map, transform,
                                      &REG_TO_LOCATION(entry->destination.memory.base_reg), num_of_stack_temporaries));
        }
    }

    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&transform->map, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(const struct kefir_codegen_opt_amd64_sysv_storage_transform_entry *, entry, node->value);

        if (entry->temporaries.source_preserved) {
            REQUIRE_OK(
                load_temporary(mem, codegen, storage, stack_frame_map, transform, entry, num_of_stack_temporaries));
        } else if (transform_location_ops_compare((kefir_hashtree_key_t) &entry->source,
                                                  (kefir_hashtree_key_t) &entry->destination, NULL) != 0) {
            REQUIRE_OK(
                load_from_location(mem, codegen, storage, stack_frame_map, transform, entry, num_of_stack_temporaries));
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
