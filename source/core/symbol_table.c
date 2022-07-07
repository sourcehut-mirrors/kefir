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

#include <string.h>
#include "kefir/core/symbol_table.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

static kefir_result_t destroy_string(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                     kefir_hashtree_value_t value, void *data) {
    UNUSED(tree);
    UNUSED(value);
    UNUSED(data);
    KEFIR_FREE(mem, (char *) key);
    return KEFIR_OK;
}

kefir_result_t kefir_symbol_table_init(struct kefir_symbol_table *table) {
    REQUIRE(table != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid symbol table pointer"));
    REQUIRE_OK(kefir_hashtree_init(&table->symbols, &kefir_hashtree_str_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&table->symbols, destroy_string, NULL));
    REQUIRE_OK(kefir_hashtree_init(&table->symbol_identifiers, &kefir_hashtree_uint_ops));
    table->next_id = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_symbol_table_free(struct kefir_mem *mem, struct kefir_symbol_table *table) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(table != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid symbol table pointer"));
    REQUIRE_OK(kefir_hashtree_free(mem, &table->symbol_identifiers));
    REQUIRE_OK(kefir_hashtree_free(mem, &table->symbols));
    return KEFIR_OK;
}

const char *kefir_symbol_table_insert(struct kefir_mem *mem, struct kefir_symbol_table *table, const char *symbol,
                                      kefir_id_t *id) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(table != NULL, NULL);
    REQUIRE(symbol != NULL, NULL);

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&table->symbols, (kefir_hashtree_key_t) symbol, &node);
    if (res == KEFIR_OK) {
        ASSIGN_PTR(id, (kefir_id_t) node->value);
        return (const char *) node->key;
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, NULL);
        char *symbol_copy = KEFIR_MALLOC(mem, strlen(symbol) + 1);
        REQUIRE(symbol_copy != NULL, NULL);
        strcpy(symbol_copy, symbol);
        kefir_id_t symbol_id = table->next_id;
        res = kefir_hashtree_insert(mem, &table->symbols, (kefir_hashtree_key_t) symbol_copy,
                                    (kefir_hashtree_value_t) symbol_id);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, symbol_copy);
            return NULL;
        });
        res = kefir_hashtree_insert(mem, &table->symbol_identifiers, (kefir_hashtree_key_t) symbol_id,
                                    (kefir_hashtree_value_t) symbol_copy);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_hashtree_delete(mem, &table->symbols, (kefir_hashtree_key_t) symbol_copy);
            return NULL;
        });
        table->next_id++;
        ASSIGN_PTR(id, symbol_id);
        return symbol_copy;
    }
}

const char *kefir_symbol_table_get(const struct kefir_symbol_table *table, kefir_id_t id) {
    REQUIRE(table != NULL, NULL);
    struct kefir_hashtree_node *node = NULL;
    REQUIRE_ELSE(kefir_hashtree_at(&table->symbol_identifiers, (kefir_hashtree_key_t) id, &node) == KEFIR_OK,
                 { return NULL; });
    return (const char *) node->value;
}
