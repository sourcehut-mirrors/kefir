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

#include "kefir/ast/declarator.h"
#include "kefir/ast/node.h"
#include "kefir/ast/downcast.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

kefir_result_t kefir_ast_declarator_specifier_list_init(struct kefir_ast_declarator_specifier_list *list) {
    REQUIRE(list != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST declarator specifier list"));

    list->specifiers = NULL;
    list->specifiers_length = 0;
    REQUIRE_OK(kefir_ast_node_attributes_init(&list->attributes));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_declarator_specifier_list_free(struct kefir_mem *mem,
                                                        struct kefir_ast_declarator_specifier_list *list) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(list != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST declarator specifier list"));

    REQUIRE_OK(kefir_ast_node_attributes_free(mem, &list->attributes));
    for (kefir_size_t i = 0; i < list->specifiers_length; i++) {
        REQUIRE_OK(kefir_ast_declarator_specifier_free(mem, list->specifiers[i]));
    }
    KEFIR_FREE(mem, list->specifiers);
    list->specifiers = NULL;
    list->specifiers_length = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_declarator_specifier_list_append(struct kefir_mem *mem,
                                                          struct kefir_ast_declarator_specifier_list *list,
                                                          struct kefir_ast_declarator_specifier *specifier) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(list != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST declarator specifier list"));
    REQUIRE(specifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST declarator specifier"));

    kefir_size_t new_length = list->specifiers_length + 1;
    struct kefir_ast_declarator_specifier **new_specifiers = KEFIR_REALLOC(mem, list->specifiers, sizeof(struct kefir_ast_declarator_specifier *) * new_length);
    REQUIRE(new_specifiers != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST declarator specifier list"));
    new_specifiers[list->specifiers_length] = specifier;
    list->specifiers = new_specifiers;
    list->specifiers_length = new_length;
    return KEFIR_OK;
}

kefir_bool_t kefir_ast_declarator_specifier_list_empty(const struct kefir_ast_declarator_specifier_list *list) {
    REQUIRE(list != NULL, true);
    return list->specifiers_length == 0;
}

kefir_result_t kefir_ast_declarator_specifier_list_iter(
    const struct kefir_ast_declarator_specifier_list *list, struct kefir_ast_declarator_specifier_list_iterator *iter, struct kefir_ast_declarator_specifier **specifier_ptr) {
    REQUIRE(list != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST declarator specifier list"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST declarator specifier list iterator"));

    iter->list = list;
    iter->index = 0;
    REQUIRE(iter->index < iter->list->specifiers_length, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of AST declarator specifier list iterator"));
    ASSIGN_PTR(specifier_ptr, (struct kefir_ast_declarator_specifier *) iter->list->specifiers[iter->index]);
    iter->index++;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_declarator_specifier_list_next(
    struct kefir_ast_declarator_specifier_list_iterator *iter, struct kefir_ast_declarator_specifier **specifier_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST declarator specifier list iterator"));

    REQUIRE(iter->index < iter->list->specifiers_length, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of AST declarator specifier list iterator"));
    ASSIGN_PTR(specifier_ptr, (struct kefir_ast_declarator_specifier *) iter->list->specifiers[iter->index]);
    iter->index++;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_declarator_specifier_list_move_all(struct kefir_mem *mem, struct kefir_ast_declarator_specifier_list *dst,
                                                            struct kefir_ast_declarator_specifier_list *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dst != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination AST declarator specifier list"));
    REQUIRE(src != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source AST declarator specifier list"));
    REQUIRE(src->specifiers_length > 0, KEFIR_OK);

    kefir_size_t new_length = dst->specifiers_length + src->specifiers_length;
    struct kefir_ast_declarator_specifier **new_specifiers = KEFIR_REALLOC(mem, dst->specifiers, sizeof(struct kefir_ast_declarator_specifier *) * new_length);
    REQUIRE(new_specifiers != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST declarator specifier list"));
    memcpy(&new_specifiers[dst->specifiers_length], src->specifiers, sizeof(struct kefir_ast_declarator_specifier *) * src->specifiers_length);

    dst->specifiers_length = new_length;
    dst->specifiers = new_specifiers;
    
    KEFIR_FREE(mem, src->specifiers);
    src->specifiers = NULL;
    src->specifiers_length = 0;

    REQUIRE_OK(kefir_ast_node_attributes_move(mem, &dst->attributes, &src->attributes));
    return KEFIR_OK;
}

const struct kefir_source_location *kefir_ast_declarator_specifier_list_source_location(
    const struct kefir_ast_declarator_specifier_list *list) {
    REQUIRE(list != NULL, NULL);
    REQUIRE(list->specifiers_length > 0, NULL);

    return &list->specifiers[0]->source_location;
}

static kefir_result_t struct_entry_remove(struct kefir_mem *mem, struct kefir_list *list,
                                          struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));

    ASSIGN_DECL_CAST(struct kefir_ast_structure_declaration_entry *, decl_entry, entry->value);
    REQUIRE_OK(kefir_ast_structure_declaration_entry_free(mem, decl_entry));
    return KEFIR_OK;
}

struct kefir_ast_structure_specifier *kefir_ast_structure_specifier_init(struct kefir_mem *mem,
                                                                         struct kefir_string_pool *symbols,
                                                                         const char *identifier,
                                                                         kefir_bool_t complete) {
    REQUIRE(mem != NULL, NULL);

    if (symbols != NULL && identifier != NULL) {
        identifier = kefir_string_pool_insert(mem, symbols, identifier, NULL);
        REQUIRE(identifier != NULL, NULL);
    }

    struct kefir_ast_structure_specifier *specifier = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_structure_specifier));
    REQUIRE(specifier != NULL, NULL);

    if (complete) {
        REQUIRE(kefir_list_init(&specifier->entries) == KEFIR_OK, NULL);
        REQUIRE(kefir_list_on_remove(&specifier->entries, struct_entry_remove, NULL) == KEFIR_OK, NULL);
    }

    specifier->identifier = identifier;
    specifier->complete = complete;
    return specifier;
}

kefir_result_t kefir_ast_structure_specifier_free(struct kefir_mem *mem,
                                                  struct kefir_ast_structure_specifier *specifier) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(specifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST structure specifier"));

    if (specifier->complete) {
        REQUIRE_OK(kefir_list_free(mem, &specifier->entries));
    }
    specifier->complete = false;
    specifier->identifier = NULL;
    KEFIR_FREE(mem, specifier);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_structure_specifier_append_entry(struct kefir_mem *mem,
                                                          struct kefir_ast_structure_specifier *specifier,
                                                          struct kefir_ast_structure_declaration_entry *entry) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(specifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST structure specifier"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST structure declaration entry"));
    REQUIRE(specifier->complete,
            KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Unable to insert into incomplete structure declaration"));

    REQUIRE_OK(kefir_list_insert_after(mem, &specifier->entries, kefir_list_tail(&specifier->entries), entry));
    return KEFIR_OK;
}

static kefir_result_t declaration_declarator_remove(struct kefir_mem *mem, struct kefir_list *list,
                                                    struct kefir_list_entry *entry, void *payload) {
    UNUSED(list != NULL);
    UNUSED(payload != NULL);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));

    ASSIGN_DECL_CAST(struct kefir_ast_structure_entry_declarator *, declarator, entry->value);
    REQUIRE_OK(kefir_ast_declarator_free(mem, declarator->declarator));
    declarator->declarator = NULL;
    if (declarator->bitwidth != NULL) {
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, declarator->bitwidth));
        declarator->bitwidth = NULL;
    }
    KEFIR_FREE(mem, declarator);
    return KEFIR_OK;
}

struct kefir_ast_structure_declaration_entry *kefir_ast_structure_declaration_entry_alloc(struct kefir_mem *mem) {
    REQUIRE(mem != NULL, NULL);

    struct kefir_ast_structure_declaration_entry *entry =
        KEFIR_MALLOC(mem, sizeof(struct kefir_ast_structure_declaration_entry));
    REQUIRE(entry != NULL, NULL);

    entry->is_static_assertion = false;
    kefir_result_t res = kefir_ast_declarator_specifier_list_init(&entry->declaration.specifiers);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, entry);
        return NULL;
    });

    res = kefir_list_init(&entry->declaration.declarators);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_declarator_specifier_list_free(mem, &entry->declaration.specifiers);
        KEFIR_FREE(mem, entry);
        return NULL;
    });

    res = kefir_list_on_remove(&entry->declaration.declarators, declaration_declarator_remove, NULL);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &entry->declaration.declarators);
        kefir_ast_declarator_specifier_list_free(mem, &entry->declaration.specifiers);
        KEFIR_FREE(mem, entry);
        return NULL;
    });

    return entry;
}

