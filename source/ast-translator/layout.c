/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#include "kefir/ast-translator/layout.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/hashtree.h"

struct eval_param {
    struct kefir_mem *mem;
    const struct kefir_ast_translator_environment *env;
    kefir_ir_target_platform_type_handle_t platform_type;
    struct kefir_hashtree *tree;
    struct kefir_ir_type_visitor *visitor;
};

static kefir_result_t traverse_layout(struct kefir_mem *mem, struct kefir_ast_type_layout *layout,
                                      struct eval_param *param) {
    struct kefir_list *list = NULL;
    struct kefir_hashtree_node *list_node = NULL;
    kefir_result_t res = kefir_hashtree_at(param->tree, (kefir_hashtree_key_t) layout->value, &list_node);
    if (res == KEFIR_NOT_FOUND) {
        list = KEFIR_MALLOC(mem, sizeof(struct kefir_list));
        REQUIRE(list != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate type layout list"));
        res = kefir_list_init(list);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, list);
            return res;
        });
        res = kefir_hashtree_insert(mem, param->tree, (kefir_hashtree_key_t) layout->value,
                                    (kefir_hashtree_value_t) list);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_list_free(mem, list);
            KEFIR_FREE(mem, list);
            return res;
        });
    } else {
        REQUIRE_OK(res);
        list = (struct kefir_list *) list_node->value;
    }

    REQUIRE_OK(kefir_list_insert_after(mem, list, kefir_list_tail(list), layout));
    if (layout->type == NULL) {
        for (const struct kefir_list_entry *iter = kefir_list_head(&layout->custom_layout.sublayouts); iter != NULL;
             kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(struct kefir_ast_type_layout *, sublayout, iter->value);
            REQUIRE_OK(traverse_layout(mem, sublayout, param));
        }
    } else {
        switch (layout->type->tag) {
            case KEFIR_AST_TYPE_STRUCTURE:
            case KEFIR_AST_TYPE_UNION: {
                for (const struct kefir_list_entry *iter = kefir_list_head(&layout->structure_layout.member_list);
                     iter != NULL; kefir_list_next(&iter)) {
                    ASSIGN_DECL_CAST(struct kefir_ast_type_layout_structure_member *, member, iter->value);
                    REQUIRE_OK(traverse_layout(mem, member->layout, param));
                }
            } break;

            case KEFIR_AST_TYPE_ARRAY:
                if (!KEFIR_AST_TYPE_IS_VL_ARRAY(layout->type)) {
                    REQUIRE_OK(traverse_layout(mem, layout->array_layout.element_type, param));
                }
                break;

            default:
                break;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t type_visit(const struct kefir_ir_type *type, kefir_size_t index,
                                 const struct kefir_ir_typeentry *typeentry, void *payload) {
    ASSIGN_DECL_CAST(struct eval_param *, param, payload);

    struct kefir_ir_target_platform_typeentry_info type_info;
    REQUIRE_OK(KEFIR_IR_TARGET_PLATFORM_TYPEENTRY_INFO(param->mem, param->env->target_platform, param->platform_type,
                                                       index, &type_info));

    struct kefir_hashtree_node *list_node = NULL;
    kefir_result_t res = kefir_hashtree_at(param->tree, (kefir_hashtree_key_t) index, &list_node);
    if (res == KEFIR_OK) {
        ASSIGN_DECL_CAST(struct kefir_list *, list, list_node->value);
        for (const struct kefir_list_entry *iter = kefir_list_head(list); iter != NULL; kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(struct kefir_ast_type_layout *, layout, iter->value);
            layout->properties.valid = true;
            layout->properties.size = type_info.size;
            layout->properties.alignment = type_info.alignment;
            layout->properties.aligned = type_info.aligned;
            layout->properties.max_alignment = type_info.max_alignment;
            layout->properties.relative_offset = type_info.relative_offset;
        }
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, res);
    }

    switch (typeentry->typecode) {
        case KEFIR_IR_TYPE_STRUCT:
        case KEFIR_IR_TYPE_UNION: {
            struct eval_param nested = {.mem = param->mem,
                                        .env = param->env,
                                        .platform_type = param->platform_type,
                                        .tree = param->tree,
                                        .visitor = param->visitor};
            REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, param->visitor, &nested, index + 1, typeentry->param));
        } break;

        case KEFIR_IR_TYPE_ARRAY: {
            struct eval_param nested = {.mem = param->mem,
                                        .env = param->env,
                                        .platform_type = param->platform_type,
                                        .tree = param->tree,
                                        .visitor = param->visitor};
            REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, param->visitor, &nested, index + 1, 1));
        } break;

        default:
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t tree_node_free(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                     kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_list *, list, value);
    REQUIRE(list != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid list"));
    REQUIRE_OK(kefir_list_free(mem, list));
    KEFIR_FREE(mem, list);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_evaluate_type_layout(struct kefir_mem *mem,
                                                         const struct kefir_ast_translator_environment *env,
                                                         struct kefir_ast_type_layout *layout,
                                                         const struct kefir_ir_type *type) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(env != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator environment"));
    REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type layout"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));

    struct kefir_ir_type_visitor visitor;
    struct kefir_hashtree tree;
    struct eval_param param = {.mem = mem, .env = env, .platform_type = NULL, .tree = &tree, .visitor = &visitor};

    REQUIRE_OK(kefir_hashtree_init(&tree, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&tree, tree_node_free, NULL));
    kefir_result_t res = traverse_layout(mem, layout, &param);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &tree);
        return res;
    });

    res = KEFIR_IR_TARGET_PLATFORM_GET_TYPE(mem, env->target_platform, type, &param.platform_type);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &tree);
        return res;
    });

    res = kefir_ir_type_visitor_init(&visitor, type_visit);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_IR_TARGET_PLATFORM_FREE_TYPE(mem, env->target_platform, param.platform_type);
        kefir_hashtree_free(mem, &tree);
        return res;
    });

    res = kefir_ir_type_visitor_list_nodes(type, &visitor, &param, 0, kefir_ir_type_length(type));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_IR_TARGET_PLATFORM_FREE_TYPE(mem, env->target_platform, param.platform_type);
        kefir_hashtree_free(mem, &tree);
        return res;
    });

    if (layout->type != NULL && layout->type->tag == KEFIR_AST_TYPE_VOID) {
        layout->properties.valid = true;
        layout->properties.size = 0;
        layout->properties.relative_offset = 0;
        layout->properties.aligned = false;
        layout->properties.alignment = 0;
    } else if (layout->type != NULL && KEFIR_AST_TYPE_IS_VL_ARRAY(layout->type)) {
        struct kefir_ir_target_platform_typeentry_info type_info;
        REQUIRE_CHAIN(&res, KEFIR_IR_TARGET_PLATFORM_TYPEENTRY_INFO(mem, env->target_platform, param.platform_type,
                                                                    layout->vl_array.array_ptr_field, &type_info));
        layout->vl_array.array_ptr_relative_offset = type_info.relative_offset;
        REQUIRE_CHAIN(&res, KEFIR_IR_TARGET_PLATFORM_TYPEENTRY_INFO(mem, env->target_platform, param.platform_type,
                                                                    layout->vl_array.array_size_field, &type_info));
        layout->vl_array.array_size_relative_offset = type_info.relative_offset;
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_IR_TARGET_PLATFORM_FREE_TYPE(mem, env->target_platform, param.platform_type);
        kefir_hashtree_free(mem, &tree);
        return res;
    });

    REQUIRE_OK(KEFIR_IR_TARGET_PLATFORM_FREE_TYPE(mem, env->target_platform, param.platform_type));
    REQUIRE_OK(kefir_hashtree_free(mem, &tree));
    return KEFIR_OK;
}
