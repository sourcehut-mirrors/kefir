/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#ifndef KEFIR_TARGET_ASM_AMD64_XASMGEN_H_
#define KEFIR_TARGET_ASM_AMD64_XASMGEN_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"
#include <stdio.h>

typedef enum kefir_asm_amd64_xasmgen_register {
    // 8-bit
    KEFIR_AMD64_XASMGEN_REGISTER_AL,
    KEFIR_AMD64_XASMGEN_REGISTER_BL,
    KEFIR_AMD64_XASMGEN_REGISTER_CL,
    KEFIR_AMD64_XASMGEN_REGISTER_DL,
    KEFIR_AMD64_XASMGEN_REGISTER_SIL,
    KEFIR_AMD64_XASMGEN_REGISTER_DIL,
    KEFIR_AMD64_XASMGEN_REGISTER_SPL,
    KEFIR_AMD64_XASMGEN_REGISTER_BPL,
    KEFIR_AMD64_XASMGEN_REGISTER_R8B,
    KEFIR_AMD64_XASMGEN_REGISTER_R9B,
    KEFIR_AMD64_XASMGEN_REGISTER_R10B,
    KEFIR_AMD64_XASMGEN_REGISTER_R11B,
    KEFIR_AMD64_XASMGEN_REGISTER_R12B,
    KEFIR_AMD64_XASMGEN_REGISTER_R13B,
    KEFIR_AMD64_XASMGEN_REGISTER_R14B,
    KEFIR_AMD64_XASMGEN_REGISTER_R15B,
    // 16-bit
    KEFIR_AMD64_XASMGEN_REGISTER_AX,
    KEFIR_AMD64_XASMGEN_REGISTER_BX,
    KEFIR_AMD64_XASMGEN_REGISTER_CX,
    KEFIR_AMD64_XASMGEN_REGISTER_DX,
    KEFIR_AMD64_XASMGEN_REGISTER_SI,
    KEFIR_AMD64_XASMGEN_REGISTER_DI,
    KEFIR_AMD64_XASMGEN_REGISTER_SP,
    KEFIR_AMD64_XASMGEN_REGISTER_BP,
    KEFIR_AMD64_XASMGEN_REGISTER_R8W,
    KEFIR_AMD64_XASMGEN_REGISTER_R9W,
    KEFIR_AMD64_XASMGEN_REGISTER_R10W,
    KEFIR_AMD64_XASMGEN_REGISTER_R11W,
    KEFIR_AMD64_XASMGEN_REGISTER_R12W,
    KEFIR_AMD64_XASMGEN_REGISTER_R13W,
    KEFIR_AMD64_XASMGEN_REGISTER_R14W,
    KEFIR_AMD64_XASMGEN_REGISTER_R15W,
    // 32-bit
    KEFIR_AMD64_XASMGEN_REGISTER_EAX,
    KEFIR_AMD64_XASMGEN_REGISTER_EBX,
    KEFIR_AMD64_XASMGEN_REGISTER_ECX,
    KEFIR_AMD64_XASMGEN_REGISTER_EDX,
    KEFIR_AMD64_XASMGEN_REGISTER_ESI,
    KEFIR_AMD64_XASMGEN_REGISTER_EDI,
    KEFIR_AMD64_XASMGEN_REGISTER_ESP,
    KEFIR_AMD64_XASMGEN_REGISTER_EBP,
    KEFIR_AMD64_XASMGEN_REGISTER_R8D,
    KEFIR_AMD64_XASMGEN_REGISTER_R9D,
    KEFIR_AMD64_XASMGEN_REGISTER_R10D,
    KEFIR_AMD64_XASMGEN_REGISTER_R11D,
    KEFIR_AMD64_XASMGEN_REGISTER_R12D,
    KEFIR_AMD64_XASMGEN_REGISTER_R13D,
    KEFIR_AMD64_XASMGEN_REGISTER_R14D,
    KEFIR_AMD64_XASMGEN_REGISTER_R15D,
    // 64-bit
    KEFIR_AMD64_XASMGEN_REGISTER_RAX,
    KEFIR_AMD64_XASMGEN_REGISTER_RBX,
    KEFIR_AMD64_XASMGEN_REGISTER_RCX,
    KEFIR_AMD64_XASMGEN_REGISTER_RDX,
    KEFIR_AMD64_XASMGEN_REGISTER_RSI,
    KEFIR_AMD64_XASMGEN_REGISTER_RDI,
    KEFIR_AMD64_XASMGEN_REGISTER_RSP,
    KEFIR_AMD64_XASMGEN_REGISTER_RBP,
    KEFIR_AMD64_XASMGEN_REGISTER_R8,
    KEFIR_AMD64_XASMGEN_REGISTER_R9,
    KEFIR_AMD64_XASMGEN_REGISTER_R10,
    KEFIR_AMD64_XASMGEN_REGISTER_R11,
    KEFIR_AMD64_XASMGEN_REGISTER_R12,
    KEFIR_AMD64_XASMGEN_REGISTER_R13,
    KEFIR_AMD64_XASMGEN_REGISTER_R14,
    KEFIR_AMD64_XASMGEN_REGISTER_R15,
    // Floating-point
    KEFIR_AMD64_XASMGEN_REGISTER_XMM0,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM1,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM2,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM3,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM4,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM5,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM6,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM7,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM8,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM9,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM10,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM11,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM12,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM13,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM14,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM15
} kefir_asm_amd64_xasmgen_register_t;

