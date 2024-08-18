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

#include <string.h>
#include "kefir/ir/module.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

static kefir_result_t destroy_type(struct kefir_mem *mem, struct kefir_list *list, struct kefir_list_entry *entry,
                                   void *data) {
    UNUSED(list);
    UNUSED(data);
    struct kefir_ir_type *type = entry->value;
    REQUIRE_OK(kefir_ir_type_free(mem, type));
    KEFIR_FREE(mem, type);
    return KEFIR_OK;
}

static kefir_result_t destroy_function_decl(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                            kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *data) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(data);
    ASSIGN_DECL_CAST(struct kefir_ir_function_decl *, decl, value);
    if (decl != NULL) {
        REQUIRE_OK(kefir_ir_function_decl_free(mem, decl));
        KEFIR_FREE(mem, decl);
    }
    return KEFIR_OK;
}

static kefir_result_t destroy_function(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                       kefir_hashtree_value_t value, void *data) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(data);
    ASSIGN_DECL_CAST(struct kefir_ir_function *, func, value);
    if (func != NULL) {
        REQUIRE_OK(kefir_ir_function_free(mem, func));
        KEFIR_FREE(mem, func);
    }
    return KEFIR_OK;
}

static kefir_result_t destroy_named_data(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                         kefir_hashtree_value_t value, void *data) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(data);
    ASSIGN_DECL_CAST(struct kefir_ir_data *, entry, value);
    REQUIRE_OK(kefir_ir_data_free(mem, entry));
    KEFIR_FREE(mem, entry);
    return KEFIR_OK;
}

static kefir_result_t destroy_string_literal(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                             kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *data) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(data);
    ASSIGN_DECL_CAST(struct kefir_ir_module_string_literal *, literal, value);
    KEFIR_FREE(mem, literal->content);
    KEFIR_FREE(mem, literal);
    return KEFIR_OK;
}

static kefir_result_t destroy_inline_assembly(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                              kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *data) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(data);
    ASSIGN_DECL_CAST(struct kefir_ir_inline_assembly *, inline_asm, value);
    REQUIRE_OK(kefir_ir_inline_assembly_free(mem, inline_asm));
    return KEFIR_OK;
}

static kefir_result_t destroy_identifier(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                         kefir_hashtree_value_t value, void *data) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(data);
    ASSIGN_DECL_CAST(struct kefir_ir_identifier *, identifier, value);
    memset(identifier, 0, sizeof(struct kefir_ir_identifier));
    KEFIR_FREE(mem, identifier);
    return KEFIR_OK;
}

