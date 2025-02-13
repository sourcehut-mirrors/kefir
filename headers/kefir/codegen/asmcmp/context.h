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

#ifndef KEFIR_CODEGEN_ASMCMP_CONTEXT_H_
#define KEFIR_CODEGEN_ASMCMP_CONTEXT_H_

#include "kefir/core/mem.h"
#include "kefir/core/basic-types.h"
#include "kefir/core/string_pool.h"
#include "kefir/codegen/asmcmp/type_defs.h"
#include "kefir/codegen/asmcmp/debug.h"
#include "kefir/core/hashtreeset.h"
#include "kefir/core/list.h"

typedef enum kefir_asmcmp_value_type {
    KEFIR_ASMCMP_VALUE_TYPE_NONE = 0,
    KEFIR_ASMCMP_VALUE_TYPE_INTEGER,
    KEFIR_ASMCMP_VALUE_TYPE_UINTEGER,
    KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER,
    KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER,
    KEFIR_ASMCMP_VALUE_TYPE_INDIRECT,
    KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL,
    KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL,
    KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL,
    KEFIR_ASMCMP_VALUE_TYPE_EXTERNAL_LABEL,
    KEFIR_ASMCMP_VALUE_TYPE_X87,
    KEFIR_ASMCMP_VALUE_TYPE_STASH_INDEX,
    KEFIR_ASMCMP_VALUE_TYPE_INLINE_ASSEMBLY_INDEX
} kefir_asmcmp_value_type_t;

typedef enum kefir_asmcmp_stack_frame_pointer_base {
    KEFIR_ASMCMP_STACK_FRAME_POINTER_LOCAL_AREA
} kefir_asmcmp_stack_frame_pointer_base_t;

typedef enum kefir_asmcmp_virtual_register_type {
    KEFIR_ASMCMP_VIRTUAL_REGISTER_UNSPECIFIED,
    KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
    KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT,
    KEFIR_ASMCMP_VIRTUAL_REGISTER_DIRECT_SPILL_SPACE,
    KEFIR_ASMCMP_VIRTUAL_REGISTER_INDIRECT_SPILL_SPACE,
    KEFIR_ASMCMP_VIRTUAL_REGISTER_STACK_FRAME_POINTER,
    KEFIR_ASMCMP_VIRTUAL_REGISTER_IMMEDIATE_VALUE,
    KEFIR_ASMCMP_VIRTUAL_REGISTER_EXTERNAL_MEMORY
} kefir_asmcmp_virtual_register_type_t;

typedef enum kefir_asmcmp_operand_variant {
    KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT,
    KEFIR_ASMCMP_OPERAND_VARIANT_8BIT,
    KEFIR_ASMCMP_OPERAND_VARIANT_16BIT,
    KEFIR_ASMCMP_OPERAND_VARIANT_32BIT,
    KEFIR_ASMCMP_OPERAND_VARIANT_64BIT,
    KEFIR_ASMCMP_OPERAND_VARIANT_80BIT,
    KEFIR_ASMCMP_OPERAND_VARIANT_128BIT,
    KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE,
    KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE
} kefir_asmcmp_operand_variant_t;

typedef enum kefir_asmcmp_indirect_basis_type {
    KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS,
    KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS,
    KEFIR_ASMCMP_INDIRECT_INTERNAL_LABEL_BASIS,
    KEFIR_ASMCMP_INDIRECT_EXTERNAL_LABEL_BASIS,
    KEFIR_ASMCMP_INDIRECT_LOCAL_VAR_BASIS,
    KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS,
    KEFIR_ASMCMP_INDIRECT_TEMPORARY_AREA_BASIS,
    KEFIR_ASMCMP_INDIRECT_VARARG_SAVE_AREA_BASIS
} kefir_asmcmp_indirect_basis_type_t;

typedef struct kefir_asmcmp_stash {
    kefir_asmcmp_stash_index_t index;
    kefir_asmcmp_virtual_register_index_t stash_area_vreg;
    kefir_asmcmp_instruction_index_t liveness_instr_index;
    struct kefir_hashtreeset stashed_physical_regs;
    struct kefir_hashtreeset excluded_virtual_regs;
} kefir_asmcmp_stash_t;

