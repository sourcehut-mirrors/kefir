#include "kefir/codegen/amd64/system-v/inline_assembly.h"
#include "kefir/codegen/amd64/system-v/runtime.h"
#include "kefir/codegen/amd64/system-v/abi/qwords.h"
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
        kefir_amd64_xasmgen_operand_label(
            &codegen->xasmgen_helpers.operands[0],
            kefir_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers,
                                               KEFIR_AMD64_SYSTEM_V_RUNTIME_INLINE_ASSEMBLY_FRAGMENT, inline_asm->id)),
        kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], 0)));
    return KEFIR_OK;
}

struct inline_assembly_params {
    struct kefir_hashtree dirty_regs;
    struct kefir_list available_int_regs;
    struct kefir_list preserved_regs;
    struct kefir_hashtree parameter_mapping;
    struct kefir_list register_aggregate_params;
    kefir_size_t num_of_inputs;
    kefir_size_t num_of_outputs;
    kefir_size_t parameter_adress_index;
    kefir_size_t parameter_base_offset;
    kefir_size_t input_param_base_offset;
    kefir_size_t output_param_base_offset;
    struct kefir_string_builder formatted_asm;
};

enum inline_assembly_param_map_type { INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT, INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT };

struct inline_assembly_param_map {
    enum inline_assembly_param_map_type type;
    union {
        kefir_amd64_xasmgen_register_t reg;
    };

    struct {
        kefir_size_t aggregate;
        kefir_size_t parameter_size;
        kefir_size_t address_index;
    } props;
};

static kefir_bool_t inline_assembly_is_register_preserved(kefir_amd64_xasmgen_register_t reg) {
    UNUSED(reg);
    // TODO Implement specific preserved register list
    return true;
}

static kefir_result_t mark_clobbers(struct kefir_mem *mem, const struct kefir_ir_inline_assembly *inline_asm,
                                    struct inline_assembly_params *params) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->clobbers, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, clobber, node->key);

        kefir_amd64_xasmgen_register_t dirty_reg;
        kefir_result_t res = kefir_amd64_xasmgen_register_from_symbolic_name(clobber, &dirty_reg);
        if (res == KEFIR_NOT_FOUND) {
            // Ignore unknown clobbers
            continue;
        }
        REQUIRE_OK(res);
        REQUIRE_OK(kefir_amd64_xasmgen_register_normalize(dirty_reg, &dirty_reg));

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
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_R8);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_R10);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_R11);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_R12);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_R13);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_R15);
    // Used for special purposes by runtime
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_R14);
    AVAIL_REG(&params->available_int_regs, KEFIR_AMD64_XASMGEN_REGISTER_RBX);
#undef AVAIL_REG
    return KEFIR_OK;
}

