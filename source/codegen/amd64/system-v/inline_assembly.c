#include "kefir/codegen/amd64/system-v/inline_assembly.h"
#include "kefir/codegen/amd64/system-v/runtime.h"
#include "kefir/target/abi/system-v-amd64/qwords.h"
#include "kefir/codegen/amd64/system-v/abi.h"
#include "kefir/codegen/amd64/labels.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/string_builder.h"
#include <string.h>

kefir_result_t kefir_codegen_amd64_sysv_inline_assembly_invoke(struct kefir_mem *mem,
                                                               struct kefir_codegen_amd64_sysv_module *sysv_module,
                                                               struct kefir_codegen_amd64 *codegen,
                                                               const struct kefir_ir_inline_assembly *inline_asm) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(sysv_module != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen AMD64 System-V module"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen AMD64 codegen"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly"));

    if (!kefir_hashtree_has(&sysv_module->inline_assembly, inline_asm->id)) {
        REQUIRE_OK(kefir_hashtree_insert(mem, &sysv_module->inline_assembly, (kefir_hashtree_key_t) inline_asm->id,
                                         (kefir_hashtree_value_t) inline_asm));
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
        kefir_asm_amd64_xasmgen_operand_label(
            &codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_helpers_format(
                &codegen->xasmgen_helpers, KEFIR_AMD64_SYSTEM_V_RUNTIME_INLINE_ASSEMBLY_FRAGMENT, inline_asm->id)),
        kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], 0)));
    return KEFIR_OK;
}

struct inline_assembly_params {
    struct kefir_hashtree dirty_regs;
    kefir_bool_t dirty_cc;
    struct kefir_list available_int_regs;
    struct kefir_list preserved_regs;
    struct kefir_hashtree parameter_mapping;
    struct kefir_list register_aggregate_params;
    kefir_size_t num_of_inputs;
    kefir_size_t parameter_base_offset;
    struct kefir_string_builder formatted_asm;
};

enum inline_assembly_param_map_type {
    INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT,
    INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT,
    INLINE_ASSEMBLY_PARAM_STACK
};

enum inline_assembly_param_type { INLINE_ASSEMBLY_PARAM_NORMAL_SCALAR, INLINE_ASSEMBLY_PARAM_AGGREGATE };

struct inline_assembly_param_map {
    enum inline_assembly_param_map_type type;
    kefir_asm_amd64_xasmgen_register_t reg;
    kefir_int64_t stack_index;
    kefir_bool_t direct_read;
    kefir_size_t input_index;

    struct {
        enum inline_assembly_param_type type;
        kefir_size_t size;
    } parameter_props;
    struct {
        enum inline_assembly_param_type type;
        kefir_size_t size;
    } parameter_read_props;
};

static kefir_bool_t inline_assembly_is_register_preserved(kefir_asm_amd64_xasmgen_register_t reg) {
    switch (reg) {
        case KEFIR_AMD64_SYSV_ABI_PROGRAM_REG:
        case KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG:
            return true;

        default:
            return false;
    }
}

static kefir_result_t mark_clobbers(struct kefir_mem *mem, const struct kefir_ir_inline_assembly *inline_asm,
                                    struct inline_assembly_params *params) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->clobbers, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, clobber, node->key);

        if (strcmp(clobber, "cc") == 0) {
            params->dirty_cc = true;
            continue;
        }

        kefir_asm_amd64_xasmgen_register_t dirty_reg;
        kefir_result_t res = kefir_asm_amd64_xasmgen_register_from_symbolic_name(clobber, &dirty_reg);
        if (res == KEFIR_NOT_FOUND) {
            // Ignore unknown clobbers
            continue;
        }
        REQUIRE_OK(res);
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(dirty_reg, &dirty_reg));

        res = kefir_hashtree_insert(mem, &params->dirty_regs, (kefir_hashtree_key_t) dirty_reg,
                                    (kefir_hashtree_value_t) 0);
        if (res != KEFIR_ALREADY_EXISTS) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t init_available_regs(struct kefir_mem *mem, struct inline_assembly_params *params) {
#define AVAIL_REG(_list, _reg)                                                                            \
    do {                                                                                                  \
        if (!kefir_hashtree_has(&params->dirty_regs, (_reg))) {                                           \
            REQUIRE_OK(kefir_list_insert_after(mem, (_list), kefir_list_tail((_list)), (void *) (_reg))); \
        }                                                                                                 \
    } while (0)
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_RAX);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_RCX);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_RDX);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_RSI);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_RDI);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_R8);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_R9);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_R10);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_R11);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_R12);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_R13);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_R15);
    // Used for special purposes by runtime, thus least priority
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_R14);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_RBX);
#undef AVAIL_REG
    return KEFIR_OK;
}

static kefir_result_t insert_param_map(struct kefir_mem *mem, struct inline_assembly_params *params, kefir_id_t key,
                                       enum inline_assembly_param_map_type type, kefir_asm_amd64_xasmgen_register_t reg,
                                       kefir_int64_t stack_index, struct inline_assembly_param_map **map_ptr) {
    struct inline_assembly_param_map *map = KEFIR_MALLOC(mem, sizeof(struct inline_assembly_param_map));
    REQUIRE(map != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR inline assembly parameter mapping"));

    map->type = type;
    switch (type) {
        case INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT:
        case INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT:
            map->reg = reg;
            break;

        case INLINE_ASSEMBLY_PARAM_STACK:
            map->stack_index = stack_index;
            break;
    }

    kefir_result_t res = kefir_hashtree_insert(mem, &params->parameter_mapping, (kefir_hashtree_key_t) key,
                                               (kefir_hashtree_value_t) map);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, map);
        return res;
    });

    ASSIGN_PTR(map_ptr, map);
    return KEFIR_OK;
}