struct kefir_ast_structure_declaration_entry *kefir_ast_structure_declaration_entry_alloc_assert(
    struct kefir_mem *mem, struct kefir_ast_static_assertion *static_assertion) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(static_assertion != NULL, NULL);

    struct kefir_ast_structure_declaration_entry *entry =
        KEFIR_MALLOC(mem, sizeof(struct kefir_ast_structure_declaration_entry));
    REQUIRE(entry != NULL, NULL);

    entry->is_static_assertion = true;
    entry->static_assertion = static_assertion;
    return entry;
}

kefir_result_t kefir_ast_structure_declaration_entry_free(struct kefir_mem *mem,
                                                          struct kefir_ast_structure_declaration_entry *entry) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST structure declaration entry"));

    if (entry->is_static_assertion) {
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(entry->static_assertion)));
        entry->static_assertion = NULL;
        entry->is_static_assertion = false;
    } else {
        REQUIRE_OK(kefir_ast_declarator_specifier_list_free(mem, &entry->declaration.specifiers));
        REQUIRE_OK(kefir_list_free(mem, &entry->declaration.declarators));
    }
    KEFIR_FREE(mem, entry);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_structure_declaration_entry_append(struct kefir_mem *mem,
                                                            struct kefir_ast_structure_declaration_entry *entry,
                                                            struct kefir_ast_declarator *declarator,
                                                            struct kefir_ast_node_base *bitwidth) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST structure declaration entry"));
    REQUIRE(declarator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST declarator"));
    REQUIRE(!entry->is_static_assertion,
            KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Unable to append declarators to a static assertion entry"));

    struct kefir_ast_structure_entry_declarator *entry_declarator =
        KEFIR_MALLOC(mem, sizeof(struct kefir_ast_structure_entry_declarator));
    REQUIRE(entry_declarator != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST structure entry declarator"));
    entry_declarator->declarator = declarator;
    entry_declarator->bitwidth = bitwidth;

    kefir_result_t res = kefir_list_insert_after(mem, &entry->declaration.declarators,
                                                 kefir_list_tail(&entry->declaration.declarators), entry_declarator);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, entry_declarator);
        return res;
    });

    return KEFIR_OK;
}

