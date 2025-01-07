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

kefir_result_t kefir_amd64_dwarf_signed_qword(struct kefir_amd64_xasmgen *xasmgen, kefir_int64_t qword) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 xasmgen"));

    struct kefir_asm_amd64_xasmgen_operand operands[1];
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                                        kefir_asm_amd64_xasmgen_operand_imm(&operands[0], qword)));

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

kefir_result_t kefir_amd64_dwarf_sleb128(struct kefir_amd64_xasmgen *xasmgen, kefir_int64_t value) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 xasmgen"));

    kefir_bool_t more = true;
    const kefir_bool_t negative = (value < 0);
    while (more) {
        kefir_uint8_t byte = value & 0x7f;
        value >>= ULEB128_WIDTH;

        if (negative) {
            value |= (~0ull << (sizeof(kefir_int64_t) * CHAR_BIT - ULEB128_WIDTH));
        }

        const kefir_uint8_t sign_bit = byte & 0x40;
        if ((value == 0 && sign_bit == 0) || (value == -1 && sign_bit != 0)) {
            more = 0;
        } else {
            byte |= 0x80;
        }
        REQUIRE_OK(kefir_amd64_dwarf_byte(xasmgen, byte));
    }
    return KEFIR_OK;
}

kefir_size_t kefir_amd64_dwarf_uleb128_length(kefir_uint64_t value) {
    kefir_size_t length = 0;
    do {
        value >>= ULEB128_WIDTH;
        length++;
    } while (value != 0);
    return length;
}

kefir_size_t kefir_amd64_dwarf_sleb128_length(kefir_int64_t value) {
    kefir_size_t length = 0;
    kefir_bool_t more = true;
    const kefir_bool_t negative = (value < 0);
    while (more) {
        kefir_uint8_t byte = value & 0x7f;
        value >>= ULEB128_WIDTH;

        if (negative) {
            value |= (~0ull << (sizeof(kefir_int64_t) * CHAR_BIT - ULEB128_WIDTH));
        }

        const kefir_uint8_t sign_bit = byte & 0x40;
        if ((value == 0 && sign_bit == 0) || (value == -1 && sign_bit != 0)) {
            more = 0;
        } else {
            byte |= 0x80;
        }
        length++;
    }
    return length;
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

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(xasmgen, KEFIR_AMD64_DWARF_DEBUG_ABBREV_ENTRY, identifier));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(xasmgen, identifier));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(xasmgen, tag));
    REQUIRE_OK(KEFIR_AMD64_DWARF_BYTE(xasmgen, children));
    return KEFIR_OK;
}

kefir_result_t kefir_amd64_dwarf_entry_info_begin(struct kefir_amd64_xasmgen *xasmgen, kefir_uint64_t identifier,
                                                  kefir_uint64_t abbrev_identifier) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 xasmgen"));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(xasmgen, KEFIR_AMD64_DWARF_DEBUG_INFO_ENTRY, identifier));
    REQUIRE_OK(KEFIR_AMD64_DWARF_ULEB128(xasmgen, abbrev_identifier));
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
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(xasmgen, "%s", KEFIR_AMD64_DWARF_DEBUG_INFO_SECTION));
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

        case KEFIR_DWARF_GENERATOR_SECTION_LOCLISTS:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(xasmgen, 1));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(xasmgen, ".debug_loclists", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(xasmgen, "%s", KEFIR_AMD64_DWARF_DEBUG_LOCLISTS));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                kefir_asm_amd64_xasmgen_operand_subtract(
                    &operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(&operands[1], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                          KEFIR_AMD64_DWARF_DEBUG_LOCLISTS_END),
                    kefir_asm_amd64_xasmgen_operand_label(&operands[2], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE,
                                                          KEFIR_AMD64_DWARF_DEBUG_LOCLISTS_BEGIN))));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(xasmgen, "%s", KEFIR_AMD64_DWARF_DEBUG_LOCLISTS_BEGIN));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                xasmgen, KEFIR_AMD64_XASMGEN_DATA_WORD, 1,
                kefir_asm_amd64_xasmgen_operand_immu(&operands[0], (kefir_uint16_t) KEFIR_DWARF_VESION)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                xasmgen, KEFIR_AMD64_XASMGEN_DATA_BYTE, 1,
                kefir_asm_amd64_xasmgen_operand_immu(&operands[0], (kefir_uint8_t) KEFIR_AMD64_ABI_QWORD)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(xasmgen, KEFIR_AMD64_XASMGEN_DATA_BYTE, 1,
                                                kefir_asm_amd64_xasmgen_operand_immu(&operands[0], (kefir_uint8_t) 0)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                                                kefir_asm_amd64_xasmgen_operand_immu(&operands[0], (kefir_uint8_t) 0)));
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

        case KEFIR_DWARF_GENERATOR_SECTION_LOCLISTS:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(xasmgen, "%s", KEFIR_AMD64_DWARF_DEBUG_LOCLISTS_END));
            break;

        case KEFIR_DWARF_GENERATOR_SECTION_LINES:
            // Intentionally left blank
            break;

        case KEFIR_DWARF_GENERATOR_SECTION_COUNT:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected DWARF generator section");
    }

    return KEFIR_OK;
}