static kefir_result_t process_parameter_type(struct kefir_mem *mem, const struct kefir_ir_type *ir_type,
                                             kefir_size_t ir_type_idx, enum inline_assembly_param_type *param_type,
                                             kefir_size_t *parameter_size) {
    const struct kefir_ir_typeentry *param_typeentry = kefir_ir_type_at(ir_type, ir_type_idx);
    REQUIRE(param_typeentry != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain IR inline assembly parameter type"));

    struct kefir_abi_sysv_amd64_type_layout layouts;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout(ir_type, mem, &layouts));
    const struct kefir_abi_sysv_amd64_typeentry_layout *layout = NULL;
    kefir_result_t res = kefir_abi_sysv_amd64_type_layout_at(&layouts, ir_type_idx, &layout);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_sysv_amd64_type_layout_free(mem, &layouts);
        return res;
    });
    *parameter_size = layout->size;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_free(mem, &layouts));

    switch (param_typeentry->typecode) {
        case KEFIR_IR_TYPE_BOOL:
        case KEFIR_IR_TYPE_CHAR:
        case KEFIR_IR_TYPE_INT8:
        case KEFIR_IR_TYPE_SHORT:
        case KEFIR_IR_TYPE_INT16:
        case KEFIR_IR_TYPE_INT:
        case KEFIR_IR_TYPE_INT32:
        case KEFIR_IR_TYPE_FLOAT32:
        case KEFIR_IR_TYPE_INT64:
        case KEFIR_IR_TYPE_LONG:
        case KEFIR_IR_TYPE_WORD:
        case KEFIR_IR_TYPE_FLOAT64:
            *param_type = INLINE_ASSEMBLY_PARAM_NORMAL_SCALAR;
            break;

        case KEFIR_IR_TYPE_LONG_DOUBLE:
        case KEFIR_IR_TYPE_STRUCT:
        case KEFIR_IR_TYPE_ARRAY:
        case KEFIR_IR_TYPE_UNION:
            *param_type = INLINE_ASSEMBLY_PARAM_AGGREGATE;
            break;

        case KEFIR_IR_TYPE_BITS:
        case KEFIR_IR_TYPE_BUILTIN:
        case KEFIR_IR_TYPE_COUNT:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter type");
    }

    return KEFIR_OK;
}

static kefir_result_t map_parameter_to_register(struct kefir_mem *mem, struct inline_assembly_params *params,
                                                const struct kefir_ir_inline_assembly_parameter *asm_param,
                                                struct inline_assembly_param_map **param_map,
                                                enum inline_assembly_param_type param_type) {
    kefir_asm_amd64_xasmgen_register_t reg;
    REQUIRE(kefir_list_length(&params->available_int_regs) > 0,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to satisfy IR inline assembly constraints"));
    reg = (kefir_asm_amd64_xasmgen_register_t) ((kefir_uptr_t) kefir_list_head(&params->available_int_regs)->value);
    REQUIRE_OK(kefir_list_pop(mem, &params->available_int_regs, kefir_list_head(&params->available_int_regs)));
    REQUIRE_OK(insert_param_map(mem, params, asm_param->parameter_id, INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT, reg, 0,
                                param_map));
    REQUIRE_OK(kefir_hashtree_insert(mem, &params->dirty_regs, (kefir_hashtree_key_t) reg, (kefir_hashtree_value_t) 0));
    if (param_type == INLINE_ASSEMBLY_PARAM_AGGREGATE) {
        REQUIRE_OK(kefir_list_insert_after(mem, &params->register_aggregate_params,
                                           kefir_list_tail(&params->register_aggregate_params), (void *) asm_param));
    }

    return KEFIR_OK;
}

static kefir_result_t map_parameter_to_memory(struct kefir_mem *mem, struct inline_assembly_params *params,
                                              const struct kefir_ir_inline_assembly_parameter *asm_param,
                                              struct inline_assembly_param_map **param_map,
                                              enum inline_assembly_param_type param_type) {
    if (param_type == INLINE_ASSEMBLY_PARAM_AGGREGATE || asm_param->klass != KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ) {
        kefir_asm_amd64_xasmgen_register_t reg;
        REQUIRE(kefir_list_length(&params->available_int_regs) > 0,
                KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to satisfy IR inline assembly constraints"));
        reg = (kefir_asm_amd64_xasmgen_register_t) ((kefir_uptr_t) kefir_list_head(&params->available_int_regs)->value);
        REQUIRE_OK(kefir_list_pop(mem, &params->available_int_regs, kefir_list_head(&params->available_int_regs)));
        REQUIRE_OK(insert_param_map(mem, params, asm_param->parameter_id, INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT, reg,
                                    0, param_map));
        REQUIRE_OK(
            kefir_hashtree_insert(mem, &params->dirty_regs, (kefir_hashtree_key_t) reg, (kefir_hashtree_value_t) 0));
    } else {
        REQUIRE_OK(insert_param_map(mem, params, asm_param->parameter_id, INLINE_ASSEMBLY_PARAM_STACK, 0,
                                    asm_param->load_store_index, param_map));
    }

    return KEFIR_OK;
}

static kefir_result_t map_parameters(struct kefir_mem *mem, const struct kefir_ir_inline_assembly *inline_asm,
                                     struct inline_assembly_params *params) {

    for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->parameter_list); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, asm_param, iter->value);

        enum inline_assembly_param_type param_read_type = INLINE_ASSEMBLY_PARAM_NORMAL_SCALAR,
                                        param_type = INLINE_ASSEMBLY_PARAM_NORMAL_SCALAR;
        kefir_size_t param_read_size = 0, param_size = 0;

        kefir_bool_t parameter_immediate = false;
        kefir_bool_t direct_input = false;
        kefir_size_t input_index = 0;
        switch (asm_param->klass) {
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE:
                params->num_of_inputs = MAX(params->num_of_inputs, asm_param->load_store_index + 1);
                input_index = asm_param->load_store_index;
                REQUIRE_OK(
                    process_parameter_type(mem, asm_param->type.type, asm_param->type.index, &param_type, &param_size));
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE:
                REQUIRE_OK(
                    process_parameter_type(mem, asm_param->type.type, asm_param->type.index, &param_type, &param_size));
                REQUIRE_OK(process_parameter_type(mem, asm_param->read_type.type, asm_param->read_type.index,
                                                  &param_read_type, &param_read_size));
                params->num_of_inputs = MAX(params->num_of_inputs, asm_param->load_store_index + 1);
                params->num_of_inputs = MAX(params->num_of_inputs, asm_param->read_index + 1);
                direct_input = true;
                input_index = asm_param->read_index;
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ:
                direct_input = true;
                input_index = asm_param->read_index;
                REQUIRE_OK(
                    process_parameter_type(mem, asm_param->type.type, asm_param->type.index, &param_type, &param_size));
                params->num_of_inputs = MAX(params->num_of_inputs, asm_param->read_index + 1);
                param_read_type = param_type;
                param_read_size = param_size;
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE:
                parameter_immediate = true;
                REQUIRE_OK(
                    process_parameter_type(mem, asm_param->type.type, asm_param->type.index, &param_type, &param_size));
                break;
        }

        if (!parameter_immediate) {
            struct inline_assembly_param_map *param_map = NULL;
            switch (asm_param->constraint) {
                case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_NONE:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unexpected IR inline assembly parameter constraint");

                case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER:
                    REQUIRE(param_type != INLINE_ASSEMBLY_PARAM_AGGREGATE || param_size <= KEFIR_AMD64_SYSV_ABI_QWORD,
                            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to satisfy IR inline assembly constraints"));
                    REQUIRE_OK(map_parameter_to_register(mem, params, asm_param, &param_map, param_type));
                    break;

                case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER_MEMORY:
                    if ((param_type == INLINE_ASSEMBLY_PARAM_NORMAL_SCALAR ||
                         param_size <= KEFIR_AMD64_SYSV_ABI_QWORD) &&
                        kefir_list_length(&params->available_int_regs) > 0) {
                        REQUIRE_OK(map_parameter_to_register(mem, params, asm_param, &param_map, param_type));
                    } else {
                        REQUIRE_OK(map_parameter_to_memory(mem, params, asm_param, &param_map, param_type));
                    }
                    break;

                case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_MEMORY:
                    REQUIRE_OK(map_parameter_to_memory(mem, params, asm_param, &param_map, param_type));
                    break;
            }

            param_map->parameter_props.size = param_size;
            param_map->parameter_props.type = param_type;
            param_map->direct_read = direct_input;
            param_map->input_index = input_index;
            if (asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE ||
                asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ) {
                param_map->parameter_read_props.type = param_read_type;
                param_map->parameter_read_props.size = param_read_size;
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t preserve_dirty_regs(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                          struct inline_assembly_params *params) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&params->dirty_regs, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t, reg, node->key);

        if (inline_assembly_is_register_preserved(reg)) {
            REQUIRE_OK(kefir_list_insert_after(mem, &params->preserved_regs, kefir_list_tail(&params->preserved_regs),
                                               (void *) reg));

            if (kefir_asm_amd64_xasmgen_register_is_floating_point(reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                                         2 * KEFIR_AMD64_SYSV_ABI_QWORD)));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVDQU(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0),
                    kefir_asm_amd64_xasmgen_operand_reg(reg)));
                params->parameter_base_offset += 2 * KEFIR_AMD64_SYSV_ABI_QWORD;
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg)));
                params->parameter_base_offset += KEFIR_AMD64_SYSV_ABI_QWORD;
            }
        }
    }

    if (params->dirty_cc) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSHFQ(&codegen->xasmgen));
        params->parameter_base_offset += KEFIR_AMD64_SYSV_ABI_QWORD;
    }
    return KEFIR_OK;
}

