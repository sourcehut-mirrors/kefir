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

#include "kefir/preprocessor/preprocessor.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/preprocessor/directives.h"
#include "kefir/core/source_error.h"
#include "kefir/parser/parser.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/ast/constant_expression.h"
#include "kefir/preprocessor/format.h"
#include "kefir/core/string_buffer.h"

static const struct kefir_preprocessor_configuration DefaultConfiguration = {
    .named_macro_vararg = false, .include_next = false, .va_args_concat = false};

kefir_result_t kefir_preprocessor_configuration_default(struct kefir_preprocessor_configuration *config) {
    REQUIRE(config != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to preprocessor configuration"));

    *config = DefaultConfiguration;
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_context_init(struct kefir_mem *mem, struct kefir_preprocessor_context *context,
                                               const struct kefir_preprocessor_source_locator *locator,
                                               struct kefir_ast_context *ast_context,
                                               const struct kefir_preprocessor_context_extensions *extensions) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to preprocessor context"));
    REQUIRE(locator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to preprocessor source locator"));
    REQUIRE(ast_context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid preprocessor AST context"));

    REQUIRE_OK(kefir_preprocessor_user_macro_scope_init(NULL, &context->user_macros));
    REQUIRE_OK(kefir_hashtree_init(&context->undefined_macros, &kefir_hashtree_str_ops));
    context->source_locator = locator;
    context->ast_context = ast_context;

    // Predefined macros
    context->environment.timestamp = time(NULL);
    context->environment.hosted = true;
    context->environment.version = 201710L;

    // Environment macros
    context->environment.stdc_iso10646 = 0;
    context->environment.stdc_mb_might_neq_wc = false;
    context->environment.stdc_utf16 = false;
    context->environment.stdc_utf32 = false;

#ifdef __STDC_ISO_10646__
    context->environment.stdc_iso10646 = __STDC_ISO_10646__;
#endif
#ifdef __STDC_MB_MIGHT_NEQ_WC__
    context->environment.stdc_mb_might_neq_wc = true;
#endif
#ifdef __STDC_UTF_16__
    context->environment.stdc_utf16 = true;
#endif
#ifdef __STDC_UTF_32__
    context->environment.stdc_utf32 = true;
#endif

    // Conditional macros
    context->environment.stdc_analyzable = false;
    context->environment.stdc_iec559 = true;
    context->environment.stdc_iec559_complex = false;
    context->environment.stdc_lib_ext1 = 0;
    context->environment.stdc_no_atomics = false;
    context->environment.stdc_no_complex = false;
    context->environment.stdc_no_threads = false;
    context->environment.stdc_no_vla = false;

    context->preprocessor_config = &DefaultConfiguration;

    // Extension macros
    context->environment.data_model = NULL;

    // Supported attributes & builtins
    REQUIRE_OK(kefir_hashtreeset_init(&context->environment.supported_attributes, &kefir_hashtree_str_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&context->environment.supported_builtins, &kefir_hashtree_str_ops));

    // State
    context->state.counter = 0;

    context->extensions = extensions;
    context->extensions_payload = NULL;
    kefir_result_t res;
    KEFIR_RUN_EXTENSION0(&res, mem, context, on_init);
    REQUIRE_OK(res);
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_context_free(struct kefir_mem *mem, struct kefir_preprocessor_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to preprocessor context"));

    kefir_result_t res;
    KEFIR_RUN_EXTENSION0(&res, mem, context, on_free);
    REQUIRE_OK(res);
    context->extensions = NULL;
    context->extensions_payload = NULL;

    REQUIRE_OK(kefir_hashtreeset_free(mem, &context->environment.supported_attributes));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &context->environment.supported_builtins));
    REQUIRE_OK(kefir_hashtree_free(mem, &context->undefined_macros));
    REQUIRE_OK(kefir_preprocessor_user_macro_scope_free(mem, &context->user_macros));
    context->source_locator = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_init(struct kefir_mem *mem, struct kefir_preprocessor *preprocessor,
                                       struct kefir_string_pool *symbols, struct kefir_lexer_source_cursor *cursor,
                                       const struct kefir_lexer_context *context,
                                       struct kefir_preprocessor_context *preprocessor_context,
                                       const struct kefir_preprocessor_source_file_info *current_file,
                                       const struct kefir_preprocessor_extensions *extensions) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(preprocessor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid preprocessor"));
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer source cursor"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser context"));
    REQUIRE(preprocessor_context != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid preprocessor context"));

    preprocessor->context = preprocessor_context;
    REQUIRE_OK(kefir_lexer_init(mem, &preprocessor->lexer, symbols, cursor, context,
                                extensions != NULL ? extensions->lexer_extensions : NULL));
    kefir_result_t res = kefir_preprocessor_directive_scanner_init(&preprocessor->directive_scanner,
                                                                   &preprocessor->lexer, preprocessor->context);
    REQUIRE_CHAIN(&res,
                  kefir_preprocessor_predefined_macro_scope_init(mem, &preprocessor->predefined_macros, preprocessor));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_lexer_free(mem, &preprocessor->lexer);
        return res;
    });
    res = kefir_preprocessor_overlay_macro_scope_init(
        &preprocessor->macro_overlay, &preprocessor->predefined_macros.scope, &preprocessor_context->user_macros.scope);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_preprocessor_predefined_macro_scope_free(mem, &preprocessor->predefined_macros);
        kefir_lexer_free(mem, &preprocessor->lexer);
        return res;
    });
    preprocessor->macros = &preprocessor->macro_overlay.scope;
    preprocessor->current_file = current_file;
    preprocessor->parent = NULL;

    preprocessor->extensions = extensions;
    preprocessor->extension_payload = NULL;
    if (preprocessor->extensions != NULL) {
        KEFIR_RUN_EXTENSION0(&res, mem, preprocessor, on_init);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_preprocessor_predefined_macro_scope_free(mem, &preprocessor->predefined_macros);
            kefir_lexer_free(mem, &preprocessor->lexer);
            return res;
        });
    }
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_free(struct kefir_mem *mem, struct kefir_preprocessor *preprocessor) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(preprocessor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid preprocessor"));

    if (preprocessor->extensions != NULL) {
        kefir_result_t res;
        KEFIR_RUN_EXTENSION0(&res, mem, preprocessor, on_free);
        REQUIRE_OK(res);
        preprocessor->extensions = NULL;
        preprocessor->extension_payload = NULL;
    }
    REQUIRE_OK(kefir_preprocessor_predefined_macro_scope_free(mem, &preprocessor->predefined_macros));
    REQUIRE_OK(kefir_lexer_free(mem, &preprocessor->lexer));
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_skip_group(struct kefir_mem *mem, struct kefir_preprocessor *preprocessor) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(preprocessor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid preprocessor"));

    kefir_preprocessor_directive_type_t directive;
    kefir_size_t nested_ifs = 0;
    kefir_bool_t skip = true;

    while (skip) {
        struct kefir_preprocessor_directive_scanner_state scanner_state;
        REQUIRE_OK(kefir_preprocessor_directive_scanner_save(&preprocessor->directive_scanner, &scanner_state));
        REQUIRE_OK(kefir_preprocessor_directive_scanner_match(mem, &preprocessor->directive_scanner, &directive));
        switch (directive) {
            case KEFIR_PREPROCESSOR_DIRECTIVE_IF:
            case KEFIR_PREPROCESSOR_DIRECTIVE_IFDEF:
            case KEFIR_PREPROCESSOR_DIRECTIVE_IFNDEF:
                nested_ifs++;
                REQUIRE_OK(kefir_preprocessor_directive_scanner_skip_line(mem, &preprocessor->directive_scanner));
                break;

            case KEFIR_PREPROCESSOR_DIRECTIVE_ELIF:
            case KEFIR_PREPROCESSOR_DIRECTIVE_ELSE:
                if (nested_ifs == 0) {
                    REQUIRE_OK(
                        kefir_preprocessor_directive_scanner_restore(&preprocessor->directive_scanner, &scanner_state));
                    skip = false;
                }
                break;

            case KEFIR_PREPROCESSOR_DIRECTIVE_ENDIF:
                if (nested_ifs > 0) {
                    nested_ifs--;
                    REQUIRE_OK(kefir_preprocessor_directive_scanner_skip_line(mem, &preprocessor->directive_scanner));
                } else {
                    REQUIRE_OK(
                        kefir_preprocessor_directive_scanner_restore(&preprocessor->directive_scanner, &scanner_state));
                    skip = false;
                }
                break;

            case KEFIR_PREPROCESSOR_DIRECTIVE_SENTINEL:
                skip = false;
                break;

            case KEFIR_PREPROCESSOR_DIRECTIVE_INCLUDE:
            case KEFIR_PREPROCESSOR_DIRECTIVE_INCLUDE_NEXT:
            case KEFIR_PREPROCESSOR_DIRECTIVE_DEFINE:
            case KEFIR_PREPROCESSOR_DIRECTIVE_UNDEF:
            case KEFIR_PREPROCESSOR_DIRECTIVE_LINE:
            case KEFIR_PREPROCESSOR_DIRECTIVE_ERROR:
            case KEFIR_PREPROCESSOR_DIRECTIVE_PRAGMA:
            case KEFIR_PREPROCESSOR_DIRECTIVE_EMPTY:
            case KEFIR_PREPROCESSOR_DIRECTIVE_NON:
            case KEFIR_PREPROCESSOR_DIRECTIVE_PP_TOKEN:
                REQUIRE_OK(kefir_preprocessor_directive_scanner_skip_line(mem, &preprocessor->directive_scanner));
                break;
        }
    }
    return KEFIR_OK;
}

enum if_condition_state { IF_CONDITION_SUCCESS, IF_CONDITION_FAIL };

static kefir_result_t process_raw_string(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                         const struct kefir_token *token, const char **str) {
    struct kefir_list list;
    REQUIRE_OK(kefir_list_init(&list));
    REQUIRE_OK(kefir_list_insert_after(mem, &list, kefir_list_tail(&list), (void *) token));

    struct kefir_token string_token;
    kefir_result_t res = kefir_lexer_merge_raw_string_literals(mem, &list, &string_token);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &list);
        return res;
    });
    res = kefir_list_free(mem, &list);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_token_free(mem, &string_token);
        return res;
    });

    *str = kefir_string_pool_insert(mem, symbols, string_token.string_literal.literal, NULL);
    REQUIRE_ELSE(*str != NULL, {
        kefir_token_free(mem, &string_token);
        return KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert string literal into symbol table");
    });
    REQUIRE_OK(kefir_token_free(mem, &string_token));
    return KEFIR_OK;
}