static kefir_result_t remove_enum_entry(struct kefir_mem *mem, struct kefir_list *list, struct kefir_list_entry *entry,
                                        void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));

    ASSIGN_DECL_CAST(struct kefir_ast_enum_specifier_entry *, enum_entry, entry->value);
    REQUIRE_OK(kefir_ast_node_attributes_free(mem, &enum_entry->attributes));
    if (enum_entry->value != NULL) {
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, enum_entry->value));
    }
    enum_entry->value = NULL;
    enum_entry->constant = NULL;
    KEFIR_FREE(mem, enum_entry);
    return KEFIR_OK;
}

struct kefir_ast_enum_specifier *kefir_ast_enum_specifier_init(
    struct kefir_mem *mem, struct kefir_string_pool *symbols, const char *identifier, kefir_bool_t complete,
    struct kefir_ast_declarator_specifier_list *enum_type_spec) {
    REQUIRE(mem != NULL, NULL);

    if (symbols != NULL && identifier != NULL) {
        identifier = kefir_string_pool_insert(mem, symbols, identifier, NULL);
        REQUIRE(identifier != NULL, NULL);
    }

    struct kefir_ast_enum_specifier *specifier = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_enum_specifier));
    REQUIRE(specifier != NULL, NULL);

    specifier->identifier = identifier;
    specifier->complete = complete;
    if (complete) {
        REQUIRE(kefir_list_init(&specifier->entries) == KEFIR_OK, NULL);
        REQUIRE(kefir_list_on_remove(&specifier->entries, remove_enum_entry, NULL) == KEFIR_OK, NULL);
    }

    specifier->type_spec.present = enum_type_spec != NULL;
    kefir_result_t res;
    if (specifier->type_spec.present) {
        res = kefir_ast_declarator_specifier_list_init(&specifier->type_spec.specifier_list);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, specifier);
            return NULL;
        });
        res = kefir_ast_declarator_specifier_list_move_all(mem, &specifier->type_spec.specifier_list, enum_type_spec);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_declarator_specifier_list_free(mem, &specifier->type_spec.specifier_list);
            KEFIR_FREE(mem, specifier);
            return NULL;
        });
    }

    return specifier;
}

kefir_result_t kefir_ast_enum_specifier_free(struct kefir_mem *mem, struct kefir_ast_enum_specifier *specifier) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(specifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST enum specifier"));

    if (specifier->type_spec.present) {
        REQUIRE_OK(kefir_ast_declarator_specifier_list_free(mem, &specifier->type_spec.specifier_list));
    }
    if (specifier->complete) {
        REQUIRE_OK(kefir_list_free(mem, &specifier->entries));
    }
    specifier->complete = false;
    specifier->identifier = NULL;
    KEFIR_FREE(mem, specifier);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_enum_specifier_append(struct kefir_mem *mem, struct kefir_ast_enum_specifier *specifier,
                                               struct kefir_string_pool *symbols, const char *identifier,
                                               struct kefir_ast_node_base *value,
                                               const struct kefir_ast_node_attributes *attributes) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(specifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST enum specifier"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST enum entry identifier"));
    REQUIRE(specifier->complete,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected AST enum specifier to be complete"));

    if (symbols != NULL) {
        identifier = kefir_string_pool_insert(mem, symbols, identifier, NULL);
        REQUIRE(identifier != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to insert identifier into symbol table"));
    }

    struct kefir_ast_enum_specifier_entry *entry = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_enum_specifier_entry));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST enum specifier entry"));
    entry->constant = identifier;
    entry->value = value;

    kefir_result_t res = kefir_ast_node_attributes_init(&entry->attributes);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, entry);
        return res;
    });
    if (attributes != NULL) {
        REQUIRE_CHAIN(&res, kefir_ast_node_attributes_clone(mem, &entry->attributes, attributes));
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_node_attributes_free(mem, &entry->attributes);
            KEFIR_FREE(mem, entry);
            return res;
        });
    }

    res = kefir_list_insert_after(mem, &specifier->entries, kefir_list_tail(&specifier->entries), entry);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, entry);
        return res;
    });

    return KEFIR_OK;
}

