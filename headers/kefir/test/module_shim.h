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

#ifndef KEFIR_TEST_MODULE_H_
#define KEFIR_TEST_MODULE_H_

#include "kefir/ir/module.h"

kefir_result_t kefir_ir_module_declare_global(struct kefir_mem *, struct kefir_ir_module *, const char *,
                                              kefir_ir_identifier_type_t);
kefir_result_t kefir_ir_module_declare_external(struct kefir_mem *, struct kefir_ir_module *, const char *,
                                                kefir_ir_identifier_type_t);
kefir_result_t kefir_ir_module_declare_local(struct kefir_mem *, struct kefir_ir_module *, const char *,
                                             kefir_ir_identifier_type_t);

kefir_result_t kefir_ir_function_push_arguments(struct kefir_mem *, struct kefir_ir_function *);
struct kefir_ir_function *kefir_ir_module_new_function_with_args(struct kefir_mem *, struct kefir_ir_module *,
                                                                 struct kefir_ir_function_decl *, kefir_size_t);

#endif