static kefir_result_t process_include(struct kefir_mem *mem, struct kefir_preprocessor *preprocessor,
                                      struct kefir_token_allocator *token_allocator, struct kefir_token_buffer *buffer,
                                      struct kefir_preprocessor_directive *directive) {
    REQUIRE_OK(kefir_preprocessor_run_substitutions(mem, preprocessor, token_allocator, &directive->pp_tokens, NULL,
                                                    KEFIR_PREPROCESSOR_SUBSTITUTION_NORMAL));
    const kefir_size_t pp_tokens_length = kefir_token_buffer_length(&directive->pp_tokens);
    REQUIRE(pp_tokens_length > 0,
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &directive->source_location, "Expected file path"));
    const struct kefir_token *token = kefir_token_buffer_at(&directive->pp_tokens, 0);
    const char *include_path = NULL;
    kefir_bool_t system_include = false;
    if (token->klass == KEFIR_TOKEN_PP_HEADER_NAME) {
        include_path = token->pp_header_name.header_name;
        system_include = token->pp_header_name.system;
    } else if (token->klass == KEFIR_TOKEN_STRING_LITERAL &&
               token->string_literal.type == KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE &&
               token->string_literal.raw_literal) {
        REQUIRE_OK(process_raw_string(mem, preprocessor->lexer.symbols, token, &include_path));
    } else {
        return KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &directive->source_location, "Expected file path");
    }

    struct kefir_preprocessor_source_file source_file;
    REQUIRE_OK(preprocessor->context->source_locator->open(
        mem, preprocessor->context->source_locator, include_path, system_include, preprocessor->current_file,
        directive->type == KEFIR_PREPROCESSOR_DIRECTIVE_INCLUDE_NEXT ? KEFIR_PREPROCESSOR_SOURCE_LOCATOR_MODE_NEXT
                                                                     : KEFIR_PREPROCESSOR_SOURCE_LOCATOR_MODE_NORMAL,
        &source_file));

    struct kefir_preprocessor subpreprocessor;
    kefir_result_t res = kefir_preprocessor_init(mem, &subpreprocessor, preprocessor->lexer.symbols,
                                                 &source_file.cursor, preprocessor->lexer.context,
                                                 preprocessor->context, &source_file.info, preprocessor->extensions);
    REQUIRE_ELSE(res == KEFIR_OK, {
        source_file.close(mem, &source_file);
        return res;
    });
    subpreprocessor.parent = preprocessor;

    res = kefir_preprocessor_run(mem, &subpreprocessor, token_allocator, buffer);
    kefir_size_t buffer_length = kefir_token_buffer_length(buffer);
    if (buffer_length > 0 && kefir_token_buffer_at(buffer, buffer_length - 1)->klass == KEFIR_TOKEN_SENTINEL) {
        REQUIRE_CHAIN(&res, kefir_token_buffer_pop(mem, buffer));
        buffer_length = kefir_token_buffer_length(buffer);
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_preprocessor_free(mem, &subpreprocessor);
        source_file.close(mem, &source_file);
        return res;
    });

    res = kefir_preprocessor_free(mem, &subpreprocessor);
    REQUIRE_ELSE(res == KEFIR_OK, {
        source_file.close(mem, &source_file);
        return res;
    });
    REQUIRE_OK(source_file.close(mem, &source_file));
    return KEFIR_OK;
}

