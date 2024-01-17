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

#include "kefir/ast/declarator.h"
#include "kefir/ast/analyzer/declarator.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/ast/global_context.h"
#include "kefir/ast/type.h"
#include "kefir/ast/downcast.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/ast/constant_expression.h"
#include "kefir/ast/function_declaration_context.h"
#include "kefir/ast/type_completion.h"
#include "kefir/core/source_error.h"

enum signedness { SIGNEDNESS_DEFAULT, SIGNEDNESS_SIGNED, SIGNEDNESS_UNSIGNED };

enum real_class { REAL_SCALAR, REAL_COMPLEX, REAL_COMPLEX_LONG };

static kefir_result_t analyze_declaration_specifiers_impl(struct kefir_mem *, const struct kefir_ast_context *,
                                                          const struct kefir_ast_declarator_specifier_list *,
                                                          const struct kefir_ast_type **,
                                                          kefir_ast_scoped_identifier_storage_t *,
                                                          kefir_ast_function_specifier_t *, kefir_size_t *,
                                                          kefir_uint64_t, const struct kefir_source_location *);

static struct kefir_ast_alignment *wrap_alignment(struct kefir_mem *mem, kefir_size_t alignment) {
    if (alignment > 0) {
        return kefir_ast_alignment_const_expression(mem, kefir_ast_constant_expression_integer(mem, alignment));
    } else {
        return NULL;
    }
}

static kefir_result_t process_struct_declaration_entry(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                       struct kefir_ast_struct_type *struct_type,
                                                       struct kefir_ast_structure_declaration_entry *entry,
                                                       kefir_uint64_t flags,
                                                       const struct kefir_source_location *source_location) {
    const struct kefir_ast_type *base_field_type = NULL;
    kefir_ast_scoped_identifier_storage_t storage_class = KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN;
    kefir_size_t alignment = 0;
    REQUIRE_OK(analyze_declaration_specifiers_impl(
        mem, context, &entry->declaration.specifiers, &base_field_type, &storage_class, NULL, &alignment,
        (flags & (~KEFIR_AST_DECLARATION_ANALYSIS_IGNORE_ALIGNMENT_SPECIFIER)) |
            KEFIR_AST_DECLARATION_ANALYSIS_FORBID_ALIGNMENT_DECREASE,
        source_location));

    for (const struct kefir_list_entry *iter = kefir_list_head(&entry->declaration.declarators); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_structure_entry_declarator *, entry_declarator, iter->value);

        const struct kefir_ast_type *field_type = base_field_type;
        const char *identifier = NULL;
        REQUIRE_OK(kefir_ast_analyze_declaration_declarator(
            mem, context, entry_declarator->declarator, &identifier, &field_type, &alignment,
            KEFIR_AST_DECLARATION_ANALYSIS_FORBID_ALIGNMENT_DECREASE, NULL));
        REQUIRE(storage_class == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &entry_declarator->declarator->source_location,
                                       "Structure/union field cannot have storage class specifiers"));
        REQUIRE(!kefir_ast_type_is_variably_modified(field_type),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &entry_declarator->declarator->source_location,
                                       "Structure/union field cannot have variably-modified type"));

        if (entry_declarator->bitwidth == NULL) {
            struct kefir_ast_alignment *ast_alignment = wrap_alignment(mem, alignment);
            REQUIRE(alignment == 0 || ast_alignment != NULL,
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST alignment"));
            kefir_result_t res =
                kefir_ast_struct_type_field(mem, context->symbols, struct_type, identifier, field_type, ast_alignment);
            REQUIRE_ELSE(res == KEFIR_OK, {
                if (ast_alignment != NULL) {
                    kefir_ast_alignment_free(mem, ast_alignment);
                }
                return res;
            });
        } else {
            struct kefir_ast_constant_expression_value value;
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, entry_declarator->bitwidth));
            REQUIRE_OK(kefir_ast_constant_expression_value_evaluate(mem, context, entry_declarator->bitwidth, &value));
            REQUIRE(value.klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &entry_declarator->bitwidth->source_location,
                                           "Bit-field width shall be an integral constant expression"));

            kefir_ast_target_environment_opaque_type_t target_type;
            struct kefir_ast_target_environment_object_info target_obj_info;
            REQUIRE_OK(context->target_env->get_type(mem, context, context->target_env,
                                                     kefir_ast_unqualified_type(field_type), &target_type,
                                                     &entry_declarator->declarator->source_location));
            kefir_result_t res =
                context->target_env->object_info(mem, context->target_env, target_type, NULL, &target_obj_info);
            REQUIRE_ELSE(res == KEFIR_OK, {
                context->target_env->free_type(mem, context->target_env, target_type);
                return res;
            });
            REQUIRE_ELSE(target_obj_info.max_bitfield_width > 0, {
                context->target_env->free_type(mem, context->target_env, target_type);
                return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &entry_declarator->declarator->source_location,
                                              "Bit-field has invalid type");
            });
            REQUIRE_ELSE((kefir_size_t) value.integer <= target_obj_info.max_bitfield_width, {
                context->target_env->free_type(mem, context->target_env, target_type);
                return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &entry_declarator->declarator->source_location,
                                              "Bit-field width exceeds undelying type");
            });
            REQUIRE_OK(context->target_env->free_type(mem, context->target_env, target_type));

            struct kefir_ast_alignment *ast_alignment = wrap_alignment(mem, alignment);
            REQUIRE(alignment == 0 || ast_alignment != NULL,
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST alignment"));
            struct kefir_ast_constant_expression *ast_bitwidth =
                kefir_ast_constant_expression_integer(mem, value.integer);
            REQUIRE_ELSE(ast_bitwidth != NULL, {
                if (ast_alignment != NULL) {
                    kefir_ast_alignment_free(mem, ast_alignment);
                }
                return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST constant expression");
            });
            res = kefir_ast_struct_type_bitfield(mem, context->symbols, struct_type, identifier, field_type,
                                                 ast_alignment, ast_bitwidth);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_ast_constant_expression_free(mem, ast_bitwidth);
                if (ast_alignment != NULL) {
                    kefir_ast_alignment_free(mem, ast_alignment);
                }
                return res;
            });
        }
    }
    if (kefir_list_head(&entry->declaration.declarators) == NULL) {
        const struct kefir_ast_type *field_type = base_field_type;

        const struct kefir_source_location *source_location =
            kefir_ast_declarator_specifier_list_source_location(&entry->declaration.specifiers);
        REQUIRE(storage_class == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, source_location,
                                       "Structure/union field cannot have storage class specified"));
        REQUIRE(field_type->tag == KEFIR_AST_TYPE_STRUCTURE || field_type->tag == KEFIR_AST_TYPE_UNION,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, source_location,
                                       "Anonymous member shall be a structure/union"));
        struct kefir_ast_alignment *ast_alignment = wrap_alignment(mem, alignment);
        REQUIRE(alignment == 0 || ast_alignment != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST alignment"));
        kefir_result_t res =
            kefir_ast_struct_type_field(mem, context->symbols, struct_type, NULL, field_type, ast_alignment);
        REQUIRE_ELSE(res == KEFIR_OK, {
            if (ast_alignment != NULL) {
                kefir_ast_alignment_free(mem, ast_alignment);
            }
            return res;
        });
    }
    return KEFIR_OK;
}

