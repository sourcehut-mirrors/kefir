#include "kefir/target/dwarf/generator.h"
#include "kefir/target/abi/amd64/base.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

#define ULEB128_WIDTH 7

kefir_result_t kefir_amd64_dwarf_byte(struct kefir_amd64_xasmgen *xasmgen, kefir_uint8_t byte) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 xasmgen"));

    struct kefir_asm_amd64_xasmgen_operand operands[1];
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(xasmgen, KEFIR_AMD64_XASMGEN_DATA_BYTE, 1,
                                        kefir_asm_amd64_xasmgen_operand_immu(&operands[0], byte)));

    return KEFIR_OK;
}

kefir_result_t kefir_amd64_dwarf_word(struct kefir_amd64_xasmgen *xasmgen, kefir_uint16_t byte) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 xasmgen"));

    struct kefir_asm_amd64_xasmgen_operand operands[1];
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(xasmgen, KEFIR_AMD64_XASMGEN_DATA_WORD, 1,
                                        kefir_asm_amd64_xasmgen_operand_immu(&operands[0], byte)));

    return KEFIR_OK;
}

kefir_result_t kefir_amd64_dwarf_qword(struct kefir_amd64_xasmgen *xasmgen, kefir_uint64_t qword) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 xasmgen"));

    struct kefir_asm_amd64_xasmgen_operand operands[1];
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                                        kefir_asm_amd64_xasmgen_operand_immu(&operands[0], qword)));

    return KEFIR_OK;
}

kefir_result_t kefir_amd64_dwarf_string(struct kefir_amd64_xasmgen *xasmgen, const char *content) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 xasmgen"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string"));

    struct kefir_asm_amd64_xasmgen_operand operands[1];
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        xasmgen, KEFIR_AMD64_XASMGEN_DATA_ASCII, 1,
        kefir_asm_amd64_xasmgen_operand_string_literal(&operands[0], content, strlen(content) + 1)));

    return KEFIR_OK;
}

kefir_result_t kefir_amd64_dwarf_uleb128(struct kefir_amd64_xasmgen *xasmgen, kefir_uint64_t value) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 xasmgen"));

    const kefir_uint64_t mask = (1 << ULEB128_WIDTH) - 1;
    do {
        kefir_uint8_t byte = value & mask;
        value >>= ULEB128_WIDTH;
        if (value != 0) {
            byte |= 1 << 7;
        }

        REQUIRE_OK(kefir_amd64_dwarf_byte(xasmgen, byte));
    } while (value != 0);

    return KEFIR_OK;
}

kefir_result_t kefir_amd64_dwarf_attribute_abbrev(struct kefir_amd64_xasmgen *xasmgen,
                                                  kefir_dwarf_attribute_t attribute, kefir_dwarf_form_t form) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 xasmgen"));

    REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(xasmgen, attribute));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(xasmgen, form));
    return KEFIR_OK;
}

kefir_result_t kefir_amd64_dwarf_entry_abbrev(struct kefir_amd64_xasmgen *xasmgen, kefir_uint64_t identifier,
                                              kefir_dwarf_tag_t tag, kefir_dwarf_children_t children) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 xasmgen"));

    REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(xasmgen, identifier));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(xasmgen, tag));
    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(xasmgen, children));
    return KEFIR_OK;
}