static kefir_result_t process_define(struct kefir_mem *mem, struct kefir_preprocessor *preprocessor,
                                     struct kefir_preprocessor_directive *directive) {
    struct kefir_preprocessor_user_macro *macro = NULL;
    if (directive->define_directive.object) {
        macro = kefir_preprocessor_user_macro_new_object(mem, preprocessor->lexer.symbols,
                                                         directive->define_directive.identifier);
        REQUIRE(macro != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate user macro"));
        kefir_result_t res =
            kefir_token_buffer_insert(mem, &macro->replacement, &directive->define_directive.replacement);
        REQUIRE_CHAIN(&res,
                      kefir_preprocessor_user_macro_scope_insert(mem, &preprocessor->context->user_macros, macro));
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_preprocessor_user_macro_free(mem, macro);
            return res;
        });
    } else {
        macro = kefir_preprocessor_user_macro_new_function(mem, preprocessor->lexer.symbols,
                                                           directive->define_directive.identifier);
        REQUIRE(macro != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate user macro"));
        kefir_result_t res =
            kefir_token_buffer_insert(mem, &macro->replacement, &directive->define_directive.replacement);
        REQUIRE_CHAIN(&res, kefir_list_move_all(&macro->parameters, &directive->define_directive.parameters));
        if (res == KEFIR_OK) {
            macro->vararg = directive->define_directive.vararg;
            macro->vararg_parameter = directive->define_directive.vararg_parameter;
        }
        REQUIRE_CHAIN(&res,
                      kefir_preprocessor_user_macro_scope_insert(mem, &preprocessor->context->user_macros, macro));
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_preprocessor_user_macro_free(mem, macro);
            return res;
        });
    }
    return KEFIR_OK;
}