kefir_result_t kefir_ir_module_alloc(struct kefir_mem *mem, struct kefir_ir_module *module) {
    UNUSED(mem);
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module pointer"));
    REQUIRE_OK(kefir_string_pool_init(&module->symbols));
    REQUIRE_OK(kefir_list_init(&module->types));
    REQUIRE_OK(kefir_list_on_remove(&module->types, destroy_type, NULL));
    REQUIRE_OK(kefir_hashtree_init(&module->function_declarations, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&module->function_declarations, destroy_function_decl, NULL));
    REQUIRE_OK(kefir_hashtree_init(&module->identifiers, &kefir_hashtree_str_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&module->identifiers, destroy_identifier, NULL));
    REQUIRE_OK(kefir_hashtree_init(&module->functions, &kefir_hashtree_str_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&module->functions, destroy_function, NULL));
    REQUIRE_OK(kefir_hashtree_init(&module->named_types, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&module->named_data, &kefir_hashtree_str_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&module->named_data, destroy_named_data, NULL));
    REQUIRE_OK(kefir_hashtree_init(&module->string_literals, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&module->string_literals, destroy_string_literal, NULL));
    REQUIRE_OK(kefir_hashtree_init(&module->inline_assembly, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&module->inline_assembly, destroy_inline_assembly, NULL));
    REQUIRE_OK(kefir_hashtree_init(&module->global_inline_asm, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_ir_module_debug_info_init(&module->debug_info));
    module->next_type_id = 0;
    module->next_string_literal_id = 0;
    module->next_function_decl_id = 0;
    module->next_inline_assembly_id = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_module_free(struct kefir_mem *mem, struct kefir_ir_module *module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module pointer"));
    REQUIRE_OK(kefir_ir_module_debug_info_free(mem, &module->debug_info));
    REQUIRE_OK(kefir_hashtree_free(mem, &module->global_inline_asm));
    REQUIRE_OK(kefir_hashtree_free(mem, &module->inline_assembly));
    REQUIRE_OK(kefir_hashtree_free(mem, &module->string_literals));
    REQUIRE_OK(kefir_hashtree_free(mem, &module->named_data));
    REQUIRE_OK(kefir_hashtree_free(mem, &module->named_types));
    REQUIRE_OK(kefir_hashtree_free(mem, &module->functions));
    REQUIRE_OK(kefir_hashtree_free(mem, &module->identifiers));
    REQUIRE_OK(kefir_hashtree_free(mem, &module->function_declarations));
    REQUIRE_OK(kefir_list_free(mem, &module->types));
    REQUIRE_OK(kefir_string_pool_free(mem, &module->symbols));
    return KEFIR_OK;
}

const char *kefir_ir_module_symbol(struct kefir_mem *mem, struct kefir_ir_module *module, const char *symbol,
                                   kefir_id_t *id) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(module != NULL, NULL);
    REQUIRE(symbol != NULL, NULL);
    return kefir_string_pool_insert(mem, &module->symbols, symbol, id);
}

kefir_result_t kefir_ir_module_string_literal(struct kefir_mem *mem, struct kefir_ir_module *module,
                                              kefir_ir_string_literal_type_t type, kefir_bool_t public,
                                              const void *content, kefir_size_t length, kefir_id_t *id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(content != NULL && length > 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid literal"));
    REQUIRE(id != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid literal id pointer"));

    struct kefir_ir_module_string_literal *literal = KEFIR_MALLOC(mem, sizeof(struct kefir_ir_module_string_literal));
    REQUIRE(literal != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate string literal"));

    literal->type = type;
    literal->public = public;
    kefir_size_t sz = 0;
    switch (literal->type) {
        case KEFIR_IR_STRING_LITERAL_MULTIBYTE:
            sz = length;
            break;

        case KEFIR_IR_STRING_LITERAL_UNICODE16:
            sz = length * 2;
            break;

        case KEFIR_IR_STRING_LITERAL_UNICODE32:
            sz = length * 4;
            break;
    }

    literal->content = KEFIR_MALLOC(mem, sz);
    REQUIRE_ELSE(literal->content != NULL, {
        KEFIR_FREE(mem, literal);
        return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate string literal content");
    });

    memcpy(literal->content, content, sz);
    literal->length = length;

    kefir_result_t res =
        kefir_hashtree_insert(mem, &module->string_literals, (kefir_hashtree_key_t) module->next_string_literal_id,
                              (kefir_hashtree_value_t) literal);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, literal->content);
        KEFIR_FREE(mem, literal);
        return res;
    });

    *id = module->next_string_literal_id++;
    return KEFIR_OK;
}

struct kefir_ir_type *kefir_ir_module_new_type(struct kefir_mem *mem, struct kefir_ir_module *module, kefir_size_t size,
                                               kefir_id_t *identifier) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(module != NULL, NULL);
    struct kefir_ir_type *type = KEFIR_MALLOC(mem, sizeof(struct kefir_ir_type));
    kefir_result_t result = kefir_ir_type_alloc(mem, size, type);
    REQUIRE_ELSE(result == KEFIR_OK, {
        KEFIR_FREE(mem, type);
        return NULL;
    });
    result = kefir_list_insert_after(mem, &module->types, kefir_list_tail(&module->types), type);
    REQUIRE_ELSE(result == KEFIR_OK, {
        kefir_ir_type_free(mem, type);
        KEFIR_FREE(mem, type);
        return NULL;
    });
    if (identifier != NULL) {
        *identifier = module->next_type_id++;
        result = kefir_hashtree_insert(mem, &module->named_types, (kefir_hashtree_key_t) *identifier,
                                       (kefir_hashtree_value_t) type);
        REQUIRE_ELSE(result == KEFIR_OK, {
            kefir_list_pop(mem, &module->types, kefir_list_tail(&module->types));
            kefir_ir_type_free(mem, type);
            KEFIR_FREE(mem, type);
            return NULL;
        });
    }
    return type;
}

