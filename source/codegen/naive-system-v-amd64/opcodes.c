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

#include "kefir/codegen/naive-system-v-amd64/opcodes.h"
#include "kefir/core/util.h"
#include "kefir/ir/mnemonic.h"

#define KEFIR_IROPCODE_MNEMONICS(_id, _mnemonic, _code, _type) \
    const char KEFIR_IROPCODE_HANDLER_LABEL_##_id[] = "__kefirrt_" _mnemonic "_impl";
KEFIR_IR_OPCODE_DEFS(KEFIR_IROPCODE_MNEMONICS, )
#undef KEFIR_IROPCODE_MNEMONICS

#define HANDLER(opcode) \
    { KEFIR_IROPCODE_##opcode, KEFIR_IROPCODE_HANDLER_LABEL_##opcode }

static struct {
    kefir_iropcode_t opcode;
    const char *handler;
} OPCODE_HANDLERS[] = {
    HANDLER(NOP),       HANDLER(JMP),        HANDLER(IJMP),      HANDLER(BRANCH),     HANDLER(PUSHI64),
    HANDLER(POP),       HANDLER(PICK),       HANDLER(XCHG),      HANDLER(IADD),       HANDLER(IADD1),
    HANDLER(ISUB),      HANDLER(IMUL),       HANDLER(IDIV),      HANDLER(IMOD),       HANDLER(UDIV),
    HANDLER(UMOD),      HANDLER(INEG),       HANDLER(INOT),      HANDLER(IAND),       HANDLER(IOR),
    HANDLER(IXOR),      HANDLER(IRSHIFT),    HANDLER(IARSHIFT),  HANDLER(ILSHIFT),    HANDLER(IEQUALS),
    HANDLER(IGREATER),  HANDLER(ILESSER),    HANDLER(IABOVE),    HANDLER(IBELOW),     HANDLER(BAND),
    HANDLER(BOR),       HANDLER(BNOT),       HANDLER(TRUNCATE1), HANDLER(EXTEND8),    HANDLER(EXTEND16),
    HANDLER(EXTEND32),  HANDLER(IADDX),      HANDLER(LOAD8U),    HANDLER(LOAD8I),     HANDLER(LOAD16U),
    HANDLER(LOAD16I),   HANDLER(LOAD32U),    HANDLER(LOAD32I),   HANDLER(LOAD64),     HANDLER(STORE8),
    HANDLER(STORE16),   HANDLER(STORE32),    HANDLER(STORE64),   HANDLER(STORELD),    HANDLER(BZERO),
    HANDLER(BCOPY),     HANDLER(EXTUBITS),   HANDLER(EXTSBITS),  HANDLER(INSERTBITS), HANDLER(GETLOCAL),
    HANDLER(F32ADD),    HANDLER(F32SUB),     HANDLER(F32MUL),    HANDLER(F32DIV),     HANDLER(F32NEG),
    HANDLER(F64ADD),    HANDLER(F64SUB),     HANDLER(F64MUL),    HANDLER(F64DIV),     HANDLER(F64NEG),
    HANDLER(LDADD),     HANDLER(LDSUB),      HANDLER(LDMUL),     HANDLER(LDDIV),      HANDLER(LDNEG),
    HANDLER(F32EQUALS), HANDLER(F32GREATER), HANDLER(F32LESSER), HANDLER(F64EQUALS),  HANDLER(F64GREATER),
    HANDLER(F64LESSER), HANDLER(LDEQUALS),   HANDLER(LDGREATER), HANDLER(LDLESSER),   HANDLER(LDTRUNC1),
    HANDLER(F32CINT),   HANDLER(F64CINT),    HANDLER(F32CUINT),  HANDLER(F64CUINT),   HANDLER(INTCF32),
    HANDLER(INTCF64),   HANDLER(UINTCF32),   HANDLER(UINTCF64),  HANDLER(F32CF64),    HANDLER(F64CF32),
    HANDLER(ALLOCA),    HANDLER(INTCLD),     HANDLER(UINTCLD),   HANDLER(LDCINT),     HANDLER(LDCUINT),
    HANDLER(F32CLD),    HANDLER(F64CLD),     HANDLER(LDCF32),    HANDLER(LDCF64),     HANDLER(PUSHSCOPE),
    HANDLER(POPSCOPE),  HANDLER(LDINITH),    HANDLER(LDINITL)};

const char *kefir_amd64_iropcode_handler(kefir_iropcode_t opcode) {
    for (kefir_size_t i = 0; i < sizeof(OPCODE_HANDLERS) / sizeof(OPCODE_HANDLERS[0]); i++) {
        if (OPCODE_HANDLERS[i].opcode == opcode) {
            return OPCODE_HANDLERS[i].handler;
        }
    }
    return NULL;
}

kefir_result_t kefir_amd64_iropcode_handler_list(kefir_result_t (*callback)(kefir_iropcode_t, const char *, void *),
                                                 void *payload) {
    for (kefir_size_t i = 0; i < sizeof(OPCODE_HANDLERS) / sizeof(OPCODE_HANDLERS[0]); i++) {
        REQUIRE_OK(callback(OPCODE_HANDLERS[i].opcode, OPCODE_HANDLERS[i].handler, payload));
    }
    return KEFIR_OK;
}