static kefir_result_t process_undef(struct kefir_mem *mem, struct kefir_preprocessor *preprocessor,
                                    struct kefir_preprocessor_directive *directive) {
    REQUIRE_OK(kefir_preprocessor_user_macro_scope_remove(mem, &preprocessor->context->user_macros,
                                                          directive->undef_directive.identifier));
    return KEFIR_OK;
}

static kefir_result_t evaluate_pp_tokens(struct kefir_mem *mem, struct kefir_preprocessor *preprocessor,
                                         struct kefir_token_allocator *token_allocator,
                                         struct kefir_token_buffer *pp_tokens,
                                         struct kefir_source_location *source_location, kefir_bool_t *result) {
    REQUIRE_OK(kefir_preprocessor_run_substitutions(mem, preprocessor, token_allocator, pp_tokens, NULL,
                                                    KEFIR_PREPROCESSOR_SUBSTITUTION_IF_CONDITION));
    struct kefir_token_buffer tokens;
    struct kefir_parser_token_cursor cursor;
    struct kefir_parser parser;
    struct kefir_ast_node_base *expression = NULL;
    struct kefir_ast_constant_expression_value expr_value;

    REQUIRE_OK(kefir_token_buffer_init(&tokens));
    kefir_result_t res =
        kefir_preprocessor_token_convert_buffer(mem, preprocessor, token_allocator, &tokens, pp_tokens);

    struct kefir_token_cursor_handle tokens_handle;
    REQUIRE_OK(kefir_token_buffer_cursor_handle(&tokens, &tokens_handle));

    REQUIRE_CHAIN_SET(&res, kefir_token_buffer_length(&tokens) > 0,
                      KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location, "Expected non-empty if condition"));
    REQUIRE_CHAIN(&res, kefir_parser_token_cursor_init(&cursor, &tokens_handle));
    REQUIRE_CHAIN(&res, kefir_parser_init(mem, &parser, preprocessor->lexer.symbols, &cursor,
                                          preprocessor->extensions != NULL ? preprocessor->extensions->parser : NULL));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_token_buffer_free(mem, &tokens);
        return res;
    });

    res = KEFIR_PARSER_NEXT_EXPRESSION(mem, &parser, &expression);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_parser_free(mem, &parser);
        kefir_token_buffer_free(mem, &tokens);
        return res;
    });

    res = kefir_parser_free(mem, &parser);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_token_buffer_free(mem, &tokens);
        return res;
    });

    res = kefir_token_buffer_free(mem, &tokens);
    REQUIRE_CHAIN(&res, kefir_ast_analyze_node(mem, preprocessor->context->ast_context, expression));
    REQUIRE_CHAIN(&res, kefir_ast_constant_expression_value_evaluate(mem, preprocessor->context->ast_context,
                                                                     expression, &expr_value));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_NODE_FREE(mem, expression);
        return res;
    });
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, expression));
    switch (expr_value.klass) {
        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER:
            *result = expr_value.integer != 0;
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT:
            *result = expr_value.floating_point != 0;
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT:
            *result = expr_value.complex_floating_point.real != 0 || expr_value.complex_floating_point.imaginary != 0;
            break;

        default:
            return KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location, "Unexpected constant expression type");
    }
    return KEFIR_OK;
}

