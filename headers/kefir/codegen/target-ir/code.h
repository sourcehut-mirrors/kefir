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

#ifndef KEFIR_CODEGEN_TARGET_IR_CODE_H_
#define KEFIR_CODEGEN_TARGET_IR_CODE_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"
#include "kefir/core/hashtable.h"
#include "kefir/core/hashset.h"

#define KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS 4

typedef kefir_id_t kefir_codegen_target_ir_block_ref_t;
typedef kefir_id_t kefir_codegen_target_ir_instruction_ref_t;
typedef kefir_uint32_t kefir_codegen_target_ir_opcode_t;
typedef kefir_uint64_t kefir_codegen_target_ir_native_id_t;
typedef kefir_codegen_target_ir_native_id_t kefir_codegen_target_ir_physical_register_t;
typedef kefir_codegen_target_ir_native_id_t kefir_codegen_target_ir_asmcmp_label_t;
typedef kefir_codegen_target_ir_native_id_t kefir_codegen_target_ir_stash_index_t;
typedef kefir_codegen_target_ir_native_id_t kefir_codegen_target_ir_inline_assembly_index_t;

// clang-format off
#define KEFIR_CODEGEN_TARGET_IR_SPECIAL_OPCODES(_instr, _separator) \
    _instr(assign, "assign") _separator \
    _instr(touch, "touch") _separator \
    _instr(phi, "phi") _separator \
    _instr(placeholder, "placeholder")
// clang-format on

typedef enum kefir_codegen_target_ir_operand_type {
    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_NONE = 0,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UINTEGER,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_PHYSICAL_REGISTER,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_BLOCK_REF,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_ASMCMP,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_EXTERNAL,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_BLOCK_REF,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_ASMCMP_LABEL,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_EXTERNAL_LABEL,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_STASH_INDEX,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INLINE_ASSEMBLY_INDEX
} kefir_codegen_target_ir_operand_type_t;

typedef enum kefir_codegen_target_ir_operand_variant {
    KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT_HIGHER,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_80BIT,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_128BIT,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_FP_SINGLE,
    KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_FP_DOUBLE
} kefir_codegen_target_ir_operand_variant_t;

typedef enum kefir_codegen_target_ir_indirect_basis_type {
    KEFIR_CODEGEN_TARGET_IR_INDIRECT_PHYSICAL_BASIS,
    KEFIR_CODEGEN_TARGET_IR_INDIRECT_VALUE_REF_BASIS,
    KEFIR_CODEGEN_TARGET_IR_INDIRECT_BLOCK_REF_BASIS,
    KEFIR_CODEGEN_TARGET_IR_INDIRECT_ASMCMP_LABEL_BASIS,
    KEFIR_CODEGEN_TARGET_IR_INDIRECT_EXTERNAL_LABEL_BASIS,
    KEFIR_CODEGEN_TARGET_IR_INDIRECT_LOCAL_AREA_BASIS,
    KEFIR_CODEGEN_TARGET_IR_INDIRECT_SPILL_AREA_BASIS
} kefir_codegen_target_ir_indirect_basis_type_t;

typedef enum kefir_codegen_target_ir_external_label_relocation {
    KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_ABSOLUTE,
    KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_PLT,
    KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_GOTPCREL,
    KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_TPOFF,
    KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_GOTTPOFF,
    KEFIR_CODEGEN_TARGET_IR_EXTERNAL_LABEL_TLSGD
} kefir_codegen_target_ir_external_label_relocation_t;

typedef enum kefir_codegen_target_ir_value_aspect {
    KEFIR_CODEGEN_TARGET_IR_VALUE_ASPECT_NONE,
    KEFIR_CODEGEN_TARGET_IR_VALUE_ASPECT_OUTPUT_REGISTER,
    KEFIR_CODEGEN_TARGET_IR_VALUE_ASPECT_FLAGS
} kefir_codegen_target_ir_value_aspect_t;

typedef enum kefir_codegen_target_ir_value_type_kind {
    KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_UNSPECIFIED,
    KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE,
    KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLAGS,
    KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT,
    KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_SPILL_SPACE,
    KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_LOCAL_VARIABLE,
    KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_EXTERNAL_MEMORY
} kefir_codegen_target_ir_value_type_kind_t;

typedef struct kefir_codegen_target_ir_value_type {
    kefir_codegen_target_ir_value_type_kind_t kind;
    union {
        struct {
            kefir_size_t length;
            kefir_size_t alignment;
        } spill_space_allocation;
        struct {
            kefir_codegen_target_ir_physical_register_t base_reg;
            kefir_int64_t offset;
        } memory;
        struct {
            kefir_id_t identifier;
            kefir_int64_t offset;
        } local_variable;
        kefir_int64_t immediate_int;
    } parameters;
} kefir_codegen_target_ir_value_type_t;

