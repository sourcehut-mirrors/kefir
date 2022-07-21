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

#include "kefir/codegen/amd64/xasmgen/xasmgen.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <stdarg.h>

struct xasmgen_payload {
    FILE *output;
};

// kefir_result_t (*prologue)(struct kefir_amd64_xasmgen *);

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

    fprintf(payload->output, "# ");
    va_list args;
    va_start(args, format);
    vfprintf(payload->output, format, args);
    va_end(args);
    fprintf(payload->output, "\n");
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

static const char *register_literals[] = {
    [KEFIR_AMD64_XASMGEN_REGISTER_RAX] = "rax",   [KEFIR_AMD64_XASMGEN_REGISTER_RBX] = "rbx",
    [KEFIR_AMD64_XASMGEN_REGISTER_RCX] = "rcx",   [KEFIR_AMD64_XASMGEN_REGISTER_RDX] = "rdx",
    [KEFIR_AMD64_XASMGEN_REGISTER_RSI] = "rsi",   [KEFIR_AMD64_XASMGEN_REGISTER_RDI] = "rdi",
    [KEFIR_AMD64_XASMGEN_REGISTER_RSP] = "rsp",   [KEFIR_AMD64_XASMGEN_REGISTER_RBP] = "rbp",
    [KEFIR_AMD64_XASMGEN_REGISTER_R8] = "r8",     [KEFIR_AMD64_XASMGEN_REGISTER_R9] = "r9",
    [KEFIR_AMD64_XASMGEN_REGISTER_R10] = "r10",   [KEFIR_AMD64_XASMGEN_REGISTER_R11] = "r11",
    [KEFIR_AMD64_XASMGEN_REGISTER_R12] = "r12",   [KEFIR_AMD64_XASMGEN_REGISTER_R13] = "r13",
    [KEFIR_AMD64_XASMGEN_REGISTER_R14] = "r14",   [KEFIR_AMD64_XASMGEN_REGISTER_R15] = "r15",
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM0] = "xmm0", [KEFIR_AMD64_XASMGEN_REGISTER_XMM1] = "xmm1",
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM2] = "xmm2", [KEFIR_AMD64_XASMGEN_REGISTER_XMM3] = "xmm3",
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM4] = "xmm4", [KEFIR_AMD64_XASMGEN_REGISTER_XMM5] = "xmm5",
    [KEFIR_AMD64_XASMGEN_REGISTER_XMM6] = "xmm6", [KEFIR_AMD64_XASMGEN_REGISTER_XMM7] = "xmm7"};

