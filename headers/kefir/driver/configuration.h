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

#ifndef KEFIR_DRIVER_CONFIGURATION_H_
#define KEFIR_DRIVER_CONFIGURATION_H_

#include "kefir/core/list.h"
#include "kefir/core/string_pool.h"
#include "kefir/core/standard_version.h"
#include "kefir/driver/target.h"

typedef enum kefir_driver_assembler_target {
    KEFIR_DRIVER_ASSEMBLER_GAS_INTEL,
    KEFIR_DRIVER_ASSEMBLER_GAS_INTEL_PREFIX,
    KEFIR_DRIVER_ASSEMBLER_GAS_ATT,
    KEFIR_DRIVER_ASSEMBLER_YASM
} kefir_driver_assembler_target_t;

typedef struct kefir_driver_assembler_configuration {
    kefir_driver_assembler_target_t target;
    kefir_bool_t verbose;
    struct kefir_list arguments;
} kefir_driver_assembler_configuration_t;

kefir_result_t kefir_driver_assembler_configuration_init(struct kefir_driver_assembler_configuration *);
kefir_result_t kefir_driver_assembler_configuration_free(struct kefir_mem *,
                                                         struct kefir_driver_assembler_configuration *);
kefir_result_t kefir_driver_assembler_configuration_add_argument(struct kefir_mem *,
                                                                 struct kefir_driver_assembler_configuration *,
                                                                 const char *);

typedef struct kefir_driver_linker_configuration {
    struct kefir_list arguments;

    struct {
        kefir_bool_t static_linking;
        kefir_bool_t shared_linking;
        kefir_bool_t pie_linking;
        kefir_bool_t export_dynamic;
        kefir_bool_t link_start_files;
        kefir_bool_t link_default_libs;
        kefir_bool_t link_libc;
        kefir_bool_t link_atomics;
        kefir_bool_t verbose;
    } flags;
} kefir_driver_linker_configuration_t;

kefir_result_t kefir_driver_linker_configuration_init(struct kefir_driver_linker_configuration *);
kefir_result_t kefir_driver_linker_configuration_free(struct kefir_mem *, struct kefir_driver_linker_configuration *);
kefir_result_t kefir_driver_linker_configuration_add_argument(struct kefir_mem *,
                                                              struct kefir_driver_linker_configuration *, const char *);

typedef enum kefir_driver_stage {
    KEFIR_DRIVER_STAGE_PREPROCESS,
    KEFIR_DRIVER_STAGE_PREPROCESS_SAVE,
    KEFIR_DRIVER_STAGE_PRINT_TOKENS,
    KEFIR_DRIVER_STAGE_PRINT_AST,
    KEFIR_DRIVER_STAGE_PRINT_IR,
    KEFIR_DRIVER_STAGE_PRINT_OPT,
    KEFIR_DRIVER_STAGE_COMPILE,
    KEFIR_DRIVER_STAGE_ASSEMBLE,
    KEFIR_DRIVER_STAGE_LINK,
    KEFIR_DRIVER_STAGE_RUN
} kefir_driver_stage_t;

typedef enum kefir_driver_argument_type {
    KEFIR_DRIVER_ARGUMENT_INPUT_FILE_CODE,
    KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED,
    KEFIR_DRIVER_ARGUMENT_INPUT_FILE_ASSEMBLY,
    KEFIR_DRIVER_ARGUMENT_INPUT_FILE_ASSEMBLY_WITH_PREPROCESSING,
    KEFIR_DRIVER_ARGUMENT_INPUT_FILE_OBJECT,
    KEFIR_DRIVER_ARGUMENT_INPUT_FILE_LIBRARY,
    KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_LIBRARY,
    KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_PATH,
    KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_ENTRY_POINT,
    KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_STRIP,
    KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_RETAIN_RELOC,
    KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_UNDEFINED_SYMBOL,
    KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_EXTRA
} kefir_driver_argument_type_t;

typedef struct kefir_driver_argument {
    kefir_driver_argument_type_t type;
    const char *value;
} kefir_driver_argument_t;

typedef struct kefir_driver_definition {
    const char *name;
    const char *value;
} kefir_driver_definition_t;

typedef enum kefir_driver_frame_pointer_omission {
    KEFIR_DRIVER_FRAME_POINTER_OMISSION_ENABLE,
    KEFIR_DRIVER_FRAME_POINTER_OMISSION_DISABLE,
    KEFIR_DRIVER_FRAME_POINTER_OMISSION_UNSPECIFIED
} kefir_driver_frame_pointer_omission_t;

typedef enum kefir_driver_char_signedness {
    KEFIR_DRIVER_CHAR_SIGNEDNESS_DEFAULT,
    KEFIR_DRIVER_CHAR_SIGNED,
    KEFIR_DRIVER_CHAR_UNSIGNED
} kefir_driver_char_signedness_t;

typedef enum kefir_driver_tentative_definition_placement {
    KEFIR_DRIVER_TENTATIVE_DEFINITION_PLACEMENT_DEFAULT,
    KEFIR_DRIVER_TENTATIVE_DEFINITION_PLACEMENT_COMMON,
    KEFIR_DRIVER_TENTATIVE_DEFINITION_PLACEMENT_NO_COMMON
} kefir_driver_tentative_definition_placement_t;