typedef enum kefir_asmcmp_inline_assembly_fragment_type {
    KEFIR_ASMCMP_INLINE_ASSEMBLY_FRAGMENT_TEXT,
    KEFIR_ASMCMP_INLINE_ASSEMBLY_FRAGMENT_VALUE
} kefir_asmcmp_inline_assembly_fragment_type_t;

typedef struct kefir_asmcmp_inline_assembly {
    kefir_asmcmp_inline_assembly_index_t index;
    const char *template;
    kefir_size_t template_length;
    struct kefir_list fragments;
} kefir_asmcmp_inline_assembly_t;

typedef enum kefir_asmcmp_external_label_relocation {
    KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
    KEFIR_ASMCMP_EXTERNAL_LABEL_PLT,
    KEFIR_ASMCMP_EXTERNAL_LABEL_GOTPCREL,
    KEFIR_ASMCMP_EXTERNAL_LABEL_TPOFF,
    KEFIR_ASMCMP_EXTERNAL_LABEL_GOTTPOFF,
    KEFIR_ASMCMP_EXTERNAL_LABEL_TLSGD
} kefir_asmcmp_external_label_relocation_t;

typedef struct kefir_asmcmp_value {
    kefir_asmcmp_value_type_t type;

    union {
        kefir_int64_t int_immediate;
        kefir_uint64_t uint_immediate;
        struct {
            kefir_asmcmp_virtual_register_index_t index;
            kefir_asmcmp_operand_variant_t variant;
        } vreg;
        kefir_asmcmp_physical_register_index_t phreg;
        struct {
            kefir_asmcmp_indirect_basis_type_t type;
            union {
                kefir_asmcmp_physical_register_index_t phreg;
                kefir_asmcmp_virtual_register_index_t vreg;
                kefir_asmcmp_label_index_t internal_label;
                struct {
                    kefir_asmcmp_external_label_relocation_t external_type;
                    const char *external_label;
                };
                kefir_size_t spill_index;
            } base;
            kefir_int64_t offset;
            kefir_asmcmp_operand_variant_t variant;
        } indirect;
        struct {
            kefir_asmcmp_external_label_relocation_t position;
            union {
                kefir_asmcmp_label_index_t internal;
                const char *external;
            };
            kefir_asmcmp_operand_variant_t variant;
        } rip_indirection;
        kefir_asmcmp_label_index_t internal_label;
        struct {
            kefir_asmcmp_external_label_relocation_t position;
            const char *symbolic;
            kefir_int64_t offset;
        } external_label;
        kefir_size_t x87;
        kefir_asmcmp_stash_index_t stash_idx;
        kefir_asmcmp_inline_assembly_index_t inline_asm_idx;
    };

    struct {
        kefir_bool_t present;
        kefir_asmcmp_physical_register_index_t reg;
    } segment;

    struct {
        kefir_bool_t write64_to_spill;
    } internal;
} kefir_asmcmp_value_t;

typedef struct kefir_asmcmp_inline_assembly_fragment {
    kefir_asmcmp_inline_assembly_fragment_type_t type;
    union {
        const char *text;

        struct kefir_asmcmp_value value;
    };
} kefir_asmcmp_inline_assembly_fragment_t;

#define KEFIR_ASMCMP_MAKE_NONE ((struct kefir_asmcmp_value) {.type = KEFI_ASMCMP_VALUE_NONE})
#define KEFIR_ASMCMP_MAKE_INT(_value) \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_INTEGER, .int_immediate = (_value)})
#define KEFIR_ASMCMP_MAKE_UINT(_value) \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_UINTEGER, .uint_immediate = (_value)})
#define KEFIR_ASMCMP_MAKE_VREG8(_vreg)                                              \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER, \
                                  .vreg = {.index = (_vreg), .variant = KEFIR_ASMCMP_OPERAND_VARIANT_8BIT}})