static kefir_result_t process_if(struct kefir_mem *mem, struct kefir_preprocessor *preprocessor,
                                 struct kefir_preprocessor_directive *directive, struct kefir_list *condition_stack,
                                 struct kefir_token_allocator *token_allocator) {
    kefir_bool_t condition;
    REQUIRE_OK(evaluate_pp_tokens(mem, preprocessor, token_allocator, &directive->pp_tokens,
                                  &directive->source_location, &condition));
    if (!condition) {
        REQUIRE_OK(kefir_list_insert_after(mem, condition_stack, kefir_list_tail(condition_stack),
                                           (void *) (kefir_uptr_t) IF_CONDITION_FAIL));
        REQUIRE_OK(kefir_preprocessor_skip_group(mem, preprocessor));
    } else {
        REQUIRE_OK(kefir_list_insert_after(mem, condition_stack, kefir_list_tail(condition_stack),
                                           (void *) (kefir_uptr_t) IF_CONDITION_SUCCESS));
    }
    return KEFIR_OK;
}

static kefir_result_t process_elif(struct kefir_mem *mem, struct kefir_preprocessor *preprocessor,
                                   struct kefir_preprocessor_directive *directive, struct kefir_list *condition_stack,
                                   struct kefir_token_allocator *token_allocator) {
    struct kefir_list_entry *const top_condition = kefir_list_tail(condition_stack);
    if (top_condition == NULL) {
        return KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &directive->source_location, "Unexpected elif directive");
    } else if ((kefir_uptr_t) top_condition->value == IF_CONDITION_FAIL) {
        kefir_bool_t condition;
        REQUIRE_OK(evaluate_pp_tokens(mem, preprocessor, token_allocator, &directive->pp_tokens,
                                      &directive->source_location, &condition));
        if (condition) {
            top_condition->value = (void *) (kefir_uptr_t) IF_CONDITION_SUCCESS;
        } else {
            REQUIRE_OK(kefir_preprocessor_skip_group(mem, preprocessor));
        }
    } else {
        REQUIRE_OK(kefir_preprocessor_skip_group(mem, preprocessor));
    }
    return KEFIR_OK;
}

