#ifndef KEFIR_CODEGEN_ASMCMP_TYPE_DEFS_H_
#define KEFIR_CODEGEN_ASMCMP_TYPE_DEFS_H_

#include "kefir/core/basic-types.h"
#include "kefir/codegen/asmcmp/base.h"

typedef kefir_size_t kefir_asmcmp_virtual_register_index_t;
typedef kefir_size_t kefir_asmcmp_physical_register_index_t;
typedef kefir_size_t kefir_asmcmp_instruction_opcode_t;
typedef kefir_size_t kefir_asmcmp_instruction_index_t;
typedef kefir_size_t kefir_asmcmp_label_index_t;
typedef kefir_size_t kefir_asmcmp_stash_index_t;
typedef kefir_size_t kefir_asmcmp_inline_assembly_index_t;
typedef kefir_size_t kefir_asmcmp_linear_reference_index_t;

#define KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS 3
#define KEFIR_ASMCMP_INDEX_NONE (~(kefir_asmcmp_instruction_index_t) 0ull)

#endif