#define KEFIR_ASMCMP_MAKE_VREG16(_vreg)                                             \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER, \
                                  .vreg = {.index = (_vreg), .variant = KEFIR_ASMCMP_OPERAND_VARIANT_16BIT}})
#define KEFIR_ASMCMP_MAKE_VREG32(_vreg)                                             \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER, \
                                  .vreg = {.index = (_vreg), .variant = KEFIR_ASMCMP_OPERAND_VARIANT_32BIT}})
#define KEFIR_ASMCMP_MAKE_VREG64(_vreg)                                             \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER, \
                                  .vreg = {.index = (_vreg), .variant = KEFIR_ASMCMP_OPERAND_VARIANT_64BIT}})
#define KEFIR_ASMCMP_MAKE_VREG128(_vreg)                                            \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER, \
                                  .vreg = {.index = (_vreg), .variant = KEFIR_ASMCMP_OPERAND_VARIANT_128BIT}})
#define KEFIR_ASMCMP_MAKE_VREG(_vreg)                                               \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER, \
                                  .vreg = {.index = (_vreg), .variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT}})
#define KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(_label) \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL, .internal_label = (_label)})
#define KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(_position, _label, _offset) \
    ((struct kefir_asmcmp_value) {                                   \
        .type = KEFIR_ASMCMP_VALUE_TYPE_EXTERNAL_LABEL,              \
        .external_label = {.position = (_position), .symbolic = (_label), .offset = (_offset)}})
#define KEFIR_ASMCMP_MAKE_PHREG(_reg) \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER, .phreg = (_reg)})
#define KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(_vreg, _offset, _variant)                        \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_INDIRECT,                 \
                                  .indirect = {.type = KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS, \
                                               .base.vreg = (_vreg),                        \
                                               .offset = (_offset),                         \
                                               .variant = (_variant)}})
#define KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(_reg, _offset, _variant)                         \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_INDIRECT,                  \
                                  .indirect = {.type = KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS, \
                                               .base.phreg = (_reg),                         \
                                               .offset = (_offset),                          \
                                               .variant = (_variant)}})
#define KEFIR_ASMCMP_MAKE_INDIRECT_INTERNAL_LABEL(_label, _variant)                                \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_INDIRECT,                        \
                                  .indirect = {.type = KEFIR_ASMCMP_INDIRECT_INTERNAL_LABEL_BASIS, \
                                               .base.internal_label = (_label),                    \
                                               .offset = 0,                                        \
                                               .variant = (_variant)}})
#define KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(_type, _label, _offset, _variant)                \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_INDIRECT,                        \
                                  .indirect = {.type = KEFIR_ASMCMP_INDIRECT_EXTERNAL_LABEL_BASIS, \
                                               .base.external_type = (_type),                      \
                                               .base.external_label = (_label),                    \
                                               .offset = (_offset),                                \
                                               .variant = (_variant)}})
#define KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_VAR(_offset, _variant) \
    ((struct kefir_asmcmp_value) {                              \
        .type = KEFIR_ASMCMP_VALUE_TYPE_INDIRECT,               \
        .indirect = {.type = KEFIR_ASMCMP_INDIRECT_LOCAL_VAR_BASIS, .offset = (_offset), .variant = (_variant)}})
#define KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(_index, _offset, _variant)                            \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_INDIRECT,                    \
                                  .indirect = {.type = KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS, \
                                               .base.spill_index = (_index),                   \
                                               .offset = (_offset),                            \
                                               .variant = (_variant)}})
#define KEFIR_ASMCMP_MAKE_INDIRECT_TEMPORARY(_offset, _variant) \
    ((struct kefir_asmcmp_value) {                              \
        .type = KEFIR_ASMCMP_VALUE_TYPE_INDIRECT,               \
        .indirect = {.type = KEFIR_ASMCMP_INDIRECT_TEMPORARY_AREA_BASIS, .offset = (_offset), .variant = (_variant)}})
