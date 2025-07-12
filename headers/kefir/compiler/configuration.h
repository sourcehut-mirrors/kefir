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

#ifndef KEFIR_COMPILER_CONFIGURATION_H_
#define KEFIR_COMPILER_CONFIGURATION_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"
#include "kefir/core/list.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/hashtreeset.h"
#include "kefir/compiler/profile.h"
#include <time.h>

typedef enum kefir_compiler_runner_action {
    KEFIR_COMPILER_RUNNER_ACTION_PREPROCESS,
    KEFIR_COMPILER_RUNNER_ACTION_DUMP_TOKENS,
    KEFIR_COMPILER_RUNNER_ACTION_DUMP_AST,
    KEFIR_COMPILER_RUNNER_ACTION_DUMP_IR,
    KEFIR_COMPILER_RUNNER_ACTION_DUMP_OPT,
    KEFIR_COMPILER_RUNNER_ACTION_DUMP_ASSEMBLY
} kefir_compiler_runner_action_t;

typedef enum kefir_compiler_runner_error_report_type {
    KEFIR_COMPILER_RUNNER_ERROR_REPORT_TABULAR,
    KEFIR_COMPILER_RUNNER_ERROR_REPORT_JSON
} kefir_compiler_runner_error_report_type_t;

typedef struct kefir_compiler_runner_configuration {
    kefir_compiler_runner_action_t action;
    const char *input_filepath;
    const char *output_filepath;
    const char *target_profile;
    const char *source_id;
    kefir_compiler_runner_error_report_type_t error_report_type;
    kefir_bool_t debug_info;
    kefir_bool_t skip_preprocessor;
    struct kefir_list include_path;
    struct kefir_list embed_path;
    struct kefir_hashtreeset quote_include_directories;
    struct kefir_hashtreeset system_include_directories;
    struct kefir_list include_files;
    struct kefir_hashtree defines;
    struct kefir_list undefines;
    time_t pp_timestamp;
    kefir_bool_t default_pp_timestamp;
    const char *optimizer_pipeline_spec;
    kefir_bool_t verbose;
    kefir_bool_t preprocessor_assembly_mode;

    struct kefir_compiler_profile_configuration target_profile_config;

    const char *extension_lib;

    struct {
        const char *target_name;
        kefir_bool_t output_dependencies;
        kefir_bool_t output_system_deps;
        kefir_bool_t add_phony_targets;
        const char *output_filename;
    } dependency_output;

    struct {
        kefir_size_t max_inline_depth;
        kefir_size_t max_inlines_per_function;
    } optimizer;

    struct {
        kefir_bool_t fail_on_attributes;
        kefir_bool_t missing_function_return_type;
        kefir_bool_t designated_initializer_colons;
        kefir_bool_t labels_as_values;
        kefir_bool_t non_strict_qualifiers;
        kefir_bool_t signed_enum_type;
        kefir_bool_t implicit_function_declaration;
        kefir_bool_t empty_structs;
        kefir_bool_t ext_pointer_arithmetics;
        kefir_bool_t missing_braces_subobject;
        kefir_bool_t statement_expressions;
        kefir_bool_t omitted_conditional_operand;
        kefir_bool_t int_to_pointer;
        kefir_bool_t permissive_pointer_conv;
        kefir_bool_t named_macro_vararg;
        kefir_bool_t include_next;
        kefir_bool_t fail_on_assembly;
        kefir_bool_t va_args_concat;
        kefir_bool_t precise_bitfield_load_store;
        kefir_bool_t declare_atomic_support;
        kefir_bool_t switch_case_ranges;
        kefir_bool_t designator_subscript_ranges;
    } features;

    struct {
        kefir_bool_t emulated_tls;
        kefir_bool_t position_independent_code;
        kefir_bool_t omit_frame_pointer;
        kefir_bool_t valgrind_compatible_x87;
        const char *syntax;
        const char *print_details;
        const char *pipeline_spec;
    } codegen;
} kefir_compiler_runner_configuration_t;

kefir_result_t kefir_compiler_runner_configuration_init(struct kefir_compiler_runner_configuration *);
kefir_result_t kefir_compiler_runner_configuration_free(struct kefir_mem *,
                                                        struct kefir_compiler_runner_configuration *);
kefir_result_t kefir_compiler_runner_configuration_define(struct kefir_mem *,
                                                          struct kefir_compiler_runner_configuration *, const char *,
                                                          const char *);
#endif