static kefir_result_t resolve_struct_type(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                          const struct kefir_ast_declarator_specifier *decl_specifier,
                                          const struct kefir_ast_type **base_type, kefir_uint64_t flags,
                                          const struct kefir_source_location *source_location) {
    if ((flags & KEFIR_AST_DECLARATION_ANALYSIS_FUNCTION_DEFINITION_CONTEXT) != 0) {
        context = &context->global_context->context;
    }

    const struct kefir_ast_structure_specifier *specifier = decl_specifier->type_specifier.value.structure;
    kefir_bool_t resolved = false;
    const struct kefir_ast_type *type = NULL;
    if (specifier->complete) {
        struct kefir_ast_struct_type *struct_type = NULL;
        type = decl_specifier->type_specifier.specifier == KEFIR_AST_TYPE_SPECIFIER_STRUCT
                   ? kefir_ast_type_structure(mem, context->type_bundle, specifier->identifier, &struct_type)
                   : kefir_ast_type_union(mem, context->type_bundle, specifier->identifier, &struct_type);
        REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Unable to allocate AST struct/union type"));

        for (const struct kefir_list_entry *iter = kefir_list_head(&specifier->entries); iter != NULL;
             kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(struct kefir_ast_structure_declaration_entry *, entry, iter->value);
            if (entry->is_static_assertion) {
                REQUIRE_OK(kefir_ast_analyze_node(mem, context, KEFIR_AST_NODE_BASE(entry->static_assertion)));
            } else {
                REQUIRE_OK(process_struct_declaration_entry(mem, context, struct_type, entry, flags, source_location));
            }
        }
    } else {
        if (specifier->identifier != NULL) {
            const struct kefir_ast_scoped_identifier *scoped_identifier = NULL;
            kefir_result_t res = context->resolve_tag_identifier(context, specifier->identifier, &scoped_identifier);
            if (res == KEFIR_OK) {
                REQUIRE((decl_specifier->type_specifier.specifier == KEFIR_AST_TYPE_SPECIFIER_STRUCT &&
                         scoped_identifier->type->tag == KEFIR_AST_TYPE_STRUCTURE) ||
                            (decl_specifier->type_specifier.specifier == KEFIR_AST_TYPE_SPECIFIER_UNION &&
                             scoped_identifier->type->tag == KEFIR_AST_TYPE_UNION),
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                               "Tagged type declaration mismatch"));
                type = scoped_identifier->type;
                resolved = true;
            } else {
                REQUIRE(res == KEFIR_NOT_FOUND, res);
            }
        }

        if (!resolved) {
            type = decl_specifier->type_specifier.specifier == KEFIR_AST_TYPE_SPECIFIER_STRUCT
                       ? kefir_ast_type_incomplete_structure(mem, context->type_bundle, specifier->identifier)
                       : kefir_ast_type_incomplete_union(mem, context->type_bundle, specifier->identifier);
            REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Unable to allocate AST struct/union type"));
        }
    }

    if (specifier->identifier != NULL && !resolved) {
        REQUIRE_OK(context->define_tag(mem, context, type, &decl_specifier->source_location));
    }
    ASSIGN_PTR(base_type, type);
    return KEFIR_OK;
}

static kefir_result_t resolve_enum_type(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                        const struct kefir_ast_declarator_specifier *decl_specifier,
                                        const struct kefir_ast_type **base_type, kefir_uint64_t flags) {
    if ((flags & KEFIR_AST_DECLARATION_ANALYSIS_FUNCTION_DEFINITION_CONTEXT) != 0) {
        context = &context->global_context->context;
    }

    const struct kefir_ast_enum_specifier *specifier = decl_specifier->type_specifier.value.enumeration;
    kefir_bool_t resolved = false;
    const struct kefir_ast_type *type = NULL;
    if (specifier->complete) {
        struct kefir_ast_enum_type *enum_type = NULL;
        type = kefir_ast_type_enumeration(mem, context->type_bundle, specifier->identifier,
                                          context->type_traits->underlying_enumeration_type, &enum_type);
        REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Unable to allocate AST enum type"));

        kefir_ast_constant_expression_int_t constant_value = 0;
        kefir_ast_constant_expression_int_t min_constant = KEFIR_AST_CONSTANT_EXPRESSION_INT_MAX;
        kefir_ast_constant_expression_int_t max_constant = KEFIR_AST_CONSTANT_EXPRESSION_INT_MIN;
        for (const struct kefir_list_entry *iter = kefir_list_head(&specifier->entries); iter != NULL;
             kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(struct kefir_ast_enum_specifier_entry *, entry, iter->value);

            if (entry->value != NULL) {
                struct kefir_ast_constant_expression_value value;
                REQUIRE_OK(kefir_ast_analyze_node(mem, context, entry->value));
                REQUIRE_OK(kefir_ast_constant_expression_value_evaluate(mem, context, entry->value, &value));
                REQUIRE(value.klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER,
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &entry->value->source_location,
                                               "Enumeration constant value shall be an integer constant expression"));
                constant_value = value.integer;
                REQUIRE_OK(
                    kefir_ast_enumeration_type_constant(mem, context->symbols, enum_type, entry->constant,
                                                        kefir_ast_constant_expression_integer(mem, constant_value)));
            } else {
                REQUIRE_OK(kefir_ast_enumeration_type_constant_auto(mem, context->symbols, enum_type, entry->constant));
            }

            min_constant = MIN(min_constant, constant_value);
            max_constant = MAX(max_constant, constant_value);
            REQUIRE_OK(context->define_constant(
                mem, context, entry->constant, kefir_ast_constant_expression_integer(mem, constant_value++),
                context->type_traits->underlying_enumeration_type, &decl_specifier->source_location));
        }

        if (!context->configuration->analysis.fixed_enum_type && kefir_list_length(&specifier->entries) > 0) {
            if (min_constant >= 0) {
                enum_type->underlying_type = kefir_ast_type_unsigned_int();
            } else {
                enum_type->underlying_type = kefir_ast_type_signed_int();
            }
        }
    } else {
        if (specifier->identifier != NULL) {
            const struct kefir_ast_scoped_identifier *scoped_identifier = NULL;
            kefir_result_t res = context->resolve_tag_identifier(context, specifier->identifier, &scoped_identifier);
            if (res == KEFIR_OK) {
                REQUIRE(scoped_identifier->type->tag == KEFIR_AST_TYPE_ENUMERATION,
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                               "Tagged type declaration mismatch"));
                type = scoped_identifier->type;
                resolved = true;
            } else {
                REQUIRE(res == KEFIR_NOT_FOUND, res);
            }
        }

        if (!resolved) {
            type = kefir_ast_type_incomplete_enumeration(mem, context->type_bundle, specifier->identifier,
                                                         context->type_traits->underlying_enumeration_type);
            REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Unable to allocate AST enum type"));
        }
    }

    if (specifier->identifier != NULL && !resolved) {
        REQUIRE_OK(context->define_tag(mem, context, type, &decl_specifier->source_location));
    }
    ASSIGN_PTR(base_type, type);
    return KEFIR_OK;
}

