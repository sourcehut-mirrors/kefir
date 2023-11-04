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

#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_Virtual virtual
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_Repeat repeat
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_None 0
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_Jump 1
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_RegR 1
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_RegW 1
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_RegMemW_RegMemR 2
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_RegW_RegMemR 2
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_RegW_Mem 2
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_RegMemRW_RegMemR 2
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_RegRW_RegMemR 2
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_RegMemRW_RegR 2
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_RegMemR_RegR 2
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_RegMemRW 1
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_RegMemW 1
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR(_klass) KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_##_klass

// clang-format off
#define KEFIR_ASMCMP_AMD64_OPCODES(_opcode, _separator) \
    /* Virtual opcodes */ \
    _opcode(virtual_register_link, _, Virtual) _separator \
    _opcode(touch_virtual_register, _, Virtual) _separator \
    _opcode(load_local_var_address, _, Virtual) _separator \
    _opcode(function_prologue, _, Virtual) _separator \
    _opcode(function_epilogue, _, Virtual) _separator \
    _opcode(noop, _, Virtual) _separator \
    _opcode(stash_activate, _, Virtual) _separator \
    _opcode(stash_deactivate, _, Virtual) _separator \
    /* AMD64 opcodes */ \
    /* Control flow */ \
    _opcode(call, CALL, Jump) _separator \
    _opcode(ret, RET, None) _separator \
    _opcode(jmp, JMP, Jump) _separator \
    _opcode(jz, JZ, Jump) _separator \
    /* Data moves */ \
    _opcode(push, PUSH, RegR) _separator \
    _opcode(pop, POP, RegW) _separator \
    _opcode(mov, MOV, RegMemW_RegMemR) _separator \
    _opcode(movabs, MOVABS, RegMemW_RegMemR) _separator \
    _opcode(movsx, MOVSX, RegW_RegMemR) _separator \
    _opcode(movzx, MOVZX, RegW_RegMemR) _separator \
    _opcode(lea, LEA, RegW_Mem) _separator \
    _opcode(movsb_rep, MOVSB, Repeat) _separator \
    _opcode(stosb_rep, STOSB, Repeat) _separator \
    _opcode(cmovl, CMOVL, RegW_RegMemR) _separator \
    /* Flags */ \
    _opcode(cld, CLD, None) _separator \
    /* Special */ \
    _opcode(data16, DATA16, None) _separator \
    _opcode(rexw, REXW, None) _separator \
    /* Integral arithmetics & logic */ \
    _opcode(add, ADD, RegMemRW_RegMemR) _separator \
    _opcode(sub, SUB, RegMemRW_RegMemR) _separator \
    _opcode(imul, IMUL, RegRW_RegMemR) _separator \
    _opcode(idiv, IDIV, RegMemRW) _separator \
    _opcode(div, DIV, RegMemRW) _separator \
    _opcode(shl, SHL, RegMemRW_RegR) _separator \
    _opcode(shr, SHR, RegMemRW_RegR) _separator \
    _opcode(sar, SAR, RegMemRW_RegR) _separator \
    _opcode(cqo, CQO, None) _separator \
    _opcode(and, AND, RegRW_RegMemR) _separator \
    _opcode(or, OR, RegRW_RegMemR) _separator \
    _opcode(xor, XOR, RegRW_RegMemR) _separator \
    _opcode(not, NOT, RegMemRW) _separator \
    _opcode(neg, NEG, RegMemRW) _separator \
    /* Conditionals */ \
    _opcode(test, TEST, RegMemR_RegR) _separator \
    _opcode(cmp, CMP, RegMemR_RegR) _separator \
    _opcode(sete, SETE, RegMemW) _separator \
    _opcode(setne, SETNE, RegMemW) _separator \
    _opcode(setg, SETG, RegMemW) _separator \
    _opcode(setl, SETL, RegMemW) _separator \
    _opcode(seta, SETA, RegMemW) _separator \
    _opcode(setb, SETB, RegMemW) _separator \
    /* Data */ \
    _opcode(data_word, _, Virtual)
// clang-format on

