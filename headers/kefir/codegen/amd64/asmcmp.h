/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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
#include "kefir/target/asm/amd64/db.h"
#include "kefir/target/abi/amd64/base.h"
#include "kefir/core/util.h"

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
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_Any_Any 2
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_XmmdW_RegMemR 2
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_XmmqW_RegMemR 2
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_XmmRW_XmmMemR 2
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_X87MemR 1
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_X87MemW 1
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_MemR 1
#define KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR(_klass) KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR_IMPL_##_klass

#define KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODE_HELPER(_instr0, _opcode) \
    _instr0(_opcode, #_opcode, VIRTUAL, KEFIR_AMD64_INSTRDB_NONE)
// clang-format off
#define KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODES(_instr0, _separator) \
    /* Virtual opcodes */ \
    KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODE_HELPER(_instr0, virtual_register_link) _separator \
    KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODE_HELPER(_instr0, touch_virtual_register) _separator \
    KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODE_HELPER(_instr0, preserve_active_virtual_registers) _separator \
    KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODE_HELPER(_instr0, load_local_var_address) _separator \
    KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODE_HELPER(_instr0, function_prologue) _separator \
    KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODE_HELPER(_instr0, function_epilogue) _separator \
    KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODE_HELPER(_instr0, noop) _separator \
    KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODE_HELPER(_instr0, stash_activate) _separator \
    KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODE_HELPER(_instr0, stash_deactivate) _separator \
    KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODE_HELPER(_instr0, virtual_block_begin) _separator \
    KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODE_HELPER(_instr0, virtual_block_end) _separator \
    KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODE_HELPER(_instr0, inline_assembly) _separator \
    KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODE_HELPER(_instr0, data_word)
// clang-format on

#define KEFIR_ASMCMP_AMD64_OPCODE(_opcode) KEFIR_ASMCMP_AMD64_##_opcode
typedef enum kefir_asmcmp_amd64_opcode {
#define DEF_OPCODE(_opcode, ...) KEFIR_ASMCMP_AMD64_OPCODE(_opcode)
    KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODES(DEF_OPCODE, COMMA),
    KEFIR_AMD64_INSTRUCTION_DATABASE(DEF_OPCODE, DEF_OPCODE, DEF_OPCODE, DEF_OPCODE, COMMA)
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
    kefir_bool_t position_independent_code;
    struct kefir_hashtreeset externals;
} kefir_asmcmp_amd64_t;

kefir_result_t kefir_asmcmp_amd64_init(const char *, kefir_abi_amd64_variant_t, kefir_bool_t,
                                       struct kefir_asmcmp_amd64 *);
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

#define DEF_OPCODE0_(_opcode)                                                                    \
    kefir_result_t kefir_asmcmp_amd64_##_opcode(struct kefir_mem *, struct kefir_asmcmp_amd64 *, \
                                                kefir_asmcmp_instruction_index_t, kefir_asmcmp_instruction_index_t *);
#define DEF_OPCODE0_PREFIX(_opcode) DEF_OPCODE0_(_opcode)
#define DEF_OPCODE0_REPEATABLE(_opcode) DEF_OPCODE0_(_opcode##_rep)
#define DEF_OPCODE0(_opcode, _mnemonic, _variant, _flags) DEF_OPCODE0_##_variant(_opcode)
#define DEF_OPCODE1(_opcode, _mnemonic, _variant, _flags, _op1)                                                      \
    kefir_result_t kefir_asmcmp_amd64_##_opcode(struct kefir_mem *, struct kefir_asmcmp_amd64 *,                     \
                                                kefir_asmcmp_instruction_index_t, const struct kefir_asmcmp_value *, \
                                                kefir_asmcmp_instruction_index_t *);
#define DEF_OPCODE2(_opcode, _mnemonic, _variant, _flags, _op1, _op2)                      \
    kefir_result_t kefir_asmcmp_amd64_##_opcode(                                           \
        struct kefir_mem *, struct kefir_asmcmp_amd64 *, kefir_asmcmp_instruction_index_t, \
        const struct kefir_asmcmp_value *, const struct kefir_asmcmp_value *, kefir_asmcmp_instruction_index_t *);
#define DEF_OPCODE3(_opcode, _mnemonic, _variant, _flags, _op1, _op2, _op3)                                           \
    kefir_result_t kefir_asmcmp_amd64_##_opcode(struct kefir_mem *, struct kefir_asmcmp_amd64 *,                      \
                                                kefir_asmcmp_instruction_index_t, const struct kefir_asmcmp_value *,  \
                                                const struct kefir_asmcmp_value *, const struct kefir_asmcmp_value *, \
                                                kefir_asmcmp_instruction_index_t *);

KEFIR_AMD64_INSTRUCTION_DATABASE(DEF_OPCODE0, DEF_OPCODE1, DEF_OPCODE2, DEF_OPCODE3, )

#undef DEF_OPCODE0_
#undef DEF_OPCODE0_REPEATABLE
#undef DEF_OPCODE0_PREFIX
#undef DEF_OPCODE0
#undef DEF_OPCODE1
#undef DEF_OPCODE2
#undef DEF_OPCODE3

kefir_result_t kefir_asmcmp_amd64_link_virtual_registers(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                         kefir_asmcmp_instruction_index_t,
                                                         kefir_asmcmp_virtual_register_index_t,
                                                         kefir_asmcmp_virtual_register_index_t,
                                                         kefir_asmcmp_instruction_index_t *);

kefir_result_t kefir_asmcmp_amd64_touch_virtual_register(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                         kefir_asmcmp_instruction_index_t,
                                                         kefir_asmcmp_virtual_register_index_t,
                                                         kefir_asmcmp_instruction_index_t *);

kefir_result_t kefir_asmcmp_amd64_preserve_active_virtual_registers(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                                    kefir_asmcmp_instruction_index_t,
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

kefir_result_t kefir_asmcmp_amd64_inline_assembly(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                  kefir_asmcmp_instruction_index_t,
                                                  kefir_asmcmp_inline_assembly_index_t,
                                                  kefir_asmcmp_instruction_index_t *);

kefir_result_t kefir_asmcmp_amd64_virtual_block_begin(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                      kefir_asmcmp_instruction_index_t, kefir_uint64_t,
                                                      kefir_asmcmp_instruction_index_t *);

kefir_result_t kefir_asmcmp_amd64_virtual_block_end(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                    kefir_asmcmp_instruction_index_t, kefir_uint64_t,
                                                    kefir_asmcmp_instruction_index_t *);

kefir_result_t kefir_asmcmp_amd64_noop(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                       kefir_asmcmp_instruction_index_t, kefir_asmcmp_instruction_index_t *);

kefir_result_t kefir_asmcmp_amd64_data_word(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                            kefir_asmcmp_instruction_index_t, kefir_uint16_t,
                                            kefir_asmcmp_instruction_index_t *);

kefir_result_t kefir_asmcmp_amd64_generate_code(struct kefir_mem *, struct kefir_amd64_xasmgen *,
                                                kefir_amd64_xasmgen_debug_info_tracker_t,
                                                const struct kefir_asmcmp_amd64 *,
                                                const struct kefir_codegen_amd64_stack_frame *);

extern const struct kefir_asmcmp_context_class KEFIR_ASMCMP_AMD64_KLASS;

#endif
