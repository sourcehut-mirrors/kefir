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

#ifndef KEFIR_PARSER_PARSER_H_
#define KEFIR_PARSER_PARSER_H_

#include "kefir/core/mem.h"
#include "kefir/parser/cursor.h"
#include "kefir/parser/scope.h"
#include "kefir/parser/ruleset.h"
#include "kefir/parser/pragma.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/ast/node.h"

typedef struct kefir_parser kefir_parser_t;

typedef struct kefir_parser_configuration {
    kefir_bool_t fail_on_attributes;
    kefir_bool_t implicit_function_definition_int;
    kefir_bool_t designated_initializer_colons;
    kefir_bool_t label_addressing;
    kefir_bool_t statement_expressions;
    kefir_bool_t omitted_conditional_operand;
    kefir_bool_t fail_on_assembly;
    kefir_bool_t switch_case_ranges;
    kefir_bool_t designator_subscript_ranges;
    kefir_uint32_t max_errors;
} kefir_parser_configuration_t;

kefir_result_t kefir_parser_configuration_default(struct kefir_parser_configuration *);

typedef struct kefir_parser_extensions {
    kefir_result_t (*on_init)(struct kefir_mem *, struct kefir_parser *);
    kefir_result_t (*on_free)(struct kefir_mem *, struct kefir_parser *);
    void *payload;
} kefir_parser_extensions_t;

typedef struct kefir_parser {
    struct kefir_string_pool *symbols;
    struct kefir_parser_token_cursor *cursor;
    struct kefir_parser_ruleset ruleset;
    const struct kefir_parser_configuration *configuration;

    struct kefir_parser_scope local_scope;
    struct kefir_parser_scope *scope;
    struct kefir_parser_pragmas pragmas;
    kefir_uint32_t encountered_errors;

    const struct kefir_parser_extensions *extensions;
    void *extension_payload;
} kefir_parser_t;

typedef kefir_result_t (*kefir_parser_rule_fn_t)(struct kefir_mem *, struct kefir_parser *,
                                                 struct kefir_ast_node_base **, void *);
typedef kefir_result_t (*kefir_parser_invocable_fn_t)(struct kefir_mem *, struct kefir_parser *, void *);

kefir_result_t kefir_parser_init(struct kefir_mem *, struct kefir_parser *, struct kefir_string_pool *,
                                 struct kefir_parser_token_cursor *, const struct kefir_parser_extensions *);
kefir_result_t kefir_parser_free(struct kefir_mem *, struct kefir_parser *);

kefir_result_t kefir_parser_set_scope(struct kefir_parser *, struct kefir_parser_scope *);

typedef struct kefir_parser_checkpoint {
    kefir_size_t cursor;
    struct kefir_ast_pragma_state pragmas_file_scope;
    kefir_bool_t pragmas_in_function_scope;
    kefir_size_t pack_stack_top;
} kefir_parser_checkpoint_t;

kefir_result_t kefir_parser_checkpoint_save(const struct kefir_parser *, struct kefir_parser_checkpoint *);
kefir_result_t kefir_parser_checkpoint_restore(struct kefir_parser *, const struct kefir_parser_checkpoint *);
kefir_result_t kefir_parser_consume_pack_pragmas(struct kefir_mem *, struct kefir_parser *);

kefir_result_t kefir_parser_try_invoke(struct kefir_mem *, struct kefir_parser *, kefir_parser_invocable_fn_t, void *);
kefir_result_t kefir_parser_apply(struct kefir_mem *, struct kefir_parser *, struct kefir_ast_node_base **,
                                  kefir_parser_rule_fn_t, void *);

