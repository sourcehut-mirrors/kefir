/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#include "kefir/codegen/amd64/xasmgen.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>

struct xasmgen_payload {
    FILE *output;
};

static kefir_result_t amd64_ident(struct kefir_amd64_xasmgen *xasmgen) {
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);
    if (xasmgen->settings.enable_identation) {
        fprintf(payload->output, "    ");
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_prologue(struct kefir_amd64_xasmgen *xasmgen) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, ".intel_syntax prefix\n\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_close(struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fflush(payload->output);
    payload->output = NULL;
    KEFIR_FREE(mem, payload);
    xasmgen->payload = NULL;
    return KEFIR_OK;
}

static kefir_result_t amd64_newline(struct kefir_amd64_xasmgen *xasmgen, unsigned int count) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    while (count--) {
        fprintf(payload->output, "\n");
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_comment(struct kefir_amd64_xasmgen *xasmgen, const char *format, ...) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    if (xasmgen->settings.enable_comments) {
        fprintf(payload->output, "# ");
        va_list args;
        va_start(args, format);
        vfprintf(payload->output, format, args);
        va_end(args);
        fprintf(payload->output, "\n");
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_label(struct kefir_amd64_xasmgen *xasmgen, const char *format, ...) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    va_list args;
    va_start(args, format);
    vfprintf(payload->output, format, args);
    va_end(args);
    fprintf(payload->output, ":\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_global(struct kefir_amd64_xasmgen *xasmgen, const char *format, ...) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, ".global ");
    va_list args;
    va_start(args, format);
    vfprintf(payload->output, format, args);
    va_end(args);
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_external(struct kefir_amd64_xasmgen *xasmgen, const char *format, ...) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, ".extern ");
    va_list args;
    va_start(args, format);
    vfprintf(payload->output, format, args);
    va_end(args);
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_section(struct kefir_amd64_xasmgen *xasmgen, const char *identifier) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, ".section %s\n", identifier);
    return KEFIR_OK;
}

static kefir_result_t amd64_align(struct kefir_amd64_xasmgen *xasmgen, kefir_size_t alignment) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, ".align " KEFIR_SIZE_FMT "\n", alignment);
    return KEFIR_OK;
}

static const char *register_literals[] = {
    [KEFIR_AMD64_XASMGEN_REGISTER_EAX] = "eax",   [KEFIR_AMD64_XASMGEN_REGISTER_RAX] = "rax",
    [KEFIR_AMD64_XASMGEN_REGISTER_RBX] = "rbx",   [KEFIR_AMD64_XASMGEN_REGISTER_RCX] = "rcx",
    [KEFIR_AMD64_XASMGEN_REGISTER_RDX] = "rdx",   [KEFIR_AMD64_XASMGEN_REGISTER_RSI] = "rsi",
    [KEFIR_AMD64_XASMGEN_REGISTER_RDI] = "rdi",   [KEFIR_AMD64_XASMGEN_REGISTER_RSP] = "rsp",
    [KEFIR_AMD64_XASMGEN_REGISTER_RBP] = "rbp",   [KEFIR_AMD64_XASMGEN_REGISTER_R8] = "r8",
    [KEFIR_AMD64_XASMGEN_REGISTER_R9] = "r9",     [KEFIR_AMD64_XASMGEN_REGISTER_R10] = "r10",
    [KEFIR_AMD64_XASMGEN_REGISTER_R11] = "r11",   [KEFIR_AMD64_XASMGEN_REGISTER_R12] = "r12",
    [KEFIR_AMD64_XASMGEN_REGISTER_R13] = "r13",   [KEFIR_AMD64_XASMGEN_REGISTER_R14] = "r14",
    [KEFIR_AMD64_XASMGEN_REGISTER_R15] = "r15",   [KEFIR_AMD64_XASMGEN_REGISTER_XMM0] = "xmm0",
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM1] = "xmm1", [KEFIR_AMD64_XASMGEN_REGISTER_XMM2] = "xmm2",
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM3] = "xmm3", [KEFIR_AMD64_XASMGEN_REGISTER_XMM4] = "xmm4",
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM5] = "xmm5", [KEFIR_AMD64_XASMGEN_REGISTER_XMM6] = "xmm6",
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM7] = "xmm7"};

