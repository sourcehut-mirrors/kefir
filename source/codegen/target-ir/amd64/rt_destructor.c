#include "kefir/codegen/target-ir/amd64/rt_destructor.h"
#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/codegen/amd64/function.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t classify_instruction(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref,
    struct kefir_codegen_target_ir_target_ir_instruction_destructor_classification *classification, void *payload) {
    UNUSED(payload);
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(classification != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction classification"));

    const struct kefir_codegen_target_ir_instruction *instruction;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instruction));

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

        case KEFIR_TARGET_IR_AMD64_OPCODE(placeholder):
            classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(produce_virtual_register);
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[0].index = 0;
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
        } else if ((_op) & KEFIR_AMD64_INSTRDB_FPU_STACK) { \
            classification->operands[(_index)].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ; \
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

#define INSTR0_VIRT(_opcode, _mnemonic)        \
    case KEFIR_TARGET_IR_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_ASMCMP_AMD64_OPCODE(_opcode); \
        break;

        KEFIR_CODEGEN_TARGET_IR_AMD64_VIRTUAL_OPCODES(INSTR0_VIRT, )
        KEFIR_AMD64_INSTRUCTION_DATABASE(INSTR0, INSTR1, INSTR2, INSTR3, )
#undef INSTR0
#undef INSTR1
#undef INSTR2
#undef INSTR3

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 opcode");
    }

    kefir_size_t output_index = 0;
    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        if (classification->operands[i].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE) {
            const struct kefir_codegen_target_ir_value_type *value_type;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction_output(code, instr_ref, output_index++, NULL, &value_type));
            if (value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT || value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT) {
                classification->operands[i].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
            }
        }
    }

    switch (instruction->operation.opcode) {
        case KEFIR_TARGET_IR_AMD64_OPCODE(movsb):
        case KEFIR_TARGET_IR_AMD64_OPCODE(movsw):
        case KEFIR_TARGET_IR_AMD64_OPCODE(movsl):
        case KEFIR_TARGET_IR_AMD64_OPCODE(movsq):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_params.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDI;
            classification->operands[0].implicit_params.vreg_type = KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_params.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RSI;
            classification->operands[1].implicit_params.vreg_type = KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE;
            break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(stosb):
        case KEFIR_TARGET_IR_AMD64_OPCODE(stosw):
        case KEFIR_TARGET_IR_AMD64_OPCODE(stosl):
        case KEFIR_TARGET_IR_AMD64_OPCODE(stosq):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_params.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDI;
            classification->operands[0].implicit_params.vreg_type = KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_params.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RSI;
            classification->operands[1].implicit_params.vreg_type = KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE;
            break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(cmpxchg): {
            REQUIRE(num_of_params == 2 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[2].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[2].implicit = true;
            classification->operands[2].implicit_params.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            classification->operands[2].implicit_params.vreg_type = KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE;
        } break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(mul):
        case KEFIR_TARGET_IR_AMD64_OPCODE(imul):
            if (implicit_params) {
                REQUIRE(num_of_params == 1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
                classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
                classification->operands[1].implicit = true;
                classification->operands[1].implicit_params.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
                classification->operands[1].implicit_params.vreg_type = KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE;
                kefir_codegen_target_ir_value_ref_t value_ref;
                kefir_result_t res = kefir_codegen_target_ir_code_instruction_output(code, instr_ref, 1, &value_ref, NULL);
                if (res != KEFIR_NOT_FOUND) {
                    REQUIRE_OK(res);
                    classification->operands[2].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
                    classification->operands[2].implicit = true;
                    classification->operands[2].implicit_params.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
                    classification->operands[2].implicit_params.vreg_type = KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE;
                }
            }
            break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(div):
        case KEFIR_TARGET_IR_AMD64_OPCODE(idiv):
            if (implicit_params) {
                REQUIRE(num_of_params == 1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
                classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
                classification->operands[1].implicit = true;
                classification->operands[1].implicit_params.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
                classification->operands[1].implicit_params.vreg_type = KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE;
                kefir_codegen_target_ir_value_ref_t value_ref;
                kefir_result_t res = kefir_codegen_target_ir_code_instruction_output(code, instr_ref, 1, &value_ref, NULL);
                if (res != KEFIR_NOT_FOUND) {
                    REQUIRE_OK(res);
                    classification->operands[2].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
                    classification->operands[2].implicit = true;
                    classification->operands[2].implicit_params.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
                    classification->operands[2].implicit_params.vreg_type = KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE;
                }
            }
            break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(cwd):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_params.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            classification->operands[0].implicit_params.vreg_type = KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_params.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
            classification->operands[1].implicit_params.vreg_type = KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE;
            break;
            
        case KEFIR_TARGET_IR_AMD64_OPCODE(cdq):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_params.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            classification->operands[0].implicit_params.vreg_type = KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_params.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
            classification->operands[1].implicit_params.vreg_type = KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE;
            break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(cqo):
            REQUIRE(num_of_params == 0 && implicit_params, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 instruction shape"));
            classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
            classification->operands[0].implicit = true;
            classification->operands[0].implicit_params.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
            classification->operands[0].implicit_params.vreg_type = KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE;
            classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
            classification->operands[1].implicit = true;
            classification->operands[1].implicit_params.phreg = KEFIR_AMD64_XASMGEN_REGISTER_RDX;
            classification->operands[1].implicit_params.vreg_type = KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE;
            break;
    }

    return KEFIR_OK;
}

static kefir_result_t bind_native_id(struct kefir_mem *mem, kefir_asmcmp_label_index_t label, kefir_codegen_target_ir_native_id_t native_id, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_round_trip_destructor_amd64_ops *, ops,
        payload);
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid target IR destructor amd64 ops"));
    
    kefir_result_t res = kefir_hashtree_insert(mem, &ops->constants, (kefir_hashtree_key_t) label, (kefir_hashtree_value_t) native_id);
    if (res == KEFIR_ALREADY_EXISTS) {
        REQUIRE_OK(res);
    }
    REQUIRE_OK(res);
    return KEFIR_OK;
}

static kefir_result_t preallocation_requirement(struct kefir_mem *mem, kefir_asmcmp_virtual_register_index_t vreg_idx, kefir_codegen_target_ir_physical_register_t phreg, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_round_trip_destructor_amd64_ops *, ops,
        payload);
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid target IR destructor amd64 ops"));
    
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, ops->code, vreg_idx, (kefir_asm_amd64_xasmgen_register_t) phreg));
    return KEFIR_OK;
}