static kefir_result_t restore_dirty_regs(struct kefir_codegen_amd64 *codegen, struct inline_assembly_params *params) {
    if (params->dirty_cc) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POPFQ(&codegen->xasmgen));
    }

    for (const struct kefir_list_entry *iter = kefir_list_tail(&params->preserved_regs); iter != NULL;
         iter = iter->prev) {
        ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t, reg, (kefir_uptr_t) iter->value);

        if (kefir_asm_amd64_xasmgen_register_is_floating_point(reg)) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVDQU(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                                     2 * KEFIR_AMD64_SYSV_ABI_QWORD)));
        } else {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg)));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t drop_inputs(struct kefir_codegen_amd64 *codegen, struct inline_assembly_params *params) {
    if (params->num_of_inputs > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                                 KEFIR_AMD64_SYSV_ABI_QWORD * params->num_of_inputs)));
    }
    return KEFIR_OK;
}

static kefir_result_t match_register_to_size(kefir_asm_amd64_xasmgen_register_t in, kefir_size_t sz,
                                             kefir_asm_amd64_xasmgen_register_t *out) {
    if (sz <= 1) {
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(in, out));
    } else if (sz <= 2) {
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register16(in, out));
    } else if (sz <= 4) {
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(in, out));
    } else if (sz <= 8) {
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register64(in, out));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to match register variant wider than 8 bytes");
    }
    return KEFIR_OK;
}

