/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#include "kefir/ast/node.h"
#include "kefir/ast/node_internal.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

NODE_VISIT_IMPL(ast_inline_assembly_visit, kefir_ast_inline_assembly, inline_assembly)

struct kefir_ast_node_base *ast_inline_assembly_clone(struct kefir_mem *, struct kefir_ast_node_base *);

kefir_result_t ast_inline_assembly_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_inline_assembly *, node, base->self);
    REQUIRE_OK(kefir_list_free(mem, &node->outputs));
    REQUIRE_OK(kefir_list_free(mem, &node->inputs));
    REQUIRE_OK(kefir_list_free(mem, &node->clobbers));
    REQUIRE_OK(kefir_list_free(mem, &node->jump_labels));
    node->asm_template = NULL;
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_INLINE_ASSEMBLY_CLASS = {.type = KEFIR_AST_INLINE_ASSEMBLY,
                                                               .visit = ast_inline_assembly_visit,
                                                               .clone = ast_inline_assembly_clone,
                                                               .free = ast_inline_assembly_free};

static kefir_result_t inline_asm_param_free(struct kefir_mem *mem, struct kefir_list *list,
                                            struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));
    ASSIGN_DECL_CAST(struct kefir_ast_inline_assembly_parameter *, param, entry->value);

    param->parameter_name = NULL;
    param->constraint = NULL;
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, param->parameter));
    KEFIR_FREE(mem, param);
    return KEFIR_OK;
}

static kefir_result_t inline_asm_params_clone(struct kefir_mem *mem, struct kefir_list *dst,
                                              const struct kefir_list *src) {
    for (const struct kefir_list_entry *iter = kefir_list_head(src); iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_inline_assembly_parameter *, param, iter->value);
        struct kefir_ast_inline_assembly_parameter *param_clone =
            KEFIR_MALLOC(mem, sizeof(struct kefir_ast_inline_assembly_parameter));
        REQUIRE(param_clone != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to clone AST inline assembly parameter"));

        param_clone->parameter_name = param->parameter_name;
        param_clone->constraint = param->constraint;
        param_clone->parameter = KEFIR_AST_NODE_CLONE(mem, param->parameter);
        REQUIRE_ELSE(param_clone->parameter != NULL, {
            KEFIR_FREE(mem, param_clone);
            return KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to clone AST inline assembly parameter");
        });

        kefir_result_t res = kefir_list_insert_after(mem, dst, kefir_list_tail(dst), (void *) param_clone);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, param_clone);
            return KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to clone AST inline assembly parameter");
        });
    }
    return KEFIR_OK;
}

static kefir_result_t inline_asm_strings_clone(struct kefir_mem *mem, struct kefir_list *dst,
                                               const struct kefir_list *src) {
    for (const struct kefir_list_entry *iter = kefir_list_head(src); iter != NULL; kefir_list_next(&iter)) {
        REQUIRE_OK(kefir_list_insert_after(mem, dst, kefir_list_tail(dst), iter->value));
    }
    return KEFIR_OK;
}

struct kefir_ast_node_base *ast_inline_assembly_clone(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(base != NULL, NULL);
    ASSIGN_DECL_CAST(struct kefir_ast_inline_assembly *, node, base->self);
    struct kefir_ast_inline_assembly *clone = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_inline_assembly));
    REQUIRE(clone != NULL, NULL);
    clone->base.klass = &AST_INLINE_ASSEMBLY_CLASS;
    clone->base.self = clone;
    clone->base.source_location = base->source_location;

    clone->qualifiers = node->qualifiers;
    clone->asm_template = node->asm_template;

    kefir_result_t res = kefir_ast_node_properties_clone(&clone->base.properties, &node->base.properties);
    REQUIRE_CHAIN(&res, kefir_list_init(&clone->outputs));
    REQUIRE_CHAIN(&res, kefir_list_on_remove(&clone->outputs, inline_asm_param_free, NULL));
    REQUIRE_CHAIN(&res, kefir_list_init(&clone->inputs));
    REQUIRE_CHAIN(&res, kefir_list_on_remove(&clone->inputs, inline_asm_param_free, NULL));
    REQUIRE_CHAIN(&res, kefir_list_init(&clone->clobbers));
    REQUIRE_CHAIN(&res, kefir_list_init(&clone->jump_labels));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, clone);
        return NULL;
    });

    REQUIRE_CHAIN(&res, inline_asm_params_clone(mem, &clone->outputs, &node->outputs));
    REQUIRE_CHAIN(&res, inline_asm_params_clone(mem, &clone->inputs, &node->inputs));
    REQUIRE_CHAIN(&res, inline_asm_strings_clone(mem, &clone->clobbers, &node->clobbers));
    REQUIRE_CHAIN(&res, inline_asm_strings_clone(mem, &clone->jump_labels, &node->jump_labels));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(clone));
        return NULL;
    });

    return KEFIR_AST_NODE_BASE(clone);
}