static kefir_result_t process_error(struct kefir_mem *mem, struct kefir_preprocessor_directive *directive) {
    char *error_message;
    kefir_size_t error_length;
    REQUIRE_OK(kefir_preprocessor_format_string(mem, &error_message, &error_length, &directive->pp_tokens,
                                                KEFIR_PREPROCESSOR_WHITESPACE_FORMAT_SINGLE_SPACE));
    kefir_result_t res =
        KEFIR_SET_SOURCE_ERRORF(KEFIR_PREPROCESSOR_ERROR_DIRECTIVE, &directive->source_location, "%s", error_message);
    KEFIR_FREE(mem, error_message);
    return res;
}

static kefir_result_t process_line(struct kefir_mem *mem, struct kefir_preprocessor *preprocessor,
                                   struct kefir_token_allocator *token_allocator,
                                   struct kefir_preprocessor_directive *directive) {
    REQUIRE_OK(kefir_preprocessor_run_substitutions(mem, preprocessor, token_allocator, &directive->pp_tokens, NULL,
                                                    KEFIR_PREPROCESSOR_SUBSTITUTION_NORMAL));
    const kefir_size_t pp_tokens_length = kefir_token_buffer_length(&directive->pp_tokens);
    REQUIRE(pp_tokens_length > 0,
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &directive->source_location, "Expected line number"));
    const struct kefir_token *token = kefir_token_buffer_at(&directive->pp_tokens, 0);
    REQUIRE(token->klass == KEFIR_TOKEN_PP_NUMBER,
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &token->source_location, "Expected line number"));

    char *linenum_end;
    unsigned long line = strtoul(token->pp_number.number_literal, &linenum_end, 10);
    REQUIRE(linenum_end == token->pp_number.number_literal + strlen(token->pp_number.number_literal),
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &token->source_location,
                                   "Unable to parse line number as unsigned integer"));

    const char *source_file = NULL;
    for (kefir_size_t i = 1; i < pp_tokens_length && source_file == NULL; i++) {
        token = kefir_token_buffer_at(&directive->pp_tokens, i);
        if (token->klass != KEFIR_TOKEN_PP_WHITESPACE) {
            REQUIRE(token->klass == KEFIR_TOKEN_STRING_LITERAL &&
                        token->string_literal.type == KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE &&
                        token->string_literal.raw_literal,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &token->source_location, "Expected valid file name"));

            REQUIRE_OK(process_raw_string(mem, preprocessor->lexer.symbols, token, &source_file));
        }
    }

    preprocessor->lexer.cursor->location.line = line;
    if (source_file != NULL) {
        preprocessor->lexer.cursor->location.source = source_file;
    }
    return KEFIR_OK;
}