static kefir_result_t insert_param_map(struct kefir_mem *mem, struct inline_assembly_params *params, const char *key,
                                       enum inline_assembly_param_map_type type, kefir_amd64_xasmgen_register_t reg,
                                       kefir_int64_t offset, struct inline_assembly_param_map **map_ptr) {
    UNUSED(offset);
    struct inline_assembly_param_map *map = KEFIR_MALLOC(mem, sizeof(struct inline_assembly_param_map));
    REQUIRE(map != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR inline assembly parameter mapping"));

    map->type = type;
    switch (type) {
        case INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT:
        case INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT:
            map->reg = reg;
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

static kefir_result_t map_parameters(struct kefir_mem *mem, const struct kefir_ir_inline_assembly *inline_asm,
                                     struct inline_assembly_params *params) {

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->parameters, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, asm_param, node->value);

        const struct kefir_ir_typeentry *param_type = kefir_ir_type_at(asm_param->type.type, asm_param->type.index);
        REQUIRE(param_type != NULL,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain IR inline assembly parameter type"));

        kefir_bool_t aggregate_value = false;
        kefir_size_t parameter_size = 0;
        switch (param_type->typecode) {
            case KEFIR_IR_TYPE_BOOL:
            case KEFIR_IR_TYPE_CHAR:
            case KEFIR_IR_TYPE_INT8:
                parameter_size = 1;
                break;

            case KEFIR_IR_TYPE_SHORT:
            case KEFIR_IR_TYPE_INT16:
                parameter_size = 2;
                break;

            case KEFIR_IR_TYPE_INT:
            case KEFIR_IR_TYPE_INT32:
            case KEFIR_IR_TYPE_FLOAT32:
                parameter_size = 4;
                break;

            case KEFIR_IR_TYPE_INT64:
            case KEFIR_IR_TYPE_LONG:
            case KEFIR_IR_TYPE_WORD:
            case KEFIR_IR_TYPE_FLOAT64:
                parameter_size = 8;
                break;

            case KEFIR_IR_TYPE_LONG_DOUBLE:
                return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                       "Long doubles as IR inline assembly parameters are not supported yet");

            case KEFIR_IR_TYPE_STRUCT:
            case KEFIR_IR_TYPE_ARRAY:
            case KEFIR_IR_TYPE_UNION:
            case KEFIR_IR_TYPE_MEMORY: {
                struct kefir_vector layouts;
                REQUIRE_OK(kefir_amd64_sysv_type_layout(asm_param->type.type, mem, &layouts));
                ASSIGN_DECL_CAST(struct kefir_amd64_sysv_data_layout *, layout,
                                 kefir_vector_at(&layouts, asm_param->type.index));
                REQUIRE_ELSE(layout != NULL, {
                    kefir_vector_free(mem, &layouts);
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to calculate IR type layout");
                });
                aggregate_value = true;
                parameter_size = layout->size;
                REQUIRE_OK(kefir_vector_free(mem, &layouts));
            } break;

            case KEFIR_IR_TYPE_PAD:
            case KEFIR_IR_TYPE_BITS:
            case KEFIR_IR_TYPE_BUILTIN:
            case KEFIR_IR_TYPE_COUNT:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter type");
        }

        switch (asm_param->klass) {
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ:
                params->num_of_inputs = MAX(params->num_of_inputs, asm_param->input_index + 1);
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_WRITE:
                params->num_of_outputs = MAX(params->num_of_outputs, asm_param->output_index + 1);
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_WRITE:
                params->num_of_inputs = MAX(params->num_of_inputs, asm_param->input_index + 1);
                params->num_of_outputs = MAX(params->num_of_outputs, asm_param->output_index + 1);
                break;
        }

        struct inline_assembly_param_map *param_map = NULL;
        switch (asm_param->constraint) {
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER: {
                REQUIRE(!aggregate_value || parameter_size <= KEFIR_AMD64_SYSV_ABI_QWORD,
                        KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to satisfy IR inline assembly constraints"));
                kefir_amd64_xasmgen_register_t reg;
                REQUIRE(kefir_list_length(&params->available_int_regs) > 0,
                        KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to satisfy IR inline assembly constraints"));
                reg = (kefir_amd64_xasmgen_register_t) kefir_list_head(&params->available_int_regs)->value;
                REQUIRE_OK(
                    kefir_list_pop(mem, &params->available_int_regs, kefir_list_head(&params->available_int_regs)));
                REQUIRE_OK(insert_param_map(mem, params, asm_param->template_parameter,
                                            INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT, reg, 0, &param_map));
                REQUIRE_OK(kefir_hashtree_insert(mem, &params->dirty_regs, (kefir_hashtree_key_t) reg,
                                                 (kefir_hashtree_value_t) 0));
                if (aggregate_value) {
                    REQUIRE_OK(kefir_list_insert_after(mem, &params->register_aggregate_params,
                                                       kefir_list_tail(&params->register_aggregate_params),
                                                       (void *) asm_param));
                }
            } break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_REGISTER_MEMORY: {
                if ((!aggregate_value || parameter_size <= KEFIR_AMD64_SYSV_ABI_QWORD) &&
                    kefir_list_length(&params->available_int_regs) > 0) {
                    kefir_amd64_xasmgen_register_t reg =
                        (kefir_amd64_xasmgen_register_t) kefir_list_head(&params->available_int_regs)->value;
                    REQUIRE_OK(
                        kefir_list_pop(mem, &params->available_int_regs, kefir_list_head(&params->available_int_regs)));
                    REQUIRE_OK(insert_param_map(mem, params, asm_param->template_parameter,
                                                INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT, reg, 0, &param_map));
                    REQUIRE_OK(kefir_hashtree_insert(mem, &params->dirty_regs, (kefir_hashtree_key_t) reg,
                                                     (kefir_hashtree_value_t) 0));
                    if (aggregate_value) {
                        REQUIRE_OK(kefir_list_insert_after(mem, &params->register_aggregate_params,
                                                           kefir_list_tail(&params->register_aggregate_params),
                                                           (void *) asm_param));
                    }
                } else if (aggregate_value) {
                    kefir_amd64_xasmgen_register_t reg;
                    REQUIRE(kefir_list_length(&params->available_int_regs) > 0,
                            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to satisfy IR inline assembly constraints"));
                    reg = (kefir_amd64_xasmgen_register_t) kefir_list_head(&params->available_int_regs)->value;
                    REQUIRE_OK(
                        kefir_list_pop(mem, &params->available_int_regs, kefir_list_head(&params->available_int_regs)));
                    REQUIRE_OK(insert_param_map(mem, params, asm_param->template_parameter,
                                                INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT, reg, 0, &param_map));
                    REQUIRE_OK(kefir_hashtree_insert(mem, &params->dirty_regs, (kefir_hashtree_key_t) reg,
                                                     (kefir_hashtree_value_t) 0));
                } else {
                    return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to satisfy IR inline assembly constraints");
                }
            } break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_CONSTRAINT_MEMORY: {
                if (aggregate_value) {
                    kefir_amd64_xasmgen_register_t reg;
                    REQUIRE(kefir_list_length(&params->available_int_regs) > 0,
                            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to satisfy IR inline assembly constraints"));
                    reg = (kefir_amd64_xasmgen_register_t) kefir_list_head(&params->available_int_regs)->value;
                    REQUIRE_OK(
                        kefir_list_pop(mem, &params->available_int_regs, kefir_list_head(&params->available_int_regs)));
                    REQUIRE_OK(insert_param_map(mem, params, asm_param->template_parameter,
                                                INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT, reg, 0, &param_map));
                    REQUIRE_OK(kefir_hashtree_insert(mem, &params->dirty_regs, (kefir_hashtree_key_t) reg,
                                                     (kefir_hashtree_value_t) 0));
                } else {
                    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                           "In-memoty IR inline assembly parameters are not supported yet");
                }
            } break;
        }

        param_map->props.parameter_size = parameter_size;
        param_map->props.aggregate = aggregate_value;
    }
    return KEFIR_OK;
}

static kefir_result_t allocate_output_params(struct kefir_codegen_amd64 *codegen,
                                             struct inline_assembly_params *params) {
    if (params->num_of_inputs > params->num_of_outputs) {
        params->output_param_base_offset =
            (params->num_of_inputs - params->num_of_outputs) * KEFIR_AMD64_SYSV_ABI_QWORD;
    } else if (params->num_of_inputs < params->num_of_outputs) {
        params->input_param_base_offset = (params->num_of_outputs - params->num_of_inputs) * KEFIR_AMD64_SYSV_ABI_QWORD;
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
            &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], params->input_param_base_offset)));
    }
    return KEFIR_OK;
}

