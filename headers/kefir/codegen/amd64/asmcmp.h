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

#ifndef KEFIR_CODEGEN_AMD64_ASMCMP_H_
#define KEFIR_CODEGEN_AMD64_ASMCMP_H_

#include "kefir/codegen/asmcmp/context.h"
#include "kefir/target/asm/amd64/xasmgen.h"

#define KEFIR_ASMCMP_AMD64_OPCODES(_opcode, _separator) _opcode(ret, RET, arg0)

#define KEFIR_ASMCMP_AMD64_OPCODE(_opcode) KEFIR_ASMCMP_AMD64_##_opcode
typedef enum kefir_asmcmp_amd64_opcode {
#define DEF_OPCODE(_opcode, _xasmgen, _argtp) KEFIR_ASMCMP_AMD64_OPCODE(_opcode)
    KEFIR_ASMCMP_AMD64_OPCODES(DEF_OPCODE, COMMA)
#undef DEF_OPCODE
} kefir_asmcmp_amd64_opcode_t;

typedef struct kefir_asmcmp_amd64 {
    struct kefir_asmcmp_context context;
    const char *function_name;
} kefir_asmcmp_amd64_t;

kefir_result_t kefir_asmcmp_amd64_init(const char *, struct kefir_asmcmp_amd64 *);
kefir_result_t kefir_asmcmp_amd64_free(struct kefir_mem *, struct kefir_asmcmp_amd64 *);

#define DEF_OPCODE_arg0(_opcode)                                                                 \
    kefir_result_t kefir_asmcmp_amd64_##_opcode(struct kefir_mem *, struct kefir_asmcmp_amd64 *, \
                                                kefir_asmcmp_instruction_index_t, kefir_asmcmp_instruction_index_t *)
#define DEF_OPCODE_arg2(_opcode)                                                                                     \
    kefir_result_t kefir_asmcmp_amd64_##_opcode(struct kefir_mem *, struct kefir_asmcmp_amd64 *,                     \
                                                kefir_asmcmp_instruction_index_t, const struct kefir_asmcmp_value *, \
                                                const struct kefir_asmcmp_value *, kefir_asmcmp_instruction_index_t *)
#define DEF_OPCODE(_opcode, _mnemonic, _argtp) DEF_OPCODE_##_argtp(_opcode)

KEFIR_ASMCMP_AMD64_OPCODES(DEF_OPCODE, ;);

#undef DEF_OPCODE_arg0
#undef DEF_OPCODE_arg2
#undef DEF_OPCODE

kefir_result_t kefir_asmcmp_amd64_generate_code(struct kefir_amd64_xasmgen *, const struct kefir_asmcmp_amd64 *);

#endif
