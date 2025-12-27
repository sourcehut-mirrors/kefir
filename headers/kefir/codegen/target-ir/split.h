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

#ifndef KEFIR_CODEGEN_TARGET_IR_SPLIT_H_
#define KEFIR_CODEGEN_TARGET_IR_SPLIT_H_

#include "kefir/codegen/target-ir/code.h"

typedef struct kefir_codegen_target_ir_split_live_ranges_profile {
    kefir_uint32_t general_purpose_interference_threshold;
    kefir_uint32_t floating_point_interference_threshold;
    kefir_uint32_t max_splits_per_use_pct;
    kefir_uint32_t max_splits_baseline;
    kefir_uint32_t max_blocks;
    kefir_uint32_t max_branching;
    kefir_uint32_t max_code;
} kefir_codegen_target_ir_split_live_ranges_profile_t;

kefir_result_t kefir_codegen_target_ir_transform_split_live_ranges(struct kefir_mem *, struct kefir_codegen_target_ir_code *, const struct kefir_codegen_target_ir_split_live_ranges_profile *);

#endif