struct kefir_ast_inline_assembly *kefir_ast_new_inline_assembly(struct kefir_mem *mem,
                                                                struct kefir_ast_inline_assembly_qualifiers qualifiers,
                                                                const char *asm_template) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(asm_template != NULL, NULL);

    struct kefir_ast_inline_assembly *inline_assembly = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_inline_assembly));
    REQUIRE(inline_assembly != NULL, NULL);
    inline_assembly->base.klass = &AST_INLINE_ASSEMBLY_CLASS;
    inline_assembly->base.self = inline_assembly;
    kefir_result_t res = kefir_ast_node_properties_init(&inline_assembly->base.properties);
    REQUIRE_CHAIN(&res, kefir_source_location_empty(&inline_assembly->base.source_location));
    REQUIRE_CHAIN(&res, kefir_list_init(&inline_assembly->outputs));
    REQUIRE_CHAIN(&res, kefir_list_on_remove(&inline_assembly->outputs, inline_asm_param_free, NULL));
    REQUIRE_CHAIN(&res, kefir_list_init(&inline_assembly->inputs));
    REQUIRE_CHAIN(&res, kefir_list_on_remove(&inline_assembly->inputs, inline_asm_param_free, NULL));
    REQUIRE_CHAIN(&res, kefir_list_init(&inline_assembly->clobbers));
    REQUIRE_CHAIN(&res, kefir_list_init(&inline_assembly->jump_labels));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, inline_assembly);
        return NULL;
    });
    inline_assembly->qualifiers = qualifiers;
    inline_assembly->asm_template = asm_template;
    return inline_assembly;
}

kefir_result_t kefir_ast_inline_assembly_add_output(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                                    struct kefir_ast_inline_assembly *inline_asm,
                                                    const char *param_name, const char *constraint,
                                                    struct kefir_ast_node_base *param) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST inline assembly"));
    REQUIRE(constraint != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST inline assembly parameter constraint"));
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST inline assembly parameter"));

    if (symbols != NULL) {
        constraint = kefir_string_pool_insert(mem, symbols, constraint, NULL);
        REQUIRE(constraint != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                "Failed to insert AST inline assembly parameter constraint into symbol table"));

        if (param_name != NULL) {
            param_name = kefir_string_pool_insert(mem, symbols, param_name, NULL);
            REQUIRE(param_name != NULL,
                    KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                    "Failed to insert AST inline assembly parameter name into symbol table"));
        }
    }

    struct kefir_ast_inline_assembly_parameter *parameter =
        KEFIR_MALLOC(mem, sizeof(struct kefir_ast_inline_assembly_parameter));
    REQUIRE(parameter != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST inline assembly"));

    parameter->parameter = param;
    parameter->constraint = constraint;
    parameter->explicit_register = NULL;
    parameter->parameter_name = param_name;

    kefir_result_t res =
        kefir_list_insert_after(mem, &inline_asm->outputs, kefir_list_tail(&inline_asm->outputs), parameter);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, parameter);
        return res;
    });

    return KEFIR_OK;
}

kefir_result_t kefir_ast_inline_assembly_add_input(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                                   struct kefir_ast_inline_assembly *inline_asm, const char *param_name,
                                                   const char *constraint, struct kefir_ast_node_base *param) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST inline assembly"));
    REQUIRE(constraint != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST inline assembly parameter constraint"));
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST inline assembly parameter"));

    if (symbols != NULL) {
        constraint = kefir_string_pool_insert(mem, symbols, constraint, NULL);
        REQUIRE(constraint != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                "Failed to insert AST inline assembly parameter constraint into symbol table"));

        if (param_name != NULL) {
            param_name = kefir_string_pool_insert(mem, symbols, param_name, NULL);
            REQUIRE(param_name != NULL,
                    KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                    "Failed to insert AST inline assembly parameter name into symbol table"));
        }
    }

    struct kefir_ast_inline_assembly_parameter *parameter =
        KEFIR_MALLOC(mem, sizeof(struct kefir_ast_inline_assembly_parameter));
    REQUIRE(parameter != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST inline assembly"));

    parameter->parameter = param;
    parameter->constraint = constraint;
    parameter->explicit_register = NULL;
    parameter->parameter_name = param_name;

    kefir_result_t res =
        kefir_list_insert_after(mem, &inline_asm->inputs, kefir_list_tail(&inline_asm->inputs), parameter);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, parameter);
        return res;
    });

    return KEFIR_OK;
}

kefir_result_t kefir_ast_inline_assembly_add_clobber(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                                     struct kefir_ast_inline_assembly *inline_asm,
                                                     const char *clobber) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST inline assembly"));
    REQUIRE(clobber != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST inline assembly clobber"));

    if (symbols != NULL) {
        clobber = kefir_string_pool_insert(mem, symbols, clobber, NULL);
        REQUIRE(clobber != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                                 "Failed to insert AST inline assembly clobber into symbol table"));
    }

    REQUIRE_OK(
        kefir_list_insert_after(mem, &inline_asm->clobbers, kefir_list_tail(&inline_asm->clobbers), (void *) clobber));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_inline_assembly_add_jump_label(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                                        struct kefir_ast_inline_assembly *inline_asm,
                                                        const char *jump_label) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST inline assembly"));
    REQUIRE(jump_label != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST inline assembly jump label"));

    if (symbols != NULL) {
        jump_label = kefir_string_pool_insert(mem, symbols, jump_label, NULL);
        REQUIRE(jump_label != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                "Failed to insert AST inline assembly jump label into symbol table"));
    }

    REQUIRE_OK(kefir_list_insert_after(mem, &inline_asm->jump_labels, kefir_list_tail(&inline_asm->jump_labels),
                                       (void *) jump_label));
    return KEFIR_OK;
}