static kefir_result_t amd64_string_literal(FILE *out, const char *literal, kefir_size_t length) {
    fprintf(out, "\"");
    const char *end = literal + length;
    for (; literal < end; ++literal) {
        switch (*literal) {
            case U'\0':
                fprintf(out, "\\000");
                break;

            case U'\"':
                fprintf(out, "\\\"");
                break;

            case U'\\':
                fprintf(out, "\\\\");
                break;

            case U'\a':
                fprintf(out, "\\%03o", '\a');
                break;

            case U'\b':
                fprintf(out, "\\b");
                break;

            case U'\t':
                fprintf(out, "\\t");
                break;

            case U'\n':
                fprintf(out, "\\n");
                break;

            case U'\v':
                fprintf(out, "\\%03o", '\v');
                break;

            case U'\f':
                fprintf(out, "\\f");
                break;

            case U'\r':
                fprintf(out, "\\r");
                break;

            default:
                if (isprint(*literal)) {
                    fprintf(out, "%c", *literal);
                } else {
                    fprintf(out, "\\%03o", (unsigned char) *literal);
                }
                break;
        }
    }
    fprintf(out, "\"");
    return KEFIR_OK;
}

static kefir_result_t amd64_symbol_arg(FILE *out, const char *symbol) {
    const char *EscapedSymbols[] = {// TODO Expand number of escaped symbols
                                    "mod"};
    for (kefir_size_t i = 0; i < sizeof(EscapedSymbols) / sizeof(EscapedSymbols[0]); i++) {
        if (strcasecmp(symbol, EscapedSymbols[i]) == 0) {
            fprintf(out, "$%s", symbol);
            return KEFIR_OK;
        }
    }

    fprintf(out, "%s", symbol);
    return KEFIR_OK;
}