struct kefir_ir_function_decl *kefir_ir_module_new_function_declaration(struct kefir_mem *mem,
                                                                        struct kefir_ir_module *module,
                                                                        const char *name, kefir_id_t parameters_type_id,
                                                                        bool vararg, kefir_id_t returns_type_id) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(module != NULL, NULL);

    struct kefir_ir_type *parameters = NULL;
    struct kefir_ir_type *returns = NULL;

    if (parameters_type_id != KEFIR_ID_NONE) {
        parameters = kefir_ir_module_get_named_type(module, parameters_type_id);
        REQUIRE(parameters != NULL, NULL);
    }

    if (returns_type_id != KEFIR_ID_NONE) {
        returns = kefir_ir_module_get_named_type(module, returns_type_id);
        REQUIRE(returns != NULL, NULL);
    }

    const char *symbol = kefir_ir_module_symbol(mem, module, name, NULL);
    kefir_id_t func_decl_id = module->next_function_decl_id;
    struct kefir_ir_function_decl *decl = KEFIR_MALLOC(mem, sizeof(struct kefir_ir_function_decl));
    REQUIRE(decl != NULL, NULL);
    kefir_result_t res = kefir_ir_function_decl_alloc(mem, func_decl_id, symbol, parameters, parameters_type_id, vararg,
                                                      returns, returns_type_id, decl);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, decl);
        return NULL;
    });
    res = kefir_hashtree_insert(mem, &module->function_declarations, (kefir_hashtree_key_t) decl->id,
                                (kefir_hashtree_value_t) decl);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ir_function_decl_free(mem, decl);
        KEFIR_FREE(mem, decl);
        return NULL;
    });

    module->next_function_decl_id++;
    return decl;
}