#define KEFIR_ASMCMP_MAKE_INDIRECT_VARARG(_offset, _variant) \
    ((struct kefir_asmcmp_value) {                           \
        .type = KEFIR_ASMCMP_VALUE_TYPE_INDIRECT,            \
        .indirect = {                                        \
            .type = KEFIR_ASMCMP_INDIRECT_VARARG_SAVE_AREA_BASIS, .offset = (_offset), .variant = (_variant)}})
#define KEFIR_ASMCMP_MAKE_RIP_INDIRECT_INTERNAL(_base, _variant) \
    ((struct kefir_asmcmp_value) {                               \
        .type = KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL,   \
        .rip_indirection = {                                     \
            .position = KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE, .internal = (_base), .variant = (_variant)}})
#define KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(_position, _base, _variant) \
    ((struct kefir_asmcmp_value) {                                          \
        .type = KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL,              \
        .rip_indirection = {.position = (_position), .external = (_base), .variant = (_variant)}})
#define KEFIR_ASMCMP_MAKE_X87(_idx) ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_X87, .x87 = (_idx)})
#define KEFIR_ASMCMP_MAKE_STASH_INDEX(_idx) \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_STASH_INDEX, .stash_idx = (_idx)})
#define KEFIR_ASMCMP_MAKE_INLINE_ASSEMBLY(_idx) \
    ((struct kefir_asmcmp_value) {.type = KEFIR_ASMCMP_VALUE_TYPE_INLINE_ASSEMBLY_INDEX, .inline_asm_idx = (_idx)})

#define KEFIR_ASMCMP_SET_SEGMENT(_value, _reg) \
    do {                                       \
        (_value)->segment.present = true;      \
        (_value)->segment.reg = (_reg);        \
    } while (false)

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

    struct kefir_hashtreeset public_labels;
    kefir_bool_t external_dependencies;
} kefir_asmcmp_label_t;

typedef struct kefir_asmcmp_virtual_register {
    kefir_asmcmp_virtual_register_index_t index;
    kefir_asmcmp_virtual_register_type_t type;
    union {
        struct {
            kefir_size_t length;
            kefir_size_t alignment;
        } spill_space_allocation;
        struct {
            kefir_asmcmp_physical_register_index_t base_reg;
            kefir_int64_t offset;
        } memory;
        struct {
            kefir_asmcmp_stack_frame_pointer_base_t base;
            kefir_int64_t offset;
        } stack_frame;
        struct kefir_asmcmp_value immediate_value;
    } parameters;
} kefir_asmcmp_virtual_register_t;

typedef struct kefir_asmcmp_context_class {
    kefir_result_t (*opcode_mnemonic)(kefir_asmcmp_instruction_opcode_t, const char **, void *);
    kefir_result_t (*register_mnemonic)(kefir_asmcmp_physical_register_index_t, const char **, void *);
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

    struct kefir_hashtree vreg_type_dependents;

    struct kefir_hashtree stashes;
    kefir_asmcmp_stash_index_t next_stash_idx;

    struct kefir_hashtree inline_assembly;
    kefir_asmcmp_inline_assembly_index_t next_inline_asm_idx;

    struct kefir_asmcmp_debug_info debug_info;

    struct kefir_string_pool strings;

    const struct kefir_asmcmp_context_class *klass;
    void *payload;
} kefir_asmcmp_context_t;

#define KEFIR_ASMCMP_CONTEXT_OPCODE_MNEMONIC(_ctx, _opcode, _mnemonic) \
    ((_ctx)->klass->opcode_mnemonic((_opcode), (_mnemonic), (_ctx)->payload))

kefir_result_t kefir_asmcmp_context_init(const struct kefir_asmcmp_context_class *, void *,
                                         struct kefir_asmcmp_context *);
kefir_result_t kefir_asmcmp_context_free(struct kefir_mem *, struct kefir_asmcmp_context *);

kefir_result_t kefir_asmcmp_context_instr_at(const struct kefir_asmcmp_context *, kefir_asmcmp_instruction_index_t,
                                             struct kefir_asmcmp_instruction **);
kefir_asmcmp_instruction_index_t kefir_asmcmp_context_instr_prev(const struct kefir_asmcmp_context *,
                                                                 kefir_asmcmp_instruction_index_t);
