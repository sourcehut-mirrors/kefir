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

#include "kefir/ir/assembly.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t free_inline_asm_parameter(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                                kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *payload) {
    UNUSED(key);
    UNUSED(tree);
    UNUSED(payload);
    ASSIGN_DECL_CAST(struct kefir_ir_inline_assembly_parameter *, param, value);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly parameter"));

    param->template_parameter = NULL;
    KEFIR_FREE(mem, param);
    return KEFIR_OK;
}

struct kefir_ir_inline_assembly *kefir_ir_inline_assembly_alloc(struct kefir_mem *mem,
                                                                struct kefir_symbol_table *symbols, kefir_id_t id,
                                                                const char *template) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(template != NULL, NULL);

    if (symbols != NULL) {
        template = kefir_symbol_table_insert(mem, symbols, template, NULL);
        REQUIRE(template != NULL, NULL);
    }

    struct kefir_ir_inline_assembly *inline_asm = KEFIR_MALLOC(mem, sizeof(struct kefir_ir_inline_assembly));
    REQUIRE(inline_asm != NULL, NULL);

    inline_asm->id = id;
    inline_asm->template = template;

    kefir_result_t res = kefir_hashtree_init(&inline_asm->parameters, &kefir_hashtree_str_ops);
    REQUIRE_CHAIN(&res, kefir_hashtree_on_removal(&inline_asm->parameters, free_inline_asm_parameter, NULL));
    REQUIRE_CHAIN(&res, kefir_hashtree_init(&inline_asm->clobbers, &kefir_hashtree_str_ops));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, inline_asm);
        return NULL;
    });
    return inline_asm;
}

kefir_result_t kefir_ir_inline_assembly_free(struct kefir_mem *mem, struct kefir_ir_inline_assembly *inline_asm) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly"));

    REQUIRE_OK(kefir_hashtree_free(mem, &inline_asm->clobbers));
    REQUIRE_OK(kefir_hashtree_free(mem, &inline_asm->parameters));
    inline_asm->template = NULL;
    inline_asm->id = 0;
    KEFIR_FREE(mem, inline_asm);
    return KEFIR_OK;
}

kefir_result_t kefir_ir_inline_assembly_add_parameter(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                                      struct kefir_ir_inline_assembly *inline_asm,
                                                      const char *identifier,
                                                      kefir_ir_inline_assembly_parameter_class_t param_class,
                                                      const struct kefir_ir_type *param_type,
                                                      kefir_size_t param_type_idx, kefir_size_t input_index,
                                                      kefir_size_t output_index) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly"));
    REQUIRE(param_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parameter IR type"));
    REQUIRE(identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly parameter identifier"));

    if (symbols != NULL) {
        identifier = kefir_symbol_table_insert(mem, symbols, identifier, NULL);
        REQUIRE(identifier != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                "Failed to insert IR inline assembly parameter identifier into symbol table"));
    }

    struct kefir_ir_inline_assembly_parameter *param =
        KEFIR_MALLOC(mem, sizeof(struct kefir_ir_inline_assembly_parameter));
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR inline assembly parameter"));

    param->template_parameter = identifier;
    param->klass = param_class;
    param->type.type = param_type;
    param->type.index = param_type_idx;
    param->constraint = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER;
    param->input_index = input_index;
    param->output_index = output_index;

    kefir_result_t res = kefir_hashtree_insert(mem, &inline_asm->parameters, (kefir_hashtree_key_t) identifier,
                                               (kefir_hashtree_value_t) param);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, param);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_ir_inline_assembly_add_clobber(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                                    struct kefir_ir_inline_assembly *inline_asm, const char *clobber) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly"));
    REQUIRE(clobber != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly clobber"));

    if (symbols != NULL) {
        clobber = kefir_symbol_table_insert(mem, symbols, clobber, NULL);
        REQUIRE(clobber != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                                 "Failed to insert IR inline assembly clobber into symbol table"));
    }

    REQUIRE_OK(
        kefir_hashtree_insert(mem, &inline_asm->clobbers, (kefir_hashtree_key_t) clobber, (kefir_hashtree_value_t) 0));
    return KEFIR_OK;
}