static kefir_result_t format_pointer(FILE *out, kefir_amd64_xasmgen_indirection_pointer_type_t type) {
    switch (type) {
        case KEFIR_AMD64_XASMGEN_INDIRECTION_POINTER_NONE:
            // Intentionally left blank
            break;

        case KEFIR_AMD64_XASMGEN_INDIRECTION_POINTER_DWORD:
            fprintf(out, "DWORD PTR ");
            break;

        case KEFIR_AMD64_XASMGEN_INDIRECTION_POINTER_QWORD:
            fprintf(out, "QWORD PTR ");
            break;

        case KEFIR_AMD64_XASMGEN_INDIRECTION_POINTER_TBYTE:
            fprintf(out, "TBYTE PTR ");
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_format_operand(struct kefir_amd64_xasmgen *xasmgen,
                                           const struct kefir_amd64_xasmgen_operand *op) {
    UNUSED(xasmgen);
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);
    switch (op->klass) {
        case KEFIR_AMD64_XASMGEN_OPERAND_IMMEDIATE:
            fprintf(payload->output, KEFIR_INT64_FMT, op->imm);
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_IMMEDIATE_UNSIGNED:
            fprintf(payload->output, KEFIR_UINT64_FMT, op->immu);
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_REGISTER:
            fprintf(payload->output, "%%%s", register_literals[op->reg]);
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_LABEL:
            fprintf(payload->output, "%s", op->label);
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_SYMBOL:
            REQUIRE_OK(amd64_symbol_arg(payload->output, op->symbol));
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_INDIRECTION:
            REQUIRE_OK(format_pointer(payload->output, op->indirection.type));
            fprintf(payload->output, "[");
            REQUIRE_OK(amd64_format_operand(xasmgen, op->indirection.base));
            if (op->indirection.offset > 0) {
                fprintf(payload->output, " + " KEFIR_INT64_FMT, op->indirection.offset);
            } else if (op->indirection.offset < 0) {
                fprintf(payload->output, " - " KEFIR_INT64_FMT, -op->indirection.offset);
            }
            fprintf(payload->output, "]");
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_OFFSET:
            REQUIRE_OK(amd64_format_operand(xasmgen, op->offset.base));
            if (op->offset.offset > 0) {
                fprintf(payload->output, " + " KEFIR_INT64_FMT, op->offset.offset);
            } else if (op->offset.offset < 0) {
                fprintf(payload->output, " - " KEFIR_INT64_FMT, -op->offset.offset);
            }
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_SEGMENT:
            switch (op->segment.segment) {
                case KEFIR_AMD64_XASMGEN_SEGMENT_FS:
                    fprintf(payload->output, "%%fs:");
                    break;
            }
            REQUIRE_OK(amd64_format_operand(xasmgen, op->segment.base));
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_POINTER:
            REQUIRE_OK(format_pointer(payload->output, op->pointer.type));
            REQUIRE_OK(amd64_format_operand(xasmgen, op->pointer.base));
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_RIP_INDIRECTION:
            fprintf(payload->output, "%s[%%rip]", op->rip_indirection.identifier);
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_STRING_LITERAL:
            REQUIRE_OK(amd64_string_literal(payload->output, op->string_literal.content, op->string_literal.length));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_data(struct kefir_amd64_xasmgen *xasmgen, kefir_amd64_xasmgen_data_type_t type,
                                 kefir_size_t length, ...) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    switch (type) {
        case KEFIR_AMD64_XASMGEN_DATA_BYTE:
            fprintf(payload->output, ".byte ");
            break;

        case KEFIR_AMD64_XASMGEN_DATA_WORD:
            fprintf(payload->output, ".word ");
            break;

        case KEFIR_AMD64_XASMGEN_DATA_DOUBLE:
            fprintf(payload->output, ".long ");
            break;

        case KEFIR_AMD64_XASMGEN_DATA_QUAD:
            fprintf(payload->output, ".quad ");
            break;

        case KEFIR_AMD64_XASMGEN_DATA_ASCII:
            fprintf(payload->output, ".ascii ");
            break;
    }

    va_list args;
    va_start(args, length);
    while (length--) {
        const struct kefir_amd64_xasmgen_operand *op = va_arg(args, const struct kefir_amd64_xasmgen_operand *);
        REQUIRE_OK(amd64_format_operand(xasmgen, op));
        if (length > 0) {
            fprintf(payload->output, ", ");
        }
    }
    va_end(args);
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_zerodata(struct kefir_amd64_xasmgen *xasmgen, kefir_size_t length) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, ".zero " KEFIR_SIZE_FMT "\n", length);
    return KEFIR_OK;
}

static kefir_result_t amd64_bindata(struct kefir_amd64_xasmgen *xasmgen, kefir_amd64_xasmgen_data_type_t type,
                                    const void *ptr, kefir_size_t length) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid binary data"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    switch (type) {
        case KEFIR_AMD64_XASMGEN_DATA_BYTE:
            fprintf(payload->output, ".byte ");
            for (kefir_size_t i = 0; i < length; i++) {
                fprintf(payload->output, "0x%02x", ((const kefir_uint8_t *) ptr)[i]);
                if (i + 1 < length) {
                    fprintf(payload->output, ", ");
                }
            }
            fprintf(payload->output, "\n");
            break;

        case KEFIR_AMD64_XASMGEN_DATA_WORD:
            fprintf(payload->output, ".word ");
            for (kefir_size_t i = 0; i < length; i++) {
                fprintf(payload->output, "0x%04x", ((const kefir_uint16_t *) ptr)[i]);
                if (i + 1 < length) {
                    fprintf(payload->output, ", ");
                }
            }
            fprintf(payload->output, "\n");
            break;

        case KEFIR_AMD64_XASMGEN_DATA_DOUBLE:
            fprintf(payload->output, ".long ");
            for (kefir_size_t i = 0; i < length; i++) {
                fprintf(payload->output, "0x%08x", ((const kefir_uint32_t *) ptr)[i]);
                if (i + 1 < length) {
                    fprintf(payload->output, ", ");
                }
            }
            fprintf(payload->output, "\n");
            break;

        case KEFIR_AMD64_XASMGEN_DATA_QUAD:
            fprintf(payload->output, ".quad ");
            for (kefir_size_t i = 0; i < length; i++) {
                fprintf(payload->output, "0x%016lx", ((const kefir_uint64_t *) ptr)[i]);
                if (i + 1 < length) {
                    fprintf(payload->output, ", ");
                }
            }
            fprintf(payload->output, "\n");
            break;

        case KEFIR_AMD64_XASMGEN_DATA_ASCII:
            REQUIRE_OK(amd64_string_literal(payload->output, ptr, length));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_push(struct kefir_amd64_xasmgen *xasmgen,
                                       const struct kefir_amd64_xasmgen_operand *op1) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "push ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_pop(struct kefir_amd64_xasmgen *xasmgen,
                                      const struct kefir_amd64_xasmgen_operand *op1) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "pop ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_mov(struct kefir_amd64_xasmgen *xasmgen,
                                      const struct kefir_amd64_xasmgen_operand *op1,
                                      const struct kefir_amd64_xasmgen_operand *op2) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "mov ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_movabs(struct kefir_amd64_xasmgen *xasmgen,
                                         const struct kefir_amd64_xasmgen_operand *op1,
                                         const struct kefir_amd64_xasmgen_operand *op2) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "movabs ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_or(struct kefir_amd64_xasmgen *xasmgen, const struct kefir_amd64_xasmgen_operand *op1,
                                     const struct kefir_amd64_xasmgen_operand *op2) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "or ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_lea(struct kefir_amd64_xasmgen *xasmgen,
                                      const struct kefir_amd64_xasmgen_operand *op1,
                                      const struct kefir_amd64_xasmgen_operand *op2) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "lea ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_movq(struct kefir_amd64_xasmgen *xasmgen,
                                       const struct kefir_amd64_xasmgen_operand *op1,
                                       const struct kefir_amd64_xasmgen_operand *op2) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "movq ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_movsb(struct kefir_amd64_xasmgen *xasmgen, kefir_bool_t rep) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    if (rep) {
        fprintf(payload->output, "rep ");
    }

    fprintf(payload->output, "movsb\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_jmp(struct kefir_amd64_xasmgen *xasmgen,
                                      const struct kefir_amd64_xasmgen_operand *op1) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "jmp ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_ja(struct kefir_amd64_xasmgen *xasmgen,
                                     const struct kefir_amd64_xasmgen_operand *op1) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "ja ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_ret(struct kefir_amd64_xasmgen *xasmgen) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "ret\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_fstcw(struct kefir_amd64_xasmgen *xasmgen) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "fstcw\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_call(struct kefir_amd64_xasmgen *xasmgen,
                                       const struct kefir_amd64_xasmgen_operand *op1) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "call ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_pextrq(struct kefir_amd64_xasmgen *xasmgen,
                                         const struct kefir_amd64_xasmgen_operand *op1,
                                         const struct kefir_amd64_xasmgen_operand *op2,
                                         const struct kefir_amd64_xasmgen_operand *op3) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op3 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "pextrq ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op3));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_pinsrq(struct kefir_amd64_xasmgen *xasmgen,
                                         const struct kefir_amd64_xasmgen_operand *op1,
                                         const struct kefir_amd64_xasmgen_operand *op2,
                                         const struct kefir_amd64_xasmgen_operand *op3) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op3 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "pinsrq ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op3));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_add(struct kefir_amd64_xasmgen *xasmgen,
                                      const struct kefir_amd64_xasmgen_operand *op1,
                                      const struct kefir_amd64_xasmgen_operand *op2) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "add ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_cmp(struct kefir_amd64_xasmgen *xasmgen,
                                      const struct kefir_amd64_xasmgen_operand *op1,
                                      const struct kefir_amd64_xasmgen_operand *op2) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "cmp ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_sub(struct kefir_amd64_xasmgen *xasmgen,
                                      const struct kefir_amd64_xasmgen_operand *op1,
                                      const struct kefir_amd64_xasmgen_operand *op2) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "sub ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_cld(struct kefir_amd64_xasmgen *xasmgen) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "cld\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_and(struct kefir_amd64_xasmgen *xasmgen,
                                      const struct kefir_amd64_xasmgen_operand *op1,
                                      const struct kefir_amd64_xasmgen_operand *op2) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "and ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_shl(struct kefir_amd64_xasmgen *xasmgen,
                                      const struct kefir_amd64_xasmgen_operand *op1,
                                      const struct kefir_amd64_xasmgen_operand *op2) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "shl ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_shr(struct kefir_amd64_xasmgen *xasmgen,
                                      const struct kefir_amd64_xasmgen_operand *op1,
                                      const struct kefir_amd64_xasmgen_operand *op2) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "shr ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_movd(struct kefir_amd64_xasmgen *xasmgen,
                                       const struct kefir_amd64_xasmgen_operand *op1,
                                       const struct kefir_amd64_xasmgen_operand *op2) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "movd ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_fstp(struct kefir_amd64_xasmgen *xasmgen,
                                       const struct kefir_amd64_xasmgen_operand *op1) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "fstp ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_fld(struct kefir_amd64_xasmgen *xasmgen,
                                      const struct kefir_amd64_xasmgen_operand *op1) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    REQUIRE_OK(amd64_ident(xasmgen));
    fprintf(payload->output, "fld ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

kefir_result_t kefir_amd64_xasmgen_init(struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen, FILE *output) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 xasmgen"));
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid output FILE"));

    struct xasmgen_payload *payload = KEFIR_MALLOC(mem, sizeof(struct xasmgen_payload));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocator amd64 xasmgen payload"));
    payload->output = output;

    xasmgen->payload = payload;

    xasmgen->settings.enable_comments = true;
    xasmgen->settings.enable_identation = true;

    xasmgen->prologue = amd64_prologue;
    xasmgen->close = amd64_close;
    xasmgen->newline = amd64_newline;
    xasmgen->comment = amd64_comment;
    xasmgen->label = amd64_label;
    xasmgen->global = amd64_global;
    xasmgen->external = amd64_external;
    xasmgen->section = amd64_section;
    xasmgen->align = amd64_align;
    xasmgen->data = amd64_data;
    xasmgen->zerodata = amd64_zerodata;
    xasmgen->bindata = amd64_bindata;

    xasmgen->instr.add = amd64_instr_add;
    xasmgen->instr.and = amd64_instr_and;
    xasmgen->instr.call = amd64_instr_call;
    xasmgen->instr.cld = amd64_instr_cld;
    xasmgen->instr.cmp = amd64_instr_cmp;
    xasmgen->instr.fld = amd64_instr_fld;
    xasmgen->instr.fstcw = amd64_instr_fstcw;
    xasmgen->instr.fstp = amd64_instr_fstp;
    xasmgen->instr.ja = amd64_instr_ja;
    xasmgen->instr.jmp = amd64_instr_jmp;
    xasmgen->instr.lea = amd64_instr_lea;
    xasmgen->instr.mov = amd64_instr_mov;
    xasmgen->instr.movabs = amd64_instr_movabs;
    xasmgen->instr.movd = amd64_instr_movd;
    xasmgen->instr.movq = amd64_instr_movq;
    xasmgen->instr.movsb = amd64_instr_movsb;
    xasmgen->instr.or = amd64_instr_or;
    xasmgen->instr.pextrq = amd64_instr_pextrq;
    xasmgen->instr.pinsrq = amd64_instr_pinsrq;
    xasmgen->instr.pop = amd64_instr_pop;
    xasmgen->instr.push = amd64_instr_push;
    xasmgen->instr.ret = amd64_instr_ret;
    xasmgen->instr.shl = amd64_instr_shl;
    xasmgen->instr.shr = amd64_instr_shr;
    xasmgen->instr.sub = amd64_instr_sub;
    return KEFIR_OK;
}