kefir_result_t kefir_asm_amd64_xasmgen_register8(kefir_asm_amd64_xasmgen_register_t,
                                                 kefir_asm_amd64_xasmgen_register_t *);
kefir_result_t kefir_asm_amd64_xasmgen_register16(kefir_asm_amd64_xasmgen_register_t,
                                                  kefir_asm_amd64_xasmgen_register_t *);
kefir_result_t kefir_asm_amd64_xasmgen_register32(kefir_asm_amd64_xasmgen_register_t,
                                                  kefir_asm_amd64_xasmgen_register_t *);
kefir_result_t kefir_asm_amd64_xasmgen_register64(kefir_asm_amd64_xasmgen_register_t,
                                                  kefir_asm_amd64_xasmgen_register_t *);
kefir_bool_t kefir_asm_amd64_xasmgen_register_is_floating_point(kefir_asm_amd64_xasmgen_register_t);
kefir_bool_t kefir_asm_amd64_xasmgen_register_is_wide(kefir_asm_amd64_xasmgen_register_t, kefir_size_t);
kefir_result_t kefir_asm_amd64_xasmgen_register_widest(kefir_asm_amd64_xasmgen_register_t,
                                                       kefir_asm_amd64_xasmgen_register_t *);
const char *kefir_asm_amd64_xasmgen_register_symbolic_name(kefir_asm_amd64_xasmgen_register_t);
kefir_result_t kefir_asm_amd64_xasmgen_register_from_symbolic_name(const char *, kefir_asm_amd64_xasmgen_register_t *);

typedef enum kefir_asm_amd64_xasmgen_segment_register {
    KEFIR_AMD64_XASMGEN_SEGMENT_FS
} kefir_asm_amd64_xasmgen_segment_register_t;

typedef enum kefir_asm_amd64_xasmgen_operand_class {
    KEFIR_AMD64_XASMGEN_OPERAND_IMMEDIATE,
    KEFIR_AMD64_XASMGEN_OPERAND_IMMEDIATE_UNSIGNED,
    KEFIR_AMD64_XASMGEN_OPERAND_REGISTER,
    KEFIR_AMD64_XASMGEN_OPERAND_LABEL,
    KEFIR_AMD64_XASMGEN_OPERAND_INDIRECTION,
    KEFIR_AMD64_XASMGEN_OPERAND_RIP_INDIRECTION,
    KEFIR_AMD64_XASMGEN_OPERAND_OFFSET,
    KEFIR_AMD64_XASMGEN_OPERAND_SEGMENT,
    KEFIR_AMD64_XASMGEN_OPERAND_POINTER,
    KEFIR_AMD64_XASMGEN_OPERAND_STRING_LITERAL,
    KEFIR_AMD64_XASMGEN_OPERAND_FPU_REGISTER
} kefir_asm_amd64_xasmgen_operand_class_t;

typedef enum kefir_asm_amd64_xasmgen_pointer_type {
    KEFIR_AMD64_XASMGEN_POINTER_BYTE,
    KEFIR_AMD64_XASMGEN_POINTER_WORD,
    KEFIR_AMD64_XASMGEN_POINTER_DWORD,
    KEFIR_AMD64_XASMGEN_POINTER_QWORD,
    KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
    KEFIR_AMD64_XASMGEN_POINTER_XMMWORD,
    KEFIR_AMD64_XASMGEN_POINTER_FP_SINGLE,
    KEFIR_AMD64_XASMGEN_POINTER_FP_DOUBLE
} kefir_asm_amd64_xasmgen_pointer_type_t;

typedef struct kefir_asm_amd64_xasmgen_operand {
    kefir_asm_amd64_xasmgen_operand_class_t klass;

    union {
        kefir_int64_t imm;
        kefir_uint64_t immu;
        kefir_asm_amd64_xasmgen_register_t reg;
        const char *label;
        struct {
            const struct kefir_asm_amd64_xasmgen_operand *base;
            kefir_int64_t displacement;
        } indirection;
        struct {
            const struct kefir_asm_amd64_xasmgen_operand *base;
            kefir_int64_t offset;
        } offset;
        struct {
            kefir_asm_amd64_xasmgen_segment_register_t segment;
            const struct kefir_asm_amd64_xasmgen_operand *base;
        } segment;

        struct {
            kefir_asm_amd64_xasmgen_pointer_type_t type;
            const struct kefir_asm_amd64_xasmgen_operand *base;
        } pointer;
        struct {
            const char *content;
            kefir_size_t length;
        } string_literal;
        kefir_uint64_t fpu_register;
    };
} kefir_asm_amd64_xasmgen_operand_t;

