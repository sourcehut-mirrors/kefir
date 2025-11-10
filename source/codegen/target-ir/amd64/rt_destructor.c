#include "kefir/codegen/target-ir/amd64/rt_destructor.h"
#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t destructor_touch_virtual_register(struct kefir_mem *mem, kefir_asmcmp_instruction_index_t insert_after_idx, kefir_asmcmp_virtual_register_index_t vreg_idx, kefir_asmcmp_instruction_index_t *instr_idx_ptr, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_asmcmp_amd64 *, asmcmp_code,
        payload);
    REQUIRE(asmcmp_code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 asmcmp context"));

    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(mem, asmcmp_code, insert_after_idx, vreg_idx, instr_idx_ptr));
    return KEFIR_OK;
}

static kefir_result_t destructor_link_virtual_register(struct kefir_mem *mem, kefir_asmcmp_instruction_index_t insert_after_idx, kefir_asmcmp_virtual_register_index_t vreg1_idx, kefir_asmcmp_virtual_register_index_t vreg2_idx, kefir_asmcmp_instruction_index_t *instr_idx_ptr, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_asmcmp_amd64 *, asmcmp_code,
        payload);
    REQUIRE(asmcmp_code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 asmcmp context"));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, asmcmp_code, insert_after_idx, vreg1_idx, vreg2_idx, instr_idx_ptr));
    return KEFIR_OK;
}

static kefir_result_t amd64_classify_instruction(const struct kefir_codegen_target_ir_instruction *instruction,
    struct kefir_codegen_target_ir_target_ir_instruction_destructor_classification *classification, void *payload) {
    UNUSED(payload);
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp instruction"));
    REQUIRE(classification != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp instruction"));

    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        classification->operands[i].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_NONE;
        classification->operands[i].implicit = false;
        classification->operands[i].index = 0;
    }

    switch (instruction->operation.opcode) {
        case KEFIR_TARGET_IR_AMD64_OPCODE(assign):
            classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link);
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[0].index = 0;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[1].index = 1;
            return KEFIR_OK;

        case KEFIR_TARGET_IR_AMD64_OPCODE(touch):
            classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(touch_virtual_register);
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].index = 0;
            return KEFIR_OK;

        case KEFIR_TARGET_IR_AMD64_OPCODE(tail_call):
            classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(tail_call);
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].index = 0;
            return KEFIR_OK;

        case KEFIR_TARGET_IR_AMD64_OPCODE(phi):
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to classify target IR phi node");

        case KEFIR_TARGET_IR_AMD64_OPCODE(placeholder):
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to classify target IR placeholder instruction");

        default:
            // Intentionally left blank
            break;
    }

    kefir_bool_t implicit_params = false;
    kefir_size_t num_of_params = 0;