#define KEFIR_CODEGEN_TARGET_IR_VALUE_NONE ((((kefir_uint32_t) KEFIR_CODEGEN_TARGET_IR_VALUE_ASPECT_NONE) << 16))
#define KEFIR_CODEGEN_TARGET_IR_VALUE_OUTPUT_REGISTER(_idx) ((((kefir_uint32_t) KEFIR_CODEGEN_TARGET_IR_VALUE_ASPECT_OUTPUT_REGISTER) << 16) | (kefir_uint16_t) (_idx))
#define KEFIR_CODEGEN_TARGET_IR_VALUE_FLAGS ((((kefir_uint32_t) KEFIR_CODEGEN_TARGET_IR_VALUE_ASPECT_FLAGS) << 16))
#define KEFIR_CODEGEN_TARGET_IR_VALUE_IS_OUTPUT_REGISTER(_aspect) (((_aspect) >> 16) == KEFIR_CODEGEN_TARGET_IR_VALUE_ASPECT_OUTPUT_REGISTER)
#define KEFIR_CODEGEN_TARGET_IR_VALUE_GET_OUTPUT_REGISTER(_aspect) ((kefir_uint16_t) (_aspect))

#define KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(_value_ref) ((((kefir_uint64_t) (_value_ref)->instr_ref) << 32) | (kefir_uint32_t) (_value_ref)->aspect)
#define KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(_encoded) ((struct kefir_codegen_target_ir_value_ref) { \
        .instr_ref = ((kefir_uint64_t) (_encoded)) >> 32, \
        .aspect = (kefir_uint32_t) (_encoded) \
    })

typedef struct kefir_codegen_target_ir_value_ref {
    kefir_codegen_target_ir_instruction_ref_t instr_ref;
    kefir_uint32_t aspect;
} kefir_codegen_target_ir_value_ref_t;

typedef struct kefir_codegen_target_ir_phi_node {
    struct kefir_hashtable links;
} kefir_codegen_target_ir_phi_node_t;

typedef struct kefir_codegen_target_ir_operand {
    kefir_codegen_target_ir_operand_type_t type;

    union {
        kefir_int64_t int_immediate;
        kefir_uint64_t uint_immediate;
        struct {
            struct kefir_codegen_target_ir_value_ref value_ref;
            kefir_codegen_target_ir_operand_variant_t variant;
        } direct;
        kefir_codegen_target_ir_physical_register_t phreg;
        struct {
            kefir_codegen_target_ir_indirect_basis_type_t type;
            union {
                kefir_codegen_target_ir_physical_register_t phreg;
                struct kefir_codegen_target_ir_value_ref value_ref;
                kefir_codegen_target_ir_block_ref_t block_ref;
                kefir_codegen_target_ir_asmcmp_label_t asmcmp_label;
                struct {
                    kefir_codegen_target_ir_external_label_relocation_t external_type;
                    const char *external_label;
                };
                kefir_size_t spill_index;
                kefir_id_t local_variable_id;
            } base;
            kefir_int64_t offset;
            kefir_codegen_target_ir_operand_variant_t variant;
        } indirect;
        struct {
            kefir_codegen_target_ir_external_label_relocation_t position;
            union {
                kefir_codegen_target_ir_block_ref_t block_ref;
                kefir_codegen_target_ir_asmcmp_label_t asmcmp_label;
                const char *external;
            };
            kefir_codegen_target_ir_operand_variant_t variant;
        } rip_indirection;
        kefir_codegen_target_ir_block_ref_t block_ref;
        kefir_codegen_target_ir_asmcmp_label_t asmcmp_label;
        struct {
            kefir_codegen_target_ir_external_label_relocation_t position;
            const char *symbolic;
            kefir_int64_t offset;
        } external_label;
        kefir_size_t x87;
        kefir_codegen_target_ir_stash_index_t stash_idx;
        kefir_codegen_target_ir_inline_assembly_index_t inline_asm_idx;
    };

    struct {
        kefir_bool_t present;
        kefir_codegen_target_ir_physical_register_t reg;
    } segment;
} kefir_codegen_target_ir_operand_t;

typedef struct kefir_codegen_target_ir_operation {
    kefir_codegen_target_ir_opcode_t opcode;
    union {
        struct kefir_codegen_target_ir_operand parameters[KEFIR_CODEGEN_TARGET_IR_OPERATION_NUM_OF_PARAMETERS];
        struct kefir_codegen_target_ir_phi_node phi_node;
    };
} kefir_codegen_target_ir_operation_t;

