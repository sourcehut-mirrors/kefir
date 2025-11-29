
#define KEFIR_CODEGEN_AMD64_DWARF_INTERNAL
#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/dwarf.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_amd64_dwarf_generate_range_list(struct kefir_codegen_amd64_function *codegen_function,
                                                        kefir_opt_code_debug_info_code_ref_t begin_ref, kefir_opt_code_debug_info_code_ref_t end_ref) {
    REQUIRE(codegen_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen function"));
    
    const struct kefir_ir_identifier *ir_identifier;
    REQUIRE_OK(kefir_ir_module_get_identifier(codegen_function->module->ir_module,
                                              codegen_function->function->ir_func->name, &ir_identifier));

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
                REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(&codegen_function->codegen->xasmgen, KEFIR_DWARF(DW_RLE_start_end)));
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                            kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL, codegen_function->codegen->config->symbol_prefix,
                                                                ir_identifier->symbol, fragment->begin_label))));
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                        &codegen_function->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen_function->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                            kefir_asm_amd64_xasmgen_helpers_format(&codegen_function->codegen->xasmgen_helpers, KEFIR_AMD64_LABEL, codegen_function->codegen->config->symbol_prefix,
                                                                ir_identifier->symbol, fragment->end_label))));
                
            }
            if (res == KEFIR_NOT_FOUND) {
                res = KEFIR_OK;
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    return KEFIR_OK;
}