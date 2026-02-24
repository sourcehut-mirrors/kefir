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

#ifndef KEFIR_OPTIMIZER_ESCAPE_H_
#define KEFIR_OPTIMIZER_ESCAPE_H_

#include "kefir/core/hashtable.h"
#include "kefir/optimizer/code.h"

typedef struct kefir_opt_code_escape_analysis {
    struct kefir_hashtable local_escapes;
} kefir_opt_code_escape_analysis_t;

kefir_result_t kefir_opt_code_escape_analysis_init(struct kefir_opt_code_escape_analysis *);
kefir_result_t kefir_opt_code_escape_analysis_free(struct kefir_mem *, struct kefir_opt_code_escape_analysis *);

kefir_result_t kefir_opt_code_escape_analysis_build(struct kefir_mem *, struct kefir_opt_code_escape_analysis *,
                                                    const struct kefir_opt_code_container *);
kefir_result_t kefir_opt_code_escape_analysis_replace(struct kefir_mem *, struct kefir_opt_code_escape_analysis *,
                                                      kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t);

kefir_bool_t kefir_opt_code_escape_analysis_has_escapes(const struct kefir_opt_code_escape_analysis *,
                                                        kefir_opt_instruction_ref_t);

#endif
