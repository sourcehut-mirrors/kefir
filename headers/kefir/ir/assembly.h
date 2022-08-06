/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#ifndef KEFIR_IR_ASSEMBLY_H_
#define KEFIR_IR_ASSEMBLY_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/list.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/mem.h"
#include "kefir/core/symbol_table.h"
#include "kefir/ir/type.h"

typedef enum kefir_ir_inline_assembly_parameter_class {
    KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE,
    KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ,
    KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD,
    KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE,
    KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE
} kefir_ir_inline_assembly_parameter_class_t;

typedef enum kefir_ir_inline_assembly_parameter_constraint {
    KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER,
    KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_MEMORY,
    KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER_MEMORY
} kefir_ir_inline_assembly_parameter_constraint_t;

typedef struct kefir_ir_inline_assembly_parameter {
    kefir_id_t parameter_id;
    struct kefir_list identifiers;
    kefir_ir_inline_assembly_parameter_class_t klass;
    struct {
        const struct kefir_ir_type *type;
        kefir_size_t index;
    } type;
    kefir_ir_inline_assembly_parameter_constraint_t constraint;
    kefir_int64_t value;
} kefir_ir_inline_assembly_parameter_t;

typedef struct kefir_ir_inline_assembly_jump_target {
    kefir_id_t uid;
    struct kefir_list identifiers;
    const char *target_function;
    kefir_size_t target;
} kefir_ir_inline_assembly_jump_target_t;

typedef struct kefir_ir_inline_assembly {
    kefir_id_t id;
    const char *template;
    struct kefir_hashtree parameters;
    struct kefir_list parameter_list;
    struct kefir_hashtree clobbers;
    struct kefir_hashtree jump_targets;
    struct kefir_list jump_target_list;
    kefir_id_t next_jump_target_id;
} kefir_ir_inline_assembly_t;

struct kefir_ir_inline_assembly *kefir_ir_inline_assembly_alloc(struct kefir_mem *, struct kefir_symbol_table *,
                                                                kefir_id_t, const char *);
kefir_result_t kefir_ir_inline_assembly_free(struct kefir_mem *, struct kefir_ir_inline_assembly *);
kefir_result_t kefir_ir_inline_assembly_add_parameter(struct kefir_mem *, struct kefir_symbol_table *,
                                                      struct kefir_ir_inline_assembly *, const char *,
                                                      kefir_ir_inline_assembly_parameter_class_t,
                                                      kefir_ir_inline_assembly_parameter_constraint_t,
                                                      const struct kefir_ir_type *, kefir_size_t, kefir_int64_t,
                                                      struct kefir_ir_inline_assembly_parameter **);
kefir_result_t kefir_ir_inline_assembly_add_parameter_alias(struct kefir_mem *, struct kefir_symbol_table *,
                                                            struct kefir_ir_inline_assembly *,
                                                            struct kefir_ir_inline_assembly_parameter *, const char *);
kefir_result_t kefir_ir_inline_assembly_add_clobber(struct kefir_mem *, struct kefir_symbol_table *,
                                                    struct kefir_ir_inline_assembly *, const char *);
kefir_result_t kefir_ir_inline_assembly_add_jump_target(struct kefir_mem *, struct kefir_symbol_table *,
                                                        struct kefir_ir_inline_assembly *, const char *, const char *,
                                                        kefir_size_t, struct kefir_ir_inline_assembly_jump_target **);
kefir_result_t kefir_ir_inline_assembly_add_jump_target_alias(struct kefir_mem *, struct kefir_symbol_table *,
                                                              struct kefir_ir_inline_assembly *,
                                                              struct kefir_ir_inline_assembly_jump_target *,
                                                              const char *);

#endif
