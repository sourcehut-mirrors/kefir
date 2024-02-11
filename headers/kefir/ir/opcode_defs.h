/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
    OPCODE(RESERVED,    "reserved",     none) SEPARATOR \
    /* Flow control */ \
    OPCODE(NOP,         "nop",          none) SEPARATOR \
    OPCODE(JMP,         "jmp",          coderef) SEPARATOR \
    OPCODE(IJMP,        "ijmp",         none) SEPARATOR \
    OPCODE(BRANCH,      "branch",       coderef) SEPARATOR \
    OPCODE(RET,         "ret",          none) SEPARATOR \
    OPCODE(INVOKE,      "invoke",       funcref) SEPARATOR \
    OPCODE(INVOKEV,     "invokev",      funcref) SEPARATOR \
    /* Stack control */ \
    OPCODE(PUSHI64,     "pushi",        i64) SEPARATOR \
    OPCODE(PUSHU64,     "pushu",        u64) SEPARATOR \
    OPCODE(PUSHSTRING,  "pushstring",   string) SEPARATOR \
    OPCODE(PUSHLABEL,   "pushlabel",    u64) SEPARATOR \
    OPCODE(POP,         "pop",          none) SEPARATOR \
    OPCODE(PICK,        "pick",         u64) SEPARATOR \
    OPCODE(XCHG,        "xchg",         u64) SEPARATOR \
    /* Placeholders */ \
    OPCODE(PLACEHI64,   "placehi64",    none) SEPARATOR \
    OPCODE(PLACEHF32,   "placehf32",    none) SEPARATOR \
    OPCODE(PLACEHF64,   "placehf64",    none) SEPARATOR \
    /* Integer arithmetics & bitwise */ \
    OPCODE(IADD,        "iadd",         none) SEPARATOR \
    OPCODE(IADD1,       "iadd1",        i64) SEPARATOR \
    OPCODE(IADDX,       "iaddx",        u64) SEPARATOR \
    OPCODE(ISUB,        "isub",         none) SEPARATOR \
    OPCODE(IMUL,        "imul",         none) SEPARATOR \
    OPCODE(IDIV,        "idiv",         none) SEPARATOR \
    OPCODE(IMOD,        "imod",         none) SEPARATOR \
    OPCODE(UDIV,        "udiv",         none) SEPARATOR \
    OPCODE(UMOD,        "umod",         none) SEPARATOR \
    OPCODE(INEG,        "ineg",         none) SEPARATOR \
    OPCODE(INOT,        "inot",         none) SEPARATOR \
    OPCODE(IAND,        "iand",         none) SEPARATOR \
    OPCODE(IOR,         "ior",          none) SEPARATOR \
    OPCODE(IXOR,        "ixor",         none) SEPARATOR \
    OPCODE(ILSHIFT,     "ishl",         none) SEPARATOR \
    OPCODE(IRSHIFT,     "ishr",         none) SEPARATOR \
    OPCODE(IARSHIFT,    "isar",         none) SEPARATOR \
    /* Logics & coditions */ \
    OPCODE(IEQUALS,     "iequals",      none) SEPARATOR \
    OPCODE(IGREATER,    "igreater",     none) SEPARATOR \
    OPCODE(ILESSER,     "ilesser",      none) SEPARATOR \
    OPCODE(IABOVE,      "iabove",       none) SEPARATOR \
    OPCODE(IBELOW,      "ibelow",       none) SEPARATOR \
    OPCODE(BAND,        "band",         none) SEPARATOR \
    OPCODE(BOR,         "bor",          none) SEPARATOR \
    OPCODE(BNOT,        "bnot",         none) SEPARATOR \
    /* Type conversions */ \
    OPCODE(TRUNCATE1,   "trunc1",       none) SEPARATOR \
    OPCODE(EXTEND8,     "extend8",      none) SEPARATOR \
    OPCODE(EXTEND16,    "extend16",     none) SEPARATOR \
    OPCODE(EXTEND32,    "extend32",     none) SEPARATOR \
    /* Data access */ \
    OPCODE(GETGLOBAL,   "getglobal",    identifier) SEPARATOR \
    OPCODE(GETLOCAL,    "getlocal",     typeref) SEPARATOR \
    OPCODE(LOAD8U,      "load8u",       memflags) SEPARATOR \
    OPCODE(LOAD8I,      "load8i",       memflags) SEPARATOR \
    OPCODE(LOAD16U,     "load16u",      memflags) SEPARATOR \
    OPCODE(LOAD16I,     "load16i",      memflags) SEPARATOR \
    OPCODE(LOAD32U,     "load32u",      memflags) SEPARATOR \
    OPCODE(LOAD32I,     "load32i",      memflags) SEPARATOR \
    OPCODE(LOAD64,      "load64",       memflags) SEPARATOR \
    OPCODE(LOADLD,      "loadld",       memflags) SEPARATOR \
    OPCODE(STORE8,      "store8",       memflags) SEPARATOR \
    OPCODE(STORE16,     "store16",      memflags) SEPARATOR \
    OPCODE(STORE32,     "store32",      memflags) SEPARATOR \
    OPCODE(STORE64,     "store64",      memflags) SEPARATOR \
    OPCODE(STORELD,     "storeld",      memflags) SEPARATOR \
    OPCODE(BZERO,       "bzero",        typeref) SEPARATOR \
    OPCODE(BCOPY,       "bcopy",        typeref) SEPARATOR \
    OPCODE(EXTUBITS,    "extubits",     u32) SEPARATOR \
    OPCODE(EXTSBITS,    "extsbits",     u32) SEPARATOR \
    OPCODE(INSERTBITS,  "insertbits",   u32) SEPARATOR \
    OPCODE(GETTHRLOCAL, "getthrlocal",  identifier) SEPARATOR \
    /* Built-ins */ \
    OPCODE(VARARG_START, "startvarg",   none) SEPARATOR \
    OPCODE(VARARG_COPY,  "copyvarg",    none) SEPARATOR \
    OPCODE(VARARG_GET,   "getvarg",     typeref) SEPARATOR \
    OPCODE(VARARG_END,   "endvarg",     none) SEPARATOR \
    OPCODE(ALLOCA,       "alloca",      bool) SEPARATOR \
    OPCODE(PUSHSCOPE,    "pushscope",   none) SEPARATOR \
    OPCODE(POPSCOPE,     "popscope",    none) SEPARATOR \
    /* Floating-point basics */ \
    OPCODE(PUSHF32,      "pushf32",     f32) SEPARATOR \
    OPCODE(PUSHF64,      "pushf64",     f64) SEPARATOR \
    OPCODE(PUSHLD,       "pushld",      ldouble) SEPARATOR \
    OPCODE(F32ADD,       "f32add",      none) SEPARATOR \
    OPCODE(F32SUB,       "f32sub",      none) SEPARATOR \
    OPCODE(F32MUL,       "f32mul",      none) SEPARATOR \
    OPCODE(F32DIV,       "f32div",      none) SEPARATOR \
    OPCODE(F32NEG,       "f32neg",      none) SEPARATOR \
    OPCODE(F64ADD,       "f64add",      none) SEPARATOR \
    OPCODE(F64SUB,       "f64sub",      none) SEPARATOR \
    OPCODE(F64MUL,       "f64mul",      none) SEPARATOR \
    OPCODE(F64DIV,       "f64div",      none) SEPARATOR \
    OPCODE(F64NEG,       "f64neg",      none) SEPARATOR \
    OPCODE(LDADD,        "ldadd",       none) SEPARATOR \
    OPCODE(LDSUB,        "ldsub",       none) SEPARATOR \
    OPCODE(LDMUL,        "ldmul",       none) SEPARATOR \
    OPCODE(LDDIV,        "lddiv",       none) SEPARATOR \
    OPCODE(LDNEG,        "ldneg",       none) SEPARATOR \
    /* Floating-point comparison */ \
    OPCODE(F32EQUALS,    "f32equals",   none) SEPARATOR \
    OPCODE(F32GREATER,   "f32greater",  none) SEPARATOR \
    OPCODE(F32LESSER,    "f32lesser",   none) SEPARATOR \
    OPCODE(F64EQUALS,    "f64equals",   none) SEPARATOR \
    OPCODE(F64GREATER,   "f64greater",  none) SEPARATOR \
    OPCODE(F64LESSER,    "f64lesser",   none) SEPARATOR \
    OPCODE(LDEQUALS,     "ldequals",    none) SEPARATOR \
    OPCODE(LDGREATER,    "ldgreater",   none) SEPARATOR \
    OPCODE(LDLESSER,     "ldlesser",    none) SEPARATOR \
    /* Floating-point conversions */ \
    OPCODE(F32CINT,      "f32cint",     none) SEPARATOR \
    OPCODE(F64CINT,      "f64cint",     none) SEPARATOR \
    OPCODE(LDCINT,       "ldcint",      none) SEPARATOR \
    OPCODE(F32CUINT,     "f32cuint",    none) SEPARATOR \
    OPCODE(F64CUINT,     "f64cuint",    none) SEPARATOR \
    OPCODE(LDCUINT,      "ldcuint",     none) SEPARATOR \
    OPCODE(INTCF32,      "intcf32",     none) SEPARATOR \
    OPCODE(INTCF64,      "intcf64",     none) SEPARATOR \
    OPCODE(INTCLD,       "intcld",      none) SEPARATOR \
    OPCODE(UINTCF32,     "uintcf32",    none) SEPARATOR \
    OPCODE(UINTCF64,     "uintcf64",    none) SEPARATOR \
    OPCODE(UINTCLD,      "uintcld",     none) SEPARATOR \
    OPCODE(F32CF64,      "f32cf64",     none) SEPARATOR \
    OPCODE(F32CLD,       "f32cld",      none) SEPARATOR \
    OPCODE(F64CF32,      "f64cf32",     none) SEPARATOR \
    OPCODE(F64CLD,       "f64cld",      none) SEPARATOR \
    OPCODE(LDCF32,       "ldcf32",      none) SEPARATOR \
    OPCODE(LDCF64,       "ldcf64",      none) SEPARATOR \
    /* Complex numbers */ \
    OPCODE(CMPF32,       "cmpf32",      none) SEPARATOR \
    OPCODE(CMPF32R,      "cmpf32r",     none) SEPARATOR \
    OPCODE(CMPF32I,      "cmpf32i",     none) SEPARATOR \
    OPCODE(CMPF64,       "cmpf64",      none) SEPARATOR \
    OPCODE(CMPF64R,      "cmpf64r",     none) SEPARATOR \
    OPCODE(CMPF64I,      "cmpf64i",     none) SEPARATOR \
    OPCODE(CMPLD,        "cmpld",       none) SEPARATOR \
    OPCODE(CMPLDR,       "cmpldr",      none) SEPARATOR \
    OPCODE(CMPLDI,       "cmpldi",      none) SEPARATOR \
    OPCODE(CMPF32EQUALS, "cmpf32equals",none) SEPARATOR \
    OPCODE(CMPF64EQUALS, "cmpf64equals",none) SEPARATOR \
    OPCODE(CMPLDEQUALS,  "cmpldequals", none) SEPARATOR \
    OPCODE(CMPF32TRUNC1, "cmpf32trunc1",none) SEPARATOR \
    OPCODE(CMPF64TRUNC1, "cmpf64trunc1",none) SEPARATOR \
    OPCODE(CMPLDTRUNC1,  "cmpldtrunc1", none) SEPARATOR \
    OPCODE(CMPF32ADD,    "cmpf32add",   none) SEPARATOR \
    OPCODE(CMPF64ADD,    "cmpf64add",   none) SEPARATOR \
    OPCODE(CMPLDADD,     "cmpldadd",   none) SEPARATOR \
    OPCODE(CMPF32SUB,    "cmpf32sub",   none) SEPARATOR \
    OPCODE(CMPF64SUB,    "cmpf64sub",   none) SEPARATOR \
    OPCODE(CMPLDSUB,     "cmpldsub",   none) SEPARATOR \
    OPCODE(CMPF32MUL,    "cmpf32mul",   none) SEPARATOR \
    OPCODE(CMPF64MUL,    "cmpf64mul",   none) SEPARATOR \
    OPCODE(CMPLDMUL,     "cmpldmul",   none) SEPARATOR \
    OPCODE(CMPF32DIV,    "cmpf32div",   none) SEPARATOR \
    OPCODE(CMPF64DIV,    "cmpf64div",   none) SEPARATOR \
    OPCODE(CMPLDDIV,     "cmplddiv",   none) SEPARATOR \
    OPCODE(CMPF32NEG,    "cmpf32neg",   none) SEPARATOR \
    OPCODE(CMPF64NEG,    "cmpf64neg",   none) SEPARATOR \
    OPCODE(CMPLDNEG,     "cmpldneg",   none) SEPARATOR \
    /* Atomics */ \
    OPCODE(ATOMIC_LOAD8,   "atomic_load8",  atomic_model) SEPARATOR \
    OPCODE(ATOMIC_LOAD16,  "atomic_load16", atomic_model) SEPARATOR \
    OPCODE(ATOMIC_LOAD32,  "atomic_load32", atomic_model) SEPARATOR \
    OPCODE(ATOMIC_LOAD64,  "atomic_load64", atomic_model) SEPARATOR \
    OPCODE(ATOMIC_STORE8,  "atomic_store8",  atomic_model) SEPARATOR \
    OPCODE(ATOMIC_STORE16, "atomic_store16", atomic_model) SEPARATOR \
    OPCODE(ATOMIC_STORE32, "atomic_store32", atomic_model) SEPARATOR \
    OPCODE(ATOMIC_STORE64, "atomic_store64", atomic_model) SEPARATOR \
    /* Miscallenous */ \
    OPCODE(INLINEASM,    "inlineasm",   u64)
// clang-format on

#endif