kefir_result_t kefir_dwarf_generator_section_init(struct kefir_amd64_xasmgen *xasmgen,
                                                  kefir_dwarf_generator_section_t section) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 xasmgen"));

    struct kefir_asm_amd64_xasmgen_operand operands[3];
    switch (section) {
        case KEFIR_DWARF_GENERATOR_SECTION_ABBREV:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(xasmgen, ".debug_abbrev", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(xasmgen, "%s", KEFIR_AMD64_DWARF_DEBUG_ABBREV_BEGIN));
            break;

        case KEFIR_DWARF_GENERATOR_SECTION_INFO:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(xasmgen, 1));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(xasmgen, ".debug_info", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                kefir_asm_amd64_xasmgen_operand_subtract(
                    &operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(&operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                          KEFIR_AMD64_DWARF_DEBUG_INFO_END),
                    kefir_asm_amd64_xasmgen_operand_label(&operands[2], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                          KEFIR_AMD64_DWARF_DEBUG_INFO_BEGIN))));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(xasmgen, "%s", KEFIR_AMD64_DWARF_DEBUG_INFO_BEGIN));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                xasmgen, KEFIR_AMD64_XASMGEN_DATA_WORD, 1,
                kefir_asm_amd64_xasmgen_operand_immu(&operands[0], (kefir_uint16_t) KEFIR_DWARF_VESION)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                xasmgen, KEFIR_AMD64_XASMGEN_DATA_BYTE, 1,
                kefir_asm_amd64_xasmgen_operand_immu(&operands[0], (kefir_uint8_t) KEFIR_DWARF(DW_UT_compile))));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                xasmgen, KEFIR_AMD64_XASMGEN_DATA_BYTE, 1,
                kefir_asm_amd64_xasmgen_operand_immu(&operands[0], (kefir_uint8_t) KEFIR_AMD64_ABI_QWORD)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                kefir_asm_amd64_xasmgen_operand_label(&operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                      KEFIR_AMD64_DWARF_DEBUG_ABBREV_BEGIN)));
            break;

        case KEFIR_DWARF_GENERATOR_SECTION_LINES:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(xasmgen, 1));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(xasmgen, ".debug_line", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(xasmgen, "%s", KEFIR_AMD64_DWARF_DEBUG_LINES));
            break;

        case KEFIR_DWARF_GENERATOR_SECTION_COUNT:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected DWARF generator section");
    }

    return KEFIR_OK;
}

kefir_result_t kefir_dwarf_generator_section_finalize(struct kefir_amd64_xasmgen *xasmgen,
                                                      kefir_dwarf_generator_section_t section) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 xasmgen"));

    switch (section) {
        case KEFIR_DWARF_GENERATOR_SECTION_ABBREV:
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(xasmgen, 0));
            break;

        case KEFIR_DWARF_GENERATOR_SECTION_INFO:
            REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(xasmgen, 0));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(xasmgen, "%s", KEFIR_AMD64_DWARF_DEBUG_INFO_END));
            break;

        case KEFIR_DWARF_GENERATOR_SECTION_LINES:
            // Intentionally left blank
            break;

        case KEFIR_DWARF_GENERATOR_SECTION_COUNT:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected DWARF generator section");
    }

    return KEFIR_OK;
}

kefir_result_t kefir_dwarf_generator_debug_information_entry(
    struct kefir_amd64_xasmgen *xasmgen, kefir_uint64_t identifier, kefir_dwarf_tag_t tag,
    kefir_dwarf_children_t children, kefir_dwarf_generator_section_t section,
    kefir_dwarf_generator_debug_information_entry_stage_t stage) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 xasmgen"));

    if (stage == KEFIR_DWARF_GENERATOR_DIE_INITIALIZE) {
        switch (section) {
            case KEFIR_DWARF_GENERATOR_SECTION_ABBREV:
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(xasmgen, identifier));
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(xasmgen, tag));
                REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(xasmgen, children));
                break;

            case KEFIR_DWARF_GENERATOR_SECTION_INFO:
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(xasmgen, identifier));
                break;

            case KEFIR_DWARF_GENERATOR_SECTION_LINES:
                // Intentionally left blank
                break;

            case KEFIR_DWARF_GENERATOR_SECTION_COUNT:
                return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected DWARF generator section");
        }
    } else if (stage == KEFIR_DWARF_GENERATOR_DIE_FINALIZE && section == KEFIR_DWARF_GENERATOR_SECTION_INFO) {
        switch (children) {
            case KEFIR_DWARF(DW_CHILDREN_yes):
                REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(xasmgen, KEFIR_DWARF(null)));
                break;

            case KEFIR_DWARF(DW_CHILDREN_no):
                // Intentionally left blank
                break;
        }
    }
    return KEFIR_OK;
}
