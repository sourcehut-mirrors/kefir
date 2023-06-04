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
    KEFIR_AMD64_XASMGEN_OPERAND_STRING_LITERAL
} kefir_asm_amd64_xasmgen_operand_class_t;

typedef enum kefir_asm_amd64_xasmgen_pointer_type {
    KEFIR_AMD64_XASMGEN_POINTER_BYTE,
    KEFIR_AMD64_XASMGEN_POINTER_WORD,
    KEFIR_AMD64_XASMGEN_POINTER_DWORD,
    KEFIR_AMD64_XASMGEN_POINTER_QWORD,
    KEFIR_AMD64_XASMGEN_POINTER_TBYTE
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
        kefir_result_t (*push)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*pop)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*mov)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *,
                              const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*movabs)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *,
                                 const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (* or)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *,
                              const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*lea)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *,
                              const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*movq)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *,
                               const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*movsb)(struct kefir_amd64_xasmgen *, kefir_bool_t);
        kefir_result_t (*jmp)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*ja)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*ret)(struct kefir_amd64_xasmgen *);
        kefir_result_t (*fstcw)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*fldcw)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*call)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*pextrq)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *,
                                 const struct kefir_asm_amd64_xasmgen_operand *,
                                 const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*pinsrq)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *,
                                 const struct kefir_asm_amd64_xasmgen_operand *,
                                 const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*add)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *,
                              const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*cmp)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *,
                              const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*sub)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *,
                              const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*cld)(struct kefir_amd64_xasmgen *);
        kefir_result_t (*and)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *,
                              const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*shl)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *,
                              const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*shr)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *,
                              const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*movd)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *,
                               const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*fstp)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*fld)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*movdqu)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *,
                                 const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*pushfq)(struct kefir_amd64_xasmgen *);
        kefir_result_t (*popfq)(struct kefir_amd64_xasmgen *);
        kefir_result_t (*stmxcsr)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *);
        kefir_result_t (*ldmxcsr)(struct kefir_amd64_xasmgen *, const struct kefir_asm_amd64_xasmgen_operand *);
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

#define KEFIR_AMD64_XASMGEN_INSTR_PUSH(_xasmgen, _op1) ((_xasmgen)->instr.push((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_POP(_xasmgen, _op1) ((_xasmgen)->instr.pop((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_MOV(_xasmgen, _op1, _op2) ((_xasmgen)->instr.mov((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_MOVABS(_xasmgen, _op1, _op2) ((_xasmgen)->instr.movabs((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_OR(_xasmgen, _op1, _op2) ((_xasmgen)->instr.or ((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_LEA(_xasmgen, _op1, _op2) ((_xasmgen)->instr.lea((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_MOVQ(_xasmgen, _op1, _op2) ((_xasmgen)->instr.movq((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_MOVSB(_xasmgen, _rep) ((_xasmgen)->instr.movsb((_xasmgen), (_rep)))
#define KEFIR_AMD64_XASMGEN_INSTR_JMP(_xasmgen, _op1) ((_xasmgen)->instr.jmp((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_JA(_xasmgen, _op1) ((_xasmgen)->instr.ja((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_RET(_xasmgen) ((_xasmgen)->instr.ret((_xasmgen)))
#define KEFIR_AMD64_XASMGEN_INSTR_FSTCW(_xasmgen, _op1) ((_xasmgen)->instr.fstcw((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_FLDCW(_xasmgen, _op1) ((_xasmgen)->instr.fldcw((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_CALL(_xasmgen, _op1) ((_xasmgen)->instr.call((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_PEXTRQ(_xasmgen, _op1, _op2, _op3) \
    ((_xasmgen)->instr.pextrq((_xasmgen), (_op1), (_op2), (_op3)))
#define KEFIR_AMD64_XASMGEN_INSTR_PINSRQ(_xasmgen, _op1, _op2, _op3) \
    ((_xasmgen)->instr.pinsrq((_xasmgen), (_op1), (_op2), (_op3)))
#define KEFIR_AMD64_XASMGEN_INSTR_ADD(_xasmgen, _op1, _op2) ((_xasmgen)->instr.add((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_CMP(_xasmgen, _op1, _op2) ((_xasmgen)->instr.cmp((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_SUB(_xasmgen, _op1, _op2) ((_xasmgen)->instr.sub((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_CLD(_xasmgen) ((_xasmgen)->instr.cld((_xasmgen)))
#define KEFIR_AMD64_XASMGEN_INSTR_AND(_xasmgen, _op1, _op2) ((_xasmgen)->instr.and ((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_SHL(_xasmgen, _op1, _op2) ((_xasmgen)->instr.shl((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_SHR(_xasmgen, _op1, _op2) ((_xasmgen)->instr.shr((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_MOVD(_xasmgen, _op1, _op2) ((_xasmgen)->instr.movd((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_FSTP(_xasmgen, _op1) ((_xasmgen)->instr.fstp((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_FLD(_xasmgen, _op1) ((_xasmgen)->instr.fld((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_MOVDQU(_xasmgen, _op1, _op2) ((_xasmgen)->instr.movdqu((_xasmgen), (_op1), (_op2)))
#define KEFIR_AMD64_XASMGEN_INSTR_PUSHFQ(_xasmgen) ((_xasmgen)->instr.pushfq((_xasmgen)))
#define KEFIR_AMD64_XASMGEN_INSTR_POPFQ(_xasmgen) ((_xasmgen)->instr.popfq((_xasmgen)))
#define KEFIR_AMD64_XASMGEN_INSTR_STMXCSR(_xasmgen, _op1) ((_xasmgen)->instr.stmxcsr((_xasmgen), (_op1)))
#define KEFIR_AMD64_XASMGEN_INSTR_LDMXCSR(_xasmgen, _op1) ((_xasmgen)->instr.ldmxcsr((_xasmgen), (_op1)))

#define KEFIR_AMD64_XASMGEN_HELPERS_BUFFER_LENGTH 1024
typedef struct kefir_asm_amd64_xasmgen_helpers {
    char buffer[KEFIR_AMD64_XASMGEN_HELPERS_BUFFER_LENGTH];

    struct kefir_asm_amd64_xasmgen_operand operands[4];
} kefir_asm_amd64_xasmgen_helpers_t;

const char *kefir_asm_amd64_xasmgen_helpers_format(struct kefir_asm_amd64_xasmgen_helpers *, const char *, ...);

#endif
