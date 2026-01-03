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

#include "kefir/codegen/target-ir/interference.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct interference_entry {
    kefir_codegen_target_ir_value_ref_t *interference;
    kefir_size_t length;
    kefir_size_t capacity;
};

static kefir_result_t free_interfence_entry(struct kefir_mem *mem, struct kefir_hashtable *table,
                                                          kefir_hashtable_key_t key, kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct interference_entry *, entry,
        value);
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Expected valid target IR interference entry"));

    KEFIR_FREE(mem, entry->interference);
    KEFIR_FREE(mem, entry);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_interference_init(struct kefir_codegen_target_ir_interference *interference) {
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR interference"));

    interference->interference = NULL;
    interference->length = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_interference_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_interference *interference) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference"));

    for (kefir_size_t i = 0; i < interference->length; i++) {
        REQUIRE_OK(kefir_hashtable_free(mem, &interference->interference[i].value_entries));
    }
    KEFIR_FREE(mem, interference->interference);
    interference->interference = NULL;
    interference->length = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_interference_reset(struct kefir_mem *mem, struct kefir_codegen_target_ir_interference *interference) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference"));

    for (kefir_size_t i = 0; i < interference->length; i++) {
        REQUIRE_OK(kefir_hashtable_free(mem, &interference->interference[i].value_entries));
    }
    KEFIR_FREE(mem, interference->interference);
    interference->interference = NULL;
    interference->length = 0;
    return KEFIR_OK;
}

static kefir_result_t add_interference(struct kefir_mem *mem, struct kefir_codegen_target_ir_interference *interference, kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_value_ref_t interfere_value_ref) {
    REQUIRE(value_ref.instr_ref != KEFIR_ID_NONE && value_ref.instr_ref < interference->length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Interfering value reference is out of bounds"));

    kefir_hashtable_value_t table_value;
    struct interference_entry *entry = NULL;
    kefir_result_t res = kefir_hashtable_at(&interference->interference[value_ref.instr_ref].value_entries, (kefir_hashtable_key_t) value_ref.aspect, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        entry = KEFIR_MALLOC(mem, sizeof(struct interference_entry));
        REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR interference entry"));
        entry->interference = NULL;
        entry->length = 0;
        entry->capacity = 0;
        res = kefir_hashtable_insert(mem, &interference->interference[value_ref.instr_ref].value_entries, (kefir_hashtable_key_t) value_ref.aspect, (kefir_hashtable_value_t) entry);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, entry);
            return res;
        });
    } else {
        REQUIRE_OK(res);
        entry = (struct interference_entry *) table_value;
    }

    if (entry->length >= entry->capacity) {
        kefir_size_t new_capacity = MAX(32, entry->capacity * 2);
        kefir_codegen_target_ir_value_ref_t *new_interference = KEFIR_REALLOC(mem, entry->interference, sizeof(kefir_codegen_target_ir_value_ref_t) * new_capacity);
        REQUIRE(new_interference != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR interference entry"));
        entry->capacity = new_capacity;
        entry->interference = new_interference;
    }

    entry->interference[entry->length++] = interfere_value_ref;
    return KEFIR_OK;
}