typedef enum kefir_asm_amd64_xasmgen_data_type {
    KEFIR_AMD64_XASMGEN_DATA_BYTE,
    KEFIR_AMD64_XASMGEN_DATA_WORD,
    KEFIR_AMD64_XASMGEN_DATA_DOUBLE,
    KEFIR_AMD64_XASMGEN_DATA_QUAD,
    KEFIR_AMD64_XASMGEN_DATA_ASCII
} kefir_asm_amd64_xasmgen_data_type_t;

typedef enum kefir_asm_amd64_xasmgen_syntax {
    KEFIR_AMD64_XASMGEN_SYNTAX_INTEL_NOPREFIX,
    KEFIR_AMD64_XASMGEN_SYNTAX_INTEL_PREFIX,
    KEFIR_AMD64_XASMGEN_SYNTAX_ATT
} kefir_asm_amd64_xasmgen_syntax_t;

// clang-format off
#define KEFIR_AMD64_XASMGEN_OPCODE_DEFS(_opcode, _separator) \
    _opcode(data16, "data16", 0, prefix) _separator \
    _opcode(rexW, "rex.W", 0, prefix) _separator \
    \
    _opcode(push, "push", 1, normal) _separator \
    _opcode(pop, "pop", 1, normal) _separator \
    _opcode(pushfq, "pushfq", 0, normal) _separator \
    _opcode(popfq, "popfq", 0, normal) _separator \
    \
    _opcode(jmp, "jmp", 1, branch) _separator \
    _opcode(ja, "ja", 1, branch) _separator \
    _opcode(jae, "jae", 1, branch) _separator \
    _opcode(jb, "jb", 1, branch) _separator \
    _opcode(jbe, "jbe", 1, branch) _separator \
    _opcode(jz, "jz", 1, branch) _separator \
    _opcode(je, "je", 1, branch) _separator \
    _opcode(jne, "jne", 1, branch) _separator \
    _opcode(jg, "jg", 1, branch) _separator \
    _opcode(jge, "jge", 1, branch) _separator \
    _opcode(jl, "jl", 1, branch) _separator \
    _opcode(jle, "jle", 1, branch) _separator \
    _opcode(js, "js", 1, branch) _separator \
    _opcode(jns, "jns", 1, branch) _separator \
    _opcode(jp, "jp", 1, branch) _separator \
    _opcode(jnp, "jnp", 1, branch) _separator \
    _opcode(call, "call", 1, branch) _separator \
    _opcode(ret, "ret", 0, normal) _separator \
    \
    _opcode(mov, "mov", 2, normal) _separator \
    _opcode(movsx, "movsx", 2, normal) _separator \
    _opcode(movzx, "movzx", 2, normal) _separator \
    _opcode(movabs, "movabs", 2, normal) _separator \
    _opcode(lea, "lea", 2, normal) _separator \
    \
    _opcode(cmovl, "cmovl", 2, normal) _separator \
    _opcode(cmovne, "cmovne", 2, normal) _separator \
    \
    _opcode(movsb, "movsb", 0, repeated) _separator \
    _opcode(stosb, "stosb", 0, repeated) _separator \
    \
    _opcode(add, "add", 2, normal) _separator \
    _opcode(sub, "sub", 2, normal) _separator \
    _opcode(imul, "imul", 2, normal) _separator \
    _opcode(idiv, "idiv", 1, normal) _separator \
    _opcode(div, "div", 1, normal) _separator \
    _opcode(and, "and", 2, normal) _separator \
    _opcode(or, "or", 2, normal) _separator \
    _opcode(xor, "xor", 2, normal) _separator \
    _opcode(shl, "shl", 2, normal) _separator \
    _opcode(shr, "shr", 2, normal) _separator \
    _opcode(sar, "sar", 2, normal) _separator \
    _opcode(not, "not", 1, normal) _separator \
    _opcode(neg, "neg", 1, normal) _separator \
    \
    _opcode(cqo, "cqo", 0, normal) _separator \
    _opcode(cvtsi2ss, "cvtsi2ss", 2, normal) _separator \
    _opcode(cvtsi2sd, "cvtsi2sd", 2, normal) _separator \
    _opcode(cvttss2si, "cvttss2si", 2, normal) _separator \
    _opcode(cvttsd2si, "cvttsd2si", 2, normal) _separator \
    _opcode(cvtsd2ss, "cvtsd2ss", 2, normal) _separator \
    _opcode(cvtss2sd, "cvtss2sd", 2, normal) _separator \
    \
    _opcode(cmp, "cmp", 2, normal) _separator \
    _opcode(cld, "cld", 0, normal) _separator \
    _opcode(test, "test", 2, normal) _separator \
    _opcode(sete, "sete", 1, normal) _separator \
    _opcode(setg, "setg", 1, normal) _separator \
    _opcode(setge, "setge", 1, normal) _separator \
    _opcode(setl, "setl", 1, normal) _separator \
    _opcode(setle, "setle", 1, normal) _separator \
    _opcode(seta, "seta", 1, normal) _separator \
    _opcode(setae, "setae", 1, normal) _separator \
    _opcode(setb, "setb", 1, normal) _separator \
    _opcode(setbe, "setbe", 1, normal) _separator \
    _opcode(setne, "setne", 1, normal) _separator \
    _opcode(setnp, "setnp", 1, normal) _separator \
    \
    _opcode(fstp, "fstp", 1, normal) _separator \
    _opcode(fld, "fld", 1, normal) _separator \
    _opcode(fldz, "fldz", 0, normal) _separator \
    _opcode(fild, "fild", 1, normal) _separator \
    _opcode(fstcw, "fstcw", 1, normal) _separator \
    _opcode(fldcw, "fldcw", 1, normal) _separator \
    _opcode(faddp, "faddp", 0, normal) _separator \
    _opcode(fadd, "fadd", 1, normal) _separator \
    _opcode(fsubp, "fsubp", 0, normal) _separator \
    _opcode(fmulp, "fmulp", 0, normal) _separator \
    _opcode(fdivp, "fdivp", 0, normal) _separator \
    _opcode(fchs, "fchs", 0, normal) _separator \
    \
    _opcode(fucomip, "fucomip", 0, normal) _separator \
    _opcode(fcomip, "fcomip", 0, normal) _separator \
    \
    _opcode(pextrq, "pextrq", 3, normal) _separator \
    _opcode(pinsrq, "pinsrq", 3, normal) _separator \
    _opcode(movd, "movd", 2, normal) _separator \
    _opcode(movq, "movq", 2, normal) _separator \
    _opcode(movdqu, "movdqu", 2, normal) _separator \
    _opcode(stmxcsr, "stmxcsr", 1, normal) _separator \
    _opcode(ldmxcsr, "ldmxcsr", 1, normal) _separator \
    \
    _opcode(pxor, "pxor", 2, normal) _separator \
    _opcode(xorps, "xorps", 2, normal) _separator \
    _opcode(xorpd, "xorpd", 2, normal) _separator \
    _opcode(addss, "addss", 2, normal) _separator \
    _opcode(addsd, "addsd", 2, normal) _separator \
    _opcode(subss, "subss", 2, normal) _separator \
    _opcode(subsd, "subsd", 2, normal) _separator \
    _opcode(mulss, "mulss", 2, normal) _separator \
    _opcode(mulsd, "mulsd", 2, normal) _separator \
    _opcode(divss, "divss", 2, normal) _separator \
    _opcode(divsd, "divsd", 2, normal) _separator \
    \
    _opcode(ucomiss, "ucomiss", 2, normal) _separator \
    _opcode(ucomisd, "ucomisd", 2, normal) _separator \
    _opcode(comiss, "comiss", 2, normal) _separator \
    _opcode(comisd, "comisd", 2, normal)