static kefir_result_t preserve_dirty_regs(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                          struct inline_assembly_params *params) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&params->dirty_regs, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_amd64_xasmgen_register_t, reg, node->key);

        if (inline_assembly_is_register_preserved(reg)) {
            REQUIRE_OK(kefir_list_insert_after(mem, &params->preserved_regs, kefir_list_tail(&params->preserved_regs),
                                               (void *) reg));

            if (kefir_amd64_xasmgen_register_is_floating_point(reg)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
                    &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                                     2 * KEFIR_AMD64_SYSV_ABI_QWORD)));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVDQU(
                    &codegen->xasmgen,
                    kefir_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0),
                    kefir_amd64_xasmgen_operand_reg(reg)));
                params->parameter_base_offset += 2 * KEFIR_AMD64_SYSV_ABI_QWORD;
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(reg)));
                params->parameter_base_offset += KEFIR_AMD64_SYSV_ABI_QWORD;
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t restore_dirty_regs(struct kefir_codegen_amd64 *codegen, struct inline_assembly_params *params) {
    if (params->parameter_adress_index > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                             params->parameter_adress_index * KEFIR_AMD64_SYSV_ABI_QWORD)));
    }

    for (const struct kefir_list_entry *iter = kefir_list_tail(&params->preserved_regs); iter != NULL;
         iter = iter->prev) {
        ASSIGN_DECL_CAST(kefir_amd64_xasmgen_register_t, reg, iter->value);

        if (kefir_amd64_xasmgen_register_is_floating_point(reg)) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVDQU(
                &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(reg),
                kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                     kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                                     0)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
                &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                                 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));
        } else {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(reg)));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t clear_stack(struct kefir_codegen_amd64 *codegen, struct inline_assembly_params *params) {
    if (params->output_param_base_offset > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], params->output_param_base_offset)));
    }
    return KEFIR_OK;
}

