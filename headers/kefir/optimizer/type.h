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

#ifndef KEFIR_OPTIMIZER_TYPE_H_
#define KEFIR_OPTIMIZER_TYPE_H_

#include "kefir/core/hashtree.h"
#include "kefir/ir/type.h"
#include "kefir/ir/platform.h"
#include "kefir/optimizer/base.h"

typedef struct kefir_opt_type_descriptor {
    kefir_id_t ir_type_id;
    const struct kefir_ir_type *ir_type;

    const struct kefir_ir_target_platform *target_platform;
    kefir_ir_target_platform_type_handle_t target_type;
} kefir_opt_type_descriptor_t;

kefir_result_t kefir_opt_type_descriptor_init(struct kefir_mem *, const struct kefir_ir_target_platform *, kefir_id_t,
                                              const struct kefir_ir_type *, struct kefir_opt_type_descriptor *);
kefir_result_t kefir_opt_type_descriptor_free(struct kefir_mem *, struct kefir_opt_type_descriptor *);
kefir_result_t kefir_opt_type_descriptor_entry_info(struct kefir_mem *, const struct kefir_opt_type_descriptor *,
                                                    kefir_size_t, struct kefir_ir_target_platform_typeentry_info *);

#endif
