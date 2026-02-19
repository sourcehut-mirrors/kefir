#include "kefir/optimizer/escape.h"
#include "kefir/core/hashset.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct escape_entry {
    struct kefir_hashset escapes;
};

static kefir_result_t free_escape_entry(struct kefir_mem *mem, struct kefir_hashtable *table, kefir_hashtable_key_t key,
                                        kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct escape_entry *, entry, value);
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer escape entry"));

    REQUIRE_OK(kefir_hashset_free(mem, &entry->escapes));
    KEFIR_FREE(mem, entry);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_escape_analysis_init(struct kefir_opt_code_escape_analysis *escape) {
    REQUIRE(escape != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer escape analysis"));

    REQUIRE_OK(kefir_hashtable_init(&escape->local_escapes, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&escape->local_escapes, free_escape_entry, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_escape_analysis_free(struct kefir_mem *mem,
                                                   struct kefir_opt_code_escape_analysis *escape) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(escape != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer escape analysis"));

    REQUIRE_OK(kefir_hashtable_free(mem, &escape->local_escapes));
    return KEFIR_OK;
}

static kefir_result_t record_escape(struct kefir_mem *mem, struct kefir_opt_code_escape_analysis *escape,
                                    kefir_opt_instruction_ref_t instr_ref, kefir_opt_instruction_ref_t escape_ref) {
    struct escape_entry *entry;
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&escape->local_escapes, (kefir_hashtable_key_t) instr_ref, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        entry = (struct escape_entry *) table_value;
    } else {
        entry = KEFIR_MALLOC(mem, sizeof(struct escape_entry));
        REQUIRE(entry != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer escape analysis entry"));
        res = kefir_hashset_init(&entry->escapes, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashtable_insert(mem, &escape->local_escapes, (kefir_hashtable_key_t) instr_ref,
                                                   (kefir_hashtable_value_t) entry));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, entry);
            return res;
        });
    }

    REQUIRE_OK(kefir_hashset_add(mem, &entry->escapes, (kefir_hashset_key_t) escape_ref));
    return KEFIR_OK;
}

static kefir_result_t record_escapes(struct kefir_mem *mem, struct kefir_opt_code_escape_analysis *escape,
                                     const struct kefir_opt_code_container *code,
                                     kefir_opt_instruction_ref_t alloc_instr_ref,
                                     kefir_opt_instruction_ref_t instr_ref) {
    kefir_result_t res;
    struct kefir_opt_instruction_use_iterator use_iter;
    for (res = kefir_opt_code_container_instruction_use_instr_iter(code, instr_ref, &use_iter); res == KEFIR_OK;
         res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_instruction *use_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, use_iter.use_instr_ref, &use_instr));

        switch (use_instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_INT8_LOAD:
            case KEFIR_OPT_OPCODE_INT16_LOAD:
            case KEFIR_OPT_OPCODE_INT32_LOAD:
            case KEFIR_OPT_OPCODE_INT64_LOAD:
            case KEFIR_OPT_OPCODE_INT128_LOAD:
            case KEFIR_OPT_OPCODE_FLOAT32_LOAD:
            case KEFIR_OPT_OPCODE_FLOAT64_LOAD:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL32_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL64_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL128_LOAD:
            case KEFIR_OPT_OPCODE_BITINT_LOAD:
            case KEFIR_OPT_OPCODE_BITINT_LOAD_PRECISE:
            case KEFIR_OPT_OPCODE_ATOMIC_LOAD8:
            case KEFIR_OPT_OPCODE_ATOMIC_LOAD16:
            case KEFIR_OPT_OPCODE_ATOMIC_LOAD32:
            case KEFIR_OPT_OPCODE_ATOMIC_LOAD64:
            case KEFIR_OPT_OPCODE_ATOMIC_LOAD_LONG_DOUBLE:
            case KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_FLOAT32:
            case KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_FLOAT64:
            case KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_LONG_DOUBLE:
            case KEFIR_OPT_OPCODE_ZERO_MEMORY:
                // Intentionally left blank
                break;

            case KEFIR_OPT_OPCODE_INT8_STORE:
            case KEFIR_OPT_OPCODE_INT16_STORE:
            case KEFIR_OPT_OPCODE_INT32_STORE:
            case KEFIR_OPT_OPCODE_INT64_STORE:
            case KEFIR_OPT_OPCODE_INT128_STORE:
            case KEFIR_OPT_OPCODE_FLOAT32_STORE:
            case KEFIR_OPT_OPCODE_FLOAT64_STORE:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL64_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL128_STORE:
            case KEFIR_OPT_OPCODE_BITINT_STORE:
            case KEFIR_OPT_OPCODE_BITINT_STORE_PRECISE:
            case KEFIR_OPT_OPCODE_ATOMIC_STORE8:
            case KEFIR_OPT_OPCODE_ATOMIC_STORE16:
            case KEFIR_OPT_OPCODE_ATOMIC_STORE32:
            case KEFIR_OPT_OPCODE_ATOMIC_STORE64:
            case KEFIR_OPT_OPCODE_ATOMIC_STORE_LONG_DOUBLE:
            case KEFIR_OPT_OPCODE_ATOMIC_STORE_COMPLEX_FLOAT32:
            case KEFIR_OPT_OPCODE_ATOMIC_STORE_COMPLEX_FLOAT64:
            case KEFIR_OPT_OPCODE_ATOMIC_STORE_COMPLEX_LONG_DOUBLE:
                if (instr_ref == use_instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF]) {
                    REQUIRE_OK(record_escape(mem, escape, alloc_instr_ref, use_iter.use_instr_ref));
                }
                break;

            case KEFIR_OPT_OPCODE_REF_LOCAL:
                REQUIRE_OK(record_escapes(mem, escape, code, alloc_instr_ref, use_iter.use_instr_ref));
                break;

            default:
                REQUIRE_OK(record_escape(mem, escape, alloc_instr_ref, use_iter.use_instr_ref));
                break;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_escape_analysis_build(struct kefir_mem *mem,
                                                    struct kefir_opt_code_escape_analysis *escape,
                                                    const struct kefir_opt_code_container *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(escape != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer escape analysis"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));

    for (kefir_opt_block_id_t block_id = 0; block_id < kefir_opt_code_container_block_count(code); block_id++) {
        kefir_result_t res;
        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_instr_head(code, block_id, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_sibling(code, instr_ref, &instr_ref)) {
            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));

            if (instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
                REQUIRE_OK(record_escapes(mem, escape, code, instr_ref, instr_ref));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

kefir_bool_t kefir_opt_code_escape_analysis_has_escapes(const struct kefir_opt_code_escape_analysis *escape,
                                                        kefir_opt_instruction_ref_t instr_ref) {
    return escape != NULL && kefir_hashtable_has(&escape->local_escapes, (kefir_hashtable_key_t) instr_ref);
}
