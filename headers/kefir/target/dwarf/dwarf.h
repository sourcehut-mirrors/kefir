#ifndef KEFIR_TARGET_DWARF_DWARF_H_
#define KEFIR_TARGET_DWARF_DWARF_H_

#define KEFIR_DWARF(_id) kefir_dwarf_##_id

#define KEFIR_DWARF_VESION 0x5

enum { KEFIR_DWARF(null) = 0x0 };

typedef enum kefir_dwarf_unit_header_type { KEFIR_DWARF(DW_UT_compile) = 0x01 } kefir_dwarf_unit_header_type_t;

typedef enum kefir_dwarf_children {
    KEFIR_DWARF(DW_CHILDREN_no) = 0,
    KEFIR_DWARF(DW_CHILDREN_yes) = 1
} kefir_dwarf_children_t;

typedef enum kefir_dwarf_tag { KEFIR_DWARF(DW_TAG_compile_unit) = 0x11 } kefir_dwarf_tag_t;

typedef enum kefir_dwarf_attribute {
    KEFIR_DWARF(DW_AT_end_of_list) = 0x0,
    KEFIR_DWARF(DW_AT_stmt_list) = 0x10,
    KEFIR_DWARF(DW_AT_low_pc) = 0x11,
    KEFIR_DWARF(DW_AT_high_pc) = 0x12,
    KEFIR_DWARF(DW_AT_language) = 0x13,
    KEFIR_DWARF(DW_AT_producer) = 0x25
} kefir_dwarf_attribute_t;

typedef enum kefir_dwarf_form {
    KEFIR_DWARF(DW_FORM_end_of_list) = 0x0,
    KEFIR_DWARF(DW_FORM_addr) = 0x01,
    KEFIR_DWARF(DW_FORM_data2) = 0x5,
    KEFIR_DWARF(DW_FORM_data8) = 0x7,
    KEFIR_DWARF(DW_FORM_udata) = 0xf,
    KEFIR_DWARF(DW_FORM_string) = 0x8,
    KEFIR_DWARF(DW_FORM_sec_offset) = 0x17
} kefir_dwarf_form_t;

typedef enum kefir_dwarf_language { KEFIR_DWARF(DW_LANG_C11) = 0x1d } kefir_dwarf_language_t;

#endif