kefir_result_t kefir_ir_module_declare_identifier(struct kefir_mem *mem, struct kefir_ir_module *module,
                                                  const char *symbol_orig,
                                                  const struct kefir_ir_identifier *identifier) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module pointer"));
    REQUIRE(symbol_orig != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR identifier symbol"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR identifier"));

    const char *symbol = kefir_ir_module_symbol(mem, module, symbol_orig, NULL);
    REQUIRE(symbol != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate a symbol"));

    const char *identifier_symbol = symbol;
    const char *identifier_alias = NULL;
    if (identifier->symbol != NULL) {
        identifier_symbol = kefir_ir_module_symbol(mem, module, identifier->symbol, NULL);
        REQUIRE(identifier_symbol != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate a symbol"));
    }
    if (identifier->alias != NULL) {
        identifier_alias = kefir_ir_module_symbol(mem, module, identifier->alias, NULL);
        REQUIRE(identifier_alias != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate a symbol"));
    }

    struct kefir_ir_identifier *identifier_data = KEFIR_MALLOC(mem, sizeof(struct kefir_ir_identifier));
    REQUIRE(identifier_data != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR identifier data"));
    *identifier_data = *identifier;
    identifier_data->symbol = identifier_symbol;
    identifier_data->alias = identifier_alias;
    kefir_result_t res = kefir_hashtree_insert(mem, &module->identifiers, (kefir_hashtree_key_t) symbol,
                                               (kefir_hashtree_value_t) identifier_data);
    if (res == KEFIR_ALREADY_EXISTS) {
        struct kefir_hashtree_node *node;
        res = kefir_hashtree_at(&module->identifiers, (kefir_hashtree_key_t) symbol, &node);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, identifier_data);
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected an identifier to exist in IR module");
        });
        ASSIGN_DECL_CAST(const struct kefir_ir_identifier *, current_identifier_data, node->value);

        res = KEFIR_OK;
        REQUIRE_CHAIN_SET(&res, strcmp(identifier_data->symbol, current_identifier_data->symbol) == 0,
                          KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Mismatch with existing IR module identifier symbol"));
        REQUIRE_CHAIN_SET(&res, identifier_data->type == current_identifier_data->type,
                          KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Mismatch with existing IR module identifier type"));
        REQUIRE_CHAIN_SET(&res, identifier_data->scope == current_identifier_data->scope,
                          KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Mismatch with existing IR module identifier scope"));
        REQUIRE_CHAIN_SET(
            &res, identifier_data->visibility == current_identifier_data->visibility,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Mismatch with existing IR module identifier visibility"));
        REQUIRE_CHAIN_SET(&res,
                          (identifier_data->alias == NULL && current_identifier_data->alias == NULL) ||
                              (identifier_data->alias != NULL && current_identifier_data->alias != NULL &&
                               strcmp(identifier_data->alias, current_identifier_data->alias) == 0),
                          KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Mismatch with existing IR module identifier alias"));
        KEFIR_FREE(mem, identifier_data);
        REQUIRE_OK(res);
        return KEFIR_OK;
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, identifier_data);
        return res;
    });
    return KEFIR_OK;
}

struct kefir_ir_function *kefir_ir_module_new_function(struct kefir_mem *mem, struct kefir_ir_module *module,
                                                       struct kefir_ir_function_decl *decl, kefir_id_t locals_type_id,
                                                       kefir_size_t length) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(module != NULL, NULL);
    REQUIRE(decl != NULL, NULL);
    REQUIRE(decl->name != NULL && strlen(decl->name) != 0, NULL);

    struct kefir_ir_type *locals = NULL;
    if (locals_type_id != KEFIR_ID_NONE) {
        locals = kefir_ir_module_get_named_type(module, locals_type_id);
        REQUIRE(locals != NULL, NULL);
    }

    struct kefir_ir_function *func = KEFIR_MALLOC(mem, sizeof(struct kefir_ir_function));
    REQUIRE(func != NULL, NULL);
    kefir_result_t result = kefir_ir_function_alloc(mem, decl, locals, locals_type_id, length, func);
    REQUIRE_ELSE(result == KEFIR_OK, {
        KEFIR_FREE(mem, func);
        return NULL;
    });
    result = kefir_hashtree_insert(mem, &module->functions, (kefir_hashtree_key_t) decl->name,
                                   (kefir_hashtree_value_t) func);
    REQUIRE_ELSE(result == KEFIR_OK, {
        kefir_ir_function_free(mem, func);
        KEFIR_FREE(mem, func);
        return NULL;
    });
    return func;
}

const struct kefir_ir_function_decl *kefir_ir_module_function_declaration_iter(
    const struct kefir_ir_module *module, struct kefir_hashtree_node_iterator *iter) {
    REQUIRE(module != NULL, NULL);
    REQUIRE(iter != NULL, NULL);
    const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->function_declarations, iter);
    if (node != NULL) {
        return (const struct kefir_ir_function_decl *) node->value;
    } else {
        return NULL;
    }
}
const struct kefir_ir_function_decl *kefir_ir_module_function_declaration_next(
    struct kefir_hashtree_node_iterator *iter) {
    REQUIRE(iter != NULL, NULL);
    const struct kefir_hashtree_node *node = kefir_hashtree_next(iter);
    if (node != NULL) {
        return (const struct kefir_ir_function_decl *) node->value;
    } else {
        return NULL;
    }
}

const struct kefir_ir_function *kefir_ir_module_function_iter(const struct kefir_ir_module *module,
                                                              struct kefir_hashtree_node_iterator *iter) {
    REQUIRE(module != NULL, NULL);
    REQUIRE(iter != NULL, NULL);
    const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->functions, iter);
    if (node != NULL) {
        return (const struct kefir_ir_function *) node->value;
    } else {
        return NULL;
    }
}
const struct kefir_ir_function *kefir_ir_module_function_next(struct kefir_hashtree_node_iterator *iter) {
    REQUIRE(iter != NULL, NULL);
    const struct kefir_hashtree_node *node = kefir_hashtree_next(iter);
    if (node != NULL) {
        return (const struct kefir_ir_function *) node->value;
    } else {
        return NULL;
    }
}

