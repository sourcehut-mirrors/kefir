#include "kefir/parser/rule_helpers.h"
#include "kefir/core/source_error.h"

static kefir_result_t scan_index(struct kefir_mem *mem, struct kefir_parser *parser,
                                 struct kefir_ast_initializer_designation **designation) {
    struct kefir_source_location location = *PARSER_TOKEN_LOCATION(parser, 0);
    REQUIRE_OK(PARSER_SHIFT(parser));
    struct kefir_ast_node_base *index = NULL, *range_end_index = NULL;
    kefir_result_t res;
    REQUIRE_MATCH_OK(
        &res, KEFIR_PARSER_RULE_APPLY(mem, parser, constant_expression, &index),
        KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0), "Expected constant expression"));
    if (PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_ELLIPSIS) &&
        parser->configuration->designator_subscript_ranges) {
        REQUIRE_OK(PARSER_SHIFT(parser));
        REQUIRE_MATCH_OK(&res, KEFIR_PARSER_RULE_APPLY(mem, parser, constant_expression, &range_end_index),
                         KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0),
                                                "Expected constant expression"));
    }
    REQUIRE_ELSE(PARSER_TOKEN_IS_RIGHT_BRACKET(parser, 0), {
        if (range_end_index != NULL) {
            KEFIR_AST_NODE_FREE(mem, range_end_index);
        }
        KEFIR_AST_NODE_FREE(mem, index);
        return KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0), "Expected right bracket");
    });
    res = PARSER_SHIFT(parser);
    REQUIRE_ELSE(res == KEFIR_OK, {
        if (range_end_index != NULL) {
            KEFIR_AST_NODE_FREE(mem, range_end_index);
        }
        KEFIR_AST_NODE_FREE(mem, index);
        return res;
    });

    struct kefir_ast_initializer_designation *new_designation =
        range_end_index == NULL
            ? kefir_ast_new_initializer_index_designation(mem, index, *designation)
            : kefir_ast_new_initializer_range_designation(mem, index, range_end_index, *designation);
    REQUIRE_ELSE(new_designation != NULL, {
        if (range_end_index != NULL) {
            KEFIR_AST_NODE_FREE(mem, range_end_index);
        }
        KEFIR_AST_NODE_FREE(mem, index);
        return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST initializer index designation");
    });
    new_designation->source_location = location;

    *designation = new_designation;
    return KEFIR_OK;
}

static kefir_result_t scan_member(struct kefir_mem *mem, struct kefir_parser *parser,
                                  struct kefir_ast_initializer_designation **designation) {
    struct kefir_source_location location = *PARSER_TOKEN_LOCATION(parser, 0);
    REQUIRE_OK(PARSER_SHIFT(parser));
    REQUIRE(PARSER_TOKEN_IS_IDENTIFIER(parser, 0),
            KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0), "Expected identifier"));
    const char *identifier = PARSER_CURSOR(parser, 0)->identifier;
    REQUIRE_OK(PARSER_SHIFT(parser));
    struct kefir_ast_initializer_designation *new_designation =
        kefir_ast_new_initializer_member_designation(mem, parser->symbols, identifier, *designation);
    REQUIRE(new_designation != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST initializer index designation"));
    new_designation->source_location = location;
    *designation = new_designation;
    return KEFIR_OK;
}

static kefir_result_t scan_field_colon(struct kefir_mem *mem, struct kefir_parser *parser,
                                       struct kefir_ast_initializer_designation **designation) {
    struct kefir_source_location location = *PARSER_TOKEN_LOCATION(parser, 0);
    const char *identifier = PARSER_CURSOR(parser, 0)->identifier;
    REQUIRE_OK(PARSER_SHIFT(parser));
    REQUIRE_OK(PARSER_SHIFT(parser));
    struct kefir_ast_initializer_designation *new_designation =
        kefir_ast_new_initializer_member_designation(mem, parser->symbols, identifier, *designation);
    REQUIRE(new_designation != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST initializer index designation"));
    new_designation->source_location = location;
    *designation = new_designation;
    return KEFIR_OK;
}

static kefir_result_t scan_designation(struct kefir_mem *mem, struct kefir_parser *parser, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parser != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct kefir_ast_initializer_designation **, designation_ptr, payload);
    REQUIRE(PARSER_TOKEN_IS_LEFT_BRACKET(parser, 0) || PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_DOT) ||
                (parser->configuration->designated_initializer_colons && PARSER_TOKEN_IS_IDENTIFIER(parser, 0) &&
                 PARSER_TOKEN_IS_PUNCTUATOR(parser, 1, KEFIR_PUNCTUATOR_COLON)),
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Cannot match initializer designation"));

    *designation_ptr = NULL;
    kefir_bool_t scan_designators = true;
    kefir_bool_t expect_equals_sign = true;
    kefir_result_t res = KEFIR_OK;
    if (parser->configuration->designated_initializer_colons && PARSER_TOKEN_IS_IDENTIFIER(parser, 0) &&
        PARSER_TOKEN_IS_PUNCTUATOR(parser, 1, KEFIR_PUNCTUATOR_COLON)) {
        res = scan_field_colon(mem, parser, designation_ptr);
        scan_designators = false;
        expect_equals_sign = false;
    }

    while (res == KEFIR_OK && scan_designators) {
        if (PARSER_TOKEN_IS_LEFT_BRACKET(parser, 0)) {
            res = scan_index(mem, parser, designation_ptr);
        } else if (PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_DOT)) {
            res = scan_member(mem, parser, designation_ptr);
        } else {
            scan_designators = false;
        }
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        if (*designation_ptr != NULL) {
            kefir_ast_initializer_designation_free(mem, *designation_ptr);
            *designation_ptr = NULL;
        }
        return res;
    });

    if (expect_equals_sign) {
        REQUIRE_ELSE(PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_ASSIGN), {
            kefir_ast_initializer_designation_free(mem, *designation_ptr);
            *designation_ptr = NULL;
            return KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0), "Expected equals sign");
        });
        kefir_result_t res = PARSER_SHIFT(parser);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_initializer_designation_free(mem, *designation_ptr);
            *designation_ptr = NULL;
            return res;
        });
    }
    return KEFIR_OK;
}

kefir_result_t kefir_parser_scan_initializer_designation(struct kefir_mem *mem, struct kefir_parser *parser,
                                                         struct kefir_ast_initializer_designation **designation) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parser != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser"));
    REQUIRE(designation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to initializer designation"));

    REQUIRE_OK(kefir_parser_try_invoke(mem, parser, scan_designation, designation));
    return KEFIR_OK;
}
