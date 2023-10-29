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

#ifndef KEFIR_CODEGEN_ASMCMP_CONTEXT_H_
#define KEFIR_CODEGEN_ASMCMP_CONTEXT_H_

#include "kefir/core/mem.h"
#include "kefir/core/basic-types.h"
#include "kefir/codegen/asmcmp/base.h"
#include "kefir/core/hashtreeset.h"

typedef kefir_size_t kefir_asmcmp_virtual_register_index_t;
typedef kefir_size_t kefir_asmcmp_instruction_opcode_t;
typedef kefir_size_t kefir_asmcmp_instruction_index_t;
typedef kefir_size_t kefir_asmcmp_label_index_t;

#define KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS 3
#define KEFIR_ASMCMP_INDEX_NONE (~(kefir_asmcmp_instruction_index_t) 0ull)

typedef enum kefir_asmcmp_value_type {
    KEFIR_ASMCMP_VALUE_TYPE_NONE = 0,
    KEFIR_ASMCMP_VALUE_TYPE_INTEGER,
    KEFIR_ASMCMP_VALUE_TYPE_UINTEGER,
    KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER,
    KEFIR_ASMCMP_VALUE_TYPE_INDIRECT,
    KEFIR_ASMCMP_VALUE_TYPE_LOCAL_VAR_ADDRESS
} kefir_asmcmp_value_type_t;

typedef enum kefir_asmcmp_register_type { KEFIR_ASMCMP_REGISTER_GENERAL_PURPOSE } kefir_asmcmp_register_type_t;

typedef enum kefir_asmcmp_register_variant {
    KEFIR_ASMCMP_REGISTER_VARIANT_NONE,
    KEFIR_ASMCMP_REGISTER_VARIANT_8BIT,
    KEFIR_ASMCMP_REGISTER_VARIANT_16BIT,
    KEFIR_ASMCMP_REGISTER_VARIANT_32BIT,
    KEFIR_ASMCMP_REGISTER_VARIANT_64BIT
} kefir_asmcmp_register_variant_t;

typedef struct kefir_asmcmp_value {
    kefir_asmcmp_value_type_t type;

    union {
        kefir_int64_t int_immediate;
        kefir_uint64_t uint_immediate;
        struct {
            kefir_asmcmp_virtual_register_index_t index;
            kefir_asmcmp_register_variant_t variant;
        } vreg;
        struct {
            kefir_asmcmp_virtual_register_index_t base;
            kefir_int64_t offset;
            kefir_asmcmp_register_variant_t variant;
        } indirect;
        kefir_size_t local_var_offset;
    };
} kefir_asmcmp_value_t;

#define KEFIR_ASMCMP_MAKE_NONE ((struct kefir_asmcmp_value){.type = KEFI_ASMCMP_VALUE_NONE})
#define KEFIR_ASMCMP_MAKE_INT(_value) \
    ((struct kefir_asmcmp_value){.type = KEFIR_ASMCMP_VALUE_TYPE_INTEGER, .int_immediate = (_value)})
#define KEFIR_ASMCMP_MAKE_UINT(_value) \
    ((struct kefir_asmcmp_value){.type = KEFIR_ASMCMP_VALUE_TYPE_UINTEGER, .uint_immediate = (_value)})
#define KEFIR_ASMCMP_MAKE_VREG8(_vreg)                                             \
    ((struct kefir_asmcmp_value){.type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER, \
                                 .vreg = {.index = (_vreg), .variant = KEFIR_ASMCMP_REGISTER_VARIANT_8BIT}})
#define KEFIR_ASMCMP_MAKE_VREG16(_vreg)                                            \
    ((struct kefir_asmcmp_value){.type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER, \
                                 .vreg = {.index = (_vreg), .variant = KEFIR_ASMCMP_REGISTER_VARIANT_16BIT}})
#define KEFIR_ASMCMP_MAKE_VREG32(_vreg)                                            \
    ((struct kefir_asmcmp_value){.type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER, \
                                 .vreg = {.index = (_vreg), .variant = KEFIR_ASMCMP_REGISTER_VARIANT_32BIT}})
#define KEFIR_ASMCMP_MAKE_VREG64(_vreg)                                            \
    ((struct kefir_asmcmp_value){.type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER, \
                                 .vreg = {.index = (_vreg), .variant = KEFIR_ASMCMP_REGISTER_VARIANT_64BIT}})
#define KEFIR_ASMCMP_MAKE_INDIRECT(_vreg, _offset, _variant)               \
    ((struct kefir_asmcmp_value){.type = KEFIR_ASMCMP_VALUE_TYPE_INDIRECT, \
                                 .indirect = {.base = (_vreg), .offset = (_offset), .variant = (_variant)}})
#define KEFIR_ASMCMP_MAKE_LOCAL_VAR_ADDR(_offset) \
    ((struct kefir_asmcmp_value){.type = KEFIR_ASMCMP_VALUE_TYPE_LOCAL_VAR_ADDRESS, .local_var_offset = (_offset)})

typedef struct kefir_asmcmp_instruction {
    kefir_asmcmp_instruction_opcode_t opcode;
    struct kefir_asmcmp_value args[KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS];
} kefir_asmcmp_instruction_t;