// clang-format on

typedef struct kefir_amd64_xasmgen {
    kefir_result_t (*prologue)(struct kefir_amd64_xasmgen *);
    kefir_result_t (*close)(struct kefir_mem *, struct kefir_amd64_xasmgen *);
    kefir_result_t (*newline)(struct kefir_amd64_xasmgen *, unsigned int);
    kefir_result_t (*comment)(struct kefir_amd64_xasmgen *, const char *, ...);
    kefir_result_t (*label)(struct kefir_amd64_xasmgen *, const char *, ...);
    kefir_result_t (*global)(struct kefir_amd64_xasmgen *, const char *, ...);
    kefir_result_t (*external)(struct kefir_amd64_xasmgen *, const char *, ...);
    kefir_result_t (*section)(struct kefir_amd64_xasmgen *, const char *);
    kefir_result_t (*align)(struct kefir_amd64_xasmgen *, kefir_size_t);
    kefir_result_t (*data)(struct kefir_amd64_xasmgen *, kefir_asm_amd64_xasmgen_data_type_t, kefir_size_t, ...);
    kefir_result_t (*zerodata)(struct kefir_amd64_xasmgen *, kefir_size_t);
    kefir_result_t (*bindata)(struct kefir_amd64_xasmgen *, kefir_asm_amd64_xasmgen_data_type_t, const void *,
                              kefir_size_t);
    kefir_result_t (*inline_assembly)(struct kefir_amd64_xasmgen *, const char *);
    kefir_result_t (*format_operand)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *,
                                     char *, kefir_size_t);

    struct {
#define OPCODE_DEF(_id, _mnemonic, _argc, _class) OPCODE_DEF_##_class(_id, _argc)
#define OPCODE_DEF_prefix(_id, _argc) OPCODE_DEF##_argc(_id)
#define OPCODE_DEF_normal(_id, _argc) OPCODE_DEF##_argc(_id)
#define OPCODE_DEF_branch(_id, _argc) OPCODE_DEF##_argc(_id)
#define OPCODE_DEF_repeated(_id, _argc) OPCODE_DEF_REPEATED##_argc(_id)
#define OPCODE_DEF_REPEATED0(_id) kefir_result_t (*_id)(struct kefir_amd64_xasmgen *, kefir_bool_t)
#define OPCODE_DEF0(_id) kefir_result_t (*_id)(struct kefir_amd64_xasmgen *)
#define OPCODE_DEF1(_id) \
    kefir_result_t (*_id)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *)