typedef struct kefir_codegen_target_ir_instruction {
    kefir_codegen_target_ir_instruction_ref_t instr_ref;
    kefir_codegen_target_ir_block_ref_t block_ref;

    struct kefir_codegen_target_ir_operation operation;

    struct {
        kefir_codegen_target_ir_instruction_ref_t prev;
        kefir_codegen_target_ir_instruction_ref_t next;
    } control_flow;

    struct kefir_hashtable aspects;
} kefir_codegen_target_ir_instruction_t;

typedef struct kefir_codegen_target_ir_block {
    kefir_codegen_target_ir_block_ref_t block_ref;

    struct {
        kefir_codegen_target_ir_instruction_ref_t head;
        kefir_codegen_target_ir_instruction_ref_t tail;
    } control_flow;
} kefir_codegen_target_ir_block_t;

typedef enum kefir_codegen_target_ir_allocation_constraint_type {
    KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT,
    KEFIR_CODEGEN_TARGET_IR_ALLOCATION_HINT,
    KEFIR_CODEGEN_TARGET_IR_ALLOCATION_SAME_AS
} kefir_codegen_target_ir_allocation_constraint_type_t;

typedef struct kefir_codegen_target_ir_allocation_constraint {
    kefir_codegen_target_ir_allocation_constraint_type_t type;
    union {
        kefir_codegen_target_ir_physical_register_t physical_register;
        struct kefir_codegen_target_ir_value_ref value_ref;
    };
} kefir_codegen_target_ir_allocation_constraint_t;

typedef struct kefir_codegen_target_ir_block_terminator_props {
    kefir_bool_t block_terminator;
    kefir_bool_t function_terminator;
    kefir_bool_t fallthrough;
    kefir_bool_t undefined_target;
    kefir_codegen_target_ir_block_ref_t target_block_refs[2];
} kefir_codegen_target_ir_block_terminator_props_t;

typedef struct kefir_codegen_target_ir_code_class {
    kefir_result_t (*opcode_mnemonic)(kefir_codegen_target_ir_opcode_t, const char **, void *);
    kefir_result_t (*register_mnemonic)(kefir_codegen_target_ir_physical_register_t, const char **, void *);
    kefir_result_t (*attribute_mnemonic)(kefir_codegen_target_ir_native_id_t, const char **, void *);
    kefir_result_t (*is_block_terminator)(const struct kefir_codegen_target_ir_instruction *, struct kefir_codegen_target_ir_block_terminator_props *, void *);
    kefir_result_t (*make_unconditional_jump)(kefir_codegen_target_ir_block_ref_t, struct kefir_codegen_target_ir_operation *, void *);
    kefir_result_t (*finalize_conditional_jump)(const struct kefir_codegen_target_ir_operation *, kefir_codegen_target_ir_block_ref_t, struct kefir_codegen_target_ir_operation *, void *);

    kefir_codegen_target_ir_opcode_t assign_opcode;
    kefir_codegen_target_ir_opcode_t touch_opcode;
    kefir_codegen_target_ir_opcode_t phi_opcode;
    kefir_codegen_target_ir_opcode_t placeholder_opcode;
    void *payload;
} kefir_codegen_target_ir_code_class_t;

typedef struct kefir_codegen_target_ir_code {
    struct kefir_codegen_target_ir_instruction *code;
    kefir_size_t code_length;
    kefir_size_t code_capacity;

    struct kefir_codegen_target_ir_block *blocks;
    kefir_size_t blocks_length;
    kefir_size_t blocks_capacity;

    struct kefir_hashtable attributes;
    struct kefir_hashtable constraints;
    struct kefir_codegen_target_ir_value_type *value_types;
    kefir_size_t value_types_length;
    kefir_size_t value_types_capacity;

    kefir_codegen_target_ir_block_ref_t entry_block;

    const struct kefir_codegen_target_ir_code_class *klass;
} kefir_codegen_target_ir_code_t;

kefir_result_t kefir_codegen_target_ir_code_init(struct kefir_codegen_target_ir_code *, const struct kefir_codegen_target_ir_code_class *);
kefir_result_t kefir_codegen_target_ir_code_free(struct kefir_mem *, struct kefir_codegen_target_ir_code *);