#define CLASSIFY_OP(_op, _index) \
    do { \
        if (((_op) & KEFIR_AMD64_INSTRDB_READ) && ((_op) & KEFIR_AMD64_INSTRDB_WRITE)) { \
            classification->operands[(_index)].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE; \
            classification->operands[(_index)].index = (_index); \
        } else if ((_op) & KEFIR_AMD64_INSTRDB_READ) { \
            classification->operands[(_index)].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ; \
            classification->operands[(_index)].index = (_index); \
        } else if ((_op) & KEFIR_AMD64_INSTRDB_WRITE) { \
            classification->operands[(_index)].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE; \
            classification->operands[(_index)].index = (_index); \
        } \
    } while (0)

    switch (instruction->operation.opcode) {
#define INSTR0(_opcode, _mnemonic, _variant, _flags)        \
    case KEFIR_TARGET_IR_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(_opcode); \
        implicit_params = ((_flags) & KEFIR_AMD64_INSTRDB_IMPLICIT_PARAMS) != 0; \
        break;

#define INSTR1(_opcode, _mnemonic, _variant, _flags, _op1)        \
    case KEFIR_TARGET_IR_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(_opcode);            \
        CLASSIFY_OP(_op1, 0); \
        implicit_params = ((_flags) & KEFIR_AMD64_INSTRDB_IMPLICIT_PARAMS) != 0; \
        num_of_params = 1; \
        break;

#define INSTR2(_opcode, _mnemonic, _variant, _flags, _op1, _op2)        \
    case KEFIR_TARGET_IR_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(_opcode);            \
        CLASSIFY_OP(_op1, 0); \
        CLASSIFY_OP(_op2, 1); \
        implicit_params = ((_flags) & KEFIR_AMD64_INSTRDB_IMPLICIT_PARAMS) != 0; \
        num_of_params = 2; \
        break;

#define INSTR3(_opcode, _mnemonic, _variant, _flags, _op1, _op2, _op3)        \
    case KEFIR_TARGET_IR_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(_opcode);            \
        CLASSIFY_OP(_op1, 0); \
        CLASSIFY_OP(_op2, 1); \
        CLASSIFY_OP(_op2, 2); \
        implicit_params = ((_flags) & KEFIR_AMD64_INSTRDB_IMPLICIT_PARAMS) != 0; \
        num_of_params = 3; \
        break;

        KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODES(INSTR0, )
        KEFIR_AMD64_INSTRUCTION_DATABASE(INSTR0, INSTR1, INSTR2, INSTR3, )
#undef INSTR0
#undef INSTR1
#undef INSTR2
#undef INSTR3

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 opcode");
    }

    switch (instruction->operation.opcode) {
        case KEFIR_TARGET_IR_AMD64_OPCODE(movsb):
        case KEFIR_TARGET_IR_AMD64_OPCODE(movsw):
        case KEFIR_TARGET_IR_AMD64_OPCODE(movsl):
        case KEFIR_TARGET_IR_AMD64_OPCODE(movsq):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDI;
            classification->operands[0].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RSI;
            classification->operands[1].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT;
            break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(stosb):
        case KEFIR_TARGET_IR_AMD64_OPCODE(stosw):
        case KEFIR_TARGET_IR_AMD64_OPCODE(stosl):
        case KEFIR_TARGET_IR_AMD64_OPCODE(stosq):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDI;
            classification->operands[0].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            classification->operands[1].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT;
            break;

#define MATCH_VARIANT(_operand, _arg) \
        do { \
            kefir_asmcmp_operand_variant_t native_variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT; \
            if ((_arg)->type == KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER) { \
                native_variant = (_arg)->vreg.variant; \
            } else if ((_arg)->type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT) { \
                native_variant = (_arg)->indirect.variant; \
            } else if ((_arg)->type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL || (_arg)->type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL) { \
                native_variant = (_arg)->rip_indirection.variant; \
            } \
            switch (native_variant) { \
                case KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT; \
                    break; \
 \
                case KEFIR_ASMCMP_OPERAND_VARIANT_8BIT: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT; \
                    break; \
 \
                case KEFIR_ASMCMP_OPERAND_VARIANT_16BIT: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT; \
                    break; \
 \
                case KEFIR_ASMCMP_OPERAND_VARIANT_32BIT: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT; \
                    break; \
 \
                case KEFIR_ASMCMP_OPERAND_VARIANT_64BIT: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT; \
                    break; \
 \
                case KEFIR_ASMCMP_OPERAND_VARIANT_80BIT: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_80BIT; \
                    break; \
 \
                case KEFIR_ASMCMP_OPERAND_VARIANT_128BIT: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_128BIT; \
                    break; \
 \
                case KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_FP_SINGLE; \
                    break; \
                     \
                case KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE: \
                    (_operand)->implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_FP_DOUBLE; \
                    break; \
            } \
        } while (0)
        case KEFIR_TARGET_IR_AMD64_OPCODE(cmpxchg): {
            REQUIRE(num_of_params == 2 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[2].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[2].implicit = true;
            classification->operands[2].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            // MATCH_VARIANT(&classification->operands[2], &instruction->args[0]);
        } break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(mul):
        case KEFIR_TARGET_IR_AMD64_OPCODE(imul):
            if (implicit_params) {
                REQUIRE(num_of_params == 1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
                classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
                classification->operands[1].implicit = true;
                classification->operands[1].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
                // MATCH_VARIANT(&classification->operands[1], &instruction->args[0]);
                // if ((instruction->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER &&
                //     instruction->args[0].vreg.variant != KEFIR_ASMCMP_OPERAND_VARIANT_8BIT) ||
                //     (instruction->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT &&
                //     instruction->args[0].indirect.variant != KEFIR_ASMCMP_OPERAND_VARIANT_8BIT) ||
                //     ((instruction->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL || instruction->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL) &&
                //     instruction->args[0].rip_indirection.variant != KEFIR_ASMCMP_OPERAND_VARIANT_8BIT)) {
                //     classification->operands[2].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
                //     classification->operands[2].implicit = true;
                //     classification->operands[2].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
                //     MATCH_VARIANT(&classification->operands[2], &instruction->args[0]);
                // }
            }
            break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(div):
        case KEFIR_TARGET_IR_AMD64_OPCODE(idiv):
            if (implicit_params) {
                REQUIRE(num_of_params == 1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
                classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
                classification->operands[1].implicit = true;
                classification->operands[1].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
                // MATCH_VARIANT(&classification->operands[2], &instruction->args[0]);
                // if ((instruction->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER &&
                //     instruction->args[0].vreg.variant != KEFIR_ASMCMP_OPERAND_VARIANT_8BIT) ||
                //     (instruction->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT &&
                //     instruction->args[0].indirect.variant != KEFIR_ASMCMP_OPERAND_VARIANT_8BIT) ||
                //     ((instruction->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL || instruction->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL) &&
                //     instruction->args[0].rip_indirection.variant != KEFIR_ASMCMP_OPERAND_VARIANT_8BIT)) {
                //     classification->operands[2].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
                //     classification->operands[2].implicit = true;
                //     classification->operands[2].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
                //     MATCH_VARIANT(&classification->operands[2], &instruction->args[0]);
                // }
            }
            break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(cwd):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            classification->operands[0].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
            classification->operands[1].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT;
            break;
            
        case KEFIR_TARGET_IR_AMD64_OPCODE(cdq):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            classification->operands[0].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
            classification->operands[1].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT;
            break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(cqo):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            classification->operands[0].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_parameter.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
            classification->operands[1].implicit_parameter.variant = KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT;
            break;
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_round_trip_destructor_parameter_amd64_init(struct kefir_asmcmp_amd64 *asmcmp_code, struct kefir_codegen_target_ir_round_trip_destructor_parameter *parameter) {
    REQUIRE(asmcmp_code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp amd64 code"));
    REQUIRE(parameter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid pointer to target IR destructor parameter"));

    parameter->touch_virtual_register = destructor_touch_virtual_register;
    parameter->link_virtual_register = destructor_link_virtual_register;
    parameter->classify_instruction = amd64_classify_instruction;
    parameter->payload = asmcmp_code;
    return KEFIR_OK;
}