#define TYPE_SPECIFIER(_id, _spec)                                                                 \
    struct kefir_ast_declarator_specifier *kefir_ast_type_specifier_##_id(struct kefir_mem *mem) { \
        REQUIRE(mem != NULL, NULL);                                                                \
        struct kefir_ast_declarator_specifier *specifier =                                         \
            KEFIR_MALLOC(mem, sizeof(struct kefir_ast_declarator_specifier));                      \
        REQUIRE(specifier != NULL, NULL);                                                          \
                                                                                                   \
        specifier->refcount = 1;                                               \
        specifier->klass = KEFIR_AST_TYPE_SPECIFIER;                                               \
        static const struct kefir_ast_type_specifier SPECIFIER = { .specifier = (_spec) }; \
        specifier->type_specifier = &SPECIFIER;                                             \
        kefir_result_t res = kefir_source_location_empty(&specifier->source_location);             \
        REQUIRE_CHAIN(&res, kefir_ast_node_attributes_init(&specifier->attributes));               \
        REQUIRE_ELSE(res == KEFIR_OK, {                                                            \
            KEFIR_FREE(mem, specifier);                                                            \
            return NULL;                                                                           \
        });                                                                                        \
        return specifier;                                                                          \
    }

TYPE_SPECIFIER(void, KEFIR_AST_TYPE_SPECIFIER_VOID)
TYPE_SPECIFIER(char, KEFIR_AST_TYPE_SPECIFIER_CHAR)
TYPE_SPECIFIER(short, KEFIR_AST_TYPE_SPECIFIER_SHORT)
TYPE_SPECIFIER(int, KEFIR_AST_TYPE_SPECIFIER_INT)
TYPE_SPECIFIER(long, KEFIR_AST_TYPE_SPECIFIER_LONG)
TYPE_SPECIFIER(float, KEFIR_AST_TYPE_SPECIFIER_FLOAT)
TYPE_SPECIFIER(double, KEFIR_AST_TYPE_SPECIFIER_DOUBLE)
TYPE_SPECIFIER(signed, KEFIR_AST_TYPE_SPECIFIER_SIGNED)
TYPE_SPECIFIER(unsigned, KEFIR_AST_TYPE_SPECIFIER_UNSIGNED)
TYPE_SPECIFIER(unsigned_override, KEFIR_AST_TYPE_SPECIFIER_UNSIGNED_OVERRIDE)
TYPE_SPECIFIER(boolean, KEFIR_AST_TYPE_SPECIFIER_BOOL)
TYPE_SPECIFIER(complex, KEFIR_AST_TYPE_SPECIFIER_COMPLEX)
TYPE_SPECIFIER(imaginary, KEFIR_AST_TYPE_SPECIFIER_IMAGINARY)
TYPE_SPECIFIER(decimal32, KEFIR_AST_TYPE_SPECIFIER_DECIMAL32)
TYPE_SPECIFIER(decimal64, KEFIR_AST_TYPE_SPECIFIER_DECIMAL64)
TYPE_SPECIFIER(decimal128, KEFIR_AST_TYPE_SPECIFIER_DECIMAL128)
TYPE_SPECIFIER(float32, KEFIR_AST_TYPE_SPECIFIER_FLOAT32)
TYPE_SPECIFIER(float64, KEFIR_AST_TYPE_SPECIFIER_FLOAT64)
TYPE_SPECIFIER(float80, KEFIR_AST_TYPE_SPECIFIER_FLOAT80)
TYPE_SPECIFIER(float32x, KEFIR_AST_TYPE_SPECIFIER_FLOAT32X)
TYPE_SPECIFIER(float64x, KEFIR_AST_TYPE_SPECIFIER_FLOAT64X)
TYPE_SPECIFIER(decimal64x, KEFIR_AST_TYPE_SPECIFIER_DECIMAL64X)
TYPE_SPECIFIER(int128, KEFIR_AST_TYPE_SPECIFIER_INT128)
TYPE_SPECIFIER(auto_type, KEFIR_AST_TYPE_SPECIFIER_AUTO_TYPE)

#undef TYPE_SPECIFIER

struct kefir_ast_declarator_specifier *kefir_ast_type_specifier_atomic(struct kefir_mem *mem,
                                                                       struct kefir_ast_node_base *type) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(type != NULL, NULL);

    struct kefir_ast_declarator_specifier *specifier = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_declarator_specifier));
    REQUIRE(specifier != NULL, NULL);

    struct kefir_ast_type_specifier *spec = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_type_specifier));
    REQUIRE_ELSE(spec != NULL, {
        KEFIR_FREE(mem, specifier);
        return NULL;
    });

    specifier->refcount = 1;
    specifier->klass = KEFIR_AST_TYPE_SPECIFIER;
    specifier->type_specifier = spec;
    spec->specifier = KEFIR_AST_TYPE_SPECIFIER_ATOMIC;
    spec->value.atomic_type = type;
    kefir_result_t res = kefir_source_location_empty(&specifier->source_location);
    REQUIRE_CHAIN(&res, kefir_ast_node_attributes_init(&specifier->attributes));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, spec);
        KEFIR_FREE(mem, specifier);
        return NULL;
    });
    return specifier;
}