kefir_result_t kefir_codegen_target_ir_code_new_block(struct kefir_mem *, struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_block_ref_t *);
kefir_size_t kefir_codegen_target_ir_code_block_count(const struct kefir_codegen_target_ir_code *);
kefir_codegen_target_ir_block_ref_t kefir_codegen_target_ir_code_block_at(const struct kefir_codegen_target_ir_code *, kefir_size_t);
kefir_codegen_target_ir_instruction_ref_t kefir_codegen_target_ir_code_block_control_head(const struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_block_ref_t);
kefir_codegen_target_ir_instruction_ref_t kefir_codegen_target_ir_code_block_control_tail(const struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_block_ref_t);

kefir_result_t kefir_codegen_target_ir_code_instruction(const struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_instruction_ref_t, const struct kefir_codegen_target_ir_instruction **);
kefir_result_t kefir_codegen_target_ir_code_new_instruction(struct kefir_mem *, struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_block_ref_t, kefir_codegen_target_ir_instruction_ref_t, const struct kefir_codegen_target_ir_operation *, kefir_codegen_target_ir_instruction_ref_t *);
kefir_codegen_target_ir_instruction_ref_t kefir_codegen_target_ir_code_control_next(const struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_instruction_ref_t);
kefir_codegen_target_ir_instruction_ref_t kefir_codegen_target_ir_code_control_prev(const struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_instruction_ref_t);
kefir_result_t kefir_codegen_target_ir_code_drop_instruction(struct kefir_mem *, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t);

kefir_result_t kefir_codegen_target_ir_code_phi_attach(struct kefir_mem *, struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_instruction_ref_t,
    kefir_codegen_target_ir_block_ref_t, struct kefir_codegen_target_ir_value_ref);

typedef struct kefir_codegen_target_ir_value_phi_node_iterator {
    struct kefir_hashtable_iterator iter;
} kefir_codegen_target_ir_value_phi_node_iterator_t;

kefir_result_t kefir_codegen_target_ir_code_phi_link_iter(const struct kefir_codegen_target_ir_code *, struct kefir_codegen_target_ir_value_phi_node_iterator *, kefir_codegen_target_ir_instruction_ref_t, kefir_codegen_target_ir_block_ref_t *, struct kefir_codegen_target_ir_value_ref *);
kefir_result_t kefir_codegen_target_ir_code_phi_link_next(struct kefir_codegen_target_ir_value_phi_node_iterator *, kefir_codegen_target_ir_block_ref_t *, struct kefir_codegen_target_ir_value_ref *);

kefir_result_t kefir_codegen_target_ir_code_add_aspect(struct kefir_mem *, struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_value_ref_t, const struct kefir_codegen_target_ir_value_type *);
kefir_result_t kefir_codegen_target_ir_code_add_constraint(struct kefir_mem *, struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_value_ref_t, const struct kefir_codegen_target_ir_allocation_constraint *);
kefir_result_t kefir_codegen_target_ir_code_add_instruction_attribute(struct kefir_mem *, struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_instruction_ref_t, kefir_codegen_target_ir_native_id_t);
kefir_result_t kefir_codegen_target_ir_code_value_props(const struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_value_ref_t, const struct kefir_codegen_target_ir_value_type **, const struct kefir_codegen_target_ir_allocation_constraint **);

typedef struct kefir_codegen_target_ir_value_iterator {
    const struct kefir_codegen_target_ir_code *code;
    struct kefir_hashtable_iterator iter;
    kefir_codegen_target_ir_instruction_ref_t instr_ref;
} kefir_codegen_target_ir_code_value_iterator_t;

kefir_result_t kefir_codegen_target_ir_code_value_iter(const struct kefir_codegen_target_ir_code *, struct kefir_codegen_target_ir_value_iterator *, kefir_codegen_target_ir_instruction_ref_t, struct kefir_codegen_target_ir_value_ref *, const struct kefir_codegen_target_ir_value_type **);
kefir_result_t kefir_codegen_target_ir_code_value_next(struct kefir_codegen_target_ir_value_iterator *, struct kefir_codegen_target_ir_value_ref *, const struct kefir_codegen_target_ir_value_type **);

typedef struct kefir_codegen_target_ir_code_attribute_iterator {
    struct kefir_hashset_iterator iter;
} kefir_codegen_target_ir_code_attribute_iterator_t;

kefir_result_t kefir_codegen_target_ir_code_instruction_attribute_iter(const struct kefir_codegen_target_ir_code *, struct kefir_codegen_target_ir_code_attribute_iterator *, kefir_codegen_target_ir_instruction_ref_t, kefir_codegen_target_ir_native_id_t *);
kefir_result_t kefir_codegen_target_ir_code_instruction_attribute_next(struct kefir_codegen_target_ir_code_attribute_iterator *, kefir_codegen_target_ir_native_id_t *);


#endif