static kefir_result_t run_directive(struct kefir_mem *mem, struct kefir_preprocessor *preprocessor,
                                    struct kefir_token_allocator *token_allocator, struct kefir_token_buffer *buffer,
                                    struct kefir_preprocessor_directive *directive, struct kefir_list *condition_stack,
                                    kefir_preprocessor_token_destination_t *token_destination) {
    *token_destination = KEFIR_PREPROCESSOR_TOKEN_DESTINATION_NORMAL;
    switch (directive->type) {
        case KEFIR_PREPROCESSOR_DIRECTIVE_IFDEF: {
            const struct kefir_preprocessor_macro *macro = NULL;
            kefir_result_t res =
                preprocessor->macros->locate(preprocessor->macros, directive->ifdef_directive.identifier, &macro);
            if (res == KEFIR_NOT_FOUND) {
                REQUIRE_OK(kefir_list_insert_after(mem, condition_stack, kefir_list_tail(condition_stack),
                                                   (void *) (kefir_uptr_t) IF_CONDITION_FAIL));
                REQUIRE_OK(kefir_preprocessor_skip_group(mem, preprocessor));
            } else {
                REQUIRE_OK(res);
                REQUIRE_OK(kefir_list_insert_after(mem, condition_stack, kefir_list_tail(condition_stack),
                                                   (void *) (kefir_uptr_t) IF_CONDITION_SUCCESS));
            }
        } break;

        case KEFIR_PREPROCESSOR_DIRECTIVE_IFNDEF: {
            const struct kefir_preprocessor_macro *macro = NULL;
            kefir_result_t res =
                preprocessor->macros->locate(preprocessor->macros, directive->ifdef_directive.identifier, &macro);
            if (res == KEFIR_NOT_FOUND) {
                REQUIRE_OK(kefir_list_insert_after(mem, condition_stack, kefir_list_tail(condition_stack),
                                                   (void *) (kefir_uptr_t) IF_CONDITION_SUCCESS));
            } else {
                REQUIRE_OK(res);
                REQUIRE_OK(kefir_list_insert_after(mem, condition_stack, kefir_list_tail(condition_stack),
                                                   (void *) (kefir_uptr_t) IF_CONDITION_FAIL));
                REQUIRE_OK(kefir_preprocessor_skip_group(mem, preprocessor));
            }
        } break;

        case KEFIR_PREPROCESSOR_DIRECTIVE_ENDIF: {
            struct kefir_list_entry *const top_condition = kefir_list_tail(condition_stack);
            if (top_condition == NULL) {
                return KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &directive->source_location,
                                              "Unexpected endif directive");
            } else {
                REQUIRE_OK(kefir_list_pop(mem, condition_stack, kefir_list_tail(condition_stack)));
            }
        } break;

        case KEFIR_PREPROCESSOR_DIRECTIVE_ELSE: {
            struct kefir_list_entry *const top_condition = kefir_list_tail(condition_stack);
            if (top_condition == NULL) {
                return KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &directive->source_location,
                                              "Unexpected else directive");
            } else if ((kefir_uptr_t) top_condition->value == IF_CONDITION_FAIL) {
                top_condition->value = (void *) (kefir_uptr_t) IF_CONDITION_SUCCESS;
            } else {
                REQUIRE_OK(kefir_preprocessor_skip_group(mem, preprocessor));
            }
        } break;

        case KEFIR_PREPROCESSOR_DIRECTIVE_IF:
            REQUIRE_OK(process_if(mem, preprocessor, directive, condition_stack, token_allocator));
            break;

        case KEFIR_PREPROCESSOR_DIRECTIVE_ELIF:
            REQUIRE_OK(process_elif(mem, preprocessor, directive, condition_stack, token_allocator));
            break;

        case KEFIR_PREPROCESSOR_DIRECTIVE_INCLUDE:
        case KEFIR_PREPROCESSOR_DIRECTIVE_INCLUDE_NEXT:
            *token_destination = KEFIR_PREPROCESSOR_TOKEN_DESTINATION_VERBATIM;
            REQUIRE_OK(process_include(mem, preprocessor, token_allocator, buffer, directive));
            break;

        case KEFIR_PREPROCESSOR_DIRECTIVE_DEFINE:
            REQUIRE_OK(process_define(mem, preprocessor, directive));
            break;

        case KEFIR_PREPROCESSOR_DIRECTIVE_UNDEF:
            REQUIRE_OK(process_undef(mem, preprocessor, directive));
            break;

        case KEFIR_PREPROCESSOR_DIRECTIVE_ERROR:
            REQUIRE_OK(process_error(mem, directive));
            break;

        case KEFIR_PREPROCESSOR_DIRECTIVE_PRAGMA:
            // TODO Implement STDC pragmas
            break;

        case KEFIR_PREPROCESSOR_DIRECTIVE_LINE:
            REQUIRE_OK(process_line(mem, preprocessor, token_allocator, directive));
            break;

        case KEFIR_PREPROCESSOR_DIRECTIVE_EMPTY:
        case KEFIR_PREPROCESSOR_DIRECTIVE_NON:
            // Skip empty and unknown directives
            break;

        case KEFIR_PREPROCESSOR_DIRECTIVE_PP_TOKEN: {
            const struct kefir_token *allocated_token;
            REQUIRE_OK(kefir_token_allocator_emplace(mem, token_allocator, &directive->pp_token, &allocated_token));
            REQUIRE_OK(kefir_token_buffer_emplace(mem, buffer, allocated_token));
        } break;

        case KEFIR_PREPROCESSOR_DIRECTIVE_SENTINEL:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t insert_sentinel(struct kefir_mem *mem, struct kefir_preprocessor_directive *directive,
                                      struct kefir_token_allocator *token_allocator,
                                      struct kefir_token_buffer *buffer) {
    struct kefir_token *allocated_token;
    REQUIRE_OK(kefir_token_allocator_allocate_empty(mem, token_allocator, &allocated_token));
    REQUIRE_OK(kefir_token_new_sentinel(allocated_token));
    allocated_token->source_location = directive->source_location;
    REQUIRE_OK(kefir_token_buffer_emplace(mem, buffer, allocated_token));
    return KEFIR_OK;
}