struct kefir_ast_declarator_specifier *kefir_ast_type_specifier_struct(
    struct kefir_mem *mem, struct kefir_ast_structure_specifier *structure) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(structure != NULL, NULL);

    struct kefir_ast_declarator_specifier *specifier = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_declarator_specifier));
    REQUIRE(specifier != NULL, NULL);

    struct kefir_ast_type_specifier *spec = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_type_specifier));
    REQUIRE_ELSE(spec != NULL, {
        KEFIR_FREE(mem, specifier);
        return NULL;
    });

    specifier->refcount = 1;
    specifier->klass = KEFIR_AST_TYPE_SPECIFIER;
    specifier->type_specifier = spec;
    spec->specifier = KEFIR_AST_TYPE_SPECIFIER_STRUCT;
    spec->value.structure = structure;
    kefir_result_t res = kefir_source_location_empty(&specifier->source_location);
    REQUIRE_CHAIN(&res, kefir_ast_node_attributes_init(&specifier->attributes));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, spec);
        KEFIR_FREE(mem, specifier);
        return NULL;
    });
    return specifier;
}

struct kefir_ast_declarator_specifier *kefir_ast_type_specifier_union(struct kefir_mem *mem,
                                                                      struct kefir_ast_structure_specifier *structure) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(structure != NULL, NULL);

    struct kefir_ast_declarator_specifier *specifier = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_declarator_specifier));
    REQUIRE(specifier != NULL, NULL);

    struct kefir_ast_type_specifier *spec = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_type_specifier));
    REQUIRE_ELSE(spec != NULL, {
        KEFIR_FREE(mem, specifier);
        return NULL;
    });

    specifier->refcount = 1;
    specifier->klass = KEFIR_AST_TYPE_SPECIFIER;
    specifier->type_specifier = spec;
    spec->specifier = KEFIR_AST_TYPE_SPECIFIER_UNION;
    spec->value.structure = structure;
    kefir_result_t res = kefir_source_location_empty(&specifier->source_location);
    REQUIRE_CHAIN(&res, kefir_ast_node_attributes_init(&specifier->attributes));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, spec);
        KEFIR_FREE(mem, specifier);
        return NULL;
    });
    return specifier;
}

struct kefir_ast_declarator_specifier *kefir_ast_type_specifier_enum(struct kefir_mem *mem,
                                                                     struct kefir_ast_enum_specifier *enumeration) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(enumeration != NULL, NULL);

    struct kefir_ast_declarator_specifier *specifier = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_declarator_specifier));
    REQUIRE(specifier != NULL, NULL);

    struct kefir_ast_type_specifier *spec = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_type_specifier));
    REQUIRE_ELSE(spec != NULL, {
        KEFIR_FREE(mem, specifier);
        return NULL;
    });

    specifier->refcount = 1;
    specifier->klass = KEFIR_AST_TYPE_SPECIFIER;
    specifier->type_specifier = spec;
    spec->specifier = KEFIR_AST_TYPE_SPECIFIER_ENUM;
    spec->value.enumeration = enumeration;
    kefir_result_t res = kefir_source_location_empty(&specifier->source_location);
    REQUIRE_CHAIN(&res, kefir_ast_node_attributes_init(&specifier->attributes));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, spec);
        KEFIR_FREE(mem, specifier);
        return NULL;
    });
    return specifier;
}

struct kefir_ast_declarator_specifier *kefir_ast_type_specifier_typedef(struct kefir_mem *mem,
                                                                        struct kefir_string_pool *symbols,
                                                                        const char *literal) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(literal != NULL, NULL);

    if (symbols != NULL) {
        literal = kefir_string_pool_insert(mem, symbols, literal, NULL);
        REQUIRE(literal != NULL, NULL);
    }

    struct kefir_ast_declarator_specifier *specifier = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_declarator_specifier));
    REQUIRE(specifier != NULL, NULL);

    struct kefir_ast_type_specifier *spec = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_type_specifier));
    REQUIRE_ELSE(spec != NULL, {
        KEFIR_FREE(mem, specifier);
        return NULL;
    });

    specifier->refcount = 1;
    specifier->klass = KEFIR_AST_TYPE_SPECIFIER;
    specifier->type_specifier = spec;
    spec->specifier = KEFIR_AST_TYPE_SPECIFIER_TYPEDEF;
    spec->value.type_name = literal;
    kefir_result_t res = kefir_source_location_empty(&specifier->source_location);
    REQUIRE_CHAIN(&res, kefir_ast_node_attributes_init(&specifier->attributes));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, spec);
        KEFIR_FREE(mem, specifier);
        return NULL;
    });
    return specifier;
}