static kefir_result_t match_register_to_size(kefir_amd64_xasmgen_register_t in, kefir_size_t sz,
                                             kefir_amd64_xasmgen_register_t *out) {
    if (sz <= 1) {
        REQUIRE_OK(kefir_amd64_xasmgen_register8(in, out));
    } else if (sz <= 2) {
        REQUIRE_OK(kefir_amd64_xasmgen_register16(in, out));
    } else if (sz <= 4) {
        REQUIRE_OK(kefir_amd64_xasmgen_register32(in, out));
    } else if (sz <= 8) {
        REQUIRE_OK(kefir_amd64_xasmgen_register64(in, out));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to match register variant wider than 8 bytes");
    }
    return KEFIR_OK;
}

static kefir_result_t preserve_parameter_addresses(struct kefir_codegen_amd64 *codegen,
                                                   const struct kefir_ir_inline_assembly *inline_asm,
                                                   struct inline_assembly_params *params) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->parameters, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, asm_param, node->value);

        struct kefir_hashtree_node *map_node;
        REQUIRE_OK(kefir_hashtree_at(&params->parameter_mapping, (kefir_hashtree_key_t) asm_param->template_parameter,
                                     &map_node));
        ASSIGN_DECL_CAST(struct inline_assembly_param_map *, param_map, map_node->value);

        if (param_map->props.aggregate) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
                kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                     kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                                     params->parameter_base_offset + params->input_param_base_offset +
                                                         KEFIR_AMD64_SYSV_ABI_QWORD * asm_param->input_index)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(
                &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX)));

            params->parameter_base_offset += KEFIR_AMD64_SYSV_ABI_QWORD;
            param_map->props.address_index = params->parameter_adress_index++;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t load_inputs(struct kefir_codegen_amd64 *codegen,
                                  const struct kefir_ir_inline_assembly *inline_asm,
                                  struct inline_assembly_params *params) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->parameters, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, asm_param, node->value);

        if (asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_WRITE) {
            continue;
        }

        struct kefir_hashtree_node *map_node;
        REQUIRE_OK(kefir_hashtree_at(&params->parameter_mapping, (kefir_hashtree_key_t) asm_param->template_parameter,
                                     &map_node));
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
                    case INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT:
                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(param_map->reg),
                            kefir_amd64_xasmgen_operand_indirect(
                                &codegen->xasmgen_helpers.operands[0],
                                kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                params->parameter_base_offset + params->input_param_base_offset +
                                    KEFIR_AMD64_SYSV_ABI_QWORD * asm_param->input_index)));
                        break;

                    case INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT:
                        return KEFIR_SET_ERROR(
                            KEFIR_NOT_SUPPORTED,
                            "In-memory scalar parameters of IR inline assembly are not supported yet");
                }
                break;

            case KEFIR_IR_TYPE_LONG_DOUBLE:
                return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                       "Long doubles as IR inline assembly parameters are not supported yet");

            case KEFIR_IR_TYPE_STRUCT:
            case KEFIR_IR_TYPE_ARRAY:
            case KEFIR_IR_TYPE_UNION:
            case KEFIR_IR_TYPE_MEMORY:
                REQUIRE_OK(
                    KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(param_map->reg),
                                                  kefir_amd64_xasmgen_operand_indirect(
                                                      &codegen->xasmgen_helpers.operands[0],
                                                      kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                                      params->parameter_base_offset + params->input_param_base_offset +
                                                          KEFIR_AMD64_SYSV_ABI_QWORD * asm_param->input_index)));
                switch (param_map->type) {
                    case INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT: {
                        kefir_amd64_xasmgen_register_t reg;
                        REQUIRE_OK(match_register_to_size(param_map->reg, param_map->props.parameter_size, &reg));

                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(reg),
                            kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                                 kefir_amd64_xasmgen_operand_reg(param_map->reg), 0)));

                    } break;

                    case INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT:
                        // Intentionally left blank
                        break;
                }
                break;

            case KEFIR_IR_TYPE_PAD:
            case KEFIR_IR_TYPE_BITS:
            case KEFIR_IR_TYPE_BUILTIN:
            case KEFIR_IR_TYPE_COUNT:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter type");
        }
    }
    return KEFIR_OK;
}

