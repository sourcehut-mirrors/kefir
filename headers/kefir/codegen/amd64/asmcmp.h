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
#include "kefir/target/abi/amd64/base.h"

typedef struct kefir_codegen_amd64_register_allocator kefir_codegen_amd64_register_allocator_t;  // Forward declaration
typedef struct kefir_codegen_amd64_stack_frame kefir_codegen_amd64_stack_frame_t;                // Forward declaration

// clang-format off
#define KEFIR_ASMCMP_AMD64_OPCODES(_opcode, _separator) \
    /* Virtual opcodes */ \
    _opcode(virtual_register_link, _, virtual) _separator \
    _opcode(touch_virtual_register, _, virtual) _separator \
    _opcode(load_local_var_address, _, virtual) _separator \
    _opcode(function_prologue, _, virtual) _separator \
    _opcode(function_epilogue, _, virtual) _separator \
    /* AMD64 opcodes */ \
    /* Control flow */ \
    _opcode(ret, RET, arg0) _separator \
    /* Data moves */ \
    _opcode(mov, MOV, arg2) _separator \
    _opcode(movabs, MOVABS, arg2) _separator \
    _opcode(movsx, MOVSX, arg2) _separator \
    _opcode(movzx, MOVZX, arg2) _separator \
    _opcode(lea, LEA, arg2) _separator \
    /* Integral arithmetics & logic */ \
    _opcode(add, ADD, arg2) _separator \
    _opcode(sub, SUB, arg2) _separator \
    _opcode(imul, IMUL, arg2) _separator \
    _opcode(and, AND, arg2) _separator \
    _opcode(or, OR, arg2) _separator \
    _opcode(xor, XOR, arg2)
// clang-format on

#define KEFIR_ASMCMP_AMD64_OPCODE(_opcode) KEFIR_ASMCMP_AMD64_##_opcode
typedef enum kefir_asmcmp_amd64_opcode {
#define DEF_OPCODE(_opcode, _xasmgen, _argtp) KEFIR_ASMCMP_AMD64_OPCODE(_opcode)
    KEFIR_ASMCMP_AMD64_OPCODES(DEF_OPCODE, COMMA)
#undef DEF_OPCODE
} kefir_asmcmp_amd64_opcode_t;

typedef enum kefir_asmcmp_amd64_register_preallocation_type {
    KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_SAME_AS,
    KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_HINT,
    KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT
} kefir_asmcmp_amd64_register_preallocation_type_t;

typedef struct kefir_asmcmp_amd64 {
    struct kefir_asmcmp_context context;
    const char *function_name;
    struct kefir_hashtree register_preallocation;
    kefir_abi_amd64_variant_t abi_variant;
} kefir_asmcmp_amd64_t;

kefir_result_t kefir_asmcmp_amd64_init(const char *, kefir_abi_amd64_variant_t, struct kefir_asmcmp_amd64 *);
kefir_result_t kefir_asmcmp_amd64_free(struct kefir_mem *, struct kefir_asmcmp_amd64 *);

typedef struct kefir_asmcmp_amd64_register_preallocation {
    kefir_asmcmp_amd64_register_preallocation_type_t type;
    union {
        kefir_asm_amd64_xasmgen_register_t reg;
        kefir_asmcmp_virtual_register_index_t vreg;
    };
} kefir_asmcmp_amd64_register_preallocation_t;

kefir_result_t kefir_asmcmp_amd64_register_allocation_same_as(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                              kefir_asmcmp_virtual_register_index_t,
                                                              kefir_asmcmp_virtual_register_index_t);

kefir_result_t kefir_asmcmp_amd64_register_allocation_hint(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                           kefir_asmcmp_virtual_register_index_t,
                                                           kefir_asm_amd64_xasmgen_register_t);

kefir_result_t kefir_asmcmp_amd64_register_allocation_requirement(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                                  kefir_asmcmp_virtual_register_index_t,
                                                                  kefir_asm_amd64_xasmgen_register_t);

kefir_result_t kefir_asmcmp_amd64_get_register_preallocation(const struct kefir_asmcmp_amd64 *,
                                                             kefir_asmcmp_virtual_register_index_t,
                                                             const struct kefir_asmcmp_amd64_register_preallocation **);

#define DEF_OPCODE_virtual(_opcode)
#define DEF_OPCODE_arg0(_opcode)                                                                 \
    kefir_result_t kefir_asmcmp_amd64_##_opcode(struct kefir_mem *, struct kefir_asmcmp_amd64 *, \
                                                kefir_asmcmp_instruction_index_t, kefir_asmcmp_instruction_index_t *);
#define DEF_OPCODE_arg2(_opcode)                                                           \
    kefir_result_t kefir_asmcmp_amd64_##_opcode(                                           \
        struct kefir_mem *, struct kefir_asmcmp_amd64 *, kefir_asmcmp_instruction_index_t, \
        const struct kefir_asmcmp_value *, const struct kefir_asmcmp_value *, kefir_asmcmp_instruction_index_t *);
#define DEF_OPCODE(_opcode, _mnemonic, _argtp) DEF_OPCODE_##_argtp(_opcode)

KEFIR_ASMCMP_AMD64_OPCODES(DEF_OPCODE, )

#undef DEF_OPCODE_arg0
#undef DEF_OPCODE_arg2
#undef DEF_OPCODE_virtual
#undef DEF_OPCODE

kefir_result_t kefir_asmcmp_amd64_link_virtual_registers(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                         kefir_asmcmp_instruction_index_t,
                                                         kefir_asmcmp_virtual_register_index_t,
                                                         kefir_asmcmp_virtual_register_index_t,
                                                         kefir_asmcmp_instruction_index_t *);

kefir_result_t kefir_asmcmp_amd64_touch_virtual_register(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                         kefir_asmcmp_instruction_index_t,
                                                         kefir_asmcmp_virtual_register_index_t,
                                                         kefir_asmcmp_instruction_index_t *);

kefir_result_t kefir_asmcmp_amd64_function_prologue(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                    kefir_asmcmp_instruction_index_t,
                                                    kefir_asmcmp_instruction_index_t *);
kefir_result_t kefir_asmcmp_amd64_function_epilogue(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                    kefir_asmcmp_instruction_index_t,
                                                    kefir_asmcmp_instruction_index_t *);

kefir_result_t kefir_asmcmp_amd64_generate_code(struct kefir_mem *, struct kefir_amd64_xasmgen *,
                                                const struct kefir_asmcmp_amd64 *,
                                                const struct kefir_codegen_amd64_register_allocator *,
                                                const struct kefir_codegen_amd64_stack_frame *);

#endif