static kefir_result_t record_interference(struct kefir_mem *mem, struct kefir_codegen_target_ir_interference *interference, kefir_codegen_target_ir_value_ref_t value_ref, struct kefir_hashset *alive_values) {
    UNUSED(mem);
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t iter_key;
    kefir_result_t res;
    for (res = kefir_hashset_iter(alive_values, &iter, &iter_key); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &iter_key)) {
        kefir_codegen_target_ir_value_ref_t conflict_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(iter_key);
        if (value_ref.instr_ref != conflict_value_ref.instr_ref || value_ref.aspect != conflict_value_ref.aspect) {
            REQUIRE_OK(add_interference(mem, interference, value_ref, conflict_value_ref));
            REQUIRE_OK(add_interference(mem, interference, conflict_value_ref, value_ref));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t build_block_interference_impl(struct kefir_mem *mem, const struct kefir_codegen_target_ir_liveness *liveness, struct kefir_codegen_target_ir_interference *interference, const struct kefir_codegen_target_ir_control_flow *control_flow, kefir_codegen_target_ir_block_ref_t block_ref, struct kefir_hashset *alive_values) {
    const struct kefir_codegen_target_ir_liveness_value_block_ranges *liveness_ranges;
    REQUIRE_OK(kefir_codegen_target_ir_liveness_value_ranges(mem, control_flow, liveness, block_ref, &liveness_ranges));
    REQUIRE_OK(kefir_codegen_target_ir_liveness_build_update_alive_set(mem, liveness, KEFIR_ID_NONE, liveness_ranges, alive_values));
    for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(control_flow->code, block_ref);
        instr_ref != KEFIR_ID_NONE;
        instr_ref = kefir_codegen_target_ir_code_control_next(control_flow->code, instr_ref)) {
        REQUIRE_OK(kefir_codegen_target_ir_liveness_build_update_alive_set(mem, liveness, instr_ref, liveness_ranges, alive_values));

        struct kefir_codegen_target_ir_value_iterator value_iter;
        struct kefir_codegen_target_ir_value_ref value_ref;
        kefir_result_t res;
        for (res = kefir_codegen_target_ir_code_value_iter(control_flow->code, &value_iter, instr_ref, &value_ref, NULL);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_value_next(&value_iter, &value_ref, NULL)) {
            if (!KEFIR_CODEGEN_TARGET_IR_VALUE_IS_INDIRECT_OUTPUT(value_ref.aspect)) {
                REQUIRE_OK(record_interference(mem, interference, value_ref, alive_values));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        const struct kefir_codegen_target_ir_instruction *instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(control_flow->code, instr_ref, &instr));
        if (instr->operation.opcode == control_flow->code->klass->upsilon_opcode) {
            REQUIRE_OK(record_interference(mem, interference, instr->operation.parameters[0].upsilon_ref, alive_values));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t build_block_interference(struct kefir_mem *mem, struct kefir_codegen_target_ir_interference *interference, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_liveness *liveness, kefir_codegen_target_ir_block_ref_t block_ref) {
    struct kefir_hashset alive_values;
    REQUIRE_OK(kefir_hashset_init(&alive_values, &kefir_hashtable_uint_ops));
    kefir_result_t res = build_block_interference_impl(mem, liveness, interference, control_flow, block_ref, &alive_values);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &alive_values);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &alive_values));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_interference_build(struct kefir_mem *mem, struct kefir_codegen_target_ir_interference *interference, const struct kefir_codegen_target_ir_control_flow *control_flow, const struct kefir_codegen_target_ir_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));
    
    REQUIRE_OK(kefir_codegen_target_ir_interference_reset(mem, interference));
    
    interference->interference = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_target_ir_instruction_interference) * control_flow->code->code_length);
    REQUIRE(interference->interference != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR interference"));
    interference->length = control_flow->code->code_length;
    for (kefir_size_t i = 0; i < interference->length; i++) {
        REQUIRE_OK(kefir_hashtable_init(&interference->interference[i].value_entries, &kefir_hashtable_uint_ops));
        REQUIRE_OK(kefir_hashtable_on_removal(&interference->interference[i].value_entries, free_interfence_entry, NULL));
    }

    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(control_flow->code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(control_flow->code, i);
        if (kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref)) {
            REQUIRE_OK(build_block_interference(mem, interference, control_flow, liveness, block_ref));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_interference_has(const struct kefir_codegen_target_ir_interference *interference,
    kefir_codegen_target_ir_value_ref_t value_ref, kefir_codegen_target_ir_value_ref_t other_value_ref,
    kefir_bool_t *has_interference) {
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference"));
    REQUIRE(value_ref.instr_ref != KEFIR_ID_NONE && value_ref.instr_ref < interference->length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Interfering value reference is out of bounds"));
    REQUIRE(has_interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR interference flag"));
    
    *has_interference = false;

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&interference->interference[value_ref.instr_ref].value_entries, (kefir_hashtable_key_t) value_ref.aspect, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(struct interference_entry *, entry, table_value);
        for (kefir_size_t i = 0; i < entry->length && !*has_interference; i++) {
            if (entry->interference[i].instr_ref == other_value_ref.instr_ref && entry->interference[i].aspect == other_value_ref.aspect) {
                *has_interference = true;
            }
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_interference_iter(const struct kefir_codegen_target_ir_interference *interference,
    struct kefir_codegen_target_ir_interference_iterator *iter,
    kefir_codegen_target_ir_value_ref_t value_ref,
    kefir_codegen_target_ir_value_ref_t *interfere_value_ref_ptr) {
    REQUIRE(interference != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference"));
    REQUIRE(value_ref.instr_ref != KEFIR_ID_NONE && value_ref.instr_ref < interference->length, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Interfering value reference is out of bounds"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR interference iterator"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&interference->interference[value_ref.instr_ref].value_entries, (kefir_hashtable_key_t) value_ref.aspect, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR interference iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(struct interference_entry *, entry, table_value);
    iter->entry = entry;
    iter->index = 0;

    REQUIRE(iter->index < entry->length, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR interference iterator"));
    ASSIGN_PTR(interfere_value_ref_ptr, entry->interference[iter->index]);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_interference_next(struct kefir_codegen_target_ir_interference_iterator *iter,
    kefir_codegen_target_ir_value_ref_t *interfere_value_ref_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR interference iterator"));
    
    ASSIGN_DECL_CAST(struct interference_entry *, entry, iter->entry);
    iter->index++;

    REQUIRE(iter->index < entry->length, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR interference iterator"));
    ASSIGN_PTR(interfere_value_ref_ptr, entry->interference[iter->index]);
    return KEFIR_OK;
}
