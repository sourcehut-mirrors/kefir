#include "kefir/codegen/target-ir/transform.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t remove_unused_phis_from_block(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_block_ref_t block_ref, struct kefir_hashset *removal_set) {
    REQUIRE_OK(kefir_hashset_clear(mem, removal_set));

    kefir_result_t res;
    struct kefir_codegen_target_ir_value_phi_node_iterator phi_node_iter;
    kefir_codegen_target_ir_instruction_ref_t phi_ref;
    for (res = kefir_codegen_target_ir_code_phi_node_iter(code, &phi_node_iter, block_ref, &phi_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_phi_node_next(&phi_node_iter, &phi_ref)) {
        struct kefir_codegen_target_ir_use_iterator use_iter;
        res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, phi_ref, NULL);
        if (res == KEFIR_ITERATOR_END) {
            REQUIRE_OK(kefir_hashset_add(mem, removal_set, (kefir_hashset_key_t) phi_ref));
            continue;
        } else {
            REQUIRE_OK(res);
        }

        struct kefir_codegen_target_ir_value_phi_link_iterator link_iter;
        struct kefir_codegen_target_ir_value_ref linked_value_ref, other_linked_value_ref;
        res = kefir_codegen_target_ir_code_phi_link_iter(code, &link_iter, phi_ref, NULL, &linked_value_ref);
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
            kefir_bool_t do_drop = true;
            for (res = kefir_codegen_target_ir_code_phi_link_next(&link_iter, NULL, &other_linked_value_ref);
                res == KEFIR_OK && do_drop;
                res = kefir_codegen_target_ir_code_phi_link_next(&link_iter, NULL, &other_linked_value_ref)) {
                if ((other_linked_value_ref.instr_ref != linked_value_ref.instr_ref || other_linked_value_ref.aspect != linked_value_ref.aspect) &&
                    other_linked_value_ref.instr_ref != phi_ref) {
                    do_drop = false;
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
            if (do_drop) {
                REQUIRE_OK(kefir_codegen_target_ir_code_replace_instruction(mem, code, linked_value_ref.instr_ref, phi_ref));
                REQUIRE_OK(kefir_hashset_add(mem, removal_set, (kefir_hashset_key_t) phi_ref));
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_hashset_iterator removal_iter;
    kefir_hashset_key_t removal_key;
    for (res = kefir_hashset_iter(removal_set, &removal_iter, &removal_key); res == KEFIR_OK;
         res = kefir_hashset_next(&removal_iter, &removal_key)) {
        REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, (kefir_codegen_target_ir_instruction_ref_t) removal_key));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t remove_unused_phis_round(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct kefir_hashset *removal_set, kefir_bool_t *fixpoint) {
    *fixpoint = true;

    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        REQUIRE_OK(remove_unused_phis_from_block(mem, code, block_ref, removal_set));
        if (removal_set->occupied > 0) {
            *fixpoint = false;
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_transform_phi_removal(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    struct kefir_hashset removal_set;
    REQUIRE_OK(kefir_hashset_init(&removal_set, &kefir_hashtable_uint_ops));

    kefir_bool_t fixpoint = false;
    for (; !fixpoint;) {
        kefir_result_t res = remove_unused_phis_round(mem, code, &removal_set, &fixpoint);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_hashset_free(mem, &removal_set);
            return res;
        });
    }
    REQUIRE_OK(kefir_hashset_free(mem, &removal_set));
    return KEFIR_OK;
}
