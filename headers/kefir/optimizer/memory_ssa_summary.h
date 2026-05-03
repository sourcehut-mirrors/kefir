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

#ifndef KEFIR_OPTIMIZER_MEMORY_SSA_SUMMARY_H_
#define KEFIR_OPTIMIZER_MEMORY_SSA_SUMMARY_H_

#include "kefir/optimizer/memory_ssa.h"

typedef struct kefir_opt_code_memssa_chain_entry {
    kefir_opt_code_memssa_node_ref_t node_ref;
    kefir_opt_instruction_ref_t location_ref;
    kefir_size_t size;
    kefir_size_t offset;
} kefir_opt_code_memssa_chain_entry_t;

typedef struct kefir_opt_code_memssa_chain {
    kefir_size_t capacity;
    kefir_size_t length;
    struct kefir_opt_code_memssa_chain_entry entries[];
} kefir_opt_code_memssa_chain_t;

typedef struct kefir_opt_code_memssa_chain_ref {
    kefir_uint32_t chain;
    kefir_uint32_t offset;
} kefir_opt_code_memssa_chain_ref_t;

typedef struct kefir_opt_code_memssa_summary {
    struct kefir_opt_code_memssa_chain **chains;
    kefir_size_t chains_length;
    kefir_size_t chains_capacity;

    struct kefir_opt_code_memssa_chain_ref *refs;
    kefir_size_t refs_length;
} kefir_opt_code_memssa_summary_t;

kefir_result_t kefir_opt_code_memssa_summary_init(struct kefir_opt_code_memssa_summary *);
kefir_result_t kefir_opt_code_memssa_summary_free(struct kefir_mem *, struct kefir_opt_code_memssa_summary *);

kefir_result_t kefir_opt_code_memssa_summary_build(struct kefir_mem *, struct kefir_opt_code_memssa_summary *,
                                                   const struct kefir_opt_code_container *,
                                                   const struct kefir_opt_code_memssa *);

kefir_result_t kefir_opt_code_memssa_summary_chain_of(const struct kefir_opt_code_memssa_summary *,
                                                      kefir_opt_code_memssa_node_ref_t,
                                                      const struct kefir_opt_code_memssa_chain **, kefir_uint32_t *);

#endif