struct next_buffer_for_token_seq_params {
    struct kefir_preprocessor *preprocessor;
    struct kefir_token_allocator *token_allocator;
    struct kefir_list condition_stack;
};

static kefir_result_t next_buffer_for_token_seq(struct kefir_mem *mem, struct kefir_preprocessor_token_sequence *seq,
                                                void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(seq != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token sequence"));
    ASSIGN_DECL_CAST(struct next_buffer_for_token_seq_params *, params, payload);
    REQUIRE(params != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token sequence source parameters"));

    struct kefir_preprocessor_directive directive;
    REQUIRE_OK(kefir_preprocessor_directive_scanner_next(mem, &params->preprocessor->directive_scanner,
                                                         params->token_allocator, &directive));
    kefir_result_t res;
    KEFIR_RUN_EXTENSION(&res, mem, params->preprocessor, on_next_directive, &directive);
    if (res == KEFIR_OK && directive.type != KEFIR_PREPROCESSOR_DIRECTIVE_SENTINEL) {
        struct kefir_token_buffer buffer;
        kefir_preprocessor_token_destination_t token_destination;
        REQUIRE_OK(kefir_token_buffer_init(&buffer));
        REQUIRE_CHAIN(&res, run_directive(mem, params->preprocessor, params->token_allocator, &buffer, &directive,
                                          &params->condition_stack, &token_destination));
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_token_buffer_free(mem, &buffer);
            kefir_preprocessor_directive_free(mem, &directive);
            return res;
        });
        res = kefir_preprocessor_directive_free(mem, &directive);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_token_buffer_free(mem, &buffer);
            kefir_preprocessor_directive_free(mem, &directive);
            return res;
        });
        res = kefir_preprocessor_token_sequence_push_front(mem, seq, &buffer, token_destination);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_token_buffer_free(mem, &buffer);
            kefir_preprocessor_directive_free(mem, &directive);
            return res;
        });
        res = kefir_token_buffer_free(mem, &buffer);
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_preprocessor_directive_free(mem, &directive);
        return res;
    });

    REQUIRE_OK(kefir_preprocessor_directive_free(mem, &directive));
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_run(struct kefir_mem *mem, struct kefir_preprocessor *preprocessor,
                                      struct kefir_token_allocator *token_allocator,
                                      struct kefir_token_buffer *buffer) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(preprocessor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid preprocessor"));
    REQUIRE(token_allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token allocator"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));

    kefir_result_t res;
    KEFIR_RUN_EXTENSION(&res, mem, preprocessor, before_run, token_allocator, buffer);
    REQUIRE_OK(res);

    struct next_buffer_for_token_seq_params seq_source_params = {.preprocessor = preprocessor,
                                                                 .token_allocator = token_allocator};
    struct kefir_preprocessor_token_sequence_source seq_source = {.next_buffer = next_buffer_for_token_seq,
                                                                  .payload = &seq_source_params};
    REQUIRE_OK(kefir_list_init(&seq_source_params.condition_stack));
    res = kefir_preprocessor_run_substitutions(mem, preprocessor, token_allocator, buffer, &seq_source,
                                               KEFIR_PREPROCESSOR_SUBSTITUTION_NORMAL);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &seq_source_params.condition_stack);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &seq_source_params.condition_stack));

    struct kefir_preprocessor_directive directive;
    REQUIRE_OK(
        kefir_preprocessor_directive_scanner_next(mem, &preprocessor->directive_scanner, token_allocator, &directive));
    REQUIRE_ELSE(directive.type == KEFIR_PREPROCESSOR_DIRECTIVE_SENTINEL, {
        kefir_preprocessor_directive_free(mem, &directive);
        return KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &directive.source_location,
                                      "Unexpected preprocessor directive/token");
    });
    res = insert_sentinel(mem, &directive, token_allocator, buffer);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_preprocessor_directive_free(mem, &directive);
        return res;
    });
    REQUIRE_OK(kefir_preprocessor_directive_free(mem, &directive));

    KEFIR_RUN_EXTENSION(&res, mem, preprocessor, after_run, token_allocator, buffer);
    REQUIRE_OK(res);
    return KEFIR_OK;
}