static inline kefir_result_t kefir_parser_try_invoke_impl(struct kefir_mem *mem, struct kefir_parser *parser,
                                                          kefir_parser_invocable_fn_t function, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parser != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser invocable"));

    struct kefir_parser_checkpoint checkpoint;
    REQUIRE_OK(kefir_parser_checkpoint_save(parser, &checkpoint));
    REQUIRE_OK(kefir_parser_consume_pack_pragmas(mem, parser));
    kefir_result_t res = function(mem, parser, payload);
    REQUIRE_CHAIN(&res, kefir_parser_consume_pack_pragmas(mem, parser));
    if (res == KEFIR_NO_MATCH) {
        REQUIRE_OK(kefir_parser_checkpoint_restore(parser, &checkpoint));
        return res;
    } else {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static inline kefir_result_t kefir_parser_apply_impl(struct kefir_mem *mem, struct kefir_parser *parser,
                                                     struct kefir_ast_node_base **result, kefir_parser_rule_fn_t rule,
                                                     void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parser != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST node"));
    REQUIRE(rule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser rule"));

    struct kefir_parser_checkpoint checkpoint;
    REQUIRE_OK(kefir_parser_checkpoint_save(parser, &checkpoint));
    struct kefir_source_location source_location =
        kefir_parser_token_cursor_at(parser->cursor, 0, true)->source_location;
    REQUIRE_OK(kefir_parser_consume_pack_pragmas(mem, parser));
    kefir_result_t res = rule(mem, parser, result, payload);
    if (parser->cursor->failure_res != KEFIR_OK) {
        for (const struct kefir_error *err = kefir_current_error(); err != NULL && err->code == KEFIR_SYNTAX_ERROR;
             err = kefir_current_error()) {
            kefir_pop_error(KEFIR_SYNTAX_ERROR);
        }
        return parser->cursor->failure_res;
    }
    REQUIRE_CHAIN(&res, kefir_parser_consume_pack_pragmas(mem, parser));
    if (res == KEFIR_NO_MATCH) {
        REQUIRE_OK(kefir_parser_checkpoint_restore(parser, &checkpoint));
        return res;
    } else {
        REQUIRE_OK(res);
        if (*result != NULL) {
            (*result)->source_location = source_location;
        }
    }
    return KEFIR_OK;
}

#define KEFIR_PARSER_RULE_FN_PREFIX(_id) kefir_parser_apply_impl_rule_##_id
#define KEFIR_PARSER_RULE_FN(_parser, _rule) ((_parser)->ruleset.rules[KEFIR_PARSER_RULESET_IDENTIFIER(_rule)])
#ifndef KEFIR_EXTENSION_SUPPORT
#define KEFIR_PARSER_RULE_APPLY(_mem, _parser, _rule, _result) \
    (kefir_parser_apply_impl((_mem), (_parser), (_result), KEFIR_PARSER_RULE_FN_PREFIX(_rule), NULL))
#else
#define KEFIR_PARSER_RULE_APPLY(_mem, _parser, _rule, _result) \
    (kefir_parser_apply_impl((_mem), (_parser), (_result), KEFIR_PARSER_RULE_FN(_parser, _rule), NULL))
#endif
#define KEFIR_PARSER_NEXT_EXPRESSION(_mem, _parser, _result) \
    KEFIR_PARSER_RULE_APPLY((_mem), (_parser), expression, (_result))
#define KEFIR_PARSER_NEXT_DECLARATION_LIST(_mem, _parser, _result) \
    KEFIR_PARSER_RULE_APPLY((_mem), (_parser), declaration, (_result))
#define KEFIR_PARSER_NEXT_STATEMENT(_mem, _parser, _result) \
    KEFIR_PARSER_RULE_APPLY((_mem), (_parser), statement, (_result))
#define KEFIR_PARSER_NEXT_TRANSLATION_UNIT(_mem, _parser, _result) \
    KEFIR_PARSER_RULE_APPLY((_mem), (_parser), translation_unit, (_result))

#define KEFIR_PARSER_DO_ERROR_RECOVERY(_parser)                        \
    ((_parser)->configuration->max_errors == ((kefir_uint32_t) - 1) || \
     (_parser)->encountered_errors + 1 < (_parser)->configuration->max_errors)

#endif
