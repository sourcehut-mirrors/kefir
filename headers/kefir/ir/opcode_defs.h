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

#ifndef KEFIR_IR_OPCODE_DEFS_H_
#define KEFIR_IR_OPCODE_DEFS_H_

#define KEFIR_IR_OPCODES_REVISION 1
// clang-format off
#define KEFIR_IR_OPCODE_DEFS(OPCODE, SEPARATOR) \
    OPCODE(RESERVED,    "reserved",    0x00, none) SEPARATOR \
    /* Flow control */ \
    OPCODE(NOP,         "nop",         0x01, none) SEPARATOR \
    OPCODE(JMP,         "jmp",         0x02, coderef) SEPARATOR \
    OPCODE(IJMP,        "ijmp",        0x03, none) SEPARATOR \
    OPCODE(BRANCH,      "branch",      0x04, coderef) SEPARATOR \
    OPCODE(RET,         "ret",         0x05, none) SEPARATOR \
    OPCODE(INVOKE,      "invoke",      0x06, funcref) SEPARATOR \
    OPCODE(INVOKEV,     "invokev",     0x07, funcref) SEPARATOR \
    /* Stack control */ \
    OPCODE(PUSHI64,     "pushi",       0x10, i64) SEPARATOR \
    OPCODE(PUSHU64,     "pushu",       0x11, u64) SEPARATOR \
    OPCODE(PUSHSTRING,  "pushstring",  0x12, string) SEPARATOR \
    OPCODE(PUSHLABEL,   "pushlabel",   0x13, u64) SEPARATOR \
    OPCODE(POP,         "pop",         0x14, none) SEPARATOR \
    OPCODE(PICK,        "pick",        0x15, u64) SEPARATOR \
    OPCODE(XCHG,        "xchg",        0x16, u64) SEPARATOR \
    /* Placeholders */ \
    OPCODE(PLACEHI64,   "placehi64",   0x17, none) SEPARATOR \
    OPCODE(PLACEHF32,   "placehf32",   0x18, none) SEPARATOR \
    OPCODE(PLACEHF64,   "placehf64",   0x19, none) SEPARATOR \
    /* Integer arithmetics & bitwise */ \
    OPCODE(IADD,        "iadd",        0x20, none) SEPARATOR \
    OPCODE(IADD1,       "iadd1",       0x21, i64) SEPARATOR \
    OPCODE(IADDX,       "iaddx",       0x22, u64) SEPARATOR \
    OPCODE(ISUB,        "isub",        0x23, none) SEPARATOR \
    OPCODE(IMUL,        "imul",        0x24, none) SEPARATOR \
    OPCODE(IDIV,        "idiv",        0x25, none) SEPARATOR \
    OPCODE(IMOD,        "imod",        0x26, none) SEPARATOR \
    OPCODE(UDIV,        "udiv",        0x27, none) SEPARATOR \
    OPCODE(UMOD,        "umod",        0x28, none) SEPARATOR \
    OPCODE(INEG,        "ineg",        0x29, none) SEPARATOR \
    OPCODE(INOT,        "inot",        0x2a, none) SEPARATOR \
    OPCODE(IAND,        "iand",        0x2b, none) SEPARATOR \
    OPCODE(IOR,         "ior",         0x2c, none) SEPARATOR \
    OPCODE(IXOR,        "ixor",        0x2d, none) SEPARATOR \
    OPCODE(ILSHIFT,     "ishl",        0x2e, none) SEPARATOR \
    OPCODE(IRSHIFT,     "ishr",        0x2f, none) SEPARATOR \
    OPCODE(IARSHIFT,    "isar",        0x30, none) SEPARATOR \
    /* Logics & coditions */ \
    OPCODE(IEQUALS,     "iequals",     0x40, none) SEPARATOR \
    OPCODE(IGREATER,    "igreater",    0x41, none) SEPARATOR \
    OPCODE(ILESSER,     "ilesser",     0x42, none) SEPARATOR \
    OPCODE(IABOVE,      "iabove",      0x43, none) SEPARATOR \
    OPCODE(IBELOW,      "ibelow",      0x44, none) SEPARATOR \
    OPCODE(BAND,        "band",        0x45, none) SEPARATOR \
    OPCODE(BOR,         "bor",         0x46, none) SEPARATOR \
    OPCODE(BNOT,        "bnot",        0x47, none) SEPARATOR \
    /* Type conversions */ \
    OPCODE(TRUNCATE1,   "trunc1",      0x50, none) SEPARATOR \
    OPCODE(EXTEND8,     "extend8",     0x51, none) SEPARATOR \
    OPCODE(EXTEND16,    "extend16",    0x52, none) SEPARATOR \
    OPCODE(EXTEND32,    "extend32",    0x53, none) SEPARATOR \
    /* Data access */ \
    OPCODE(GETGLOBAL,   "getglobal",   0x60, identifier) SEPARATOR \
    OPCODE(GETLOCAL,    "getlocal",    0x61, typeref) SEPARATOR \
    OPCODE(LOAD8U,      "load8u",      0x62, memflags) SEPARATOR \
    OPCODE(LOAD8I,      "load8i",      0x63, memflags) SEPARATOR \
    OPCODE(LOAD16U,     "load16u",     0x64, memflags) SEPARATOR \
    OPCODE(LOAD16I,     "load16i",     0x65, memflags) SEPARATOR \
    OPCODE(LOAD32U,     "load32u",     0x66, memflags) SEPARATOR \
    OPCODE(LOAD32I,     "load32i",     0x67, memflags) SEPARATOR \
    OPCODE(LOAD64,      "load64",      0x68, memflags) SEPARATOR \
    OPCODE(STORE8,      "store8",      0x6a, memflags) SEPARATOR \
    OPCODE(STORE16,     "store16",     0x6b, memflags) SEPARATOR \
    OPCODE(STORE32,     "store32",     0x6c, memflags) SEPARATOR \
    OPCODE(STORE64,     "store64",     0x6d, memflags) SEPARATOR \
    OPCODE(STORELD,     "storeld",     0x6e, memflags) SEPARATOR \
    OPCODE(BZERO,       "bzero",       0x6f, typeref) SEPARATOR \
    OPCODE(BCOPY,       "bcopy",       0x70, typeref) SEPARATOR \
    OPCODE(EXTUBITS,    "extubits",    0x71, u32) SEPARATOR \
    OPCODE(EXTSBITS,    "extsbits",    0x72, u32) SEPARATOR \
    OPCODE(INSERTBITS,  "insertbits",  0x73, u32) SEPARATOR \
    OPCODE(GETTHRLOCAL, "getthrlocal", 0x74, identifier) SEPARATOR \
    /* Built-ins */ \
    OPCODE(VARARG_START, "startvarg",  0x80, none) SEPARATOR \
    OPCODE(VARARG_COPY,  "copyvarg",   0x81, none) SEPARATOR \
    OPCODE(VARARG_GET,   "getvarg",    0x82, typeref) SEPARATOR \
    OPCODE(VARARG_END,   "endvarg",    0x83, none) SEPARATOR \
    OPCODE(ALLOCA,       "alloca",     0x84, bool) SEPARATOR \
    OPCODE(PUSHSCOPE,    "pushscope",  0x85, none) SEPARATOR \
    OPCODE(POPSCOPE,     "popscope",   0x86, none) SEPARATOR \
    /* Floating-point basics */ \
    OPCODE(PUSHF32,      "pushf32",    0x90, f32) SEPARATOR \
    OPCODE(PUSHF64,      "pushf64",    0x91, f64) SEPARATOR \
    OPCODE(F32ADD,       "f32add",     0x92, none) SEPARATOR \
    OPCODE(F32SUB,       "f32sub",     0x93, none) SEPARATOR \
    OPCODE(F32MUL,       "f32mul",     0x94, none) SEPARATOR \
    OPCODE(F32DIV,       "f32div",     0x95, none) SEPARATOR \
    OPCODE(F32NEG,       "f32neg",     0x96, none) SEPARATOR \
    OPCODE(F64ADD,       "f64add",     0x97, none) SEPARATOR \
    OPCODE(F64SUB,       "f64sub",     0x98, none) SEPARATOR \
    OPCODE(F64MUL,       "f64mul",     0x99, none) SEPARATOR \
    OPCODE(F64DIV,       "f64div",     0x9a, none) SEPARATOR \
    OPCODE(F64NEG,       "f64neg",     0x9b, none) SEPARATOR \
    OPCODE(LDADD,        "ldadd",      0x9c, none) SEPARATOR \
    OPCODE(LDSUB,        "ldsub",      0x9d, none) SEPARATOR \
    OPCODE(LDMUL,        "ldmul",      0x9e, none) SEPARATOR \
    OPCODE(LDDIV,        "lddiv",      0x9f, none) SEPARATOR \
    OPCODE(LDNEG,        "ldneg",      0xa0, none) SEPARATOR \
    OPCODE(LDINITH,      "ldinith",    0xa1, u64) SEPARATOR \
    OPCODE(LDINITL,      "ldinitl",    0xa2, u64) SEPARATOR \
    /* Floating-point comparison */ \
    OPCODE(F32EQUALS,    "f32equals",  0xb0, none) SEPARATOR \
    OPCODE(F32GREATER,   "f32greater", 0xb1, none) SEPARATOR \
    OPCODE(F32LESSER,    "f32lesser",  0xb2, none) SEPARATOR \
    OPCODE(F64EQUALS,    "f64equals",  0xb3, none) SEPARATOR \
    OPCODE(F64GREATER,   "f64greater", 0xb4, none) SEPARATOR \
    OPCODE(F64LESSER,    "f64lesser",  0xb5, none) SEPARATOR \
    OPCODE(LDEQUALS,     "ldequals",   0xb6, none) SEPARATOR \
    OPCODE(LDGREATER,    "ldgreater",  0xb7, none) SEPARATOR \
    OPCODE(LDLESSER,     "ldlesser",   0xb8, none) SEPARATOR \
    OPCODE(LDTRUNC1,     "ldtrunc1",   0xb9, none) SEPARATOR \
    /* Floating-point conversions */ \
    OPCODE(F32CINT,      "f32cint",    0xc0, none) SEPARATOR \
    OPCODE(F64CINT,      "f64cint",    0xc1, none) SEPARATOR \
    OPCODE(LDCINT,       "ldcint",     0xc2, none) SEPARATOR \
    OPCODE(F32CUINT,     "f32cuint",   0xc3, none) SEPARATOR \
    OPCODE(F64CUINT,     "f64cuint",   0xc4, none) SEPARATOR \
    OPCODE(LDCUINT,      "ldcuint",    0xc5, none) SEPARATOR \
    OPCODE(INTCF32,      "intcf32",    0xc6, none) SEPARATOR \
    OPCODE(INTCF64,      "intcf64",    0xc7, none) SEPARATOR \
    OPCODE(INTCLD,       "intcld",     0xc8, none) SEPARATOR \
    OPCODE(UINTCF32,     "uintcf32",   0xc9, none) SEPARATOR \
    OPCODE(UINTCF64,     "uintcf64",   0xca, none) SEPARATOR \
    OPCODE(UINTCLD,      "uintcld",    0xcb, none) SEPARATOR \
    OPCODE(F32CF64,      "f32cf64",    0xcc, none) SEPARATOR \
    OPCODE(F32CLD,       "f32cld",     0xcd, none) SEPARATOR \
    OPCODE(F64CF32,      "f64cf32",    0xce, none) SEPARATOR \
    OPCODE(F64CLD,       "f64cld",     0xcf, none) SEPARATOR \
    OPCODE(LDCF32,       "ldcf32",     0xd0, none) SEPARATOR \
    OPCODE(LDCF64,       "ldcf64",     0xd1, none) SEPARATOR \
    /* Miscallenous */ \
    OPCODE(INLINEASM,    "inlineasm",  0xe0, u64)
// clang-format on

#endif