typedef struct kefir_asmcmp_instruction_handle {
    kefir_asmcmp_instruction_index_t index;

    struct {
        kefir_asmcmp_instruction_index_t prev;
        kefir_asmcmp_instruction_index_t next;
    } siblings;

    struct kefir_asmcmp_instruction instr;
} kefir_asmcmp_instruction_handle_t;

typedef struct kefir_asmcmp_label {
    kefir_asmcmp_label_index_t label;
    kefir_bool_t attached;
    kefir_asmcmp_instruction_index_t position;

    struct {
        kefir_asmcmp_instruction_index_t prev;
        kefir_asmcmp_instruction_index_t next;
    } siblings;
} kefir_asmcmp_label_t;

typedef struct kefir_asmcmp_virtual_register {
    kefir_asmcmp_virtual_register_index_t index;
    kefir_asmcmp_register_type_t type;
} kefir_asmcmp_virtual_register_t;

typedef struct kefir_asmcmp_context_class {
    kefir_result_t (*opcode_mnemonic)(kefir_asmcmp_instruction_opcode_t, const char **, void *);
} kefir_asmcmp_context_class_t;

typedef struct kefir_asmcmp_context {
    struct kefir_asmcmp_instruction_handle *code_content;
    kefir_size_t code_length;
    kefir_size_t code_capacity;

    struct {
        kefir_asmcmp_instruction_index_t head;
        kefir_asmcmp_instruction_index_t tail;
    } code;

    struct kefir_asmcmp_label *labels;
    kefir_size_t labels_length;
    kefir_size_t labels_capacity;

    struct kefir_hashtree label_positions;

    struct kefir_asmcmp_virtual_register *virtual_registers;
    kefir_size_t virtual_register_length;
    kefir_size_t virtual_register_capacity;

    const struct kefir_asmcmp_context_class *klass;
    void *payload;
} kefir_asmcmp_context_t;

#define KEFIR_ASMCMP_CONTEXT_OPCODE_MNEMONIC(_ctx, _opcode, _mnemonic) \
    ((_ctx)->klass->opcode_mnemonic((_opcode), (_mnemonic), (_ctx)->payload))

kefir_result_t kefir_asmcmp_context_init(const struct kefir_asmcmp_context_class *, void *,
                                         struct kefir_asmcmp_context *);
kefir_result_t kefir_asmcmp_context_free(struct kefir_mem *, struct kefir_asmcmp_context *);

kefir_result_t kefir_asmcmp_context_instr_at(const struct kefir_asmcmp_context *, kefir_asmcmp_instruction_index_t,
                                             const struct kefir_asmcmp_instruction **);
kefir_asmcmp_instruction_index_t kefir_asmcmp_context_instr_prev(const struct kefir_asmcmp_context *,
                                                                 kefir_asmcmp_instruction_index_t);
kefir_asmcmp_instruction_index_t kefir_asmcmp_context_instr_next(const struct kefir_asmcmp_context *,
                                                                 kefir_asmcmp_instruction_index_t);
kefir_asmcmp_instruction_index_t kefir_asmcmp_context_instr_head(const struct kefir_asmcmp_context *);
kefir_asmcmp_instruction_index_t kefir_asmcmp_context_instr_tail(const struct kefir_asmcmp_context *);
kefir_result_t kefir_asmcmp_context_instr_insert_after(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                       kefir_asmcmp_instruction_index_t,
                                                       const struct kefir_asmcmp_instruction *,
                                                       kefir_asmcmp_instruction_index_t *);
kefir_result_t kefir_asmcmp_context_instr_drop(struct kefir_asmcmp_context *, kefir_asmcmp_instruction_index_t);

kefir_result_t kefir_asmcmp_context_new_label(struct kefir_mem *, struct kefir_asmcmp_context *,
                                              kefir_asmcmp_instruction_index_t, kefir_asmcmp_label_index_t *);
kefir_result_t kefir_asmcmp_context_label_at(const struct kefir_asmcmp_context *, kefir_asmcmp_label_index_t,
                                             kefir_asmcmp_instruction_index_t *);
kefir_asmcmp_label_index_t kefir_asmcmp_context_instr_label_head(const struct kefir_asmcmp_context *,
                                                                 kefir_asmcmp_instruction_index_t);
kefir_asmcmp_label_index_t kefir_asmcmp_context_instr_label_prev(const struct kefir_asmcmp_context *,
                                                                 kefir_asmcmp_label_index_t);
kefir_asmcmp_label_index_t kefir_asmcmp_context_instr_label_next(const struct kefir_asmcmp_context *,
                                                                 kefir_asmcmp_label_index_t);
kefir_result_t kefir_asmcmp_context_bind_label(struct kefir_mem *, struct kefir_asmcmp_context *,
                                               kefir_asmcmp_instruction_index_t, kefir_asmcmp_label_index_t);
kefir_result_t kefir_asmcmp_context_unbind_label(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                 kefir_asmcmp_label_index_t);

kefir_result_t kefir_asmcmp_number_of_virtual_registers(const struct kefir_asmcmp_context *, kefir_size_t *);
kefir_result_t kefir_asmcmp_virtual_register_get(const struct kefir_asmcmp_context *,
                                                 kefir_asmcmp_virtual_register_index_t,
                                                 const struct kefir_asmcmp_virtual_register **);
kefir_result_t kefir_asmcmp_virtual_register_new(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                 kefir_asmcmp_register_type_t, kefir_asmcmp_virtual_register_index_t *);

#endif