kefir_asmcmp_instruction_index_t kefir_asmcmp_context_instr_next(const struct kefir_asmcmp_context *,
                                                                 kefir_asmcmp_instruction_index_t);
kefir_asmcmp_instruction_index_t kefir_asmcmp_context_instr_head(const struct kefir_asmcmp_context *);
kefir_asmcmp_instruction_index_t kefir_asmcmp_context_instr_tail(const struct kefir_asmcmp_context *);
kefir_asmcmp_instruction_index_t kefir_asmcmp_context_instr_length(const struct kefir_asmcmp_context *);
kefir_result_t kefir_asmcmp_context_instr_insert_after(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                       kefir_asmcmp_instruction_index_t,
                                                       const struct kefir_asmcmp_instruction *,
                                                       kefir_asmcmp_instruction_index_t *);
kefir_result_t kefir_asmcmp_context_instr_drop(struct kefir_asmcmp_context *, kefir_asmcmp_instruction_index_t);
kefir_result_t kefir_asmcmp_context_instr_replace(struct kefir_asmcmp_context *, kefir_asmcmp_instruction_index_t,
                                                  const struct kefir_asmcmp_instruction *);

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
kefir_result_t kefir_asmcmp_context_bind_label_after_tail(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                          kefir_asmcmp_label_index_t);
kefir_result_t kefir_asmcmp_context_unbind_label(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                 kefir_asmcmp_label_index_t);
kefir_result_t kefir_asmcmp_context_move_labels(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                kefir_asmcmp_instruction_index_t, kefir_asmcmp_instruction_index_t);
kefir_result_t kefir_asmcmp_context_label_add_public_name(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                          kefir_asmcmp_label_index_t, const char *);
kefir_result_t kefir_asmcmp_context_label_mark_external_dependencies(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                                     kefir_asmcmp_label_index_t);
kefir_result_t kefir_asmcmp_context_label_head(const struct kefir_asmcmp_context *, kefir_asmcmp_label_index_t *);
kefir_result_t kefir_asmcmp_context_label_next(const struct kefir_asmcmp_context *, kefir_asmcmp_label_index_t,
                                               kefir_asmcmp_label_index_t *);
kefir_result_t kefir_asmcmp_context_label_prev(const struct kefir_asmcmp_context *, kefir_asmcmp_label_index_t,
                                               kefir_asmcmp_label_index_t *);
kefir_result_t kefir_asmcmp_context_get_label(const struct kefir_asmcmp_context *, kefir_asmcmp_label_index_t,
                                              const struct kefir_asmcmp_label **);

kefir_result_t kefir_asmcmp_number_of_virtual_registers(const struct kefir_asmcmp_context *, kefir_size_t *);
kefir_result_t kefir_asmcmp_virtual_register_get(const struct kefir_asmcmp_context *,
                                                 kefir_asmcmp_virtual_register_index_t,
                                                 const struct kefir_asmcmp_virtual_register **);
kefir_result_t kefir_asmcmp_virtual_register_new(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                 kefir_asmcmp_virtual_register_type_t,
                                                 kefir_asmcmp_virtual_register_index_t *);
kefir_result_t kefir_asmcmp_virtual_register_new_direct_spill_space_allocation(struct kefir_mem *,
                                                                               struct kefir_asmcmp_context *,
                                                                               kefir_size_t, kefir_size_t,
                                                                               kefir_asmcmp_virtual_register_index_t *);
kefir_result_t kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
    struct kefir_mem *, struct kefir_asmcmp_context *, kefir_size_t, kefir_size_t,
    kefir_asmcmp_virtual_register_index_t *);
kefir_result_t kefir_asmcmp_virtual_register_new_stack_frame_pointer(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                                     kefir_asmcmp_stack_frame_pointer_base_t,
                                                                     kefir_int64_t,
                                                                     kefir_asmcmp_virtual_register_index_t *);
kefir_result_t kefir_asmcmp_virtual_register_new_immediate(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                           const struct kefir_asmcmp_value *,
                                                           kefir_asmcmp_virtual_register_index_t *);