const struct kefir_amd64_xasmgen_operand operand_regs[] = {
    [KEFIR_AMD64_XASMGEN_REGISTER_EAX] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                          .reg = KEFIR_AMD64_XASMGEN_REGISTER_EAX},
    [KEFIR_AMD64_XASMGEN_REGISTER_RAX] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                          .reg = KEFIR_AMD64_XASMGEN_REGISTER_RAX},
    [KEFIR_AMD64_XASMGEN_REGISTER_RBX] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                          .reg = KEFIR_AMD64_XASMGEN_REGISTER_RBX},
    [KEFIR_AMD64_XASMGEN_REGISTER_RCX] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                          .reg = KEFIR_AMD64_XASMGEN_REGISTER_RCX},
    [KEFIR_AMD64_XASMGEN_REGISTER_RDX] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                          .reg = KEFIR_AMD64_XASMGEN_REGISTER_RDX},
    [KEFIR_AMD64_XASMGEN_REGISTER_RSI] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                          .reg = KEFIR_AMD64_XASMGEN_REGISTER_RSI},
    [KEFIR_AMD64_XASMGEN_REGISTER_RDI] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                          .reg = KEFIR_AMD64_XASMGEN_REGISTER_RDI},
    [KEFIR_AMD64_XASMGEN_REGISTER_RSP] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                          .reg = KEFIR_AMD64_XASMGEN_REGISTER_RSP},
    [KEFIR_AMD64_XASMGEN_REGISTER_RBP] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                          .reg = KEFIR_AMD64_XASMGEN_REGISTER_RBP},
    [KEFIR_AMD64_XASMGEN_REGISTER_R8] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                         .reg = KEFIR_AMD64_XASMGEN_REGISTER_R8},
    [KEFIR_AMD64_XASMGEN_REGISTER_R9] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                         .reg = KEFIR_AMD64_XASMGEN_REGISTER_R9},
    [KEFIR_AMD64_XASMGEN_REGISTER_R10] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                          .reg = KEFIR_AMD64_XASMGEN_REGISTER_R10},
    [KEFIR_AMD64_XASMGEN_REGISTER_R11] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                          .reg = KEFIR_AMD64_XASMGEN_REGISTER_R11},
    [KEFIR_AMD64_XASMGEN_REGISTER_R12] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                          .reg = KEFIR_AMD64_XASMGEN_REGISTER_R12},
    [KEFIR_AMD64_XASMGEN_REGISTER_R13] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                          .reg = KEFIR_AMD64_XASMGEN_REGISTER_R13},
    [KEFIR_AMD64_XASMGEN_REGISTER_R14] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                          .reg = KEFIR_AMD64_XASMGEN_REGISTER_R14},
    [KEFIR_AMD64_XASMGEN_REGISTER_R15] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                          .reg = KEFIR_AMD64_XASMGEN_REGISTER_R15},
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM0] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                           .reg = KEFIR_AMD64_XASMGEN_REGISTER_XMM0},
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM1] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                           .reg = KEFIR_AMD64_XASMGEN_REGISTER_XMM1},
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM2] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                           .reg = KEFIR_AMD64_XASMGEN_REGISTER_XMM2},
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM3] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                           .reg = KEFIR_AMD64_XASMGEN_REGISTER_XMM3},
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM4] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                           .reg = KEFIR_AMD64_XASMGEN_REGISTER_XMM4},
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM5] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                           .reg = KEFIR_AMD64_XASMGEN_REGISTER_XMM5},
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM6] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                           .reg = KEFIR_AMD64_XASMGEN_REGISTER_XMM6},
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM7] = {.klass = KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
                                           .reg = KEFIR_AMD64_XASMGEN_REGISTER_XMM7}};