static kefir_result_t load_inputs(struct kefir_codegen_amd64 *codegen,
                                  const struct kefir_ir_inline_assembly *inline_asm,
                                  struct inline_assembly_params *params) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->parameter_list); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, asm_param, iter->value);

        if (asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE) {
            continue;
        }

        struct kefir_hashtree_node *map_node;
        REQUIRE_OK(
            kefir_hashtree_at(&params->parameter_mapping, (kefir_hashtree_key_t) asm_param->parameter_id, &map_node));
        ASSIGN_DECL_CAST(struct inline_assembly_param_map *, param_map, map_node->value);

        if (asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE &&
            param_map->type == INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT) {
            continue;
        }

        const struct kefir_ir_typeentry *param_type = NULL;
        switch (asm_param->klass) {
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE:
                param_type = kefir_ir_type_at(asm_param->read_type.type, asm_param->read_type.index);
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE:
                param_type = kefir_ir_type_at(asm_param->type.type, asm_param->type.index);
                break;
        }
        REQUIRE(param_type != NULL,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain IR inline assembly parameter type"));

        switch (param_type->typecode) {
            case KEFIR_IR_TYPE_INT8:
            case KEFIR_IR_TYPE_INT16:
            case KEFIR_IR_TYPE_INT32:
            case KEFIR_IR_TYPE_INT64:
            case KEFIR_IR_TYPE_BOOL:
            case KEFIR_IR_TYPE_CHAR:
            case KEFIR_IR_TYPE_SHORT:
            case KEFIR_IR_TYPE_INT:
            case KEFIR_IR_TYPE_LONG:
            case KEFIR_IR_TYPE_WORD:
            case KEFIR_IR_TYPE_FLOAT32:
            case KEFIR_IR_TYPE_FLOAT64:
                switch (param_map->type) {
                    case INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT:
                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(param_map->reg),
                            kefir_asm_amd64_xasmgen_operand_indirect(
                                &codegen->xasmgen_helpers.operands[0],
                                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                params->parameter_base_offset + KEFIR_AMD64_SYSV_ABI_QWORD * param_map->input_index)));
                        if (!param_map->direct_read) {
                            kefir_asm_amd64_xasmgen_register_t reg;
                            REQUIRE_OK(match_register_to_size(param_map->reg, param_map->parameter_props.size, &reg));
                            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                                kefir_asm_amd64_xasmgen_operand_indirect(
                                    &codegen->xasmgen_helpers.operands[0],
                                    kefir_asm_amd64_xasmgen_operand_reg(param_map->reg), 0)));
                        }
                        break;

                    case INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT:
                        REQUIRE(
                            !param_map->direct_read,
                            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter properties"));
                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(param_map->reg),
                            kefir_asm_amd64_xasmgen_operand_indirect(
                                &codegen->xasmgen_helpers.operands[0],
                                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                params->parameter_base_offset + KEFIR_AMD64_SYSV_ABI_QWORD * param_map->input_index)));
                        break;

                    case INLINE_ASSEMBLY_PARAM_STACK:
                        // Intentionally left blank
                        break;
                }
                break;

            case KEFIR_IR_TYPE_LONG_DOUBLE:
            case KEFIR_IR_TYPE_STRUCT:
            case KEFIR_IR_TYPE_ARRAY:
            case KEFIR_IR_TYPE_UNION:
                switch (param_map->type) {
                    case INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT: {
                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(param_map->reg),
                            kefir_asm_amd64_xasmgen_operand_indirect(
                                &codegen->xasmgen_helpers.operands[0],
                                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                params->parameter_base_offset + KEFIR_AMD64_SYSV_ABI_QWORD * param_map->input_index)));

                        kefir_asm_amd64_xasmgen_register_t reg;
                        REQUIRE_OK(match_register_to_size(
                            param_map->reg,
                            param_map->direct_read
                                ? MIN(param_map->parameter_read_props.size, KEFIR_AMD64_SYSV_ABI_QWORD)
                                : param_map->parameter_props.size,
                            &reg));

                        REQUIRE_OK(
                            KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                                                          kefir_asm_amd64_xasmgen_operand_indirect(
                                                              &codegen->xasmgen_helpers.operands[0],
                                                              kefir_asm_amd64_xasmgen_operand_reg(param_map->reg), 0)));

                    } break;

                    case INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT:
                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(param_map->reg),
                            kefir_asm_amd64_xasmgen_operand_indirect(
                                &codegen->xasmgen_helpers.operands[0],
                                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                params->parameter_base_offset + KEFIR_AMD64_SYSV_ABI_QWORD * param_map->input_index)));
                        break;

                    case INLINE_ASSEMBLY_PARAM_STACK:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                               "On-stack aggregate parameters of IR inline assembly are not supported");
                }
                break;

            case KEFIR_IR_TYPE_BITS:
            case KEFIR_IR_TYPE_BUILTIN:
            case KEFIR_IR_TYPE_COUNT:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter type");
        }
    }
    return KEFIR_OK;
}

static kefir_result_t store_register_aggregate_outputs(struct kefir_codegen_amd64 *codegen,
                                                       struct inline_assembly_params *params,
                                                       kefir_asm_amd64_xasmgen_register_t *scratch_reg,
                                                       kefir_bool_t *pushed_scratch) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&params->register_aggregate_params); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, asm_param, iter->value);

        if (asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ ||
            asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD ||
            asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE) {
            continue;
        }

        struct kefir_hashtree_node *map_node;
        REQUIRE_OK(
            kefir_hashtree_at(&params->parameter_mapping, (kefir_hashtree_key_t) asm_param->parameter_id, &map_node));
        ASSIGN_DECL_CAST(struct inline_assembly_param_map *, param_map, map_node->value);

        if (*scratch_reg == param_map->reg) {
            if (*pushed_scratch) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&codegen->xasmgen,
                                                         kefir_asm_amd64_xasmgen_operand_reg(*scratch_reg)));
            }
            *scratch_reg = param_map->reg == KEFIR_AMD64_XASMGEN_REGISTER_RAX ? KEFIR_AMD64_XASMGEN_REGISTER_RCX
                                                                              : KEFIR_AMD64_XASMGEN_REGISTER_RAX;
        }
        if (!*pushed_scratch && (kefir_hashtree_has(&params->dirty_regs, (kefir_hashtree_key_t) *scratch_reg) ||
                                 inline_assembly_is_register_preserved(*scratch_reg))) {
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(*scratch_reg)));
            *pushed_scratch = true;
        }

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(*scratch_reg),
            kefir_asm_amd64_xasmgen_operand_indirect(
                &codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                params->parameter_base_offset + KEFIR_AMD64_SYSV_ABI_QWORD * asm_param->load_store_index +
                    (*pushed_scratch ? KEFIR_AMD64_SYSV_ABI_QWORD : 0))));

        kefir_asm_amd64_xasmgen_register_t reg;
        REQUIRE_OK(match_register_to_size(param_map->reg, param_map->parameter_props.size, &reg));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen,
            kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                     kefir_asm_amd64_xasmgen_operand_reg(*scratch_reg), 0),
            kefir_asm_amd64_xasmgen_operand_reg(reg)));
    }
    return KEFIR_OK;
}