kefir_result_t kefir_asmcmp_virtual_register_new_memory_pointer(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                                kefir_asmcmp_physical_register_index_t, kefir_int64_t,
                                                                kefir_asmcmp_virtual_register_index_t *);
kefir_result_t kefir_asmcmp_virtual_register_specify_type_dependent(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                                    kefir_asmcmp_virtual_register_index_t,
                                                                    kefir_asmcmp_virtual_register_index_t);
kefir_result_t kefir_asmcmp_virtual_set_spill_space_size(const struct kefir_asmcmp_context *,
                                                         kefir_asmcmp_virtual_register_index_t, kefir_size_t,
                                                         kefir_size_t);

kefir_result_t kefir_asmcmp_register_stash_new(struct kefir_mem *, struct kefir_asmcmp_context *,
                                               kefir_asmcmp_stash_index_t *);
kefir_result_t kefir_asmcmp_register_stash_add(struct kefir_mem *, struct kefir_asmcmp_context *,
                                               kefir_asmcmp_stash_index_t, kefir_asmcmp_physical_register_index_t);
kefir_result_t kefir_asmcmp_register_stash_exclude(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                   kefir_asmcmp_stash_index_t, kefir_asmcmp_virtual_register_index_t);
kefir_result_t kefir_asmcmp_register_stash_set_liveness_index(const struct kefir_asmcmp_context *,
                                                              kefir_asmcmp_stash_index_t,
                                                              kefir_asmcmp_instruction_index_t);
kefir_result_t kefir_asmcmp_register_stash_has(const struct kefir_asmcmp_context *, kefir_asmcmp_stash_index_t,
                                               kefir_asmcmp_physical_register_index_t, kefir_bool_t *);
kefir_result_t kefir_asmcmp_register_stash_has_virtual(const struct kefir_asmcmp_context *, kefir_asmcmp_stash_index_t,
                                                       kefir_asmcmp_virtual_register_index_t, kefir_bool_t *);
kefir_result_t kefir_asmcmp_register_stash_vreg(const struct kefir_asmcmp_context *, kefir_asmcmp_stash_index_t,
                                                kefir_asmcmp_virtual_register_index_t *);
kefir_result_t kefir_asmcmp_register_stash_liveness_index(const struct kefir_asmcmp_context *,
                                                          kefir_asmcmp_stash_index_t,
                                                          kefir_asmcmp_instruction_index_t *);

kefir_result_t kefir_asmcmp_inline_assembly_new(struct kefir_mem *, struct kefir_asmcmp_context *, const char *,
                                                kefir_asmcmp_inline_assembly_index_t *);
kefir_result_t kefir_asmcmp_inline_assembly_add_text(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                     kefir_asmcmp_inline_assembly_index_t, const char *, ...);
kefir_result_t kefir_asmcmp_inline_assembly_add_value(struct kefir_mem *, struct kefir_asmcmp_context *,
                                                      kefir_asmcmp_inline_assembly_index_t,
                                                      const struct kefir_asmcmp_value *);

typedef struct kefir_asmcmp_inline_assembly_fragment_iterator {
    const struct kefir_list_entry *iter;
    struct kefir_asmcmp_inline_assembly_fragment *fragment;
} kefir_asmcmp_inline_assembly_fragment_iterator_t;

kefir_result_t kefir_asmcmp_inline_assembly_fragment_iter(const struct kefir_asmcmp_context *,
                                                          kefir_asmcmp_inline_assembly_index_t,
                                                          struct kefir_asmcmp_inline_assembly_fragment_iterator *);
kefir_result_t kefir_asmcmp_inline_assembly_fragment_next(struct kefir_asmcmp_inline_assembly_fragment_iterator *);

kefir_result_t kefir_asmcmp_format(struct kefir_mem *, struct kefir_asmcmp_context *, const char **, const char *, ...);

kefir_result_t kefir_asmcmp_replace_labels(const struct kefir_asmcmp_context *, kefir_asmcmp_label_index_t,
                                           kefir_asmcmp_label_index_t);

#endif