static kefir_result_t store_register_aggregate_outputs(struct kefir_codegen_amd64 *codegen,
                                                       struct inline_assembly_params *params) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&params->register_aggregate_params); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, asm_param, iter->value);

        if (asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ) {
            continue;
        }

        struct kefir_hashtree_node *map_node;
        REQUIRE_OK(kefir_hashtree_at(&params->parameter_mapping, (kefir_hashtree_key_t) asm_param->template_parameter,
                                     &map_node));
        ASSIGN_DECL_CAST(struct inline_assembly_param_map *, param_map, map_node->value);

        kefir_amd64_xasmgen_register_t data_reg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
        if (param_map->reg == data_reg) {
            data_reg = KEFIR_AMD64_XASMGEN_REGISTER_RCX;
        }

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(data_reg),
            kefir_amd64_xasmgen_operand_indirect(
                &codegen->xasmgen_helpers.operands[0],
                kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                KEFIR_AMD64_SYSV_ABI_QWORD * (params->parameter_adress_index - param_map->props.address_index - 1))));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen,
            kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                                 params->parameter_base_offset + params->output_param_base_offset +
                                                     KEFIR_AMD64_SYSV_ABI_QWORD * asm_param->output_index),
            kefir_amd64_xasmgen_operand_reg(data_reg)));

        kefir_amd64_xasmgen_register_t reg;
        REQUIRE_OK(match_register_to_size(param_map->reg, param_map->props.parameter_size, &reg));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen,
            kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_amd64_xasmgen_operand_reg(data_reg), 0),
            kefir_amd64_xasmgen_operand_reg(reg)));
    }
    return KEFIR_OK;
}

static kefir_result_t store_outputs(struct kefir_codegen_amd64 *codegen,
                                    const struct kefir_ir_inline_assembly *inline_asm,
                                    struct inline_assembly_params *params) {

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->parameters, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, asm_param, node->value);

        if (asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ) {
            continue;
        }

        struct kefir_hashtree_node *map_node;
        REQUIRE_OK(kefir_hashtree_at(&params->parameter_mapping, (kefir_hashtree_key_t) asm_param->template_parameter,
                                     &map_node));
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
                        kefir_amd64_xasmgen_register_t reg;
                        REQUIRE_OK(match_register_to_size(param_map->reg, param_map->props.parameter_size, &reg));
                        if (param_map->props.parameter_size <= 2) {
                            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_AND(
                                &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(param_map->reg),
                                kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                                                 (1 << (param_map->props.parameter_size << 3)) - 1)));
                        }
                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &codegen->xasmgen,
                            kefir_amd64_xasmgen_operand_indirect(
                                &codegen->xasmgen_helpers.operands[0],
                                kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                params->parameter_base_offset + params->output_param_base_offset +
                                    KEFIR_AMD64_SYSV_ABI_QWORD * asm_param->output_index),
                            kefir_amd64_xasmgen_operand_reg(param_map->reg)));
                    } break;

                    case INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT:
                        return KEFIR_SET_ERROR(
                            KEFIR_NOT_SUPPORTED,
                            "In-memory scalar parameters of IR inline assembly are not supported yet");
                }
                break;

            case KEFIR_IR_TYPE_LONG_DOUBLE:
                return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                       "Long doubles as IR inline assembly parameters are not supported yet");

            case KEFIR_IR_TYPE_STRUCT:
            case KEFIR_IR_TYPE_ARRAY:
            case KEFIR_IR_TYPE_UNION:
            case KEFIR_IR_TYPE_MEMORY:
                switch (param_map->type) {
                    case INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT:
                        // Intentionally left blank
                        break;

                    case INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT:
                        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                            &codegen->xasmgen,
                            kefir_amd64_xasmgen_operand_indirect(
                                &codegen->xasmgen_helpers.operands[0],
                                kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                params->parameter_base_offset + params->output_param_base_offset +
                                    KEFIR_AMD64_SYSV_ABI_QWORD * asm_param->output_index),
                            kefir_amd64_xasmgen_operand_reg(param_map->reg)));
                        break;
                }
                break;

            case KEFIR_IR_TYPE_PAD:
            case KEFIR_IR_TYPE_BITS:
            case KEFIR_IR_TYPE_BUILTIN:
            case KEFIR_IR_TYPE_COUNT:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR inline assembly parameter type");
        }
    }

    REQUIRE_OK(store_register_aggregate_outputs(codegen, params));
    return KEFIR_OK;
}