static kefir_result_t store_outputs(struct kefir_codegen_amd64 *codegen,
                                    const struct kefir_ir_inline_assembly *inline_asm,
                                    struct inline_assembly_params *params) {

    kefir_asm_amd64_xasmgen_register_t scratch_reg;
    kefir_bool_t pushed_scratch = false;
    if (kefir_list_length(&params->available_int_regs)) {
        scratch_reg =
            (kefir_asm_amd64_xasmgen_register_t) ((kefir_uptr_t) kefir_list_head(&params->available_int_regs)->value);
    } else {
        scratch_reg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->parameter_list); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, asm_param, iter->value);

        if (asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ ||
            asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD ||
            asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE) {
            continue;
        }

        struct kefir_hashtree_node *map_node;
        REQUIRE_OK(
            kefir_hashtree_at(&params->parameter_mapping, (kefir_hashtree_key_t) asm_param->parameter_id, &map_node));
        ASSIGN_DECL_CAST(struct inline_assembly_param_map *, param_map, map_node->value);

        const struct kefir_ir_typeentry *param_type = kefir_ir_type_at(asm_param->type.type, asm_param->type.index);
        REQUIRE(param_type != NULL,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain IR inline assembly parameter type"));

        switch (param_type->typecode) {
            case KEFIR_IR_TYPE_INT8:
            case KEFIR_IR_TYPE_INT16:
            case KEFIR_IR_TYPE_INT32:
            case KEFIR_IR_TYPE_INT64:
            case KEFIR_IR_TYPE_BOOL:
            case KEFIR_IR_TYPE_CHAR:
            case KEFIR_IR_TYPE_SHORT:
            case KEFIR_IR_TYPE_INT:
            case KEFIR_IR_TYPE_LONG:
            case KEFIR_IR_TYPE_WORD:
            case KEFIR_IR_TYPE_FLOAT32:
            case KEFIR_IR_TYPE_FLOAT64:
                switch (param_map->type) {
                    case INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT: {
                        if (scratch_reg == param_map->reg) {
                            if (pushed_scratch) {
                                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(
                                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(scratch_reg)));
                            }
                            scratch_reg = param_map->reg == KEFIR_AMD64_XASMGEN_REGISTER_RAX
                                              ? KEFIR_AMD64_XASMGEN_REGISTER_RCX
                                              : KEFIR_AMD64_XASMGEN_REGISTER_RAX;
                        }
                        if (!pushed_scratch &&
                            (kefir_hashtree_has(&params->dirty_regs, (kefir_hashtree_key_t) scratch_reg) ||
                             inline_assembly_is_register_preserved(scratch_reg))) {
                            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(
                                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(scratch_reg)));
                            pushed_scratch = true;
                        }
                        kefir_asm_amd64_xasmgen_register_t reg;
                        REQUIRE_OK(match_register_to_size(param_map->reg, param_map->parameter_props.size, &reg));

                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(scratch_reg),
                            kefir_asm_amd64_xasmgen_operand_indirect(
                                &codegen->xasmgen_helpers.operands[0],
                                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                params->parameter_base_offset +
                                    KEFIR_AMD64_SYSV_ABI_QWORD * asm_param->load_store_index +
                                    (pushed_scratch ? KEFIR_AMD64_SYSV_ABI_QWORD : 0))));

                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &codegen->xasmgen,
                            kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                                     kefir_asm_amd64_xasmgen_operand_reg(scratch_reg),
                                                                     0),
                            kefir_asm_amd64_xasmgen_operand_reg(reg)));
                    } break;

                    case INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT:
                        REQUIRE(
                            !param_map->direct_read,
                            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter properties"));
                        break;

                    case INLINE_ASSEMBLY_PARAM_STACK:
                        // Intentionally left blank
                        break;
                }
                break;

            case KEFIR_IR_TYPE_LONG_DOUBLE:
            case KEFIR_IR_TYPE_STRUCT:
            case KEFIR_IR_TYPE_ARRAY:
            case KEFIR_IR_TYPE_UNION:
                switch (param_map->type) {
                    case INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT:
                    case INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT:
                        // Intentionally left blank
                        break;

                    case INLINE_ASSEMBLY_PARAM_STACK:
                        return KEFIR_SET_ERROR(
                            KEFIR_INVALID_STATE,
                            "On-stack aggregate parameters of IR inline assembly are not supported yet");
                }
                break;

            case KEFIR_IR_TYPE_BITS:
            case KEFIR_IR_TYPE_BUILTIN:
            case KEFIR_IR_TYPE_COUNT:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter type");
        }
    }

    REQUIRE_OK(store_register_aggregate_outputs(codegen, params, &scratch_reg, &pushed_scratch));
    if (pushed_scratch) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(scratch_reg)));
    }
    return KEFIR_OK;
}

static kefir_result_t pointer_operand(const struct kefir_ir_inline_assembly_parameter *asm_param,
                                      struct inline_assembly_param_map *param_map,
                                      const struct kefir_asm_amd64_xasmgen_operand *base,
                                      struct kefir_asm_amd64_xasmgen_operand *new_op,
                                      const struct kefir_asm_amd64_xasmgen_operand **result,
                                      kefir_size_t override_size) {

    struct kefir_ir_typeentry *typeentry = kefir_ir_type_at(asm_param->type.type, asm_param->type.index);
    kefir_size_t param_size = override_size == 0 ? param_map->parameter_props.size : override_size;

    if (override_size == 0 && param_map->parameter_props.type == INLINE_ASSEMBLY_PARAM_AGGREGATE &&
        typeentry->typecode != KEFIR_IR_TYPE_LONG_DOUBLE) {
        *result = base;
        return KEFIR_OK;
    }

    if (param_size <= 1) {
        *result = kefir_asm_amd64_xasmgen_operand_pointer(new_op, KEFIR_AMD64_XASMGEN_POINTER_BYTE, base);
    } else if (param_size <= 2) {
        *result = kefir_asm_amd64_xasmgen_operand_pointer(new_op, KEFIR_AMD64_XASMGEN_POINTER_WORD, base);
    } else if (param_size <= 4) {
        *result = kefir_asm_amd64_xasmgen_operand_pointer(new_op, KEFIR_AMD64_XASMGEN_POINTER_DWORD, base);
    } else if (param_size <= 8) {
        *result = kefir_asm_amd64_xasmgen_operand_pointer(new_op, KEFIR_AMD64_XASMGEN_POINTER_QWORD, base);
    } else if (override_size == 10 || typeentry->typecode == KEFIR_IR_TYPE_LONG_DOUBLE) {
        *result = kefir_asm_amd64_xasmgen_operand_pointer(new_op, KEFIR_AMD64_XASMGEN_POINTER_TBYTE, base);
    } else {
        *result = base;
    }
    return KEFIR_OK;
}

static kefir_result_t match_parameter(const struct kefir_ir_inline_assembly *inline_asm, const char **input_str,
                                      const struct kefir_ir_inline_assembly_parameter **asm_param_ptr,
                                      const struct kefir_ir_inline_assembly_jump_target **jump_target_ptr) {
    *asm_param_ptr = NULL;
    *jump_target_ptr = NULL;
    struct kefir_hashtree_node_iterator iter;
    kefir_size_t last_match_length = 0;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->parameters, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, identifier, node->key);
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, asm_param, node->value);

        kefir_size_t template_parameter_length = strlen(identifier);
        if (template_parameter_length > last_match_length &&
            strncmp(*input_str, identifier, template_parameter_length) == 0) {
            *asm_param_ptr = asm_param;
            last_match_length = template_parameter_length;
        }
    }

    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->jump_targets, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, identifier, node->key);
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_jump_target *, jump_target, node->value);

        kefir_size_t identifier_length = strlen(identifier);
        if (identifier_length > last_match_length && strncmp(*input_str, identifier, identifier_length) == 0) {
            *asm_param_ptr = NULL;
            *jump_target_ptr = jump_target;
            last_match_length = identifier_length;
        }
    }

    if (last_match_length == 0) {
        return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Inline assembly parameter not found");
    } else {
        *input_str += last_match_length;
        return KEFIR_OK;
    }
}

