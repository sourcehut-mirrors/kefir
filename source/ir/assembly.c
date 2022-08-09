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

static kefir_result_t free_inline_asm_parameter(struct kefir_mem *mem, struct kefir_list *list,
                                                struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));
    ASSIGN_DECL_CAST(struct kefir_ir_inline_assembly_parameter *, param, entry->value);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly parameter"));

    kefir_list_free(mem, &param->identifiers);
    KEFIR_FREE(mem, param);
    return KEFIR_OK;
}

static kefir_result_t free_inline_asm_jump_target(struct kefir_mem *mem, struct kefir_list *list,
                                                  struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));
    ASSIGN_DECL_CAST(struct kefir_ir_inline_assembly_jump_target *, jump_target, entry->value);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(jump_target != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly jump target"));

    REQUIRE_OK(kefir_list_free(mem, &jump_target->identifiers));
    jump_target->target_function = NULL;
    jump_target->target = 0;
    jump_target->uid = 0;
    KEFIR_FREE(mem, jump_target);
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
    inline_asm->next_jump_target_id = 0;

    kefir_result_t res = kefir_hashtree_init(&inline_asm->parameters, &kefir_hashtree_str_ops);
    REQUIRE_CHAIN(&res, kefir_list_init(&inline_asm->parameter_list));
    REQUIRE_CHAIN(&res, kefir_list_on_remove(&inline_asm->parameter_list, free_inline_asm_parameter, NULL));
    REQUIRE_CHAIN(&res, kefir_hashtree_init(&inline_asm->clobbers, &kefir_hashtree_str_ops));
    REQUIRE_CHAIN(&res, kefir_hashtree_init(&inline_asm->jump_targets, &kefir_hashtree_str_ops));
    REQUIRE_CHAIN(&res, kefir_list_init(&inline_asm->jump_target_list));
    REQUIRE_CHAIN(&res, kefir_list_on_remove(&inline_asm->jump_target_list, free_inline_asm_jump_target, NULL));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, inline_asm);
        return NULL;
    });
    return inline_asm;
}

kefir_result_t kefir_ir_inline_assembly_free(struct kefir_mem *mem, struct kefir_ir_inline_assembly *inline_asm) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly"));

    REQUIRE_OK(kefir_hashtree_free(mem, &inline_asm->jump_targets));
    REQUIRE_OK(kefir_hashtree_free(mem, &inline_asm->clobbers));
    REQUIRE_OK(kefir_hashtree_free(mem, &inline_asm->parameters));
    REQUIRE_OK(kefir_list_free(mem, &inline_asm->jump_target_list));
    REQUIRE_OK(kefir_list_free(mem, &inline_asm->parameter_list));
    inline_asm->template = NULL;
    inline_asm->id = 0;
    KEFIR_FREE(mem, inline_asm);
    return KEFIR_OK;
}

kefir_result_t kefir_ir_inline_assembly_add_parameter(
    struct kefir_mem *mem, struct kefir_symbol_table *symbols, struct kefir_ir_inline_assembly *inline_asm,
    const char *identifier, kefir_ir_inline_assembly_parameter_class_t param_class,
    kefir_ir_inline_assembly_parameter_constraint_t constraint, const struct kefir_ir_type *param_type,
    kefir_size_t param_type_idx, kefir_int64_t value, struct kefir_ir_inline_assembly_parameter **param_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly"));
    REQUIRE(param_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parameter IR type"));
    REQUIRE(identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly parameter identifier"));

    REQUIRE(param_class != KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Cannot directly add read-store IR inline assembly parameter"));

    if (symbols != NULL) {
        identifier = kefir_symbol_table_insert(mem, symbols, identifier, NULL);
        REQUIRE(identifier != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                "Failed to insert IR inline assembly parameter identifier into symbol table"));
    }

    struct kefir_ir_inline_assembly_parameter *param =
        KEFIR_MALLOC(mem, sizeof(struct kefir_ir_inline_assembly_parameter));
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR inline assembly parameter"));

    param->parameter_id = kefir_list_length(&inline_asm->parameter_list);
    param->klass = param_class;
    param->constraint = constraint;
    param->type.type = param_type;
    param->type.index = param_type_idx;
    switch (param_class) {
        case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ:
            param->read_index = value;
            param->read_type.type = param_type;
            param->read_type.index = param_type_idx;
            break;

        case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD:
        case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE:
        case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE:
            param->load_store_index = value;
            break;

        case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE:
            param->immediate_value = value;
            break;

        case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE:
            // Intentionally left blank
            break;
    }

    kefir_result_t res = kefir_list_init(&param->identifiers);
    REQUIRE_CHAIN(&res, kefir_list_insert_after(mem, &param->identifiers, kefir_list_tail(&param->identifiers),
                                                (void *) identifier));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, param);
        return res;
    });

    res = kefir_list_insert_after(mem, &inline_asm->parameter_list, kefir_list_tail(&inline_asm->parameter_list),
                                  (void *) param);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &param->identifiers);
        KEFIR_FREE(mem, param);
        return res;
    });

    res = kefir_hashtree_insert(mem, &inline_asm->parameters, (kefir_hashtree_key_t) identifier,
                                (kefir_hashtree_value_t) param);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_pop(mem, &inline_asm->parameter_list, kefir_list_tail(&inline_asm->parameter_list));
        return res;
    });

    ASSIGN_PTR(param_ptr, param);
    return KEFIR_OK;
}

