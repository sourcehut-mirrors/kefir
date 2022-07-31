#include "kefir/codegen/amd64/system-v/inline_assembly.h"
#include "kefir/codegen/amd64/system-v/runtime.h"
#include "kefir/codegen/amd64/system-v/abi/qwords.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

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
    struct kefir_list available_float_regs;
    struct kefir_list preserved_regs;
};

static kefir_bool_t inline_assembly_is_register_preserved(kefir_amd64_xasmgen_register_t reg) {
    UNUSED(reg);
    // TODO Implement specific preserved register list
    return true;
}

static kefir_result_t inline_assembly_embed_impl(struct kefir_mem *mem,
                                                 struct kefir_codegen_amd64_sysv_module *sysv_module,
                                                 struct kefir_codegen_amd64 *codegen,
                                                 const struct kefir_ir_inline_assembly *inline_asm,
                                                 struct inline_assembly_params *params) {
    UNUSED(sysv_module);

    // Mark clobber regs as dirty
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

    // Initialize available register lists
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

    AVAIL_REG(&params->available_float_regs, KEFIR_AMD64_XASMGEN_REGISTER_XMM0);
    AVAIL_REG(&params->available_float_regs, KEFIR_AMD64_XASMGEN_REGISTER_XMM1);
    AVAIL_REG(&params->available_float_regs, KEFIR_AMD64_XASMGEN_REGISTER_XMM2);
    AVAIL_REG(&params->available_float_regs, KEFIR_AMD64_XASMGEN_REGISTER_XMM3);
    AVAIL_REG(&params->available_float_regs, KEFIR_AMD64_XASMGEN_REGISTER_XMM4);
    AVAIL_REG(&params->available_float_regs, KEFIR_AMD64_XASMGEN_REGISTER_XMM5);
    AVAIL_REG(&params->available_float_regs, KEFIR_AMD64_XASMGEN_REGISTER_XMM6);
    AVAIL_REG(&params->available_float_regs, KEFIR_AMD64_XASMGEN_REGISTER_XMM7);
#undef AVAIL_REG

    // TODO Map registers to parameters

    // Preserve dirty regs
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
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(reg)));
            }
        }
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INLINE_ASSEMBLY(&codegen->xasmgen, inline_asm->template));

    // Restore dirty regs
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

kefir_result_t kefir_codegen_amd64_sysv_inline_assembly_embed(struct kefir_mem *mem,
                                                              struct kefir_codegen_amd64_sysv_module *sysv_module,
                                                              struct kefir_codegen_amd64 *codegen,
                                                              const struct kefir_ir_inline_assembly *inline_asm) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(sysv_module != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen AMD64 System-V module"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen AMD64 codegen"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly"));

    struct inline_assembly_params params;
    REQUIRE_OK(kefir_hashtree_init(&params.dirty_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_list_init(&params.available_int_regs));
    REQUIRE_OK(kefir_list_init(&params.available_float_regs));
    REQUIRE_OK(kefir_list_init(&params.preserved_regs));

    kefir_result_t res = inline_assembly_embed_impl(mem, sysv_module, codegen, inline_asm, &params);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &params.preserved_regs);
        kefir_list_free(mem, &params.available_float_regs);
        kefir_list_free(mem, &params.available_int_regs);
        kefir_hashtree_free(mem, &params.dirty_regs);
        return res;
    });

    res = kefir_list_free(mem, &params.preserved_regs);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &params.available_float_regs);
        kefir_list_free(mem, &params.available_int_regs);
        kefir_hashtree_free(mem, &params.dirty_regs);
        return res;
    });

    res = kefir_list_free(mem, &params.available_float_regs);
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