static kefir_result_t format_label_parameter(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                             const struct kefir_ir_inline_assembly *inline_asm,
                                             struct inline_assembly_params *params,
                                             const struct kefir_ir_inline_assembly_jump_target *jump_target) {
    char jump_target_text[1024];
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_FORMAT_OPERAND(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_label(
            &codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers,
                                                   KEFIR_AMD64_SYSTEM_V_RUNTIME_INLINE_ASSEMBLY_JUMP_TRAMPOLINE,
                                                   inline_asm->id, jump_target->uid)),
        jump_target_text, sizeof(jump_target_text) - 1));

    REQUIRE_OK(kefir_string_builder_printf(mem, &params->formatted_asm, "%s", jump_target_text));
    return KEFIR_OK;
}

static kefir_result_t format_normal_parameter(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                              struct inline_assembly_params *params,
                                              const struct kefir_ir_inline_assembly_parameter *asm_param,
                                              kefir_size_t override_size) {
    if (asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE) {
        char register_symbol[128];
        const struct kefir_asm_amd64_xasmgen_operand *operand = NULL;
        switch (asm_param->immediate_type) {
            case KEFIR_IR_INLINE_ASSEMBLY_IMMEDIATE_IDENTIFIER_BASED:
                if (asm_param->immediate_identifier_base != NULL) {
                    operand = kefir_asm_amd64_xasmgen_operand_offset(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[1],
                                                              asm_param->immediate_identifier_base),
                        asm_param->immediate_value);
                } else {
                    operand = kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                                  asm_param->immediate_value);
                }
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_IMMEDIATE_LITERAL_BASED:
                operand = kefir_asm_amd64_xasmgen_operand_offset(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[1],
                        kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers,
                                                               KEFIR_AMD64_SYSTEM_V_RUNTIME_STRING_LITERAL,
                                                               asm_param->immediate_identifier_base)),
                    asm_param->immediate_value);
                break;
        }
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_FORMAT_OPERAND(&codegen->xasmgen, operand, register_symbol,
                                                      sizeof(register_symbol) - 1));
        REQUIRE_OK(kefir_string_builder_printf(mem, &params->formatted_asm, "%s", register_symbol));
        return KEFIR_OK;
    }

    struct kefir_hashtree_node *map_node;
    REQUIRE_OK(
        kefir_hashtree_at(&params->parameter_mapping, (kefir_hashtree_key_t) asm_param->parameter_id, &map_node));
    ASSIGN_DECL_CAST(struct inline_assembly_param_map *, param_map, map_node->value);

    kefir_size_t operand_size = override_size == 0 ? param_map->parameter_props.size : override_size;

    switch (param_map->type) {
        case INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT: {
            kefir_asm_amd64_xasmgen_register_t reg;
            REQUIRE_OK(match_register_to_size(param_map->reg, operand_size, &reg));

            char register_symbol[64];
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_FORMAT_OPERAND(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                                                          register_symbol, sizeof(register_symbol) - 1));
            REQUIRE_OK(kefir_string_builder_printf(mem, &params->formatted_asm, "%s", register_symbol));
        } break;

        case INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT: {
            char register_symbol[128];
            const struct kefir_asm_amd64_xasmgen_operand *operand = NULL;
            REQUIRE_OK(pointer_operand(
                asm_param, param_map,
                kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                         kefir_asm_amd64_xasmgen_operand_reg(param_map->reg), 0),
                &codegen->xasmgen_helpers.operands[1], &operand, override_size));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_FORMAT_OPERAND(&codegen->xasmgen, operand, register_symbol,
                                                          sizeof(register_symbol) - 1));
            REQUIRE_OK(kefir_string_builder_printf(mem, &params->formatted_asm, "%s", register_symbol));
        } break;

        case INLINE_ASSEMBLY_PARAM_STACK: {
            kefir_int64_t offset = params->parameter_base_offset + KEFIR_AMD64_SYSV_ABI_QWORD * param_map->input_index;
            char register_symbol[128];
            const struct kefir_asm_amd64_xasmgen_operand *operand = NULL;
            REQUIRE_OK(
                pointer_operand(asm_param, param_map,
                                kefir_asm_amd64_xasmgen_operand_indirect(
                                    &codegen->xasmgen_helpers.operands[0],
                                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), offset),
                                &codegen->xasmgen_helpers.operands[1], &operand, override_size));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_FORMAT_OPERAND(&codegen->xasmgen, operand, register_symbol,
                                                          sizeof(register_symbol) - 1));
            REQUIRE_OK(kefir_string_builder_printf(mem, &params->formatted_asm, "%s", register_symbol));
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t default_param_modifier(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                             const struct kefir_ir_inline_assembly *inline_asm,
                                             struct inline_assembly_params *params, const char **input_str) {
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(inline_asm, &input, &asm_param, &jump_target));
    *input_str = input;
    if (asm_param != NULL) {
        REQUIRE_OK(format_normal_parameter(mem, codegen, params, asm_param, 0));
    } else if (jump_target != NULL) {
        REQUIRE_OK(format_label_parameter(mem, codegen, inline_asm, params, jump_target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result");
    }
    return KEFIR_OK;
}

static kefir_result_t byte_param_modifier(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                          const struct kefir_ir_inline_assembly *inline_asm,
                                          struct inline_assembly_params *params, const char **input_str) {
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(inline_asm, &input, &asm_param, &jump_target));
    *input_str = input;
    if (asm_param != NULL) {
        REQUIRE_OK(format_normal_parameter(mem, codegen, params, asm_param, 1));
    } else if (jump_target != NULL) {
        REQUIRE_OK(format_label_parameter(mem, codegen, inline_asm, params, jump_target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result");
    }
    return KEFIR_OK;
}

static kefir_result_t word_param_modifier(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                          const struct kefir_ir_inline_assembly *inline_asm,
                                          struct inline_assembly_params *params, const char **input_str) {
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(inline_asm, &input, &asm_param, &jump_target));
    *input_str = input;
    if (asm_param != NULL) {
        REQUIRE_OK(format_normal_parameter(mem, codegen, params, asm_param, 2));
    } else if (jump_target != NULL) {
        REQUIRE_OK(format_label_parameter(mem, codegen, inline_asm, params, jump_target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result");
    }
    return KEFIR_OK;
}

static kefir_result_t dword_param_modifier(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                           const struct kefir_ir_inline_assembly *inline_asm,
                                           struct inline_assembly_params *params, const char **input_str) {
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(inline_asm, &input, &asm_param, &jump_target));
    *input_str = input;
    if (asm_param != NULL) {
        REQUIRE_OK(format_normal_parameter(mem, codegen, params, asm_param, 4));
    } else if (jump_target != NULL) {
        REQUIRE_OK(format_label_parameter(mem, codegen, inline_asm, params, jump_target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result");
    }
    return KEFIR_OK;
}

static kefir_result_t qword_param_modifier(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                           const struct kefir_ir_inline_assembly *inline_asm,
                                           struct inline_assembly_params *params, const char **input_str) {
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(inline_asm, &input, &asm_param, &jump_target));
    *input_str = input;
    if (asm_param != NULL) {
        REQUIRE_OK(format_normal_parameter(mem, codegen, params, asm_param, 8));
    } else if (jump_target != NULL) {
        REQUIRE_OK(format_label_parameter(mem, codegen, inline_asm, params, jump_target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result");
    }
    return KEFIR_OK;
}

static kefir_result_t label_param_modifier(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                           const struct kefir_ir_inline_assembly *inline_asm,
                                           struct inline_assembly_params *params, const char **input_str) {
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(inline_asm, &input, &asm_param, &jump_target));
    REQUIRE(asm_param == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                            "Inline assembly label parameter modifier cannot be applied to register parameters"));
    REQUIRE(jump_target != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result"));
    *input_str = input;
    REQUIRE_OK(format_label_parameter(mem, codegen, inline_asm, params, jump_target));
    return KEFIR_OK;
}

static kefir_result_t format_template(struct kefir_mem *mem, struct kefir_codegen_amd64_sysv_module *sysv_module,
                                      struct kefir_codegen_amd64 *codegen,
                                      const struct kefir_ir_inline_assembly *inline_asm,
                                      struct inline_assembly_params *params) {
    kefir_id_t id = sysv_module->inline_assembly_next_id++;

    const char *template_iter = inline_asm->template;
    while (template_iter != NULL) {
        const char *format_specifier = strchr(template_iter, '%');
        if (format_specifier == NULL) {
            REQUIRE_OK(kefir_string_builder_printf(mem, &params->formatted_asm, "%s", template_iter));
            template_iter = NULL;
        } else {
            REQUIRE_OK(kefir_string_builder_printf(mem, &params->formatted_asm, "%.*s",
                                                   format_specifier - template_iter, template_iter));
            switch (format_specifier[1]) {
                case '%':
                case '{':
                case '|':
                case '}':
                    REQUIRE_OK(kefir_string_builder_printf(mem, &params->formatted_asm, "%.1s", format_specifier + 1));
                    template_iter = format_specifier + 2;
                    break;

                case '=':
                    REQUIRE_OK(kefir_string_builder_printf(mem, &params->formatted_asm, KEFIR_ID_FMT, id));
                    template_iter = format_specifier + 2;
                    break;

                case 'l': {
                    template_iter = format_specifier + 2;
                    kefir_result_t res = label_param_modifier(mem, codegen, inline_asm, params, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        template_iter = format_specifier + 1;
                        res = default_param_modifier(mem, codegen, inline_asm, params, &template_iter);
                    }
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_string_builder_printf(mem, &params->formatted_asm, "%%");
                    }
                    REQUIRE_OK(res);
                } break;

                case 'b': {
                    template_iter = format_specifier + 2;
                    kefir_result_t res = byte_param_modifier(mem, codegen, inline_asm, params, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        template_iter = format_specifier + 1;
                        res = default_param_modifier(mem, codegen, inline_asm, params, &template_iter);
                    }
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_string_builder_printf(mem, &params->formatted_asm, "%%");
                    }
                    REQUIRE_OK(res);
                } break;

                case 'w': {
                    template_iter = format_specifier + 2;
                    kefir_result_t res = word_param_modifier(mem, codegen, inline_asm, params, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        template_iter = format_specifier + 1;
                        res = default_param_modifier(mem, codegen, inline_asm, params, &template_iter);
                    }
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_string_builder_printf(mem, &params->formatted_asm, "%%");
                    }
                    REQUIRE_OK(res);
                } break;

                case 'd': {
                    template_iter = format_specifier + 2;
                    kefir_result_t res = dword_param_modifier(mem, codegen, inline_asm, params, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        template_iter = format_specifier + 1;
                        res = default_param_modifier(mem, codegen, inline_asm, params, &template_iter);
                    }
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_string_builder_printf(mem, &params->formatted_asm, "%%");
                    }
                    REQUIRE_OK(res);
                } break;

                case 'q': {
                    template_iter = format_specifier + 2;
                    kefir_result_t res = qword_param_modifier(mem, codegen, inline_asm, params, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        template_iter = format_specifier + 1;
                        res = default_param_modifier(mem, codegen, inline_asm, params, &template_iter);
                    }
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_string_builder_printf(mem, &params->formatted_asm, "%%");
                    }
                    REQUIRE_OK(res);
                } break;

                default: {
                    template_iter = format_specifier + 1;
                    kefir_result_t res = default_param_modifier(mem, codegen, inline_asm, params, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_string_builder_printf(mem, &params->formatted_asm, "%%");
                    }
                    REQUIRE_OK(res);
                } break;
            }
        }
    }

    REQUIRE_OK(
        KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "Begin of fragment #" KEFIR_ID_FMT " code", inline_asm->id));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INLINE_ASSEMBLY(&codegen->xasmgen, params->formatted_asm.string));
    REQUIRE_OK(
        KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "End of fragment #" KEFIR_ID_FMT " code", inline_asm->id));
    return KEFIR_OK;
}