static kefir_result_t preallocation_hint(struct kefir_mem *mem, kefir_asmcmp_virtual_register_index_t vreg_idx, kefir_codegen_target_ir_physical_register_t phreg, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_round_trip_destructor_amd64_ops *, ops,
        payload);
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid target IR destructor amd64 ops"));
    
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_hint(mem, ops->code, vreg_idx, (kefir_asm_amd64_xasmgen_register_t) phreg));
    return KEFIR_OK;
}

static kefir_result_t split_branch_instruction(struct kefir_mem *mem, const struct kefir_codegen_target_ir_instruction *instruction, struct kefir_asmcmp_instruction asmcmp_instrs[2], void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_round_trip_destructor_amd64_ops *, ops,
        payload);
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid target IR destructor amd64 ops"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid target IR instruction"));
    
    switch (instruction->operation.opcode) {
#define DEF_OPCODE_NOOP(...)
#define DEF_OPCODE1(_opcode, _mnemonic, _branch, _flags, ...) CASE_IS_##_branch(_opcode, _flags)
#define CASE_IS_BRANCH(_opcode, _flags) \
        case KEFIR_TARGET_IR_AMD64_OPCODE(_opcode): \
            REQUIRE(KEFIR_TARGET_IR_AMD64_OPCODE(_opcode) != KEFIR_TARGET_IR_AMD64_OPCODE(call), KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 target IR branch instruction")); \
            asmcmp_instrs[0].opcode = KEFIR_ASMCMP_AMD64_OPCODE(_opcode); \
            asmcmp_instrs[0].args[0].type = KEFIR_ASMCMP_VALUE_TYPE_NONE; \
            asmcmp_instrs[0].args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE; \
            asmcmp_instrs[0].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE; \
            asmcmp_instrs[1].opcode = KEFIR_ASMCMP_AMD64_OPCODE(jmp); \
            asmcmp_instrs[1].args[0].type = KEFIR_ASMCMP_VALUE_TYPE_NONE; \
            asmcmp_instrs[1].args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE; \
            asmcmp_instrs[1].args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE; \
            break;
#define CASE_IS_(...)

        KEFIR_AMD64_INSTRUCTION_DATABASE(DEF_OPCODE_NOOP, DEF_OPCODE1, DEF_OPCODE_NOOP, DEF_OPCODE_NOOP,)
#undef DEF_OPCODE_NOOP
#undef DEF_OPCODE1
#undef DEF_OPCODE0

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 target IR branch instruction");
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_round_trip_destructor_amd64_ops_init(const struct kefir_codegen_amd64_function *function, struct kefir_asmcmp_amd64 *code, struct kefir_codegen_target_ir_round_trip_destructor_amd64_ops *ops) {
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 asmcmp code"));
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid pointer to target IR destructor amd64 ops"));

    ops->function = function;
    ops->code = code;
    ops->ops.link_virtual_registers_opcode = KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link);
    ops->ops.touch_virtual_register_opcode = KEFIR_ASMCMP_AMD64_OPCODE(touch_virtual_register);
    ops->ops.classify_instruction = classify_instruction;
    ops->ops.bind_native_id = bind_native_id;
    ops->ops.preallocation_requirement = preallocation_requirement;
    ops->ops.preallocation_hint = preallocation_hint;
    ops->ops.split_branch_instruction = split_branch_instruction;
    ops->ops.payload = ops;

    REQUIRE_OK(kefir_hashtree_init(&ops->constants, &kefir_hashtree_uint_ops));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_round_trip_destructor_amd64_ops_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_round_trip_destructor_amd64_ops *ops) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid memory allocator"));
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid target IR destructor amd64 ops"));

    REQUIRE_OK(kefir_hashtree_free(mem, &ops->constants));
    return KEFIR_OK;
}