typedef enum kefir_driver_symbol_visibility {
    KEFIR_DRIVER_SYMBOL_VISIBILITY_UNSET,
    KEFIR_DRIVER_SYMBOL_VISIBILITY_DEFAULT,
    KEFIR_DRIVER_SYMBOL_VISIBILITY_INTERNAL,
    KEFIR_DRIVER_SYMBOL_VISIBILITY_HIDDEN,
    KEFIR_DRIVER_SYMBOL_VISIBILITY_PROTECTED
} kefir_driver_symbol_visibility_t;

typedef struct kefir_driver_configuration {
    kefir_driver_stage_t stage;
    const char *output_file;
    struct kefir_list arguments;
    struct kefir_list assembler_arguments;
    struct kefir_list compiler_arguments;
    struct kefir_hashtree defines;
    struct kefir_list undefines;
    struct kefir_list include_directories;
    struct kefir_list quote_include_directories;
    struct kefir_list system_include_directories;
    struct kefir_list embed_directories;
    struct kefir_list after_include_directories;
    struct kefir_list include_files;
    struct kefir_driver_target target;
    kefir_c_language_standard_version_t standard_version;

    struct {
        kefir_int_t optimization_level;
        kefir_driver_char_signedness_t char_signedness;
        kefir_driver_tentative_definition_placement_t tentative_definition_placement;
        kefir_driver_symbol_visibility_t symbol_visibility;
    } compiler;

    struct {
        kefir_driver_assembler_target_t target;
    } assembler;

    struct {
        kefir_bool_t skip_preprocessor;
        kefir_bool_t restrictive_mode;
        kefir_bool_t static_linking;
        kefir_bool_t shared_linking;
        kefir_bool_t export_dynamic;
        kefir_bool_t position_independent_code;
        kefir_bool_t position_independent_executable;
        kefir_bool_t debug_info;
        kefir_driver_frame_pointer_omission_t omit_frame_pointer;
        kefir_bool_t link_start_files;
        kefir_bool_t link_default_libs;
        kefir_bool_t include_stdinc;
        kefir_bool_t link_libc;
        kefir_bool_t include_rtinc;
        kefir_bool_t enable_atomics;
        kefir_bool_t pthread;
        kefir_bool_t verbose;
        kefir_bool_t preprocessor_linemarkers;
    } flags;

    struct {
        kefir_bool_t output_dependencies;
        kefir_bool_t output_system_deps;
        kefir_bool_t add_phony_targets;
        const char *target_name;
        const char *output_filename;
    } dependency_output;

    struct {
        struct kefir_list args;
        const char *file_stdin;
        const char *file_stdout;
        kefir_bool_t stderr_to_stdout;
        const char *file_stderr;
    } run;
} kefir_driver_configuration_t;

kefir_result_t kefir_driver_configuration_init(struct kefir_driver_configuration *);
kefir_result_t kefir_driver_configuration_free(struct kefir_mem *, struct kefir_driver_configuration *);

kefir_result_t kefir_driver_configuration_add_argument(struct kefir_mem *, struct kefir_string_pool *,
                                                       struct kefir_driver_configuration *, const char *,
                                                       kefir_driver_argument_type_t);
kefir_result_t kefir_driver_configuration_add_assembler_argument(struct kefir_mem *, struct kefir_string_pool *,
                                                                 struct kefir_driver_configuration *, const char *);
kefir_result_t kefir_driver_configuration_add_compiler_argument(struct kefir_mem *, struct kefir_string_pool *,
                                                                struct kefir_driver_configuration *, const char *);
kefir_result_t kefir_driver_configuration_add_define(struct kefir_mem *, struct kefir_string_pool *,
                                                     struct kefir_driver_configuration *, const char *, const char *);
kefir_result_t kefir_driver_configuration_add_undefine(struct kefir_mem *, struct kefir_string_pool *,
                                                       struct kefir_driver_configuration *, const char *);
kefir_result_t kefir_driver_configuration_add_include_directory(struct kefir_mem *, struct kefir_string_pool *,
                                                                struct kefir_driver_configuration *, const char *);
kefir_result_t kefir_driver_configuration_add_system_include_directory(struct kefir_mem *, struct kefir_string_pool *,
                                                                       struct kefir_driver_configuration *,
                                                                       const char *);
kefir_result_t kefir_driver_configuration_add_quote_include_directory(struct kefir_mem *, struct kefir_string_pool *,
                                                                      struct kefir_driver_configuration *,
                                                                      const char *);
kefir_result_t kefir_driver_configuration_add_after_include_directory(struct kefir_mem *, struct kefir_string_pool *,
                                                                      struct kefir_driver_configuration *,
                                                                      const char *);
kefir_result_t kefir_driver_configuration_add_embed_directory(struct kefir_mem *, struct kefir_string_pool *,
                                                              struct kefir_driver_configuration *, const char *);
kefir_result_t kefir_driver_configuration_add_include_file(struct kefir_mem *, struct kefir_string_pool *,
                                                           struct kefir_driver_configuration *, const char *);

#endif