static kefir_result_t generate_jump_trampolines(struct kefir_codegen_amd64 *codegen,
                                                const struct kefir_ir_inline_assembly *inline_asm,
                                                struct inline_assembly_params *params) {
    REQUIRE(kefir_list_length(&inline_asm->jump_target_list) > 0, KEFIR_OK);

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_label(
                               &codegen->xasmgen_helpers.operands[0],
                               kefir_asm_amd64_xasmgen_helpers_format(
                                   &codegen->xasmgen_helpers,
                                   KEFIR_AMD64_SYSTEM_V_RUNTIME_INLINE_ASSEMBLY_JUMP_TRAMPOLINE_END, inline_asm->id))));

    for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->jump_target_list); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_jump_target *, jump_target, iter->value);

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen,
                                             KEFIR_AMD64_SYSTEM_V_RUNTIME_INLINE_ASSEMBLY_JUMP_TRAMPOLINE,
                                             inline_asm->id, jump_target->uid));

        // Store outputs
        REQUIRE_OK(store_outputs(codegen, inline_asm, params));

        // Restore dirty regs
        REQUIRE_OK(restore_dirty_regs(codegen, params));

        // Drop inputs
        REQUIRE_OK(drop_inputs(codegen, params));

        // Perform jump
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX),
            kefir_asm_amd64_xasmgen_operand_indirect(
                &codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_operand_offset(
                    &codegen->xasmgen_helpers.operands[1],
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[2],
                        kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers,
                                                               KEFIR_AMD64_SYSV_PROCEDURE_BODY_LABEL,
                                                               jump_target->target_function)),
                    2 * KEFIR_AMD64_SYSV_ABI_QWORD * jump_target->target),
                0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_indirect(
                                   &codegen->xasmgen_helpers.operands[0],
                                   kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX), 0)));
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(
        &codegen->xasmgen, KEFIR_AMD64_SYSTEM_V_RUNTIME_INLINE_ASSEMBLY_JUMP_TRAMPOLINE_END, inline_asm->id));
    return KEFIR_OK;
}