const char *kefir_ir_module_identifiers_iter(const struct kefir_ir_module *module,
                                             struct kefir_hashtree_node_iterator *iter,
                                             const struct kefir_ir_identifier **identifier_ptr) {
    REQUIRE(module != NULL, NULL);
    REQUIRE(iter != NULL, NULL);

    const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->identifiers, iter);
    REQUIRE(node != NULL, NULL);
    ASSIGN_PTR(identifier_ptr, (const struct kefir_ir_identifier *) node->value);
    return (const char *) node->key;
}

const char *kefir_ir_module_identifiers_next(struct kefir_hashtree_node_iterator *iter,
                                             const struct kefir_ir_identifier **identifier_ptr) {
    REQUIRE(iter != NULL, NULL);

    const struct kefir_hashtree_node *node = kefir_hashtree_next(iter);
    REQUIRE(node != NULL, NULL);
    ASSIGN_PTR(identifier_ptr, (const struct kefir_ir_identifier *) node->value);
    return (const char *) node->key;
}

kefir_result_t kefir_ir_module_get_identifier(const struct kefir_ir_module *module, const char *symbol,
                                              const struct kefir_ir_identifier **identifier) {
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(symbol != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR identifier symbol"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR identifier"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&module->identifiers, (kefir_hashtree_key_t) symbol, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find request IR identifier");
    }
    REQUIRE_OK(res);

    *identifier = (const struct kefir_ir_identifier *) node->value;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_module_get_string_literal(const struct kefir_ir_module *module, kefir_id_t id,
                                                  kefir_ir_string_literal_type_t *type, kefir_bool_t *public,
                                                  const void **content, kefir_size_t *length) {
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal type"));
    REQUIRE(public != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid content pointer"));
    REQUIRE(length != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid length pointer"));

    struct kefir_hashtree_node *node = NULL;
    REQUIRE_OK(kefir_hashtree_at(&module->string_literals, (kefir_hashtree_key_t) id, &node));
    ASSIGN_DECL_CAST(struct kefir_ir_module_string_literal *, literal, node->value);
    *type = literal->type;
    *public = literal->public;
    *content = literal->content;
    *length = literal->length;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_module_string_literal_iter(const struct kefir_ir_module *module,
                                                   struct kefir_hashtree_node_iterator *iter, kefir_id_t *id,
                                                   kefir_ir_string_literal_type_t *type, kefir_bool_t *public,
                                                   const void **content, kefir_size_t *length) {
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hash tree iterator pointer"));
    REQUIRE(id != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal identifier pointer"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal type pointer"));
    REQUIRE(public != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal content pointer"));
    REQUIRE(length != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal length pointer"));

    const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->string_literals, iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);
    ASSIGN_DECL_CAST(struct kefir_ir_module_string_literal *, literal, node->value);
    *id = (kefir_id_t) node->key;
    *type = literal->type;
    *public = literal->public;
    *content = literal->content;
    *length = literal->length;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_module_string_literal_next(struct kefir_hashtree_node_iterator *iter, kefir_id_t *id,
                                                   kefir_ir_string_literal_type_t *type, kefir_bool_t *public,
                                                   const void **content, kefir_size_t *length) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hash tree iterator pointer"));
    REQUIRE(id != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal identifier pointer"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal type pointer"));
    REQUIRE(public != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal content pointer"));
    REQUIRE(length != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal length pointer"));

    const struct kefir_hashtree_node *node = kefir_hashtree_next(iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);
    ASSIGN_DECL_CAST(struct kefir_ir_module_string_literal *, literal, node->value);
    *id = (kefir_id_t) node->key;
    *type = literal->type;
    *public = literal->public;
    *content = literal->content;
    *length = literal->length;
    return KEFIR_OK;
}

struct kefir_ir_data *kefir_ir_module_new_named_data(struct kefir_mem *mem, struct kefir_ir_module *module,
                                                     const char *identifier, kefir_ir_data_storage_t storage,
                                                     kefir_id_t type_id) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(module != NULL, NULL);

    const struct kefir_ir_type *type = kefir_ir_module_get_named_type(module, type_id);
    REQUIRE(type != NULL, NULL);

    const char *symbol = kefir_ir_module_symbol(mem, module, identifier, NULL);
    REQUIRE(symbol != NULL, NULL);
    struct kefir_ir_data *data = KEFIR_MALLOC(mem, sizeof(struct kefir_ir_data));
    REQUIRE(data != NULL, NULL);
    kefir_result_t res = kefir_ir_data_alloc(mem, storage, type, type_id, data);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, data);
        return NULL;
    });
    res = kefir_hashtree_insert(mem, &module->named_data, (kefir_hashtree_key_t) symbol, (kefir_hashtree_value_t) data);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ir_data_free(mem, data);
        KEFIR_FREE(mem, data);
        return NULL;
    });
    return data;
}

