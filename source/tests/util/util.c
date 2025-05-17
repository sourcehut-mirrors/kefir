/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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

#include "kefir/test/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/target/abi/amd64/platform.h"
#include <float.h>

static kefir_bool_t init_done = false;
static struct kefir_ir_target_platform IR_TARGET;
static struct kefir_ast_translator_environment TRANSLATOR_ENV;

struct kefir_ir_target_platform *kft_util_get_ir_target_platform(void) {
    if (!init_done) {
        REQUIRE(kefir_abi_amd64_target_platform(KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &IR_TARGET) == KEFIR_OK, NULL);
        REQUIRE(kefir_ast_translator_environment_init(&TRANSLATOR_ENV, &IR_TARGET) == KEFIR_OK, NULL);
        init_done = true;
    }
    return &IR_TARGET;
}

struct kefir_ast_translator_environment *kft_util_get_translator_environment(void) {
    if (!init_done) {
        REQUIRE(kefir_abi_amd64_target_platform(KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &IR_TARGET) == KEFIR_OK, NULL);
        REQUIRE(kefir_ast_translator_environment_init(&TRANSLATOR_ENV, &IR_TARGET) == KEFIR_OK, NULL);
        init_done = true;
    }
    return &TRANSLATOR_ENV;
}

const struct kefir_data_model_descriptor *kefir_util_default_data_model(void) {
    static const struct kefir_data_model_descriptor DATA_MODEL_DESCRIPTOR = {
        .model = KEFIR_DATA_MODEL_LP64,
        .byte_order = KEFIR_BYTE_ORDER_LITTLE_ENDIAN,
        .scalar_width = {.bool_bits = 8, .char_bits = 8,
                         .short_bits = 16,
                         .int_bits = 32,
                         .long_bits = 64,
                         .long_long_bits = 64,
                         .float_bits = 32,
                         .double_bits = 64,
                         .long_double_bits = 128}};
    return &DATA_MODEL_DESCRIPTOR;
}

const struct kefir_ast_type_traits *kefir_util_default_type_traits(void) {
    static struct kefir_ast_type_traits DEFAULT_TYPE_TRAITS;
    static kefir_bool_t DEFAULT_TYPE_TRAITS_INIT_DONE = false;
    if (!DEFAULT_TYPE_TRAITS_INIT_DONE) {
        kefir_ast_type_traits_init(kefir_util_default_data_model(), &DEFAULT_TYPE_TRAITS);
        DEFAULT_TYPE_TRAITS_INIT_DONE = true;
    }
    return &DEFAULT_TYPE_TRAITS;
}

kefir_result_t kefir_ast_context_manager_init(struct kefir_ast_global_context *global_context,
                                              struct kefir_ast_context_manager *context_mgr) {
    REQUIRE(global_context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST global context"));
    REQUIRE(context_mgr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context manager"));

    context_mgr->global = global_context;
    context_mgr->local = NULL;
    context_mgr->current = &global_context->context;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_context_manager_attach_local(struct kefir_ast_local_context *local_context,
                                                      struct kefir_ast_context_manager *context_mgr) {
    REQUIRE(local_context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST local context"));
    REQUIRE(context_mgr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context manager"));
    REQUIRE(context_mgr->local == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Context manager already has attached local context"));

    context_mgr->local = local_context;
    context_mgr->current = &local_context->context;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_context_manager_detach_local(struct kefir_ast_context_manager *context_mgr) {
    REQUIRE(context_mgr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context manager"));
    REQUIRE(context_mgr->local != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Context manager has no attached local context"));

    context_mgr->local = NULL;
    context_mgr->current = &context_mgr->global->context;
    return KEFIR_OK;
}