static kefir_result_t amd64_format_operand(struct kefir_amd64_xasmgen *xasmgen,
                                           const struct kefir_amd64_xasmgen_operand *op) {
    UNUSED(xasmgen);
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);
    switch (op->klass) {
        case KEFIR_AMD64_XASMGEN_OPERAND_IMMEDIATE:
            fprintf(payload->output, KEFIR_INT64_FMT, op->imm);
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_REGISTER:
            fprintf(payload->output, "%%%s", register_literals[op->reg]);
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_LABEL:
            fprintf(payload->output, "%s", op->label);
            break;

        case KEFIR_AMD64_XASMGEN_OPERAND_INDIRECTION:
            switch (op->indirection.type) {
                case KEFIR_AMD64_XASMGEN_INDIRECTION_POINTER_NONE:
                    // Intentionally left blank
                    break;

                case KEFIR_AMD64_XASMGEN_INDIRECTION_POINTER_DWORD:
                    fprintf(payload->output, "DWORD PTR ");
                    break;

                case KEFIR_AMD64_XASMGEN_INDIRECTION_POINTER_QWORD:
                    fprintf(payload->output, "QWORD PTR ");
                    break;

                case KEFIR_AMD64_XASMGEN_INDIRECTION_POINTER_TBYTE:
                    fprintf(payload->output, "TBYTE PTR ");
                    break;
            }
            fprintf(payload->output, "[");
            REQUIRE_OK(amd64_format_operand(xasmgen, op->indirection.base));
            if (op->indirection.offset > 0) {
                fprintf(payload->output, " + " KEFIR_INT64_FMT, op->indirection.offset);
            } else if (op->indirection.offset < 0) {
                fprintf(payload->output, " - " KEFIR_INT64_FMT, -op->indirection.offset);
            }
            fprintf(payload->output, "]");
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_push(struct kefir_amd64_xasmgen *xasmgen,
                                       const struct kefir_amd64_xasmgen_operand *op1) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, "\tpush ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_pop(struct kefir_amd64_xasmgen *xasmgen,
                                      const struct kefir_amd64_xasmgen_operand *op1) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, "\tpop ");
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

    fprintf(payload->output, "\tmov ");
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

    fprintf(payload->output, "\tmovabs ");
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

    fprintf(payload->output, "\tor ");
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

    fprintf(payload->output, "\tlea ");
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

    fprintf(payload->output, "\tmovq ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_movsb(struct kefir_amd64_xasmgen *xasmgen,
                                        const struct kefir_amd64_xasmgen_operand *op1,
                                        const struct kefir_amd64_xasmgen_operand *op2) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, "\tmovsb ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_jmp(struct kefir_amd64_xasmgen *xasmgen,
                                      const struct kefir_amd64_xasmgen_operand *op1) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, "\tjmp ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_ja(struct kefir_amd64_xasmgen *xasmgen,
                                     const struct kefir_amd64_xasmgen_operand *op1) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, "\tja ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_ret(struct kefir_amd64_xasmgen *xasmgen) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, "\tret\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_fstcw(struct kefir_amd64_xasmgen *xasmgen) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, "\tfstcw\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_call(struct kefir_amd64_xasmgen *xasmgen,
                                       const struct kefir_amd64_xasmgen_operand *op1) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, "\tcall ");
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

    fprintf(payload->output, "\tpextrq ");
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

    fprintf(payload->output, "\tpinsrq ");
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

    fprintf(payload->output, "\tadd ");
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

    fprintf(payload->output, "\tcmp ");
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

    fprintf(payload->output, "\tsub ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, ", ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op2));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_cld(struct kefir_amd64_xasmgen *xasmgen) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, "\tcld\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_and(struct kefir_amd64_xasmgen *xasmgen,
                                      const struct kefir_amd64_xasmgen_operand *op1,
                                      const struct kefir_amd64_xasmgen_operand *op2) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    REQUIRE(op2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, "\tand ");
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

    fprintf(payload->output, "\tshl ");
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

    fprintf(payload->output, "\tshr ");
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

    fprintf(payload->output, "\tmovd ");
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

    fprintf(payload->output, "\tfstp ");
    REQUIRE_OK(amd64_format_operand(xasmgen, op1));
    fprintf(payload->output, "\n");
    return KEFIR_OK;
}

static kefir_result_t amd64_instr_fld(struct kefir_amd64_xasmgen *xasmgen,
                                      const struct kefir_amd64_xasmgen_operand *op1) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator"));
    REQUIRE(op1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 assembly generator operand"));
    ASSIGN_DECL_CAST(struct xasmgen_payload *, payload, xasmgen->payload);

    fprintf(payload->output, "\tfld ");
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

    xasmgen->payload = payload;

    xasmgen->prologue = amd64_prologue;
    xasmgen->close = amd64_close;
    xasmgen->newline = amd64_newline;
    xasmgen->comment = amd64_comment;
    xasmgen->label = amd64_label;
    xasmgen->global = amd64_global;
    xasmgen->external = amd64_external;
    xasmgen->section = amd64_section;

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

const struct kefir_amd64_xasmgen_operand *kefir_amd64_xasmgen_operand_label(struct kefir_amd64_xasmgen_operand *op,
                                                                            const char *label) {
    REQUIRE(op != NULL, NULL);
    op->klass = KEFIR_AMD64_XASMGEN_OPERAND_LABEL;
    op->label = label;
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