#define OPCODE_DEF2(_id)                                                                                \
    kefir_result_t (*_id)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *, \
                          const struct kefir_asm_amd64_xasmgen_operand *)
#define OPCODE_DEF3(_id)                                                                                \
    kefir_result_t (*_id)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *, \
                          const struct kefir_asm_amd64_xasmgen_operand *,                               \
                          const struct kefir_asm_amd64_xasmgen_operand *)

        KEFIR_AMD64_XASMGEN_OPCODE_DEFS(OPCODE_DEF, ;);

#undef OPCODE_DEF0
#undef OPCODE_DEF1
#undef OPCODE_DEF2
#undef OPCODE_DEF3
#undef OPCODE_DEF_REPEATED0
#undef OPCODE_DEF_prefix
#undef OPCODE_DEF_normal
#undef OPCODE_DEF_branch
#undef OPCODE_DEF_repeated
#undef OPCODE_DEF
    } instr;

    struct {
        kefir_bool_t enable_comments;
        kefir_bool_t enable_identation;
    } settings;

    void *payload;
} kefir_asm_amd64_xasmgen_t;

kefir_result_t kefir_asm_amd64_xasmgen_init(struct kefir_mem *, struct kefir_amd64_xasmgen *, FILE *,
                                            kefir_asm_amd64_xasmgen_syntax_t);

const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_reg(kefir_asm_amd64_xasmgen_register_t);
const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_imm(
    struct kefir_asm_amd64_xasmgen_operand *, kefir_int64_t);
const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_immu(
    struct kefir_asm_amd64_xasmgen_operand *, kefir_uint64_t);
const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_label(
    struct kefir_asm_amd64_xasmgen_operand *, const char *);
const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_indirect(
    struct kefir_asm_amd64_xasmgen_operand *, const struct kefir_asm_amd64_xasmgen_operand *, kefir_int64_t);

const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_offset(
    struct kefir_asm_amd64_xasmgen_operand *, const struct kefir_asm_amd64_xasmgen_operand *, kefir_int64_t);
const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_segment(
    struct kefir_asm_amd64_xasmgen_operand *, kefir_asm_amd64_xasmgen_segment_register_t,
    const struct kefir_asm_amd64_xasmgen_operand *);
const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_pointer(
    struct kefir_asm_amd64_xasmgen_operand *, kefir_asm_amd64_xasmgen_pointer_type_t,
    const struct kefir_asm_amd64_xasmgen_operand *);
const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_rip_indirection(
    struct kefir_asm_amd64_xasmgen_operand *, const char *);
const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_string_literal(
    struct kefir_asm_amd64_xasmgen_operand *, const char *, kefir_size_t);
const struct kefir_asm_amd64_xasmgen_operand *kefir_asm_amd64_xasmgen_operand_fpu_register(
    struct kefir_asm_amd64_xasmgen_operand *, kefir_uint64_t);