struct kefir_ast_declarator_specifier *kefir_ast_type_specifier_typeof(struct kefir_mem *mem, kefir_bool_t qualified,
                                                                       struct kefir_ast_node_base *node) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(node != NULL, NULL);

    struct kefir_ast_declarator_specifier *specifier = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_declarator_specifier));
    REQUIRE(specifier != NULL, NULL);
    
    struct kefir_ast_type_specifier *spec = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_type_specifier));
    REQUIRE_ELSE(spec != NULL, {
        KEFIR_FREE(mem, specifier);
        return NULL;
    });

    specifier->refcount = 1;
    specifier->klass = KEFIR_AST_TYPE_SPECIFIER;
    specifier->type_specifier = spec;
    spec->specifier = KEFIR_AST_TYPE_SPECIFIER_TYPEOF;
    spec->value.type_of.qualified = qualified;
    spec->value.type_of.node = node;
    kefir_result_t res = kefir_source_location_empty(&specifier->source_location);
    REQUIRE_CHAIN(&res, kefir_ast_node_attributes_init(&specifier->attributes));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, spec);
        KEFIR_FREE(mem, specifier);
        return NULL;
    });
    return specifier;
}

struct kefir_ast_declarator_specifier *kefir_ast_type_specifier_bitint(struct kefir_mem *mem,
                                                                       struct kefir_ast_node_base *width) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(width != NULL, NULL);

    struct kefir_ast_declarator_specifier *specifier = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_declarator_specifier));
    REQUIRE(specifier != NULL, NULL);
    
    struct kefir_ast_type_specifier *spec = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_type_specifier));
    REQUIRE_ELSE(spec != NULL, {
        KEFIR_FREE(mem, specifier);
        return NULL;
    });

    specifier->refcount = 1;
    specifier->klass = KEFIR_AST_TYPE_SPECIFIER;
    specifier->type_specifier = spec;
    spec->specifier = KEFIR_AST_TYPE_SPECIFIER_BITINT;
    spec->value.bitprecise.width = width;
    kefir_result_t res = kefir_source_location_empty(&specifier->source_location);
    REQUIRE_CHAIN(&res, kefir_ast_node_attributes_init(&specifier->attributes));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, spec);
        KEFIR_FREE(mem, specifier);
        return NULL;
    });
    return specifier;
}

#define STORAGE_CLASS_SPECIFIER(_id, _spec)                                                                 \
    struct kefir_ast_declarator_specifier *kefir_ast_storage_class_specifier_##_id(struct kefir_mem *mem) { \
        REQUIRE(mem != NULL, NULL);                                                                         \
        struct kefir_ast_declarator_specifier *specifier =                                                  \
            KEFIR_MALLOC(mem, sizeof(struct kefir_ast_declarator_specifier));                               \
        REQUIRE(specifier != NULL, NULL);                                                                   \
                                                                                                            \
        specifier->refcount = 1; \
        specifier->klass = KEFIR_AST_STORAGE_CLASS_SPECIFIER;                                               \
        specifier->storage_class = (_spec);                                                                 \
        kefir_result_t res = kefir_source_location_empty(&specifier->source_location);                      \
        REQUIRE_CHAIN(&res, kefir_ast_node_attributes_init(&specifier->attributes));                        \
        REQUIRE_ELSE(res == KEFIR_OK, {                                                                     \
            KEFIR_FREE(mem, specifier);                                                                     \
            return NULL;                                                                                    \
        });                                                                                                 \
        return specifier;                                                                                   \
    }

STORAGE_CLASS_SPECIFIER(typedef, KEFIR_AST_STORAGE_SPECIFIER_TYPEDEF)
STORAGE_CLASS_SPECIFIER(extern, KEFIR_AST_STORAGE_SPECIFIER_EXTERN)
STORAGE_CLASS_SPECIFIER(static, KEFIR_AST_STORAGE_SPECIFIER_STATIC)
STORAGE_CLASS_SPECIFIER(constexpr, KEFIR_AST_STORAGE_SPECIFIER_CONSTEXPR)
STORAGE_CLASS_SPECIFIER(thread_local, KEFIR_AST_STORAGE_SPECIFIER_THREAD_LOCAL)
STORAGE_CLASS_SPECIFIER(auto, KEFIR_AST_STORAGE_SPECIFIER_AUTO)
STORAGE_CLASS_SPECIFIER(register, KEFIR_AST_STORAGE_SPECIFIER_REGISTER)

#undef STORAGE_CLASS_SPECIFIER

#define TYPE_QUALIFIER(_id, _spec)                                                                 \
    struct kefir_ast_declarator_specifier *kefir_ast_type_qualifier_##_id(struct kefir_mem *mem) { \
        REQUIRE(mem != NULL, NULL);                                                                \
        struct kefir_ast_declarator_specifier *specifier =                                         \
            KEFIR_MALLOC(mem, sizeof(struct kefir_ast_declarator_specifier));                      \
        REQUIRE(specifier != NULL, NULL);                                                          \
                                                                                                   \
        specifier->refcount = 1; \
        specifier->klass = KEFIR_AST_TYPE_QUALIFIER;                                               \
        specifier->type_qualifier = (_spec);                                                       \
        kefir_result_t res = kefir_source_location_empty(&specifier->source_location);             \
        REQUIRE_CHAIN(&res, kefir_ast_node_attributes_init(&specifier->attributes));               \
        REQUIRE_ELSE(res == KEFIR_OK, {                                                            \
            KEFIR_FREE(mem, specifier);                                                            \
            return NULL;                                                                           \
        });                                                                                        \
        return specifier;                                                                          \
    }

