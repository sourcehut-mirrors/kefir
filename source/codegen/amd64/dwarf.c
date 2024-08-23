#include "kefir/codegen/amd64/dwarf.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/target/dwarf/dwarf.h"
#include "kefir/target/dwarf/generator.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/version.h"

static kefir_result_t generate_dwarf_entries(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                             const struct kefir_ir_module *ir_module,
                                             kefir_dwarf_generator_section_t section) {
    UNUSED(mem);
    UNUSED(ir_module);

    kefir_result_t res = KEFIR_OK;
    KEFIR_DWARF_GENERATOR_ENTRY(&res, &codegen->xasmgen, &section, 1, KEFIR_DWARF(DW_TAG_compile_unit),
                                KEFIR_DWARF(DW_CHILDREN_yes)) {
        REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE(section, &codegen->xasmgen, KEFIR_DWARF(DW_AT_language),
                                               KEFIR_DWARF(DW_FORM_data2),
                                               KEFIR_AMD64_DWARF_WORD(&codegen->xasmgen, KEFIR_DWARF(DW_LANG_C11))));

        REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE(
            section, &codegen->xasmgen, KEFIR_DWARF(DW_AT_producer), KEFIR_DWARF(DW_FORM_string),
            KEFIR_AMD64_DWARF_STRING(&codegen->xasmgen, "Kefir " KEFIR_VERSION_FULL)));

        REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE(
            section, &codegen->xasmgen, KEFIR_DWARF(DW_AT_low_pc), KEFIR_DWARF(DW_FORM_addr),
            KEFIR_AMD64_XASMGEN_DATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                                     kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0],
                                                                           KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                                           KEFIR_AMD64_TEXT_SECTION_BEGIN))));

        REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE(
            section, &codegen->xasmgen, KEFIR_DWARF(DW_AT_high_pc), KEFIR_DWARF(DW_FORM_data8),
            KEFIR_AMD64_XASMGEN_DATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                                     kefir_asm_amd64_xasmgen_operand_subtract(
                                         &codegen->xasmgen_helpers.operands[0],
                                         kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[1],
                                                                               KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                                               KEFIR_AMD64_TEXT_SECTION_END),
                                         kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[2],
                                                                               KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                                               KEFIR_AMD64_TEXT_SECTION_BEGIN)))));

        REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE(
            section, &codegen->xasmgen, KEFIR_DWARF(DW_AT_stmt_list), KEFIR_DWARF(DW_FORM_sec_offset),
            KEFIR_AMD64_XASMGEN_DATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                                     kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0],
                                                                           KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                                           KEFIR_AMD64_DWARF_DEBUG_LINES))));

        REQUIRE_OK(KEFIR_AMD64_DWARF_ATTRIBUTE_END(section, &codegen->xasmgen));
    }
    REQUIRE_OK(res);

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_generate_dwarf_debug_info(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                                             const struct kefir_ir_module *ir_module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen"));
    REQUIRE(ir_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));

    kefir_dwarf_generator_section_t section;
    KEFIR_DWARF_GENERATOR_DEBUG_INFO(&section) {
        REQUIRE_OK(KEFIR_AMD64_DWARF_SECTION_INIT(&codegen->xasmgen, section));
        REQUIRE_OK(generate_dwarf_entries(mem, codegen, ir_module, section));
        REQUIRE_OK(KEFIR_AMD64_DWARF_SECTION_FINALIZE(&codegen->xasmgen, section));
    }
    return KEFIR_OK;
}
