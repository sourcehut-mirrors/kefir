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

#include <string.h>
#include "kefir/core/string_pool.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

static kefir_result_t destroy_string(struct kefir_mem *mem, struct kefir_hashtable *table, kefir_hashtable_key_t key,
                                     kefir_hashtable_value_t value, void *data) {
    UNUSED(table);
    UNUSED(value);
    UNUSED(data);
    KEFIR_FREE(mem, (char *) key);
    return KEFIR_OK;
}

kefir_result_t kefir_string_pool_init(struct kefir_string_pool *table) {
    REQUIRE(table != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string table pointer"));
    REQUIRE_OK(kefir_hashtable_init(&table->strings, &kefir_hashtable_str_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&table->strings, destroy_string, NULL));
    REQUIRE_OK(kefir_hashtable_init(&table->named_strings, &kefir_hashtable_uint_ops));
    table->next_id = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_string_pool_free(struct kefir_mem *mem, struct kefir_string_pool *table) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(table != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string table pointer"));
    REQUIRE_OK(kefir_hashtable_free(mem, &table->named_strings));
    REQUIRE_OK(kefir_hashtable_free(mem, &table->strings));
    return KEFIR_OK;
}

const char *kefir_string_pool_insert(struct kefir_mem *mem, struct kefir_string_pool *table, const char *string,
                                     kefir_id_t *id) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(table != NULL, NULL);
    REQUIRE(string != NULL, NULL);

    kefir_hashtable_key_t table_key;
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at2(&table->strings, (kefir_hashtable_key_t) string, &table_key, &table_value);
    if (res == KEFIR_OK) {
        ASSIGN_PTR(id, (kefir_id_t) table_value);
        return (const char *) table_key;
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, NULL);
        char *string_copy = KEFIR_MALLOC(mem, strlen(string) + 1);
        REQUIRE(string_copy != NULL, NULL);
        strcpy(string_copy, string);
        kefir_id_t string_id = table->next_id;
        res = kefir_hashtable_insert(mem, &table->strings, (kefir_hashtable_key_t) string_copy,
                                     (kefir_hashtable_value_t) string_id);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, string_copy);
            return NULL;
        });
        res = kefir_hashtable_insert(mem, &table->named_strings, (kefir_hashtable_key_t) string_id,
                                     (kefir_hashtable_value_t) string_copy);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_hashtable_delete(mem, &table->strings, (kefir_hashtable_key_t) string_copy);
            return NULL;
        });
        table->next_id++;
        ASSIGN_PTR(id, string_id);
        return string_copy;
    }
}

const char *kefir_string_pool_get(const struct kefir_string_pool *table, kefir_id_t id) {
    REQUIRE(table != NULL, NULL);
    kefir_hashtable_value_t table_value;
    REQUIRE_ELSE(kefir_hashtable_at(&table->named_strings, (kefir_hashtable_key_t) id, &table_value) == KEFIR_OK,
                 { return NULL; });
    return (const char *) table_value;
}