TYPE_QUALIFIER(const, KEFIR_AST_TYPE_QUALIFIER_CONST)
TYPE_QUALIFIER(restrict, KEFIR_AST_TYPE_QUALIFIER_RESTRICT)
TYPE_QUALIFIER(volatile, KEFIR_AST_TYPE_QUALIFIER_VOLATILE)
TYPE_QUALIFIER(atomic, KEFIR_AST_TYPE_QUALIFIER_ATOMIC)

#undef TYPE_QUALIFIER

#define FUNCTION_SPECIFIER(_id, _spec)                                                                 \
    struct kefir_ast_declarator_specifier *kefir_ast_function_specifier_##_id(struct kefir_mem *mem) { \
        REQUIRE(mem != NULL, NULL);                                                                    \
        struct kefir_ast_declarator_specifier *specifier =                                             \
            KEFIR_MALLOC(mem, sizeof(struct kefir_ast_declarator_specifier));                          \
        REQUIRE(specifier != NULL, NULL);                                                              \
                                                                                                       \
        specifier->refcount = 1; \
        specifier->klass = KEFIR_AST_FUNCTION_SPECIFIER;                                               \
        specifier->function_specifier = (_spec);                                                       \
        kefir_result_t res = kefir_source_location_empty(&specifier->source_location);                 \
        REQUIRE_CHAIN(&res, kefir_ast_node_attributes_init(&specifier->attributes));                   \
        REQUIRE_ELSE(res == KEFIR_OK, {                                                                \
            KEFIR_FREE(mem, specifier);                                                                \
            return NULL;                                                                               \
        });                                                                                            \
        return specifier;                                                                              \
    }

FUNCTION_SPECIFIER(inline, KEFIR_AST_FUNCTION_SPECIFIER_TYPE_INLINE)
FUNCTION_SPECIFIER(noreturn, KEFIR_AST_FUNCTION_SPECIFIER_TYPE_NORETURN)

#undef FUNCTION_SPECIFIER

struct kefir_ast_declarator_specifier *kefir_ast_alignment_specifier(struct kefir_mem *mem,
                                                                     struct kefir_ast_node_base *alignment) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(alignment != NULL, NULL);

    struct kefir_ast_declarator_specifier *specifier = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_declarator_specifier));
    REQUIRE(specifier != NULL, NULL);

    specifier->refcount = 1;
    specifier->klass = KEFIR_AST_ALIGNMENT_SPECIFIER;
    specifier->alignment_specifier = alignment;
    kefir_result_t res = kefir_source_location_empty(&specifier->source_location);
    REQUIRE_CHAIN(&res, kefir_ast_node_attributes_init(&specifier->attributes));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, specifier);
        return NULL;
    });
    return specifier;
}

struct kefir_ast_declarator_specifier *kefir_ast_declarator_specifier_ref(
    struct kefir_ast_declarator_specifier *specifier) {
    REQUIRE(specifier != NULL, NULL);

    specifier->refcount++;
    return specifier;
}