static kefir_result_t inline_assembly_embed_impl(struct kefir_mem *mem,
                                                 struct kefir_codegen_amd64_sysv_module *sysv_module,
                                                 struct kefir_codegen_amd64 *codegen,
                                                 const struct kefir_ir_inline_assembly *inline_asm,
                                                 struct inline_assembly_params *params) {
    // Mark clobber regs as dirty
    REQUIRE_OK(mark_clobbers(mem, inline_asm, params));

    // Initialize available register lists
    REQUIRE_OK(init_available_regs(mem, params));

    // Map parameters to their storage
    REQUIRE_OK(map_parameters(mem, inline_asm, params));

    // Preserve dirty regs
    REQUIRE_OK(preserve_dirty_regs(mem, codegen, params));

    // Load inputs
    REQUIRE_OK(load_inputs(codegen, inline_asm, params));

    // Generate assembly code from template
    REQUIRE_OK(format_template(mem, sysv_module, codegen, inline_asm, params));

    // Store outputs
    REQUIRE_OK(store_outputs(codegen, inline_asm, params));

    // Restore dirty regs
    REQUIRE_OK(restore_dirty_regs(codegen, params));

    // Drop inputs
    REQUIRE_OK(drop_inputs(codegen, params));

    // Jump trampolines
    REQUIRE_OK(generate_jump_trampolines(codegen, inline_asm, params));
    return KEFIR_OK;
}

static kefir_result_t param_map_free(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                     kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct inline_assembly_param_map *, map, value);
    REQUIRE(map != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly parameter mapping"));

    KEFIR_FREE(mem, map);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_sysv_inline_assembly_embed(struct kefir_mem *mem,
                                                              struct kefir_codegen_amd64_sysv_module *sysv_module,
                                                              struct kefir_codegen_amd64 *codegen,
                                                              const struct kefir_ir_inline_assembly *inline_asm) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(sysv_module != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen AMD64 System-V module"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen AMD64 codegen"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly"));

    struct inline_assembly_params params = {0};
    REQUIRE_OK(kefir_hashtree_init(&params.dirty_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_list_init(&params.available_int_regs));
    REQUIRE_OK(kefir_list_init(&params.preserved_regs));
    REQUIRE_OK(kefir_hashtree_init(&params.parameter_mapping, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&params.parameter_mapping, param_map_free, NULL));
    REQUIRE_OK(kefir_list_init(&params.register_aggregate_params));
    REQUIRE_OK(kefir_string_builder_init(&params.formatted_asm));

    kefir_result_t res = inline_assembly_embed_impl(mem, sysv_module, codegen, inline_asm, &params);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_string_builder_free(mem, &params.formatted_asm);
        kefir_list_free(mem, &params.register_aggregate_params);
        kefir_hashtree_free(mem, &params.parameter_mapping);
        kefir_list_free(mem, &params.preserved_regs);
        kefir_list_free(mem, &params.available_int_regs);
        kefir_hashtree_free(mem, &params.dirty_regs);
        return res;
    });

    res = kefir_string_builder_free(mem, &params.formatted_asm);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &params.register_aggregate_params);
        kefir_hashtree_free(mem, &params.parameter_mapping);
        kefir_list_free(mem, &params.preserved_regs);
        kefir_list_free(mem, &params.available_int_regs);
        kefir_hashtree_free(mem, &params.dirty_regs);
        return res;
    });

    res = kefir_list_free(mem, &params.register_aggregate_params);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &params.parameter_mapping);
        kefir_list_free(mem, &params.preserved_regs);
        kefir_list_free(mem, &params.available_int_regs);
        kefir_hashtree_free(mem, &params.dirty_regs);
        return res;
    });

    res = kefir_hashtree_free(mem, &params.parameter_mapping);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &params.preserved_regs);
        kefir_list_free(mem, &params.available_int_regs);
        kefir_hashtree_free(mem, &params.dirty_regs);
        return res;
    });

    res = kefir_list_free(mem, &params.preserved_regs);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &params.available_int_regs);
        kefir_hashtree_free(mem, &params.dirty_regs);
        return res;
    });

    res = kefir_list_free(mem, &params.available_int_regs);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &params.dirty_regs);
        return res;
    });

    REQUIRE_OK(kefir_hashtree_free(mem, &params.dirty_regs));
    return KEFIR_OK;
}