#define KEFIR_AMD64_XASMGEN_PROLOGUE(_xasmgen) ((_xasmgen)->prologue((_xasmgen)))
#define KEFIR_AMD64_XASMGEN_CLOSE(_mem, _xasmgen) ((_xasmgen)->close((_mem), (_xasmgen)))
#define KEFIR_AMD64_XASMGEN_NEWLINE(_xasmgen, _lines) ((_xasmgen)->newline((_xasmgen), (_lines)))
#define KEFIR_AMD64_XASMGEN_COMMENT(_xasmgen, _fmt, ...) ((_xasmgen)->comment((_xasmgen), (_fmt), __VA_ARGS__))
#define KEFIR_AMD64_XASMGEN_LABEL(_xasmgen, _fmt, ...) ((_xasmgen)->label((_xasmgen), (_fmt), __VA_ARGS__))
#define KEFIR_AMD64_XASMGEN_GLOBAL(_xasmgen, _fmt, ...) ((_xasmgen)->global((_xasmgen), (_fmt), __VA_ARGS__))
#define KEFIR_AMD64_XASMGEN_EXTERNAL(_xasmgen, _fmt, ...) ((_xasmgen)->external((_xasmgen), (_fmt), __VA_ARGS__))
#define KEFIR_AMD64_XASMGEN_SECTION(_xasmgen, _name) ((_xasmgen)->section((_xasmgen), (_name)))
#define KEFIR_AMD64_XASMGEN_ALIGN(_xasmgen, _arg) ((_xasmgen)->align((_xasmgen), (_arg)))
#define KEFIR_AMD64_XASMGEN_DATA(_xasmgen, _type, _length, ...) \
    ((_xasmgen)->data((_xasmgen), (_type), (_length), __VA_ARGS__))
#define KEFIR_AMD64_XASMGEN_ZERODATA(_xasmgen, _length) ((_xasmgen)->zerodata((_xasmgen), (_length)))
#define KEFIR_AMD64_XASMGEN_BINDATA(_xasmgen, _type, _ptr, _length) \
    ((_xasmgen)->bindata((_xasmgen), (_type), (_ptr), (_length)))
#define KEFIR_AMD64_XASMGEN_INLINE_ASSEMBLY(_xasmgen, _text) ((_xasmgen)->inline_assembly((_xasmgen), (_text)))
#define KEFIR_AMD64_XASMGEN_FORMAT_OPERAND(_xasmgen, _op, _buf, _buflen) \
    ((_xasmgen)->format_operand((_xasmgen), (_op), (_buf), (_buflen)))

#define KEFIR_AMD64_XASMGEN_INSTR_DATA16(_xasmgen) ((_xasmgen)->instr.data16((_xasmgen)))
#define KEFIR_AMD64_XASMGEN_INSTR_REXW(_xasmgen) ((_xasmgen)->instr.rexW((_xasmgen)))

#define KEFIR_AMD64_XASMGEN_INSTR_JMP(_xasmgen, _op1) ((_xasmgen)->instr.jmp((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_JA(_xasmgen, _op1) ((_xasmgen)->instr.ja((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_JAE(_xasmgen, _op1) ((_xasmgen)->instr.jae((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_JB(_xasmgen, _op1) ((_xasmgen)->instr.jb((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_JBE(_xasmgen, _op1) ((_xasmgen)->instr.jbe((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_JZ(_xasmgen, _op1) ((_xasmgen)->instr.jz((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_JE(_xasmgen, _op1) ((_xasmgen)->instr.je((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_JNE(_xasmgen, _op1) ((_xasmgen)->instr.jne((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_JG(_xasmgen, _op1) ((_xasmgen)->instr.jg((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_JGE(_xasmgen, _op1) ((_xasmgen)->instr.jge((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_JL(_xasmgen, _op1) ((_xasmgen)->instr.jl((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_JLE(_xasmgen, _op1) ((_xasmgen)->instr.jle((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_JS(_xasmgen, _op1) ((_xasmgen)->instr.js((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_JNS(_xasmgen, _op1) ((_xasmgen)->instr.jns((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_JP(_xasmgen, _op1) ((_xasmgen)->instr.jp((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_JNP(_xasmgen, _op1) ((_xasmgen)->instr.jnp((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_CALL(_xasmgen, _op1) ((_xasmgen)->instr.call((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_RET(_xasmgen) ((_xasmgen)->instr.ret((_xasmgen)))

#define KEFIR_AMD64_XASMGEN_INSTR_PUSH(_xasmgen, _op1) ((_xasmgen)->instr.push((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_POP(_xasmgen, _op1) ((_xasmgen)->instr.pop((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_PUSHFQ(_xasmgen) ((_xasmgen)->instr.pushfq((_xasmgen)))
#define KEFIR_AMD64_XASMGEN_INSTR_POPFQ(_xasmgen) ((_xasmgen)->instr.popfq((_xasmgen)))

#define KEFIR_AMD64_XASMGEN_INSTR_MOV(_xasmgen, _op1, _op2) ((_xasmgen)->instr.mov((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_MOVSX(_xasmgen, _op1, _op2) ((_xasmgen)->instr.movsx((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_MOVZX(_xasmgen, _op1, _op2) ((_xasmgen)->instr.movzx((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_MOVABS(_xasmgen, _op1, _op2) ((_xasmgen)->instr.movabs((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_MOVSB(_xasmgen, _rep) ((_xasmgen)->instr.movsb((_xasmgen), (_rep)))
#define KEFIR_AMD64_XASMGEN_INSTR_STOSB(_xasmgen, _rep) ((_xasmgen)->instr.stosb((_xasmgen), (_rep)))
#define KEFIR_AMD64_XASMGEN_INSTR_LEA(_xasmgen, _op1, _op2) ((_xasmgen)->instr.lea((_xasmgen), (_op1), (_op2)))

