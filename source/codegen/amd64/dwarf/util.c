
#define KEFIR_CODEGEN_AMD64_DWARF_INTERNAL
#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/dwarf.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_amd64_dwarf_collect_code_fragments(struct kefir_mem *mem, struct kefir_codegen_amd64_function *codegen_function,
    kefir_opt_code_debug_info_code_ref_t begin_ref, kefir_opt_code_debug_info_code_ref_t end_ref, struct kefir_hashtree *fragment_tree) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function"));
    REQUIRE(fragment_tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 dwarf fragment tree"));

    for (kefir_opt_code_debug_info_code_ref_t code_ref = begin_ref; code_ref <= end_ref; code_ref++) {
        const struct kefir_opt_code_debug_info_code_reference *code_reference;
        kefir_result_t res = kefir_opt_code_debug_info_code_reference(&codegen_function->function->debug_info, code_ref, &code_reference);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t iter_key;
        for (res = kefir_hashset_iter(&code_reference->instructions, &iter, &iter_key); res == KEFIR_OK;
            res = kefir_hashset_next(&iter, &iter_key)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, iter_key);

            struct kefir_asmcmp_code_map_fragment_iterator iter;
            const struct kefir_asmcmp_debug_info_code_fragment *fragment;
            for (res = kefir_asmcmp_code_map_fragment_iter(&codegen_function->code.context.debug_info.code_map, instr_ref, &iter, &fragment);
                res == KEFIR_OK;
                res = kefir_asmcmp_code_map_fragment_next(&iter, &fragment)) {
                if (fragment->begin_label == fragment->end_label) {
                    continue;
                }
                
        
                kefir_hashtree_key_t key = (((kefir_uint64_t) fragment->begin_label) << 32) | (kefir_uint32_t) fragment->end_label;
                kefir_result_t res = kefir_hashtree_insert(mem, fragment_tree, key, (kefir_hashtree_value_t) 0);
                if (res == KEFIR_ALREADY_EXISTS) {
                    continue;
                }
                REQUIRE_OK(res);
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_generate_range_list_coalesce(struct kefir_mem *mem, struct kefir_codegen_amd64_function *codegen_function,
    kefir_opt_code_debug_info_code_ref_t begin_ref, kefir_opt_code_debug_info_code_ref_t end_ref, struct kefir_hashtree *fragment_tree) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function"));
    REQUIRE(fragment_tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 dwarf fragment tree"));

    REQUIRE_OK(kefir_codegen_amd64_dwarf_collect_code_fragments(mem, codegen_function, begin_ref, end_ref, fragment_tree));
    kefir_bool_t reached_fixpoint = false;
    for (; !reached_fixpoint;) {
        reached_fixpoint = true;

        struct kefir_hashtree_node_iterator tree_iter;
        for (struct kefir_hashtree_node *node = kefir_hashtree_iter(fragment_tree, &tree_iter);
            node != NULL && reached_fixpoint;
            node = kefir_hashtree_next(&tree_iter)) {
            kefir_asmcmp_label_index_t begin_label = ((kefir_uint64_t) node->key) >> 32;
            kefir_asmcmp_label_index_t end_label = (kefir_uint32_t) node->key;

            struct kefir_hashtree_node *other_node;
            kefir_result_t res = kefir_hashtree_lower_bound(fragment_tree, (kefir_hashtree_key_t) (((kefir_uint64_t) end_label) << 32), &other_node);
            if (res == KEFIR_NOT_FOUND) {
                REQUIRE_OK(kefir_hashtree_min(fragment_tree, &other_node));
            } else {
                REQUIRE_OK(res);
                other_node = kefir_hashtree_next_node(fragment_tree, other_node);
            }

            if (other_node == NULL) {
                continue;
            }

            kefir_asmcmp_label_index_t other_begin_label = ((kefir_uint64_t) other_node->key) >> 32;
            kefir_asmcmp_label_index_t other_end_label = (kefir_uint32_t) other_node->key;

            if (other_begin_label == end_label) {
                reached_fixpoint = false;
                REQUIRE_OK(kefir_hashtree_delete(mem, fragment_tree, node->key));
                REQUIRE_OK(kefir_hashtree_delete(mem, fragment_tree, other_node->key));

                kefir_hashtree_key_t key = (((kefir_uint64_t) begin_label) << 32) | (kefir_uint32_t) other_end_label;
                res = kefir_hashtree_insert(mem, fragment_tree, key, (kefir_hashtree_value_t) 0);
                if (res == KEFIR_ALREADY_EXISTS) {
                    continue;
                }
                REQUIRE_OK(res);
            }
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_dwarf_generate_range_list(struct kefir_mem *mem, struct kefir_codegen_amd64_function *codegen_function,
                                                        kefir_opt_code_debug_info_code_ref_t begin_ref, kefir_opt_code_debug_info_code_ref_t end_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen function"));
    
    struct kefir_hashtree fragment_tree;
    REQUIRE_OK(kefir_hashtree_init(&fragment_tree, &kefir_hashtree_uint_ops));

    kefir_result_t res = kefir_codegen_amd64_dwarf_generate_range_list_coalesce(mem, codegen_function, begin_ref, end_ref, &fragment_tree);

    const struct kefir_ir_identifier *ir_identifier;
    REQUIRE_CHAIN(&res, kefir_ir_module_get_identifier(codegen_function->module->ir_module,
                                              codegen_function->function->ir_func->name, &ir_identifier));
    struct kefir_hashtree_node_iterator tree_iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&fragment_tree, &tree_iter);
        res == KEFIR_OK && node != NULL;
        node = kefir_hashtree_next(&tree_iter)) {
        kefir_asmcmp_label_index_t begin_label = ((kefir_uint64_t) node->key) >> 32;
        kefir_asmcmp_label_index_t end_label = (kefir_uint32_t) node->key;

        REQUIRE_CHAIN(&res, KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_RLE_start_end)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                    kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL, codegen_function->codegen->config->symbol_prefix,
                                                        ir_identifier->symbol, begin_label))));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                    kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL, codegen_function->codegen->config->symbol_prefix,
                                                        ir_identifier->symbol, end_label))));
    }
    
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &fragment_tree);
        return res;
    });

    REQUIRE_OK(kefir_hashtree_free(mem, &fragment_tree));

    return KEFIR_OK;
}