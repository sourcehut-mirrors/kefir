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
    KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ,
    KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_WRITE,
    KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_WRITE
} kefir_ir_inline_assembly_parameter_class_t;

typedef enum kefir_ir_inline_assembly_parameter_constraint {
    KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER,
    KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_MEMORY,
    KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER_MEMORY
} kefir_ir_inline_assembly_parameter_constraint_t;

typedef struct kefir_ir_inline_assembly_parameter {
    const char *template_parameter;
    kefir_ir_inline_assembly_parameter_class_t klass;
    struct {
        const struct kefir_ir_type *type;
        kefir_size_t index;
    } type;
    kefir_ir_inline_assembly_parameter_constraint_t constraint;
    kefir_size_t input_index;
    kefir_size_t output_index;
} kefir_ir_inline_assembly_parameter_t;

typedef struct kefir_ir_inline_assembly {
    kefir_id_t id;
    const char *template;
    struct kefir_hashtree parameters;
    struct kefir_hashtree clobbers;
} kefir_ir_inline_assembly_t;

struct kefir_ir_inline_assembly *kefir_ir_inline_assembly_alloc(struct kefir_mem *, struct kefir_symbol_table *,
                                                                kefir_id_t, const char *);
kefir_result_t kefir_ir_inline_assembly_free(struct kefir_mem *, struct kefir_ir_inline_assembly *);
kefir_result_t kefir_ir_inline_assembly_add_parameter(struct kefir_mem *, struct kefir_symbol_table *,
                                                      struct kefir_ir_inline_assembly *, const char *,
                                                      kefir_ir_inline_assembly_parameter_class_t,
                                                      const struct kefir_ir_type *, kefir_size_t, kefir_size_t,
                                                      kefir_size_t);
kefir_result_t kefir_ir_inline_assembly_add_clobber(struct kefir_mem *, struct kefir_symbol_table *,
                                                    struct kefir_ir_inline_assembly *, const char *);

#endif