#define KEFIR_AMD64_XASMGEN_INSTR_ADD(_xasmgen, _op1, _op2) ((_xasmgen)->instr.add((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_SUB(_xasmgen, _op1, _op2) ((_xasmgen)->instr.sub((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_IMUL(_xasmgen, _op1, _op2) ((_xasmgen)->instr.imul((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_IDIV(_xasmgen, _op1) ((_xasmgen)->instr.idiv((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_DIV(_xasmgen, _op1) ((_xasmgen)->instr.div((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_AND(_xasmgen, _op1, _op2) ((_xasmgen)->instr.and ((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_OR(_xasmgen, _op1, _op2) ((_xasmgen)->instr.or ((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_XOR(_xasmgen, _op1, _op2) ((_xasmgen)->instr.xor ((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_SHL(_xasmgen, _op1, _op2) ((_xasmgen)->instr.shl((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_SHR(_xasmgen, _op1, _op2) ((_xasmgen)->instr.shr((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_SAR(_xasmgen, _op1, _op2) ((_xasmgen)->instr.sar((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_NOT(_xasmgen, _op1) ((_xasmgen)->instr.not((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_NEG(_xasmgen, _op1) ((_xasmgen)->instr.neg((_xasmgen), (_op1)))

#define KEFIR_AMD64_XASMGEN_INSTR_CQO(_xasmgen) ((_xasmgen)->instr.cqo((_xasmgen)))