kefir_result_t kefir_ir_inline_assembly_add_parameter_alias(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                                            struct kefir_ir_inline_assembly *inline_asm,
                                                            struct kefir_ir_inline_assembly_parameter *param,
                                                            const char *alias) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly"));
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly parameter"));
    REQUIRE(alias != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly parameter alias"));

    REQUIRE(!kefir_hashtree_has(&inline_asm->parameters, (kefir_hashtree_key_t) alias),
            KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Provided IR inline assembly parameter alias already exists"));

    if (symbols != NULL) {
        alias = kefir_symbol_table_insert(mem, symbols, alias, NULL);
        REQUIRE(alias != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                "Failed to insert IR inline assembly parameter alias into symbol table"));
    }

    REQUIRE_OK(kefir_list_insert_after(mem, &param->identifiers, kefir_list_tail(&param->identifiers), (void *) alias));
    REQUIRE_OK(kefir_hashtree_insert(mem, &inline_asm->parameters, (kefir_hashtree_key_t) alias,
                                     (kefir_hashtree_value_t) param));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_inline_assembly_parameter_read_from(struct kefir_mem *mem,
                                                            struct kefir_ir_inline_assembly *inline_asm,
                                                            struct kefir_ir_inline_assembly_parameter *param,
                                                            const struct kefir_ir_type *type, kefir_size_t type_idx,
                                                            kefir_size_t read_index) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly"));
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly parameter"));
    REQUIRE(param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE &&
                param->constraint == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot change IR inline assembly parameter read source"));

    param->read_index = read_index;
    param->read_type.type = type;
    param->read_type.index = type_idx;
    param->klass = KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_inline_assembly_resolve_parameter(struct kefir_ir_inline_assembly *inline_asm,
                                                          const char *param_id,
                                                          struct kefir_ir_inline_assembly_parameter **param_ptr) {
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly"));
    REQUIRE(param_id != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly parameter identifier"));
    REQUIRE(param_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR inline assembly parameter"));

    struct kefir_hashtree_node *node;
    REQUIRE_OK(kefir_hashtree_at(&inline_asm->parameters, (kefir_hashtree_key_t) param_id, &node));
    *param_ptr = (struct kefir_ir_inline_assembly_parameter *) node->value;
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

kefir_result_t kefir_ir_inline_assembly_add_jump_target(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                                        struct kefir_ir_inline_assembly *inline_asm,
                                                        const char *identifier, const char *target_function,
                                                        kefir_size_t target,
                                                        struct kefir_ir_inline_assembly_jump_target **jump_target_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly"));
    REQUIRE(identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly jump target identifier"));
    REQUIRE(target_function != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly jump target target function"));

    if (symbols != NULL) {
        identifier = kefir_symbol_table_insert(mem, symbols, identifier, NULL);
        REQUIRE(identifier != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                "Failed to insert IR inline assembly jump target identifier into symbol table"));

        target_function = kefir_symbol_table_insert(mem, symbols, target_function, NULL);
        REQUIRE(identifier != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                "Failed to insert IR inline assembly jump target function into symbol table"));
    }

    struct kefir_ir_inline_assembly_jump_target *jump_target =
        KEFIR_MALLOC(mem, sizeof(struct kefir_ir_inline_assembly_jump_target));

    jump_target->uid = inline_asm->next_jump_target_id++;
    jump_target->target_function = target_function;
    jump_target->target = target;

    kefir_result_t res = kefir_list_init(&jump_target->identifiers);
    REQUIRE_CHAIN(&res, kefir_list_insert_after(mem, &jump_target->identifiers,
                                                kefir_list_tail(&jump_target->identifiers), (void *) identifier));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, jump_target);
        return res;
    });

    res = kefir_list_insert_after(mem, &inline_asm->jump_target_list, kefir_list_tail(&inline_asm->jump_target_list),
                                  (void *) jump_target);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &jump_target->identifiers);
        KEFIR_FREE(mem, jump_target);
        return res;
    });

    res = kefir_hashtree_insert(mem, &inline_asm->jump_targets, (kefir_hashtree_key_t) identifier,
                                (kefir_hashtree_value_t) jump_target);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_pop(mem, &inline_asm->jump_target_list, kefir_list_tail(&inline_asm->jump_target_list));
        return res;
    });

    ASSIGN_PTR(jump_target_ptr, jump_target);
    return KEFIR_OK;
}

kefir_result_t kefir_ir_inline_assembly_add_jump_target_alias(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                                              struct kefir_ir_inline_assembly *inline_asm,
                                                              struct kefir_ir_inline_assembly_jump_target *jump_target,
                                                              const char *alias) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly"));
    REQUIRE(jump_target != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly jump target"));
    REQUIRE(alias != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly jump target alias"));

    REQUIRE(!kefir_hashtree_has(&inline_asm->jump_targets, (kefir_hashtree_key_t) alias),
            KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Jump target with specified alias already exists"));

    if (symbols != NULL) {
        alias = kefir_symbol_table_insert(mem, symbols, alias, NULL);
        REQUIRE(alias != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                "Failed to insert IR inline assembly jump target alias into symbol table"));
    }

    REQUIRE_OK(kefir_list_insert_after(mem, &jump_target->identifiers, kefir_list_tail(&jump_target->identifiers),
                                       (void *) alias));
    REQUIRE_OK(kefir_hashtree_insert(mem, &inline_asm->jump_targets, (kefir_hashtree_key_t) alias,
                                     (kefir_hashtree_value_t) jump_target));
    return KEFIR_OK;
}