kefir_result_t kefir_ast_declarator_specifier_free(struct kefir_mem *mem,
                                                   struct kefir_ast_declarator_specifier *specifier) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(specifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST declarator specifier"));
    REQUIRE(specifier->refcount > 0, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST declarator specifier reference count"));
    REQUIRE(--specifier->refcount == 0, KEFIR_OK);

    REQUIRE_OK(kefir_ast_node_attributes_free(mem, &specifier->attributes));
    switch (specifier->klass) {
        case KEFIR_AST_TYPE_SPECIFIER:
            switch (specifier->type_specifier->specifier) {
                case KEFIR_AST_TYPE_SPECIFIER_ATOMIC:
                    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, specifier->type_specifier->value.atomic_type));
                    break;

                case KEFIR_AST_TYPE_SPECIFIER_STRUCT:
                case KEFIR_AST_TYPE_SPECIFIER_UNION:
                    REQUIRE_OK(kefir_ast_structure_specifier_free(mem, specifier->type_specifier->value.structure));
                    break;

                case KEFIR_AST_TYPE_SPECIFIER_ENUM:
                    REQUIRE_OK(kefir_ast_enum_specifier_free(mem, specifier->type_specifier->value.enumeration));
                    break;

                case KEFIR_AST_TYPE_SPECIFIER_TYPEOF:
                    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, specifier->type_specifier->value.type_of.node));
                    break;

                case KEFIR_AST_TYPE_SPECIFIER_BITINT:
                    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, specifier->type_specifier->value.bitprecise.width));
                    break;

                default:
                    // Nothing to do
                    break;
            }
            switch (specifier->type_specifier->specifier) {
                case KEFIR_AST_TYPE_SPECIFIER_VOID:
                case KEFIR_AST_TYPE_SPECIFIER_CHAR:
                case KEFIR_AST_TYPE_SPECIFIER_SHORT:
                case KEFIR_AST_TYPE_SPECIFIER_INT:
                case KEFIR_AST_TYPE_SPECIFIER_LONG:
                case KEFIR_AST_TYPE_SPECIFIER_FLOAT:
                case KEFIR_AST_TYPE_SPECIFIER_DOUBLE:
                case KEFIR_AST_TYPE_SPECIFIER_SIGNED:
                case KEFIR_AST_TYPE_SPECIFIER_UNSIGNED:
                case KEFIR_AST_TYPE_SPECIFIER_UNSIGNED_OVERRIDE:
                case KEFIR_AST_TYPE_SPECIFIER_BOOL:
                case KEFIR_AST_TYPE_SPECIFIER_COMPLEX:
                case KEFIR_AST_TYPE_SPECIFIER_IMAGINARY:
                case KEFIR_AST_TYPE_SPECIFIER_DECIMAL32:
                case KEFIR_AST_TYPE_SPECIFIER_DECIMAL64:
                case KEFIR_AST_TYPE_SPECIFIER_DECIMAL128:
                case KEFIR_AST_TYPE_SPECIFIER_FLOAT32:
                case KEFIR_AST_TYPE_SPECIFIER_FLOAT64:
                case KEFIR_AST_TYPE_SPECIFIER_FLOAT80:
                case KEFIR_AST_TYPE_SPECIFIER_FLOAT32X:
                case KEFIR_AST_TYPE_SPECIFIER_FLOAT64X:
                case KEFIR_AST_TYPE_SPECIFIER_DECIMAL64X:
                case KEFIR_AST_TYPE_SPECIFIER_INT128:
                case KEFIR_AST_TYPE_SPECIFIER_AUTO_TYPE:
                    // Intentionally left blank
                    break;

                default:
                    KEFIR_FREE(mem, (void *) specifier->type_specifier);
                    break;
            }
            break;

        case KEFIR_AST_ALIGNMENT_SPECIFIER:
            REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, specifier->alignment_specifier));
            specifier->alignment_specifier = NULL;
            break;

        case KEFIR_AST_TYPE_QUALIFIER:
        case KEFIR_AST_STORAGE_CLASS_SPECIFIER:
        case KEFIR_AST_FUNCTION_SPECIFIER:
            // Nothing to do
            break;
    }

    KEFIR_FREE(mem, specifier);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_type_qualifier_list_init(struct kefir_ast_type_qualifier_list *list) {
    REQUIRE(list != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type qualifier list"));

    list->qualifiers = NULL;
    list->qualifiers_length = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_type_qualifier_list_free(struct kefir_mem *mem, struct kefir_ast_type_qualifier_list *list) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(list != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type qualifier list"));

    KEFIR_FREE(mem, list->qualifiers);
    list->qualifiers = NULL;
    list->qualifiers_length = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_type_qualifier_list_append(struct kefir_mem *mem, struct kefir_ast_type_qualifier_list *list,
                                                    kefir_ast_type_qualifier_type_t qualifier) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(list != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type qualifier list"));

    kefir_size_t new_length = list->qualifiers_length + 1;
    kefir_ast_type_qualifier_type_t *new_qualifiers = KEFIR_REALLOC(mem, list->qualifiers, sizeof(kefir_ast_type_qualifier_type_t) * new_length);
    REQUIRE(new_qualifiers != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST type qualifier list"));
    new_qualifiers[list->qualifiers_length] = qualifier;

    list->qualifiers = new_qualifiers;
    list->qualifiers_length = new_length;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_type_qualifier_list_iter(const struct kefir_ast_type_qualifier_list *list, struct kefir_ast_type_qualifier_list_iterator *iter,
                                                            kefir_ast_type_qualifier_type_t *value) {
    REQUIRE(list != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type qualifier list"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST type qualifier list iterator"));

    iter->list = list;
    iter->index = 0;
    REQUIRE(iter->index < iter->list->qualifiers_length, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of AST type qualifier list iterator"));
    ASSIGN_PTR(value, (kefir_ast_type_qualifier_type_t) iter->list->qualifiers[iter->index]);
    iter->index++;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_type_qualifier_list_next(struct kefir_ast_type_qualifier_list_iterator *iter,
                                                            kefir_ast_type_qualifier_type_t *value) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type qualifier list iterator"));

    REQUIRE(iter->index < iter->list->qualifiers_length, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of AST type qualifier list iterator"));
    ASSIGN_PTR(value, (kefir_ast_type_qualifier_type_t) iter->list->qualifiers[iter->index]);
    iter->index++;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_type_qualifier_list_move(struct kefir_mem *mem, struct kefir_ast_type_qualifier_list *dst,
                                                   struct kefir_ast_type_qualifier_list *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dst != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination AST type qualifier list"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source AST type qualifier list"));

    kefir_result_t res;
    struct kefir_ast_type_qualifier_list_iterator iter;
    kefir_ast_type_qualifier_type_t value;
    for (res = kefir_ast_type_qualifier_list_iter(src, &iter, &value); res == KEFIR_OK;
         res = kefir_ast_type_qualifier_list_next(&iter, &value)) {
        REQUIRE_OK(kefir_ast_type_qualifier_list_append(mem, dst, value));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    KEFIR_FREE(mem, src->qualifiers);
    src->qualifiers = NULL;
    src->qualifiers_length = 0;
    return KEFIR_OK;
}