struct kefir_ir_data *kefir_ir_module_get_named_data(struct kefir_ir_module *module, const char *identifier) {
    REQUIRE(module != NULL, NULL);
    REQUIRE(identifier != NULL, NULL);

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&module->named_data, (kefir_hashtree_key_t) identifier, &node);
    REQUIRE(res == KEFIR_OK, NULL);
    ASSIGN_DECL_CAST(struct kefir_ir_data *, data, node->value);
    return data;
}

const struct kefir_ir_data *kefir_ir_module_named_data_iter(const struct kefir_ir_module *module,
                                                            struct kefir_hashtree_node_iterator *iter,
                                                            const char **identifier) {
    REQUIRE(module != NULL, NULL);
    REQUIRE(iter != NULL, NULL);
    const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->named_data, iter);
    if (node != NULL) {
        if (identifier != NULL) {
            *identifier = (const char *) node->key;
        }
        return (const struct kefir_ir_data *) node->value;
    } else {
        if (identifier != NULL) {
            *identifier = NULL;
        }
        return NULL;
    }
}
const struct kefir_ir_data *kefir_ir_module_named_data_next(struct kefir_hashtree_node_iterator *iter,
                                                            const char **identifier) {
    REQUIRE(iter != NULL, NULL);
    const struct kefir_hashtree_node *node = kefir_hashtree_next(iter);
    if (node != NULL) {
        if (identifier != NULL) {
            *identifier = (const char *) node->key;
        }
        return (const struct kefir_ir_data *) node->value;
    } else {
        if (identifier != NULL) {
            *identifier = NULL;
        }
        return NULL;
    }
}

const struct kefir_ir_type *kefir_ir_module_named_type_iter(const struct kefir_ir_module *module,
                                                            struct kefir_hashtree_node_iterator *iter,
                                                            kefir_id_t *type_id_ptr) {
    REQUIRE(module != NULL, NULL);
    REQUIRE(iter != NULL, NULL);

    const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->named_types, iter);
    if (node != NULL) {
        ASSIGN_PTR(type_id_ptr, (kefir_id_t) node->key);
        return (const struct kefir_ir_type *) node->value;
    } else {
        return NULL;
    }
}

const struct kefir_ir_type *kefir_ir_module_named_type_next(struct kefir_hashtree_node_iterator *iter,
                                                            kefir_id_t *type_id_ptr) {
    REQUIRE(iter != NULL, NULL);
    const struct kefir_hashtree_node *node = kefir_hashtree_next(iter);
    if (node != NULL) {
        ASSIGN_PTR(type_id_ptr, (kefir_id_t) node->key);
        return (const struct kefir_ir_type *) node->value;
    } else {
        return NULL;
    }
}

const char *kefir_ir_module_get_named_symbol(const struct kefir_ir_module *module, kefir_id_t id) {
    REQUIRE(module != NULL, NULL);
    return kefir_string_pool_get(&module->symbols, id);
}

