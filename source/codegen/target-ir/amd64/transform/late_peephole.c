#include "kefir/codegen/target-ir/amd64/late_transform.h"
#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/codegen/target-ir/amd64/regalloc.h"
#include "kefir/codegen/target-ir/tie.h"
#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"


kefir_result_t kefir_codegen_target_ir_amd64_transform_late_peephole(struct kefir_mem *mem, const struct kefir_codegen_target_ir_code *code,
    kefir_codegen_target_ir_instruction_ref_t instr_ref, const struct kefir_codegen_target_ir_regalloc *regalloc, const struct kefir_hashset *alive_values, struct kefir_asmcmp_instruction *asmcmp_instr) {
    UNUSED(instr_ref);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));
    REQUIRE(alive_values != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid set of alive target IR values"));
    REQUIRE(asmcmp_instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp instruction"));

#define NO_MATCH_ERROR KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match late amd64 peephole transformation")

    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));

    if (instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(mov) ||
        instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(assign)) {
        struct kefir_codegen_target_ir_tie_classification classification;
        REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr_ref, &classification));

        REQUIRE(classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE &&
            classification.classification.operands[1].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
            classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            (instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER ||
            instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UINTEGER) &&
            instr->operation.parameters[classification.operands[1].read_index].immediate.int_immediate == 0 &&
            !instr->operation.parameters[classification.operands[1].read_index].segment.present, NO_MATCH_ERROR);

        const struct kefir_codegen_target_ir_value_type *output_value_type;
        kefir_result_t res = kefir_codegen_target_ir_code_value_props(code, classification.operands[0].output, &output_value_type);
        REQUIRE(res != KEFIR_NOT_FOUND, NO_MATCH_ERROR);
        REQUIRE_OK(res);
        REQUIRE(output_value_type->kind == KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE &&
            (output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
            output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT ||
            output_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT), NO_MATCH_ERROR);

        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t key;
        for (res = kefir_hashset_iter(alive_values, &iter, &key);
            res == KEFIR_OK;
            res = kefir_hashset_next(&iter, &key)) {
            kefir_codegen_target_ir_value_ref_t alive_value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(key);
            if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_RESOURCE(alive_value_ref.aspect)) {
                kefir_codegen_target_ir_resource_id_t resource_id = KEFIR_CODEGEN_TARGET_IR_VALUE_GET_OUTPUT_INDEX(alive_value_ref.aspect);
                REQUIRE((resource_id != KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF) &&
                        (resource_id != KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF) &&
                        (resource_id != KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF) &&
                        (resource_id != KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF) &&
                        (resource_id != KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF), NO_MATCH_ERROR);
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        union kefir_codegen_target_ir_amd64_regalloc_entry regalloc_entry;
        res = kefir_codegen_target_ir_regalloc_get(regalloc, classification.operands[0].output, &regalloc_entry.allocation);
        REQUIRE(res != KEFIR_NOT_FOUND, NO_MATCH_ERROR);
        REQUIRE_OK(res);
        REQUIRE(regalloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP, NO_MATCH_ERROR);
        
        kefir_asm_amd64_xasmgen_register_t reg32;
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(regalloc_entry.reg.value, &reg32));
        *asmcmp_instr = (struct kefir_asmcmp_instruction) {
            .opcode = KEFIR_ASMCMP_AMD64_OPCODE(xor),
            .args[0] = {
                .type = KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER,
                .phreg = reg32
            },
            .args[1] = {
                .type = KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER,
                .phreg = reg32
            }
        };

        return KEFIR_OK;
    } else if (instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(add) ||
        instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(sub)) {
        struct kefir_codegen_target_ir_tie_classification classification;
        REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr_ref, &classification));

        REQUIRE(classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
            classification.classification.operands[1].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
            classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            instr->operation.parameters[classification.operands[0].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            (instr->operation.parameters[classification.operands[0].read_index].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
            instr->operation.parameters[classification.operands[0].read_index].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT ||
            instr->operation.parameters[classification.operands[0].read_index].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) &&
            instr->operation.parameters[classification.operands[1].read_index].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER &&
            !instr->operation.parameters[classification.operands[1].read_index].segment.present, NO_MATCH_ERROR);

        union kefir_codegen_target_ir_amd64_regalloc_entry src_regalloc_entry, dst_regalloc_entry;
        kefir_result_t res = kefir_codegen_target_ir_regalloc_get(regalloc, classification.operands[0].output, &dst_regalloc_entry.allocation);
        REQUIRE(res != KEFIR_NOT_FOUND, NO_MATCH_ERROR);
        REQUIRE_OK(res);
        REQUIRE(dst_regalloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP, NO_MATCH_ERROR);
        res = kefir_codegen_target_ir_regalloc_get(regalloc, instr->operation.parameters[classification.operands[0].read_index].direct.value_ref, &src_regalloc_entry.allocation);
        REQUIRE(res != KEFIR_NOT_FOUND, NO_MATCH_ERROR);
        REQUIRE_OK(res);
        REQUIRE(src_regalloc_entry.type == KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP, NO_MATCH_ERROR);
        REQUIRE(src_regalloc_entry.reg.value != dst_regalloc_entry.reg.value, NO_MATCH_ERROR);

        struct kefir_codegen_target_ir_use_iterator use_iter;
        kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
        kefir_codegen_target_ir_value_ref_t used_value_ref;
        for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, &used_value_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &used_value_ref)) {
            REQUIRE(used_value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), NO_MATCH_ERROR);
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        kefir_int64_t value = instr->operation.parameters[classification.operands[1].read_index].immediate.int_immediate;
        switch (instr->operation.parameters[classification.operands[1].read_index].immediate.variant) {
            case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT:
                value = (kefir_int8_t) value;
                break;

            case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT:
                value = (kefir_int16_t) value;
                break;

            case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT:
                value = (kefir_int32_t) value;
                break;

            default:
                // Intentionally left blank
                break;
        }
        if (instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(sub)) {
            value *= -1;
        }

        REQUIRE(value >= KEFIR_INT32_MIN && value <= KEFIR_INT32_MAX, NO_MATCH_ERROR);

        kefir_asm_amd64_xasmgen_register_t src_reg = src_regalloc_entry.reg.value, dst_reg = dst_regalloc_entry.reg.value;
        if (instr->operation.parameters[classification.operands[0].read_index].direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) {
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(src_reg, &src_reg));
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(dst_reg, &dst_reg));
        }

        *asmcmp_instr = (struct kefir_asmcmp_instruction) {
            .opcode = KEFIR_ASMCMP_AMD64_OPCODE(lea),
            .args[0] = {
                .type = KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER,
                .phreg = dst_reg
            },
            .args[1] = {
                .type = KEFIR_ASMCMP_VALUE_TYPE_INDIRECT,
                .indirect = {
                    .type = KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS,
                    .base.phreg = src_reg,
                    .offset = value,
                    .variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT
                }
            },
        };

        return KEFIR_OK;
    }

    return NO_MATCH_ERROR;
#undef NO_MATCH_ERROR
}