const struct kefir_amd64_xasmgen_operand *kefir_amd64_xasmgen_operand_reg(kefir_amd64_xasmgen_register_t reg) {
    return &operand_regs[reg];
}

const struct kefir_amd64_xasmgen_operand *kefir_amd64_xasmgen_operand_imm(struct kefir_amd64_xasmgen_operand *op,
                                                                          kefir_int64_t imm) {
    REQUIRE(op != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_IMMEDIATE;
    op->imm = imm;
    return op;
}

const struct kefir_amd64_xasmgen_operand *kefir_amd64_xasmgen_operand_immu(struct kefir_amd64_xasmgen_operand *op,
                                                                           kefir_uint64_t immu) {
    REQUIRE(op != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_IMMEDIATE_UNSIGNED;
    op->immu = immu;
    return op;
}

const struct kefir_amd64_xasmgen_operand *kefir_amd64_xasmgen_operand_label(struct kefir_amd64_xasmgen_operand *op,
                                                                            const char *label) {
    REQUIRE(op != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_LABEL;
    op->label = label;
    return op;
}

const struct kefir_amd64_xasmgen_operand *kefir_amd64_xasmgen_operand_symbol(struct kefir_amd64_xasmgen_operand *op,
                                                                             const char *symbol) {
    REQUIRE(op != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_SYMBOL;
    op->symbol = symbol;
    return op;
}

const struct kefir_amd64_xasmgen_operand *kefir_amd64_xasmgen_operand_indirect(
    struct kefir_amd64_xasmgen_operand *op, kefir_amd64_xasmgen_indirection_pointer_type_t type,
    const struct kefir_amd64_xasmgen_operand *base, kefir_int64_t offset) {
    REQUIRE(op != NULL, NULL);
    REQUIRE(base != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_INDIRECTION;
    op->indirection.type = type;
    op->indirection.base = base;
    op->indirection.offset = offset;
    return op;
}

const struct kefir_amd64_xasmgen_operand *kefir_amd64_xasmgen_operand_offset(
    struct kefir_amd64_xasmgen_operand *op, const struct kefir_amd64_xasmgen_operand *base, kefir_int64_t offset) {
    REQUIRE(op != NULL, NULL);
    REQUIRE(base != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_OFFSET;
    op->offset.base = base;
    op->offset.offset = offset;
    return op;
}

const struct kefir_amd64_xasmgen_operand *kefir_amd64_xasmgen_operand_segment(
    struct kefir_amd64_xasmgen_operand *op, kefir_amd64_xasmgen_segment_register_t segment,
    const struct kefir_amd64_xasmgen_operand *base) {
    REQUIRE(op != NULL, NULL);
    REQUIRE(base != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_SEGMENT;
    op->segment.segment = segment;
    op->segment.base = base;
    return op;
}

const struct kefir_amd64_xasmgen_operand *kefir_amd64_xasmgen_operand_pointer(
    struct kefir_amd64_xasmgen_operand *op, kefir_amd64_xasmgen_indirection_pointer_type_t type,
    const struct kefir_amd64_xasmgen_operand *base) {
    REQUIRE(op != NULL, NULL);
    REQUIRE(base != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_POINTER;
    op->indirection.type = type;
    op->indirection.base = base;
    return op;
}

const struct kefir_amd64_xasmgen_operand *kefir_amd64_xasmgen_operand_rip_indirection(
    struct kefir_amd64_xasmgen_operand *op, const char *identifier) {
    REQUIRE(op != NULL, NULL);
    REQUIRE(identifier != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_RIP_INDIRECTION;
    op->rip_indirection.identifier = identifier;
    return op;
}

const struct kefir_amd64_xasmgen_operand *kefir_amd64_xasmgen_operand_string_literal(
    struct kefir_amd64_xasmgen_operand *op, const char *content, kefir_size_t length) {
    REQUIRE(op != NULL, NULL);
    REQUIRE(content != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_STRING_LITERAL;
    op->string_literal.content = content;
    op->string_literal.length = length;
    return op;
}

const char *kefir_amd64_xasmgen_helpers_format(struct kefir_amd64_xasmgen_helpers *helpers, const char *fmt, ...) {
    REQUIRE(helpers != NULL, NULL);

    va_list args;
    va_start(args, fmt);
    vsnprintf(helpers->buffer, KEFIR_AMD64_XASMGEN_HELPERS_BUFFER_LENGTH - 1, fmt, args);
    va_end(args);
    return helpers->buffer;
}
