#ifndef KEFIR_TARGET_DWARF_GENERATOR_H_
#define KEFIR_TARGET_DWARF_GENERATOR_H_

#include "kefir/target/asm/amd64/xasmgen.h"
#include "kefir/target/dwarf/dwarf.h"

#define KEFIR_AMD64_DWARF_DEBUG_ABBREV_BEGIN "__kefir_debug_abbrev_section_begin"
#define KEFIR_AMD64_DWARF_DEBUG_INFO_BEGIN "__kefir_debug_info_section_begin"
#define KEFIR_AMD64_DWARF_DEBUG_INFO_END "__kefir_debug_info_section_end"
#define KEFIR_AMD64_DWARF_DEBUG_LINES "__kefir_debug_lines_section_begin"

kefir_result_t kefir_amd64_dwarf_byte(struct kefir_amd64_xasmgen *, kefir_uint8_t);
kefir_result_t kefir_amd64_dwarf_word(struct kefir_amd64_xasmgen *, kefir_uint16_t);
kefir_result_t kefir_amd64_dwarf_qword(struct kefir_amd64_xasmgen *, kefir_uint64_t);
kefir_result_t kefir_amd64_dwarf_string(struct kefir_amd64_xasmgen *, const char *);
kefir_result_t kefir_amd64_dwarf_uleb128(struct kefir_amd64_xasmgen *, kefir_uint64_t);

kefir_result_t kefir_amd64_dwarf_attribute_abbrev(struct kefir_amd64_xasmgen *, kefir_dwarf_attribute_t,
                                                  kefir_dwarf_form_t);
kefir_result_t kefir_amd64_dwarf_entry_abbrev(struct kefir_amd64_xasmgen *, kefir_uint64_t, kefir_dwarf_tag_t,
                                              kefir_dwarf_children_t);

typedef enum kefir_dwarf_generator_section {
    KEFIR_DWARF_GENERATOR_SECTION_INIT = 0,
    KEFIR_DWARF_GENERATOR_SECTION_ABBREV = 0,
    KEFIR_DWARF_GENERATOR_SECTION_INFO = 1,
    KEFIR_DWARF_GENERATOR_SECTION_LINES = 2,
    KEFIR_DWARF_GENERATOR_SECTION_COUNT
} kefir_dwarf_generator_section_t;

typedef enum kefir_dwarf_generator_debug_information_entry_stage {
    KEFIR_DWARF_GENERATOR_DIE_INITIALIZE = 0,
    KEFIR_DWARF_GENERATOR_DIE_FINALIZE
} kefir_dwarf_generator_debug_information_entry_stage_t;

kefir_result_t kefir_dwarf_generator_section_init(struct kefir_amd64_xasmgen *, kefir_dwarf_generator_section_t);
kefir_result_t kefir_dwarf_generator_section_finalize(struct kefir_amd64_xasmgen *, kefir_dwarf_generator_section_t);
kefir_result_t kefir_dwarf_generator_debug_information_entry(struct kefir_amd64_xasmgen *, kefir_uint64_t,
                                                             kefir_dwarf_tag_t, kefir_dwarf_children_t,
                                                             kefir_dwarf_generator_section_t,
                                                             kefir_dwarf_generator_debug_information_entry_stage_t);

#define KEFIR_AMD64_DWARF_BYTE(_xasmgen, _value) ((kefir_amd64_dwarf_byte((_xasmgen), (_value))))
#define KEFIR_AMD64_DWARF_WORD(_xasmgen, _value) ((kefir_amd64_dwarf_word((_xasmgen), (_value))))
#define KEFIR_AMD64_DWARF_QWORD(_xasmgen, _value) ((kefir_amd64_dwarf_qword((_xasmgen), (_value))))
#define KEFIR_AMD64_DWARF_STRING(_xasmgen, _value) ((kefir_amd64_dwarf_string((_xasmgen), (_value))))
#define KEFIR_AMD64_DWARF_ULEB128(_xasmgen, _value) ((kefir_amd64_dwarf_uleb128((_xasmgen), (_value))))

#define KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV(_xasmgen, _attr, _form) \
    (kefir_amd64_dwarf_attribute_abbrev((_xasmgen), (_attr), (_form)))
#define KEFIR_AMD64_DWARF_ENTRY_ABBREV(_xasmgen, _identifier, _tag, _children) \
    (kefir_amd64_dwarf_entry_abbrev((_xasmgen), (_identifier), (_tag), (_children)))
#define KEFIR_AMD64_DWARF_ENTRY_INFO(_xasmgen, _identifier) (KEFIR_AMD64_DWARF_ULEB128((_xasmgen), (_identifier)))
#define KEFIR_AMD64_DWARF_ENTRY_INFO_CHILDREN_END(_xasmgen) (KEFIR_AMD64_DWARF_ULEB128((_xasmgen), 0))

#define KEFIR_AMD64_DWARF_ATTRIBUTE(_section, _xasmgen, _attr, _form, _info)  \
    ((_section) == KEFIR_DWARF_GENERATOR_SECTION_ABBREV                       \
         ? (KEFIR_AMD64_DWARF_ATTRIBUTE_ABBREV((_xasmgen), (_attr), (_form))) \
         : ((_section) == KEFIR_DWARF_GENERATOR_SECTION_INFO                  \
                ? (_info)                                                     \
                : ((_section) < KEFIR_DWARF_GENERATOR_SECTION_COUNT           \
                       ? KEFIR_OK                                             \
                       : KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected DWARF generator section"))))
#define KEFIR_AMD64_DWARF_ATTRIBUTE_END(_section, _xasmgen)                             \
    KEFIR_AMD64_DWARF_ATTRIBUTE((_section), (_xasmgen), KEFIR_DWARF(DW_AT_end_of_list), \
                                KEFIR_DWARF(DW_FORM_end_of_list), KEFIR_OK)

#define KEFIR_AMD64_DWARF_SECTION_INIT(_xasmgen, _section) (kefir_dwarf_generator_section_init((_xasmgen), (_section)))
#define KEFIR_AMD64_DWARF_SECTION_FINALIZE(_xasmgen, _section) \
    (kefir_dwarf_generator_section_finalize((_xasmgen), (_section)))
#define KEFIR_DWARF_GENERATOR_DEBUG_INFO(_section)                                                            \
    for (*(_section) = KEFIR_DWARF_GENERATOR_SECTION_INIT; *(_section) < KEFIR_DWARF_GENERATOR_SECTION_COUNT; \
         (*(_section))++)

#define KEFIR_DWARF_GENERATOR_SECTION_ONLY(_section, _desired) if ((_section) == (_desired))
#define KEFIR_DWARF_GENERATOR_ENTRY(_res, _xasmgen, _section, _identifier, _tag, _children)                            \
    for (kefir_dwarf_generator_debug_information_entry_stage_t kefir_die_stage = KEFIR_DWARF_GENERATOR_DIE_INITIALIZE; \
         *(_res) == KEFIR_OK &&                                                                                        \
         (*(_res) = kefir_dwarf_generator_debug_information_entry((_xasmgen), (_identifier), (_tag), (_children),      \
                                                                  *(_section), kefir_die_stage),                       \
         *(_res) == KEFIR_OK) &&                                                                                       \
         kefir_die_stage < KEFIR_DWARF_GENERATOR_DIE_FINALIZE;                                                         \
         kefir_die_stage++)

#endif