const struct kefir_ir_function_decl *kefir_ir_module_get_declaration(const struct kefir_ir_module *module,
                                                                     kefir_id_t id) {
    REQUIRE(module != NULL, NULL);
    struct kefir_hashtree_node *node = NULL;
    REQUIRE(kefir_hashtree_at(&module->function_declarations, (kefir_hashtree_key_t) id, &node) == KEFIR_OK, NULL);
    REQUIRE(node != NULL, NULL);
    return (const struct kefir_ir_function_decl *) node->value;
}

struct kefir_ir_type *kefir_ir_module_get_named_type(const struct kefir_ir_module *module, kefir_id_t id) {
    REQUIRE(module != NULL, NULL);
    struct kefir_hashtree_node *node = NULL;
    REQUIRE_ELSE(kefir_hashtree_at(&module->named_types, (kefir_hashtree_key_t) id, &node) == KEFIR_OK,
                 { return NULL; });
    return (struct kefir_ir_type *) node->value;
}

struct kefir_ir_inline_assembly *kefir_ir_module_new_inline_assembly(struct kefir_mem *mem,
                                                                     struct kefir_ir_module *module,
                                                                     const char *template, kefir_id_t *inline_asm_id) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(module != NULL, NULL);
    REQUIRE(template != NULL, NULL);

    kefir_id_t id = module->next_inline_assembly_id++;
    struct kefir_ir_inline_assembly *inline_asm = kefir_ir_inline_assembly_alloc(mem, &module->symbols, id, template);
    REQUIRE(inline_asm != NULL, NULL);
    kefir_result_t res = kefir_hashtree_insert(mem, &module->inline_assembly, (kefir_hashtree_key_t) id,
                                               (kefir_hashtree_value_t) inline_asm);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ir_inline_assembly_free(mem, inline_asm);
        return NULL;
    });

    ASSIGN_PTR(inline_asm_id, id);
    return inline_asm;
}

const struct kefir_ir_inline_assembly *kefir_ir_module_get_inline_assembly(const struct kefir_ir_module *module,
                                                                           kefir_id_t id) {
    REQUIRE(module != NULL, NULL);

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&module->inline_assembly, id, &node);
    REQUIRE(res == KEFIR_OK, NULL);
    return (struct kefir_ir_inline_assembly *) node->value;
}

const struct kefir_ir_inline_assembly *kefir_ir_module_inline_assembly_iter(const struct kefir_ir_module *module,
                                                                            struct kefir_hashtree_node_iterator *iter,
                                                                            kefir_id_t *id_ptr) {
    REQUIRE(module != NULL, NULL);
    REQUIRE(iter != NULL, NULL);
    const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->inline_assembly, iter);
    if (node != NULL) {
        if (id_ptr != NULL) {
            *id_ptr = (kefir_id_t) node->key;
        }
        return (const struct kefir_ir_inline_assembly *) node->value;
    } else {
        return NULL;
    }
}

const struct kefir_ir_inline_assembly *kefir_ir_module_inline_assembly_next(struct kefir_hashtree_node_iterator *iter,
                                                                            kefir_id_t *id_ptr) {
    REQUIRE(iter != NULL, NULL);

    const struct kefir_hashtree_node *node = kefir_hashtree_next(iter);
    if (node != NULL) {
        if (id_ptr != NULL) {
            *id_ptr = (kefir_id_t) node->key;
        }
        return (const struct kefir_ir_inline_assembly *) node->value;
    } else {
        return NULL;
    }
}

kefir_result_t kefir_ir_module_inline_assembly_global(struct kefir_mem *mem, struct kefir_ir_module *module,
                                                      kefir_id_t asm_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));

    const struct kefir_ir_inline_assembly *inline_asm = kefir_ir_module_get_inline_assembly(module, asm_id);
    REQUIRE(inline_asm != NULL,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Requested inline assembly is not found in IR module"));
    kefir_result_t res = kefir_hashtree_insert(mem, &module->global_inline_asm, (kefir_hashtree_key_t) asm_id,
                                               (kefir_hashtree_value_t) inline_asm);
    if (res != KEFIR_ALREADY_EXISTS) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}