static kefir_result_t resolve_typedef(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                      const char *type_name, const struct kefir_source_location *source_location,
                                      const struct kefir_ast_type **base_type, kefir_size_t *alignment) {
    const struct kefir_ast_scoped_identifier *scoped_identifier = NULL;
    REQUIRE_OK(context->resolve_ordinary_identifier(context, type_name, &scoped_identifier));
    REQUIRE(scoped_identifier->klass == KEFIR_AST_SCOPE_IDENTIFIER_TYPE_DEFINITION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, source_location,
                                   "Referenced identifier is not a type definition"));

    REQUIRE_OK(kefir_ast_type_completion(mem, context, base_type, scoped_identifier->type_definition.type));
    if (alignment != NULL && scoped_identifier->type_definition.alignment != NULL) {
        *alignment = MAX(*alignment, scoped_identifier->type_definition.alignment->value);
    }
    return KEFIR_OK;
}

enum type_specifier_sequence_state {
    TYPE_SPECIFIER_SEQUENCE_EMPTY,
    TYPE_SPECIFIER_SEQUENCE_TYPEDEF,
    TYPE_SPECIFIER_SEQUENCE_SPECIFIERS
} type_specifier_sequence_state_t;

static kefir_result_t resolve_type(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                   enum signedness *signedness, enum real_class *real_class,
                                   enum type_specifier_sequence_state *seq_state,
                                   const struct kefir_ast_type **base_type, kefir_size_t *alignment,
                                   const struct kefir_ast_declarator_specifier *decl_specifier, kefir_uint64_t flags) {
    const struct kefir_ast_type_specifier *specifier = &decl_specifier->type_specifier;
    switch (specifier->specifier) {
        case KEFIR_AST_TYPE_SPECIFIER_VOID:
            REQUIRE(*base_type == NULL, KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                                               "Void type specifier cannot be combined with others"));
            REQUIRE(*real_class == REAL_SCALAR,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Void type specifier cannot be combined with complex type specifier"));
            *base_type = kefir_ast_type_void();
            *seq_state = TYPE_SPECIFIER_SEQUENCE_SPECIFIERS;
            break;

        case KEFIR_AST_TYPE_SPECIFIER_CHAR:
            REQUIRE(*base_type == NULL, KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                                               "Char type specifier cannot be combined with others"));
            REQUIRE(*real_class == REAL_SCALAR,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Char type specifier cannot be combined with complex type specifier"));
            *base_type = kefir_ast_type_char();
            *seq_state = TYPE_SPECIFIER_SEQUENCE_SPECIFIERS;
            break;

        case KEFIR_AST_TYPE_SPECIFIER_SHORT:
            REQUIRE(*base_type == NULL || (*base_type)->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_INT,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Short type specifier can only be combined with int"));
            REQUIRE(*seq_state != TYPE_SPECIFIER_SEQUENCE_TYPEDEF,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Cannot combine type specifiers with referenced type definition"));
            REQUIRE(*real_class == REAL_SCALAR,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Short type specifier cannot be combined with complex type specifier"));
            *base_type = kefir_ast_type_signed_short();
            break;

        case KEFIR_AST_TYPE_SPECIFIER_INT:
            REQUIRE(*seq_state != TYPE_SPECIFIER_SEQUENCE_TYPEDEF,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Cannot combine type specifiers with referenced type definition"));
            REQUIRE(*real_class == REAL_SCALAR,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Int type specifier cannot be combined with complex type specifier"));
            if (*base_type == NULL) {
                *base_type = kefir_ast_type_signed_int();
            } else {
                REQUIRE((*base_type)->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT ||
                            (*base_type)->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_LONG ||
                            (*base_type)->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG,
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                               "Int type specifier can only be combined with short or long"));
            }
            *seq_state = TYPE_SPECIFIER_SEQUENCE_SPECIFIERS;
            break;

        case KEFIR_AST_TYPE_SPECIFIER_LONG:
            REQUIRE(*seq_state != TYPE_SPECIFIER_SEQUENCE_TYPEDEF,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Cannot combine type specifiers with referenced type definition"));
            if (*base_type != NULL && (*base_type)->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_LONG) {
                REQUIRE(*real_class == REAL_SCALAR,
                        KEFIR_SET_SOURCE_ERROR(
                            KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                            "Long type specifier cannot be combined with complex type specifier in this context"));
                *base_type = kefir_ast_type_signed_long_long();
            } else if (*base_type != NULL && (*base_type)->tag == KEFIR_AST_TYPE_SCALAR_DOUBLE) {
                *base_type = kefir_ast_type_long_double();
            } else if (*base_type != NULL && (*base_type)->tag == KEFIR_AST_TYPE_COMPLEX_DOUBLE) {
                *base_type = kefir_ast_type_complex_long_double();
            } else if (*base_type == NULL && *real_class == REAL_COMPLEX) {
                *real_class = REAL_COMPLEX_LONG;
            } else {
                REQUIRE(*real_class == REAL_SCALAR,
                        KEFIR_SET_SOURCE_ERROR(
                            KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                            "Long type specifier cannot be combined with complex type specifier in this context"));
                REQUIRE((*base_type) == NULL || (*base_type)->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_INT,
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                               "Long type specifier can only be combined with int or long"));
                *base_type = kefir_ast_type_signed_long();
            }
            *seq_state = TYPE_SPECIFIER_SEQUENCE_SPECIFIERS;
            break;

        case KEFIR_AST_TYPE_SPECIFIER_FLOAT:
            REQUIRE(*base_type == NULL, KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                                               "Float type specifier cannot be combined with others"));
            REQUIRE(*seq_state != TYPE_SPECIFIER_SEQUENCE_TYPEDEF,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Cannot combine type specifiers with referenced type definition"));
            if (*real_class == REAL_SCALAR) {
                *base_type = kefir_ast_type_float();
            } else {
                REQUIRE(*real_class == REAL_COMPLEX,
                        KEFIR_SET_SOURCE_ERROR(
                            KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                            "Long and complex type specifiers cannot be combined with float type specifier"));
                *base_type = kefir_ast_type_complex_float();
            }
            *seq_state = TYPE_SPECIFIER_SEQUENCE_SPECIFIERS;
            break;

        case KEFIR_AST_TYPE_SPECIFIER_DOUBLE:
            REQUIRE(*seq_state != TYPE_SPECIFIER_SEQUENCE_TYPEDEF,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Cannot combine type specifiers with referenced type definition"));
            if ((*base_type) == NULL) {
                switch (*real_class) {
                    case REAL_SCALAR:
                        *base_type = kefir_ast_type_double();
                        break;

                    case REAL_COMPLEX:
                        *base_type = kefir_ast_type_complex_double();
                        break;

                    case REAL_COMPLEX_LONG:
                        *base_type = kefir_ast_type_complex_long_double();
                        break;
                }
            } else {
                REQUIRE((*base_type)->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_LONG,
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                               "Double type specifier can only be combined with complex and long"));
                *base_type = kefir_ast_type_long_double();
            }
            *seq_state = TYPE_SPECIFIER_SEQUENCE_SPECIFIERS;
            break;

        case KEFIR_AST_TYPE_SPECIFIER_SIGNED:
            REQUIRE(*seq_state != TYPE_SPECIFIER_SEQUENCE_TYPEDEF,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Cannot combine type specifiers with referenced type definition"));
            REQUIRE(*real_class == REAL_SCALAR,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Signed type specifier cannot be combined with complex type specifier"));
            REQUIRE(
                *signedness == SIGNEDNESS_DEFAULT,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                       "Signed type specifier cannot be combined with other signedness specifiers"));
            *signedness = SIGNEDNESS_SIGNED;
            *seq_state = TYPE_SPECIFIER_SEQUENCE_SPECIFIERS;
            break;

        case KEFIR_AST_TYPE_SPECIFIER_UNSIGNED:
            REQUIRE(*seq_state != TYPE_SPECIFIER_SEQUENCE_TYPEDEF,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Cannot combine type specifiers with referenced type definition"));
            REQUIRE(*real_class == REAL_SCALAR,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Unsigned type specifier cannot be combined with complex type specifier"));
            REQUIRE(
                *signedness == SIGNEDNESS_DEFAULT,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                       "Unsigned type specifier cannot be combined with other signedness specifiers"));
            *signedness = SIGNEDNESS_UNSIGNED;
            *seq_state = TYPE_SPECIFIER_SEQUENCE_SPECIFIERS;
            break;

        case KEFIR_AST_TYPE_SPECIFIER_BOOL:
            REQUIRE(*seq_state != TYPE_SPECIFIER_SEQUENCE_TYPEDEF,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Cannot combine type specifiers with referenced type definition"));
            REQUIRE(*real_class == REAL_SCALAR,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Bool type specifier cannot be combined with complex type specifier"));
            REQUIRE(*base_type == NULL,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Boolean type specifier cannot be combined with others"));
            *base_type = kefir_ast_type_boolean();
            *seq_state = TYPE_SPECIFIER_SEQUENCE_SPECIFIERS;
            break;

        case KEFIR_AST_TYPE_SPECIFIER_COMPLEX:
            REQUIRE(*seq_state != TYPE_SPECIFIER_SEQUENCE_TYPEDEF,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Cannot combine type specifiers with referenced type definition"));
            REQUIRE(
                *signedness == SIGNEDNESS_DEFAULT,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                       "Complex type specifier cannot be combined with other signedness specifiers"));
            REQUIRE(*real_class == REAL_SCALAR,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Duplicate complex type specifier"));

            if (*base_type == NULL) {
                *real_class = REAL_COMPLEX;
            } else if ((*base_type)->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_LONG) {
                *real_class = REAL_COMPLEX_LONG;
                *base_type = NULL;
            } else {
                REQUIRE(
                    KEFIR_AST_TYPE_IS_REAL_FLOATING_POINT(*base_type),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Complex type specified cannot be combined with non-floating-point type"));
                *real_class = REAL_COMPLEX;
                *base_type = kefir_ast_type_corresponding_complex_type(*base_type);
                REQUIRE(*base_type != NULL,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain corresponding complex type"));
            }
            break;

        case KEFIR_AST_TYPE_SPECIFIER_ATOMIC:
            return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Atomic type specifier is not supported yet");

        case KEFIR_AST_TYPE_SPECIFIER_STRUCT:
        case KEFIR_AST_TYPE_SPECIFIER_UNION:
            REQUIRE(*seq_state == TYPE_SPECIFIER_SEQUENCE_EMPTY,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Cannot combine struct/union type specifier with others"));
            REQUIRE(
                *real_class == REAL_SCALAR,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                       "Struct/union type specifier cannot be combined with complex type specifier"));
            REQUIRE_OK(
                resolve_struct_type(mem, context, decl_specifier, base_type, flags, &decl_specifier->source_location));
            *seq_state = TYPE_SPECIFIER_SEQUENCE_SPECIFIERS;
            break;

        case KEFIR_AST_TYPE_SPECIFIER_ENUM:
            REQUIRE(*seq_state == TYPE_SPECIFIER_SEQUENCE_EMPTY,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Cannot combine enum type specifier with others"));
            REQUIRE(*real_class == REAL_SCALAR,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Enum type specifier cannot be combined with complex type specifier"));
            REQUIRE_OK(resolve_enum_type(mem, context, decl_specifier, base_type, flags));
            *seq_state = TYPE_SPECIFIER_SEQUENCE_SPECIFIERS;
            break;

        case KEFIR_AST_TYPE_SPECIFIER_TYPEDEF:
            REQUIRE(*seq_state == TYPE_SPECIFIER_SEQUENCE_EMPTY,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Cannot combine referenced type definition with others"));
            REQUIRE(
                *real_class == REAL_SCALAR,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                       "Referenced type definition cannot be combined with complex type specifier"));
            REQUIRE_OK(resolve_typedef(mem, context, specifier->value.type_name, &decl_specifier->source_location,
                                       base_type, alignment));
            *seq_state = TYPE_SPECIFIER_SEQUENCE_TYPEDEF;
            break;

        case KEFIR_AST_TYPE_SPECIFIER_VA_LIST:
            REQUIRE(*seq_state == TYPE_SPECIFIER_SEQUENCE_EMPTY,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Cannot combine va_list type specifier with others"));
            REQUIRE(*base_type == NULL,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "va_list type specifier cannot be combined with others"));
            REQUIRE(*real_class == REAL_SCALAR,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "va_list type specifier cannot be combined with complex type specifier"));
            *base_type = kefir_ast_type_va_list();
            *seq_state = TYPE_SPECIFIER_SEQUENCE_SPECIFIERS;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected type specifier");
    }
    return KEFIR_OK;
}

static kefir_result_t apply_type_signedness(struct kefir_mem *mem, struct kefir_ast_type_bundle *type_bundle,
                                            enum signedness signedness,
                                            const struct kefir_source_location *source_location,
                                            const struct kefir_ast_type **base_type) {
    UNUSED(mem);
    UNUSED(type_bundle);
    if (signedness == SIGNEDNESS_DEFAULT) {
        if ((*base_type) == NULL) {
            (*base_type) = kefir_ast_type_signed_int();
        }
    } else if (signedness == SIGNEDNESS_SIGNED) {
        if ((*base_type) == NULL) {
            (*base_type) = kefir_ast_type_signed_int();
        } else {
            switch ((*base_type)->tag) {
                case KEFIR_AST_TYPE_SCALAR_CHAR:
                    (*base_type) = kefir_ast_type_signed_char();
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
                    // Nothing to be done
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
                    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected AST type");

                case KEFIR_AST_TYPE_VOID:
                case KEFIR_AST_TYPE_SCALAR_BOOL:
                case KEFIR_AST_TYPE_SCALAR_FLOAT:
                case KEFIR_AST_TYPE_SCALAR_DOUBLE:
                case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
                case KEFIR_AST_TYPE_COMPLEX_FLOAT:
                case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
                case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
                case KEFIR_AST_TYPE_SCALAR_POINTER:
                case KEFIR_AST_TYPE_ENUMERATION:
                case KEFIR_AST_TYPE_STRUCTURE:
                case KEFIR_AST_TYPE_UNION:
                case KEFIR_AST_TYPE_ARRAY:
                case KEFIR_AST_TYPE_FUNCTION:
                case KEFIR_AST_TYPE_QUALIFIED:
                case KEFIR_AST_TYPE_VA_LIST:
                    return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, source_location,
                                                  "Signed type specifier cannot be applied to the type");
            }
        }
    } else {
        if ((*base_type) == NULL) {
            (*base_type) = kefir_ast_type_unsigned_int();
        } else {
            switch ((*base_type)->tag) {
                case KEFIR_AST_TYPE_SCALAR_CHAR:
                    (*base_type) = kefir_ast_type_unsigned_char();
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
                    (*base_type) = kefir_ast_type_unsigned_short();
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
                    (*base_type) = kefir_ast_type_unsigned_int();
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
                    (*base_type) = kefir_ast_type_unsigned_long();
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
                    (*base_type) = kefir_ast_type_unsigned_long_long();
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
                    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected AST type");

                case KEFIR_AST_TYPE_VOID:
                case KEFIR_AST_TYPE_SCALAR_BOOL:
                case KEFIR_AST_TYPE_SCALAR_FLOAT:
                case KEFIR_AST_TYPE_SCALAR_DOUBLE:
                case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
                case KEFIR_AST_TYPE_COMPLEX_FLOAT:
                case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
                case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
                case KEFIR_AST_TYPE_SCALAR_POINTER:
                case KEFIR_AST_TYPE_ENUMERATION:
                case KEFIR_AST_TYPE_STRUCTURE:
                case KEFIR_AST_TYPE_UNION:
                case KEFIR_AST_TYPE_ARRAY:
                case KEFIR_AST_TYPE_FUNCTION:
                case KEFIR_AST_TYPE_QUALIFIED:
                case KEFIR_AST_TYPE_VA_LIST:
                    return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, source_location,
                                                  "Unsigned type specifier cannot be applied to the type");
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t resolve_qualification(kefir_ast_type_qualifier_type_t qualifier,
                                            struct kefir_ast_type_qualification *qualifiers) {
    switch (qualifier) {
        case KEFIR_AST_TYPE_QUALIFIER_CONST:
            qualifiers->constant = true;
            break;

        case KEFIR_AST_TYPE_QUALIFIER_RESTRICT:
            qualifiers->restricted = true;
            break;

        case KEFIR_AST_TYPE_QUALIFIER_VOLATILE:
            qualifiers->volatile_type = true;
            break;

        case KEFIR_AST_TYPE_QUALIFIER_ATOMIC:
            return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Atomic types are not supported yet");

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected type qualifier");
    }
    return KEFIR_OK;
}

static kefir_result_t resolve_storage_class(const struct kefir_ast_declarator_specifier *decl_specifier,
                                            kefir_ast_scoped_identifier_storage_t *storage_class) {
    kefir_ast_storage_class_specifier_type_t specifier = decl_specifier->storage_class;
    switch (specifier) {
        case KEFIR_AST_STORAGE_SPECIFIER_TYPEDEF:
            REQUIRE(*storage_class == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Typedef storage class cannot be combined with others"));
            *storage_class = KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_TYPEDEF;
            break;

        case KEFIR_AST_STORAGE_SPECIFIER_EXTERN:
            if (*storage_class == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN) {
                *storage_class = KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN;
            } else {
                REQUIRE(*storage_class == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_THREAD_LOCAL,
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                               "Extern storage class can only be colocated with thread_local"));
                *storage_class = KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN_THREAD_LOCAL;
            }
            break;

        case KEFIR_AST_STORAGE_SPECIFIER_STATIC:
            if (*storage_class == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN) {
                *storage_class = KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC;
            } else {
                REQUIRE(*storage_class == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_THREAD_LOCAL,
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                               "Static storage class can only be colocated with thread_local"));
                *storage_class = KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC_THREAD_LOCAL;
            }
            break;

        case KEFIR_AST_STORAGE_SPECIFIER_THREAD_LOCAL:
            if (*storage_class == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN) {
                *storage_class = KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN_THREAD_LOCAL;
            } else if (*storage_class == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC) {
                *storage_class = KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC_THREAD_LOCAL;
            } else {
                REQUIRE(
                    *storage_class == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Thread_local storage class can only be colocated with extern or static"));
                *storage_class = KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_THREAD_LOCAL;
            }
            break;

        case KEFIR_AST_STORAGE_SPECIFIER_AUTO:
            REQUIRE(*storage_class == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Auto storage class cannot be combined with others"));
            *storage_class = KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO;
            break;

        case KEFIR_AST_STORAGE_SPECIFIER_REGISTER:
            REQUIRE(*storage_class == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_specifier->source_location,
                                           "Register storage class cannot be combined with others"));
            *storage_class = KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_REGISTER;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Invalid storage-class specifier");
    }
    return KEFIR_OK;
}

static kefir_result_t resolve_function_specifier(kefir_ast_function_specifier_type_t specifier,
                                                 kefir_ast_function_specifier_t *function_specifier) {
    switch (specifier) {
        case KEFIR_AST_FUNCTION_SPECIFIER_TYPE_NORETURN:
            switch (*function_specifier) {
                case KEFIR_AST_FUNCTION_SPECIFIER_NONE:
                case KEFIR_AST_FUNCTION_SPECIFIER_NORETURN:
                    *function_specifier = KEFIR_AST_FUNCTION_SPECIFIER_NORETURN;
                    break;

                case KEFIR_AST_FUNCTION_SPECIFIER_INLINE:
                case KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN:
                    *function_specifier = KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN;
                    break;
            }
            break;

        case KEFIR_AST_FUNCTION_SPECIFIER_TYPE_INLINE:
            switch (*function_specifier) {
                case KEFIR_AST_FUNCTION_SPECIFIER_NONE:
                case KEFIR_AST_FUNCTION_SPECIFIER_INLINE:
                    *function_specifier = KEFIR_AST_FUNCTION_SPECIFIER_INLINE;
                    break;

                case KEFIR_AST_FUNCTION_SPECIFIER_NORETURN:
                case KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN:
                    *function_specifier = KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN;
                    break;
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Invalid storage-class specifier");
    }
    return KEFIR_OK;
}

static kefir_result_t type_alignment(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                     const struct kefir_ast_type *type, kefir_size_t *alignment,
                                     kefir_size_t *max_alignment, const struct kefir_source_location *source_location) {

    kefir_ast_target_environment_opaque_type_t target_type;
    struct kefir_ast_target_environment_object_info object_info;
    REQUIRE_OK(
        KEFIR_AST_TARGET_ENVIRONMENT_GET_TYPE(mem, context, context->target_env, type, &target_type, source_location));
    kefir_result_t res =
        KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_INFO(mem, context->target_env, target_type, NULL, &object_info);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, target_type);
        return res;
    });
    REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, target_type));
    ASSIGN_PTR(alignment, object_info.alignment);
    ASSIGN_PTR(max_alignment, object_info.max_alignment);
    return KEFIR_OK;
}

static kefir_result_t evaluate_alignment(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                         struct kefir_ast_node_base *node, kefir_size_t *alignment) {
    REQUIRE_OK(kefir_ast_analyze_node(mem, context, node));
    if (node->properties.category == KEFIR_AST_NODE_CATEGORY_TYPE) {
        kefir_size_t new_alignment = 0;
        REQUIRE_OK(type_alignment(mem, context, node->properties.type, &new_alignment, NULL, &node->source_location));
        *alignment = MAX(*alignment, new_alignment);
    } else {
        struct kefir_ast_constant_expression_value value;
        REQUIRE_OK(kefir_ast_constant_expression_value_evaluate(mem, context, node, &value));
        REQUIRE(value.klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->source_location,
                                       "Expected alignment specifier to produce integral constant expression"));
        *alignment = MAX(*alignment, (kefir_size_t) value.integer);
    }
    return KEFIR_OK;
}

static kefir_result_t resolve_pointer_declarator(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                 const struct kefir_ast_declarator *declarator,
                                                 const struct kefir_ast_type **base_type) {
    *base_type = kefir_ast_type_pointer(mem, context->type_bundle, *base_type);
    REQUIRE(*base_type != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST pointer type"));

    struct kefir_ast_type_qualification qualification = {false};
    kefir_ast_type_qualifier_type_t qualifier;
    for (const struct kefir_list_entry *iter =
             kefir_ast_type_qualifier_list_iter(&declarator->pointer.type_qualifiers, &qualifier);
         iter != NULL; kefir_ast_type_qualifier_list_next(&iter, &qualifier)) {
        REQUIRE_OK(resolve_qualification(qualifier, &qualification));
    }
    if (!KEFIR_AST_TYPE_IS_ZERO_QUALIFICATION(&qualification)) {
        *base_type = kefir_ast_type_qualified(mem, context->type_bundle, *base_type, qualification);
        REQUIRE(*base_type != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST qualified type"));
    }
    return KEFIR_OK;
}

static kefir_result_t resolve_array_declarator(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                               const struct kefir_ast_declarator *declarator,
                                               const struct kefir_ast_type **base_type) {
    struct kefir_ast_type_qualification qualification = {false};
    kefir_ast_type_qualifier_type_t qualifier;
    for (const struct kefir_list_entry *iter =
             kefir_ast_type_qualifier_list_iter(&declarator->array.type_qualifiers, &qualifier);
         iter != NULL; kefir_ast_type_qualifier_list_next(&iter, &qualifier)) {
        REQUIRE_OK(resolve_qualification(qualifier, &qualification));
    }

    switch (declarator->array.type) {
        case KEFIR_AST_DECLARATOR_ARRAY_UNBOUNDED:
            *base_type = kefir_ast_type_unbounded_array(mem, context->type_bundle, *base_type, &qualification);
            REQUIRE(*base_type != NULL,
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST unbounded array type"));
            break;

        case KEFIR_AST_DECLARATOR_ARRAY_VLA_UNSPECIFIED:
            if (declarator->array.static_array) {
                *base_type =
                    kefir_ast_type_vlen_array_static(mem, context->type_bundle, *base_type, NULL, &qualification);
            } else {
                *base_type = kefir_ast_type_vlen_array(mem, context->type_bundle, *base_type, NULL, &qualification);
            }
            REQUIRE(*base_type != NULL,
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST unbounded array type"));
            break;

        case KEFIR_AST_DECLARATOR_ARRAY_BOUNDED: {
            struct kefir_ast_constant_expression_value value;
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, declarator->array.length));
            kefir_result_t res =
                kefir_ast_constant_expression_value_evaluate(mem, context, declarator->array.length, &value);
            if (res == KEFIR_NOT_CONSTANT) {
                kefir_clear_error();
                REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(declarator->array.length->properties.type),
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &declarator->array.length->source_location,
                                               "Variable-length array declaration length shall have integral type"));
                if (declarator->array.static_array) {
                    *base_type = kefir_ast_type_vlen_array_static(mem, context->type_bundle, *base_type,
                                                                  KEFIR_AST_NODE_CLONE(mem, declarator->array.length),
                                                                  &qualification);
                } else {
                    *base_type =
                        kefir_ast_type_vlen_array(mem, context->type_bundle, *base_type,
                                                  KEFIR_AST_NODE_CLONE(mem, declarator->array.length), &qualification);
                }
            } else {
                REQUIRE_OK(res);
                REQUIRE(value.klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER,
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &declarator->array.length->source_location,
                                               "Constant length of AST array declaration shall have integral type"));
                if (declarator->array.static_array) {
                    *base_type = kefir_ast_type_array_static(mem, context->type_bundle, *base_type,
                                                             kefir_ast_constant_expression_integer(mem, value.integer),
                                                             &qualification);
                } else {
                    *base_type =
                        kefir_ast_type_array(mem, context->type_bundle, *base_type,
                                             kefir_ast_constant_expression_integer(mem, value.integer), &qualification);
                }
            }
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t resolve_function_declarator(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                  const struct kefir_ast_declarator *declarator, kefir_uint64_t flags,
                                                  const struct kefir_ast_type **base_type) {
    struct kefir_ast_function_type *func_type = NULL;
    const struct kefir_ast_type *type = kefir_ast_type_function(mem, context->type_bundle, *base_type, &func_type);
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocated AST type"));

    struct kefir_ast_function_declaration_context *decl_context =
        KEFIR_MALLOC(mem, sizeof(struct kefir_ast_function_declaration_context));
    REQUIRE(decl_context != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST function declaration context"));
    kefir_result_t res = kefir_ast_function_declaration_context_init(
        mem, context, (flags & KEFIR_AST_DECLARATION_ANALYSIS_FUNCTION_DEFINITION_CONTEXT) != 0, decl_context);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, decl_context);
        return res;
    });

    res = KEFIR_OK;
    for (const struct kefir_list_entry *iter = kefir_list_head(&declarator->function.parameters);
         iter != NULL && res == KEFIR_OK; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, iter->value);

        struct kefir_ast_declaration *decl_list = NULL;
        res = kefir_ast_downcast_declaration(node, &decl_list, false);
        if (res == KEFIR_OK) {
            struct kefir_ast_init_declarator *declaration = NULL;
            REQUIRE_CHAIN(&res, kefir_ast_analyze_node(mem, &decl_context->context, node));
            REQUIRE_CHAIN(&res, kefir_ast_declaration_unpack_single(decl_list, &declaration));
            REQUIRE_CHAIN(&res, kefir_ast_type_function_parameter(
                                    mem, context->type_bundle, func_type, declaration->base.properties.type,
                                    &declaration->base.properties.declaration_props.storage));
        } else if (res == KEFIR_NO_MATCH) {
            struct kefir_ast_identifier *identifier = NULL;
            REQUIRE_MATCH(
                &res, kefir_ast_downcast_identifier(node, &identifier, false),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->source_location,
                                       "Function declaration parameter shall be either declaration, or identifier"));

            REQUIRE_CHAIN(&res, kefir_ast_try_analyze_identifier(mem, context, identifier, node));
            if (res == KEFIR_OK) {
                if (node->properties.category == KEFIR_AST_NODE_CATEGORY_TYPE) {
                    REQUIRE_CHAIN(&res, kefir_ast_type_function_parameter(mem, context->type_bundle, func_type,
                                                                          node->properties.type, NULL));
                } else {
                    res = kefir_ast_type_function_parameter(mem, context->type_bundle, func_type, NULL, NULL);
                }
            } else if (res == KEFIR_NOT_FOUND) {
                res = kefir_ast_type_function_parameter(mem, context->type_bundle, func_type, NULL, NULL);
            }
        } else {
            res = KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->source_location,
                                         "Expected function parameter to be either a declaration or an identifier");
        }
    }

    REQUIRE_CHAIN(&res, kefir_ast_type_function_ellipsis(func_type, declarator->function.ellipsis));

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_function_declaration_context_free(mem, decl_context);
        KEFIR_FREE(mem, decl_context);
        return res;
    });

    res = kefir_list_insert_after(mem, context->function_decl_contexts,
                                  kefir_list_tail(context->function_decl_contexts), decl_context);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_function_declaration_context_free(mem, decl_context);
        KEFIR_FREE(mem, decl_context);
        return res;
    });

    *base_type = type;
    return KEFIR_OK;
}

static kefir_result_t analyze_declaration_declarator_alignment_attribute(
    struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_ast_attribute *attribute,
    const struct kefir_ast_type **base_type, kefir_size_t *alignment, kefir_uint64_t flags,
    struct kefir_ast_declarator_attributes *attributes, const struct kefir_source_location *source_location) {
    REQUIRE(alignment != NULL, KEFIR_OK);

    if (kefir_list_length(&attribute->parameters) == 1) {
        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, param, kefir_list_head(&attribute->parameters)->value);
        struct kefir_ast_constant_expression_value alignment_value;
        REQUIRE_OK(kefir_ast_analyze_node(mem, context, param));
        REQUIRE_OK(kefir_ast_constant_expression_value_evaluate(mem, context, param, &alignment_value));
        REQUIRE(alignment_value.klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &param->source_location,
                                       "Expected integral constant expression"));

        if ((flags & KEFIR_AST_DECLARATION_ANALYSIS_FORBID_ALIGNMENT_DECREASE) != 0) {
            kefir_size_t natural_alignment;
            REQUIRE_OK(type_alignment(mem, context, *base_type, &natural_alignment, NULL, source_location));
            if (alignment_value.uinteger >= natural_alignment) {
                *alignment = MAX(*alignment, alignment_value.uinteger);
            }
            if (attributes != NULL) {
                attributes->aligned = alignment_value.uinteger;
            }
        } else {
            *alignment = alignment_value.uinteger;
        }
    } else if (kefir_list_length(&attribute->parameters) == 0) {
        REQUIRE_OK(type_alignment(mem, context, *base_type, NULL, alignment, source_location));
        if (attributes != NULL) {
            attributes->aligned = *alignment;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t analyze_declaration_declarator_attributes(struct kefir_mem *mem,
                                                                const struct kefir_ast_context *context,
                                                                const struct kefir_ast_declarator *declarator,
                                                                const struct kefir_ast_type **base_type,
                                                                kefir_size_t *alignment, kefir_uint64_t flags,
                                                                struct kefir_ast_declarator_attributes *attributes) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&declarator->attributes.attributes); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_attribute_list *, attr_list, iter->value);

        for (const struct kefir_list_entry *iter2 = kefir_list_head(&attr_list->list); iter2 != NULL;
             kefir_list_next(&iter2)) {
            ASSIGN_DECL_CAST(struct kefir_ast_attribute *, attribute, iter2->value);

            if (strcmp(attribute->name, "aligned") == 0 || strcmp(attribute->name, "__aligned__") == 0) {
                REQUIRE_OK(analyze_declaration_declarator_alignment_attribute(
                    mem, context, attribute, base_type, alignment, flags, attributes, &declarator->source_location));
            } else if (strcmp(attribute->name, "__gnu_inline__") == 0 && attributes != NULL) {
                attributes->gnu_inline = true;
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t analyze_declaration_declarator_impl(
    struct kefir_mem *mem, const struct kefir_ast_context *context, const struct kefir_ast_declarator *declarator,
    const char **identifier, const struct kefir_ast_type **base_type, kefir_size_t *alignment, kefir_uint64_t flags,
    struct kefir_ast_declarator_attributes *attributes) {
    ASSIGN_PTR(identifier, NULL);
    REQUIRE(declarator != NULL, KEFIR_OK);
    switch (declarator->klass) {
        case KEFIR_AST_DECLARATOR_IDENTIFIER:
            ASSIGN_PTR(identifier, declarator->identifier.identifier);
            if (attributes != NULL) {
                attributes->asm_label = declarator->identifier.asm_label;
            }
            break;

        case KEFIR_AST_DECLARATOR_POINTER:
            REQUIRE_OK(resolve_pointer_declarator(mem, context, declarator, base_type));
            REQUIRE_OK(analyze_declaration_declarator_impl(mem, context, declarator->pointer.declarator, identifier,
                                                           base_type, alignment, flags, attributes));
            break;

        case KEFIR_AST_DECLARATOR_ARRAY:
            REQUIRE_OK(resolve_array_declarator(mem, context, declarator, base_type));
            REQUIRE_OK(analyze_declaration_declarator_impl(mem, context, declarator->array.declarator, identifier,
                                                           base_type, alignment, flags, attributes));
            break;

        case KEFIR_AST_DECLARATOR_FUNCTION: {
            const struct kefir_ast_declarator_function *underlying_function = NULL;
            REQUIRE_OK(kefir_ast_declarator_unpack_function(declarator->function.declarator, &underlying_function));
            REQUIRE_OK(
                resolve_function_declarator(mem, context, declarator,
                                            (flags & KEFIR_AST_DECLARATION_ANALYSIS_FUNCTION_DEFINITION_CONTEXT) != 0 &&
                                                underlying_function == NULL,
                                            base_type));
            REQUIRE_OK(analyze_declaration_declarator_impl(mem, context, declarator->function.declarator, identifier,
                                                           base_type, alignment, flags, attributes));
        } break;
    }

    REQUIRE_OK(
        analyze_declaration_declarator_attributes(mem, context, declarator, base_type, alignment, flags, attributes));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_analyze_declaration_declarator(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                        const struct kefir_ast_declarator *declarator,
                                                        const char **identifier,
                                                        const struct kefir_ast_type **base_type,
                                                        kefir_size_t *alignment, kefir_uint64_t flags,
                                                        struct kefir_ast_declarator_attributes *attributes) {
    UNUSED(flags);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));

    if (attributes != NULL) {
        *attributes = (struct kefir_ast_declarator_attributes){0};
    }

    REQUIRE_OK(analyze_declaration_declarator_impl(mem, context, declarator, identifier, base_type, alignment, flags,
                                                   attributes));
    return KEFIR_OK;
}

static kefir_result_t analyze_declaration_specifiers_impl(
    struct kefir_mem *mem, const struct kefir_ast_context *context,
    const struct kefir_ast_declarator_specifier_list *specifiers, const struct kefir_ast_type **type,
    kefir_ast_scoped_identifier_storage_t *storage, kefir_ast_function_specifier_t *function, kefir_size_t *alignment,
    kefir_uint64_t flags, const struct kefir_source_location *source_location) {
    enum signedness signedness = SIGNEDNESS_DEFAULT;
    enum type_specifier_sequence_state seq_state = TYPE_SPECIFIER_SEQUENCE_EMPTY;
    enum real_class real_class = REAL_SCALAR;
    const struct kefir_ast_type *base_type = NULL;
    struct kefir_ast_type_qualification qualification = {false};
    kefir_ast_scoped_identifier_storage_t storage_class = KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN;
    kefir_ast_function_specifier_t function_specifier = KEFIR_AST_FUNCTION_SPECIFIER_NONE;
    kefir_size_t alignment_specifier = 0;

    kefir_bool_t alignment_specifier_present = false;

    struct kefir_ast_declarator_specifier *declatator_specifier;
    for (struct kefir_list_entry *iter = kefir_ast_declarator_specifier_list_iter(specifiers, &declatator_specifier);
         iter != NULL; kefir_ast_declarator_specifier_list_next(&iter, &declatator_specifier)) {
        switch (declatator_specifier->klass) {
            case KEFIR_AST_TYPE_SPECIFIER:
                REQUIRE_OK(resolve_type(mem, context, &signedness, &real_class, &seq_state, &base_type, alignment,
                                        declatator_specifier, flags));
                break;

            case KEFIR_AST_TYPE_QUALIFIER:
                REQUIRE_OK(resolve_qualification(declatator_specifier->type_qualifier, &qualification));
                break;

            case KEFIR_AST_STORAGE_CLASS_SPECIFIER:
                REQUIRE_OK(resolve_storage_class(declatator_specifier, &storage_class));
                REQUIRE(storage_class != KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_TYPEDEF || !alignment_specifier_present,
                        KEFIR_SET_SOURCE_ERROR(
                            KEFIR_ANALYSIS_ERROR, &declatator_specifier->source_location,
                            "Type definition storage class cannot be combined with alignment specifiers"));
                break;

            case KEFIR_AST_FUNCTION_SPECIFIER:
                REQUIRE_OK(resolve_function_specifier(declatator_specifier->function_specifier, &function_specifier));
                break;

            case KEFIR_AST_ALIGNMENT_SPECIFIER:
                if ((flags & KEFIR_AST_DECLARATION_ANALYSIS_IGNORE_ALIGNMENT_SPECIFIER) == 0) {
                    REQUIRE_OK(evaluate_alignment(mem, context, declatator_specifier->alignment_specifier,
                                                  &alignment_specifier));
                    alignment_specifier_present = true;
                    REQUIRE(storage_class != KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_TYPEDEF || !alignment_specifier_present,
                            KEFIR_SET_SOURCE_ERROR(
                                KEFIR_ANALYSIS_ERROR, &declatator_specifier->source_location,
                                "Type definition storage class cannot be combined with alignment specifiers"));
                }
                break;
        }
    }
    REQUIRE_OK(apply_type_signedness(mem, context->type_bundle, signedness,
                                     kefir_ast_declarator_specifier_list_source_location(specifiers), &base_type));
    if (!KEFIR_AST_TYPE_IS_ZERO_QUALIFICATION(&qualification)) {
        base_type = kefir_ast_type_qualified(mem, context->type_bundle, base_type, qualification);
    }

    ASSIGN_PTR(type, base_type);
    ASSIGN_PTR(storage, storage_class);
    ASSIGN_PTR(function, function_specifier);

    if (alignment != NULL) {
        if (alignment_specifier > 0) {
            kefir_size_t natural_alignment = 0;
            REQUIRE_OK(type_alignment(mem, context, base_type, &natural_alignment, NULL, source_location));
            REQUIRE(natural_alignment <= alignment_specifier,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR,
                                           kefir_ast_declarator_specifier_list_source_location(specifiers),
                                           "Specified alignment shall be at least as strict as natural"));
            *alignment = MAX(*alignment, alignment_specifier);
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_analyze_declaration_specifiers(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                        const struct kefir_ast_declarator_specifier_list *specifiers,
                                                        const struct kefir_ast_type **type,
                                                        kefir_ast_scoped_identifier_storage_t *storage,
                                                        kefir_ast_function_specifier_t *function,
                                                        kefir_size_t *alignment, kefir_uint64_t flags,
                                                        const struct kefir_source_location *source_location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(specifiers != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST declarator specifier list"));

    ASSIGN_PTR(alignment, 0);
    REQUIRE_OK(analyze_declaration_specifiers_impl(mem, context, specifiers, type, storage, function, alignment, flags,
                                                   source_location));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_analyze_declaration(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                             const struct kefir_ast_declarator_specifier_list *specifiers,
                                             const struct kefir_ast_declarator *declarator, const char **identifier,
                                             const struct kefir_ast_type **type,
                                             kefir_ast_scoped_identifier_storage_t *storage,
                                             kefir_ast_function_specifier_t *function, kefir_size_t *alignment,
                                             kefir_uint64_t flags, struct kefir_ast_declarator_attributes *attributes,
                                             const struct kefir_source_location *source_location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(specifiers != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST declarator specifier list"));

    ASSIGN_PTR(alignment, 0);

    const struct kefir_ast_type *base_type = NULL;
    REQUIRE_OK(kefir_ast_analyze_declaration_specifiers(mem, context, specifiers, &base_type, storage, function,
                                                        alignment, flags, source_location));
    REQUIRE_OK(kefir_ast_analyze_declaration_declarator(mem, context, declarator, identifier, &base_type, alignment,
                                                        flags, attributes));
    ASSIGN_PTR(type, base_type);
    return KEFIR_OK;
}