static kefir_result_t format_template(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                      const struct kefir_ir_inline_assembly *inline_asm,
                                      struct inline_assembly_params *params) {
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

                default: {
                    kefir_bool_t match = false;
                    struct kefir_hashtree_node_iterator iter;
                    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->parameters, &iter);
                         !match && node != NULL; node = kefir_hashtree_next(&iter)) {
                        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, asm_param, node->value);

                        kefir_size_t template_parameter_length = strlen(asm_param->template_parameter);
                        if (strncmp(format_specifier + 1, asm_param->template_parameter, template_parameter_length) ==
                            0) {
                            struct kefir_hashtree_node *map_node;
                            REQUIRE_OK(kefir_hashtree_at(&params->parameter_mapping,
                                                         (kefir_hashtree_key_t) asm_param->template_parameter,
                                                         &map_node));
                            ASSIGN_DECL_CAST(struct inline_assembly_param_map *, param_map, map_node->value);

                            switch (param_map->type) {
                                case INLINE_ASSEMBLY_PARAM_REGISTER_DIRECT: {
                                    kefir_amd64_xasmgen_register_t reg;
                                    REQUIRE_OK(
                                        match_register_to_size(param_map->reg, param_map->props.parameter_size, &reg));

                                    char register_symbol[64];
                                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_FORMAT_OPERAND(
                                        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(reg), register_symbol,
                                        sizeof(register_symbol) - 1));
                                    REQUIRE_OK(kefir_string_builder_printf(mem, &params->formatted_asm, "%s",
                                                                           register_symbol));
                                } break;

                                case INLINE_ASSEMBLY_PARAM_REGISTER_INDIRECT: {
                                    char register_symbol[64];
                                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_FORMAT_OPERAND(
                                        &codegen->xasmgen,
                                        kefir_amd64_xasmgen_operand_indirect(
                                            &codegen->xasmgen_helpers.operands[0],
                                            kefir_amd64_xasmgen_operand_reg(param_map->reg), 0),
                                        register_symbol, sizeof(register_symbol) - 1));
                                    REQUIRE_OK(kefir_string_builder_printf(mem, &params->formatted_asm, "%s",
                                                                           register_symbol));
                                } break;
                            }
                            template_iter = format_specifier + 1 + template_parameter_length;
                            match = true;
                        }
                    }

                    if (!match) {
                        template_iter = format_specifier + 1;
                        REQUIRE_OK(kefir_string_builder_printf(mem, &params->formatted_asm, "%%"));
                    }
                } break;
            }
        }
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INLINE_ASSEMBLY(&codegen->xasmgen, params->formatted_asm.string));
    return KEFIR_OK;
}

static kefir_result_t inline_assembly_embed_impl(struct kefir_mem *mem,
                                                 struct kefir_codegen_amd64_sysv_module *sysv_module,
                                                 struct kefir_codegen_amd64 *codegen,
                                                 const struct kefir_ir_inline_assembly *inline_asm,
                                                 struct inline_assembly_params *params) {
    UNUSED(sysv_module);

    // Mark clobber regs as dirty
    REQUIRE_OK(mark_clobbers(mem, inline_asm, params));

    // Initialize available register lists
    REQUIRE_OK(init_available_regs(mem, params));

    // Map parameters to their storage
    REQUIRE_OK(map_parameters(mem, inline_asm, params));

    // Allocate stack space for outputs
    REQUIRE_OK(allocate_output_params(codegen, params));

    // Preserve dirty regs
    REQUIRE_OK(preserve_dirty_regs(mem, codegen, params));

    // Preserve parameter addresses
    REQUIRE_OK(preserve_parameter_addresses(codegen, inline_asm, params));

    // Load inputs
    REQUIRE_OK(load_inputs(codegen, inline_asm, params));

    // Generate assembly code from template
    REQUIRE_OK(format_template(mem, codegen, inline_asm, params));

    // Store outputs
    REQUIRE_OK(store_outputs(codegen, inline_asm, params));

    // Restore dirty regs
    REQUIRE_OK(restore_dirty_regs(codegen, params));

    // Clean stack
    REQUIRE_OK(clear_stack(codegen, params));
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
    REQUIRE_OK(kefir_hashtree_init(&params.parameter_mapping, &kefir_hashtree_str_ops));
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