#define KEFIR_ASMCMP_AMD64_OPCODE(_opcode) KEFIR_ASMCMP_AMD64_##_opcode
typedef enum kefir_asmcmp_amd64_opcode {
#define DEF_OPCODE(_opcode, _xasmgen, _argclass) KEFIR_ASMCMP_AMD64_OPCODE(_opcode)
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
#define DEF_OPCODE_0(_opcode)                                                                    \
    kefir_result_t kefir_asmcmp_amd64_##_opcode(struct kefir_mem *, struct kefir_asmcmp_amd64 *, \
                                                kefir_asmcmp_instruction_index_t, kefir_asmcmp_instruction_index_t *);
#define DEF_OPCODE_repeat(_opcode) DEF_OPCODE_0(_opcode)
#define DEF_OPCODE_1(_opcode)                                                                                        \
    kefir_result_t kefir_asmcmp_amd64_##_opcode(struct kefir_mem *, struct kefir_asmcmp_amd64 *,                     \
                                                kefir_asmcmp_instruction_index_t, const struct kefir_asmcmp_value *, \
                                                kefir_asmcmp_instruction_index_t *);
#define DEF_OPCODE_2(_opcode)                                                              \
    kefir_result_t kefir_asmcmp_amd64_##_opcode(                                           \
        struct kefir_mem *, struct kefir_asmcmp_amd64 *, kefir_asmcmp_instruction_index_t, \
        const struct kefir_asmcmp_value *, const struct kefir_asmcmp_value *, kefir_asmcmp_instruction_index_t *);
#define DEF_OPCODE_helper2(_argc, _opcode) DEF_OPCODE_##_argc(_opcode)
#define DEF_OPCODE_helper(_argc, _opcode) DEF_OPCODE_helper2(_argc, _opcode)
#define DEF_OPCODE(_opcode, _xasmgen, _argclass) \
    DEF_OPCODE_helper(KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR(_argclass), _opcode)

KEFIR_ASMCMP_AMD64_OPCODES(DEF_OPCODE, )

#undef DEF_OPCODE_0
#undef DEF_OPCODE_repeat
#undef DEF_OPCODE_1
#undef DEF_OPCODE_2
#undef DEF_OPCODE_virtual
#undef DEF_OPCODE
#undef DEF_OPCODE_helper
#undef DEF_OPCODE_helper2

kefir_result_t kefir_asmcmp_amd64_link_virtual_registers(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                         kefir_asmcmp_instruction_index_t,
                                                         kefir_asmcmp_virtual_register_index_t,
                                                         kefir_asmcmp_virtual_register_index_t,
                                                         kefir_asmcmp_instruction_index_t *);

kefir_result_t kefir_asmcmp_amd64_touch_virtual_register(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                         kefir_asmcmp_instruction_index_t,
                                                         kefir_asmcmp_virtual_register_index_t,
                                                         kefir_asmcmp_instruction_index_t *);

kefir_result_t kefir_asmcmp_amd64_activate_stash(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                 kefir_asmcmp_instruction_index_t, kefir_asmcmp_stash_index_t,
                                                 kefir_asmcmp_instruction_index_t *);
kefir_result_t kefir_asmcmp_amd64_deactivate_stash(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                   kefir_asmcmp_instruction_index_t, kefir_asmcmp_stash_index_t,
                                                   kefir_asmcmp_instruction_index_t *);

kefir_result_t kefir_asmcmp_amd64_function_prologue(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                    kefir_asmcmp_instruction_index_t,
                                                    kefir_asmcmp_instruction_index_t *);
kefir_result_t kefir_asmcmp_amd64_function_epilogue(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                    kefir_asmcmp_instruction_index_t,
                                                    kefir_asmcmp_instruction_index_t *);

kefir_result_t kefir_asmcmp_amd64_data_word(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                            kefir_asmcmp_instruction_index_t, kefir_uint16_t,
                                            kefir_asmcmp_instruction_index_t *);

kefir_result_t kefir_asmcmp_amd64_generate_code(struct kefir_amd64_xasmgen *, const struct kefir_asmcmp_amd64 *,
                                                const struct kefir_codegen_amd64_stack_frame *);

#endif
