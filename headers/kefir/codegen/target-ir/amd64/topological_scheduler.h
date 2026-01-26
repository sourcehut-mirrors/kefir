/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#ifndef KEFIR_CODEGE_TARGET_IR_AMD64_TOPOLOGICAL_SCHEDULER_H_
#define KEFIR_CODEGE_TARGET_IR_AMD64_TOPOLOGICAL_SCHEDULER_H_

#include "kefir/codegen/target-ir/schedule.h"
#include "kefir/codegen/target-ir/control_flow.h"

kefir_result_t kefir_codegen_target_ir_amd64_topological_scheduler_init(const struct kefir_codegen_target_ir_control_flow *, struct kefir_codegen_target_ir_code_scheduler *);

#endif
