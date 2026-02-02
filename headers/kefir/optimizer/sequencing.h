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

#ifndef KEFIR_OPTIMIZER_SEQUENCING_H_
#define KEFIR_OPTIMIZER_SEQUENCING_H_

#include "kefir/optimizer/code.h"
#include "kefir/optimizer/control_flow.h"
#include "kefir/core/hashset.h"
#include "kefir/core/hashtree.h"

typedef struct kefir_opt_code_sequencing {
    struct kefir_hashset sequenced_before;
    struct kefir_hashtree sequence_numbering;
    kefir_size_t next_seq_number;
} kefir_opt_code_sequencing_t;

kefir_result_t kefir_opt_code_sequencing_init(struct kefir_opt_code_sequencing *);
kefir_result_t kefir_opt_code_sequencing_free(struct kefir_mem *, struct kefir_opt_code_sequencing *);

kefir_result_t kefir_opt_code_is_sequenced_before(struct kefir_mem *, const struct kefir_opt_code_control_flow *,
                                                  struct kefir_opt_code_sequencing *, kefir_opt_instruction_ref_t,
                                                  kefir_opt_instruction_ref_t, kefir_bool_t *);
kefir_result_t kefir_opt_code_sequencing_drop_cache(struct kefir_mem *, struct kefir_opt_code_sequencing *);

#endif