#define KEFIR_AMD64_XASMGEN_INSTR_CVTSI2SS(_xasmgen, _op1, _op2) \
    ((_xasmgen)->instr.cvtsi2ss((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_CVTSI2SD(_xasmgen, _op1, _op2) \
    ((_xasmgen)->instr.cvtsi2sd((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_CVTTSS2SI(_xasmgen, _op1, _op2) \
    ((_xasmgen)->instr.cvttss2si((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_CVTTSD2SI(_xasmgen, _op1, _op2) \
    ((_xasmgen)->instr.cvttsd2si((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_CVTSD2SS(_xasmgen, _op1, _op2) \
    ((_xasmgen)->instr.cvtsd2ss((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_CVTSS2SD(_xasmgen, _op1, _op2) \
    ((_xasmgen)->instr.cvtss2sd((_xasmgen), (_op1), (_op2)))

#define KEFIR_AMD64_XASMGEN_INSTR_CMP(_xasmgen, _op1, _op2) ((_xasmgen)->instr.cmp((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_CLD(_xasmgen) ((_xasmgen)->instr.cld((_xasmgen)))
#define KEFIR_AMD64_XASMGEN_INSTR_TEST(_xasmgen, _op1, _op2) ((_xasmgen)->instr.test((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_SETE(_xasmgen, _op1) ((_xasmgen)->instr.sete((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_SETG(_xasmgen, _op1) ((_xasmgen)->instr.setg((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_SETGE(_xasmgen, _op1) ((_xasmgen)->instr.setge((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_SETL(_xasmgen, _op1) ((_xasmgen)->instr.setl((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_SETLE(_xasmgen, _op1) ((_xasmgen)->instr.setle((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_SETA(_xasmgen, _op1) ((_xasmgen)->instr.seta((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_SETAE(_xasmgen, _op1) ((_xasmgen)->instr.setae((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_SETB(_xasmgen, _op1) ((_xasmgen)->instr.setb((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_SETBE(_xasmgen, _op1) ((_xasmgen)->instr.setbe((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_SETNE(_xasmgen, _op1) ((_xasmgen)->instr.setne((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_SETNP(_xasmgen, _op1) ((_xasmgen)->instr.setnp((_xasmgen), (_op1)))

#define KEFIR_AMD64_XASMGEN_INSTR_CMOVL(_xasmgen, _op1, _op2) ((_xasmgen)->instr.cmovl((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_CMOVNE(_xasmgen, _op1, _op2) ((_xasmgen)->instr.cmovne((_xasmgen), (_op1), (_op2)))

#define KEFIR_AMD64_XASMGEN_INSTR_FSTCW(_xasmgen, _op1) ((_xasmgen)->instr.fstcw((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_FLDCW(_xasmgen, _op1) ((_xasmgen)->instr.fldcw((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_FSTP(_xasmgen, _op1) ((_xasmgen)->instr.fstp((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_FLD(_xasmgen, _op1) ((_xasmgen)->instr.fld((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_FLDZ(_xasmgen) ((_xasmgen)->instr.fldz((_xasmgen)))
#define KEFIR_AMD64_XASMGEN_INSTR_FILD(_xasmgen, _op1) ((_xasmgen)->instr.fild((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_FADDP(_xasmgen) ((_xasmgen)->instr.faddp((_xasmgen)))
#define KEFIR_AMD64_XASMGEN_INSTR_FADD(_xasmgen, _op1) ((_xasmgen)->instr.fadd((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_FSUBP(_xasmgen) ((_xasmgen)->instr.fsubp((_xasmgen)))
#define KEFIR_AMD64_XASMGEN_INSTR_FMULP(_xasmgen) ((_xasmgen)->instr.fmulp((_xasmgen)))
#define KEFIR_AMD64_XASMGEN_INSTR_FDIVP(_xasmgen) ((_xasmgen)->instr.fdivp((_xasmgen)))
#define KEFIR_AMD64_XASMGEN_INSTR_FCHS(_xasmgen) ((_xasmgen)->instr.fchs((_xasmgen)))

#define KEFIR_AMD64_XASMGEN_INSTR_FUCOMIP(_xasmgen) ((_xasmgen)->instr.fucomip((_xasmgen)))
#define KEFIR_AMD64_XASMGEN_INSTR_FCOMIP(_xasmgen) ((_xasmgen)->instr.fcomip((_xasmgen)))

#define KEFIR_AMD64_XASMGEN_INSTR_PEXTRQ(_xasmgen, _op1, _op2, _op3) \
    ((_xasmgen)->instr.pextrq((_xasmgen), (_op1), (_op2), (_op3)))
#define KEFIR_AMD64_XASMGEN_INSTR_PINSRQ(_xasmgen, _op1, _op2, _op3) \
    ((_xasmgen)->instr.pinsrq((_xasmgen), (_op1), (_op2), (_op3)))
#define KEFIR_AMD64_XASMGEN_INSTR_MOVD(_xasmgen, _op1, _op2) ((_xasmgen)->instr.movd((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_MOVQ(_xasmgen, _op1, _op2) ((_xasmgen)->instr.movq((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_MOVDQU(_xasmgen, _op1, _op2) ((_xasmgen)->instr.movdqu((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_STMXCSR(_xasmgen, _op1) ((_xasmgen)->instr.stmxcsr((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_LDMXCSR(_xasmgen, _op1) ((_xasmgen)->instr.ldmxcsr((_xasmgen), (_op1)))

#define KEFIR_AMD64_XASMGEN_INSTR_PXOR(_xasmgen, _op1, _op2) ((_xasmgen)->instr.pxor((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_XORPS(_xasmgen, _op1, _op2) ((_xasmgen)->instr.xorps((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_XORPD(_xasmgen, _op1, _op2) ((_xasmgen)->instr.xorpd((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_ADDSS(_xasmgen, _op1, _op2) ((_xasmgen)->instr.addss((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_ADDSD(_xasmgen, _op1, _op2) ((_xasmgen)->instr.addsd((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_SUBSS(_xasmgen, _op1, _op2) ((_xasmgen)->instr.subss((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_SUBSD(_xasmgen, _op1, _op2) ((_xasmgen)->instr.subsd((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_MULSS(_xasmgen, _op1, _op2) ((_xasmgen)->instr.mulss((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_MULSD(_xasmgen, _op1, _op2) ((_xasmgen)->instr.mulsd((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_DIVSS(_xasmgen, _op1, _op2) ((_xasmgen)->instr.divss((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_DIVSD(_xasmgen, _op1, _op2) ((_xasmgen)->instr.divsd((_xasmgen), (_op1), (_op2)))

#define KEFIR_AMD64_XASMGEN_INSTR_UCOMISS(_xasmgen, _op1, _op2) ((_xasmgen)->instr.ucomiss((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_UCOMISD(_xasmgen, _op1, _op2) ((_xasmgen)->instr.ucomisd((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_COMISS(_xasmgen, _op1, _op2) ((_xasmgen)->instr.comiss((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_COMISD(_xasmgen, _op1, _op2) ((_xasmgen)->instr.comisd((_xasmgen), (_op1), (_op2)))

#define KEFIR_AMD64_XASMGEN_HELPERS_BUFFER_LENGTH 1024
typedef struct kefir_asm_amd64_xasmgen_helpers {
    char buffer[KEFIR_AMD64_XASMGEN_HELPERS_BUFFER_LENGTH];

    struct kefir_asm_amd64_xasmgen_operand operands[4];
} kefir_asm_amd64_xasmgen_helpers_t;

const char *kefir_asm_amd64_xasmgen_helpers_format(struct kefir_asm_amd64_xasmgen_helpers *, const char *, ...);

#endif
