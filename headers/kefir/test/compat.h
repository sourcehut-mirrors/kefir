/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#ifndef KEFIR_TEST_COMPAT_H_
#define KEFIR_TEST_COMPAT_H_

#define kefir_ast_new_constant_bool_noarena(_mem, _value) (kefir_ast_new_constant_bool((_mem), NULL, (_value)))
#define kefir_ast_new_constant_char_noarena(_mem, _value) (kefir_ast_new_constant_char((_mem), NULL, (_value)))
#define kefir_ast_new_constant_wide_char_noarena(_mem, _value) (kefir_ast_new_constant_wide_char((_mem), NULL, (_value)))
#define kefir_ast_new_constant_unicode16_char_noarena(_mem, _value) (kefir_ast_new_constant_unicode16_char((_mem), NULL, (_value)))
#define kefir_ast_new_constant_unicode32_char_noarena(_mem, _value) (kefir_ast_new_constant_unicode32_char((_mem), NULL, (_value)))
#define kefir_ast_new_constant_int_noarena(_mem, _value) (kefir_ast_new_constant_int((_mem), NULL, (_value)))
#define kefir_ast_new_constant_uint_noarena(_mem, _value) (kefir_ast_new_constant_uint((_mem), NULL, (_value)))
#define kefir_ast_new_constant_long_noarena(_mem, _value) (kefir_ast_new_constant_long((_mem), NULL, (_value)))
#define kefir_ast_new_constant_ulong_noarena(_mem, _value) (kefir_ast_new_constant_ulong((_mem), NULL, (_value)))
#define kefir_ast_new_constant_long_long_noarena(_mem, _value) (kefir_ast_new_constant_long_long((_mem), NULL, (_value)))
#define kefir_ast_new_constant_ulong_long_noarena(_mem, _value) (kefir_ast_new_constant_ulong_long((_mem), NULL, (_value)))
#define kefir_ast_new_constant_float_noarena(_mem, _value) (kefir_ast_new_constant_float((_mem), NULL, (_value)))
#define kefir_ast_new_constant_double_noarena(_mem, _value) (kefir_ast_new_constant_double((_mem), NULL, (_value)))
#define kefir_ast_new_constant_long_double_noarena(_mem, _value) (kefir_ast_new_constant_long_double((_mem), NULL, (_value)))

#define kefir_ast_new_identifier_noarena(_mem, _strings, _value) (kefir_ast_new_identifier((_mem), NULL, (_strings), (_value)))

#define kefir_ast_new_string_literal_multibyte_noarena(_mem, _value, _len) (kefir_ast_new_string_literal_multibyte((_mem), NULL, (_value), (_len)))
#define kefir_ast_new_string_literal_unicode8_noarena(_mem, _value, _len) (kefir_ast_new_string_literal_unicode8((_mem), NULL, (_value), (_len)))
#define kefir_ast_new_string_literal_unicode16_noarena(_mem, _value, _len) (kefir_ast_new_string_literal_unicode16((_mem), NULL, (_value), (_len)))
#define kefir_ast_new_string_literal_unicode32_noarena(_mem, _value, _len) (kefir_ast_new_string_literal_unicode32((_mem), NULL, (_value), (_len)))
#define kefir_ast_new_string_literal_wide_noarena(_mem, _value, _len) (kefir_ast_new_string_literal_wide((_mem), NULL, (_value), (_len)))

#define KEFIR_AST_MAKE_STRING_LITERAL_MULTIBYTE(_mem, _string) \
    (kefir_ast_new_string_literal_multibyte((_mem), NULL, (_string), strlen((_string)) + 1))

#define kefir_ast_new_single_declaration_noarena(_mem, _decl, _init, _ptr) (kefir_ast_new_single_declaration((_mem), NULL, (_decl), (_init), (_ptr)))

#define kefir_ast_new_type_name_noarena(_mem, _decl) (kefir_ast_new_type_name((_mem), NULL, (_decl)))
#define kefir_ast_new_compound_literal_noarena(_mem, _type) (kefir_ast_new_compound_literal((_mem), NULL, (_type)))
#define kefir_ast_new_cast_operator_noarena(_mem, _type, _node) (kefir_ast_new_cast_operator((_mem), NULL, (_type), (_node)))
#define kefir_ast_new_array_subscript_noarena(_mem, _base, _index) (kefir_ast_new_array_subscript((_mem), NULL, (_base), (_index)))
#define kefir_ast_new_function_call_noarena(_mem, _base) (kefir_ast_new_function_call((_mem), NULL, (_base)))
#define kefir_ast_new_struct_member_noarena(_mem, _strings, _node, _field) (kefir_ast_new_struct_member((_mem), NULL, (_strings), (_node), (_field)))
#define kefir_ast_new_struct_indirect_member_noarena(_mem, _strings, _node, _field) (kefir_ast_new_struct_indirect_member((_mem), NULL, (_strings), (_node), (_field)))
#define kefir_ast_new_unary_operation_noarena(_mem, _op, _base) (kefir_ast_new_unary_operation((_mem), NULL, (_op), (_base)))
#define kefir_ast_new_binary_operation_noarena(_mem, _op, _lhs, _rhs) (kefir_ast_new_binary_operation((_mem), NULL, (_op), (_lhs), (_rhs)))
#define kefir_ast_new_conditional_operator_noarena(_mem, _cond, _lhs, _rhs) (kefir_ast_new_conditional_operator((_mem), NULL, (_cond), (_lhs), (_rhs)))
#define kefir_ast_new_simple_assignment_noarena(_mem, _lvalue, _rvalue) (kefir_ast_new_simple_assignment((_mem), NULL, (_lvalue), (_rvalue)))
#define kefir_ast_new_compound_assignment_noarena(_mem, _op, _lvalue, _rvalue) (kefir_ast_new_compound_assignment((_mem), NULL, (_op), (_lvalue), (_rvalue)))
#define kefir_ast_new_comma_operator_noarena(_mem) (kefir_ast_new_comma_operator((_mem), NULL))
#define kefir_ast_new_static_assertion_noarena(_mem, _cond, _msg) (kefir_ast_new_static_assertion((_mem), NULL, (_cond), (_msg)))
#define kefir_ast_new_labeled_statement_noarena(_mem, _strings, _label, _node) (kefir_ast_new_labeled_statement((_mem), NULL, (_strings), (_label), (_node)))
#define kefir_ast_new_case_statement_noarena(_mem, _label, _node) (kefir_ast_new_case_statement((_mem), NULL, (_label), (_node)))
#define kefir_ast_new_expression_statement_noarena(_mem, _node) (kefir_ast_new_expression_statement((_mem), NULL, (_node)))
#define kefir_ast_new_compound_statement_noarena(_mem) (kefir_ast_new_compound_statement((_mem), NULL))
#define kefir_ast_new_conditional_statement_noarena(_mem, _cond, _br1, _br2) (kefir_ast_new_conditional_statement((_mem), NULL, (_cond), (_br1), (_br2)))
#define kefir_ast_new_switch_statement_noarena(_mem, _cond, _body) (kefir_ast_new_switch_statement((_mem), NULL, (_cond), (_body)))
#define kefir_ast_new_while_statement_noarena(_mem, _cond, _body) (kefir_ast_new_while_statement((_mem), NULL, (_cond), (_body)))
#define kefir_ast_new_do_while_statement_noarena(_mem, _cond, _body) (kefir_ast_new_do_while_statement((_mem), NULL, (_cond), (_body)))
#define kefir_ast_new_for_statement_noarena(_mem, _init, _cond, _after, _body) (kefir_ast_new_for_statement((_mem), NULL, (_init), (_cond), (_after), (_body)))
#define kefir_ast_new_goto_statement_noarena(_mem, _strings, _label) (kefir_ast_new_goto_statement((_mem), NULL, (_strings), (_label)))
#define kefir_ast_new_goto_address_statement_noarena(_mem, _addr) (kefir_ast_new_goto_address_statement((_mem), NULL, (_addr)))
#define kefir_ast_new_continue_statement_noarena(_mem) (kefir_ast_new_continue_statement((_mem), NULL))
#define kefir_ast_new_break_statement_noarena(_mem) (kefir_ast_new_break_statement((_mem), NULL))
#define kefir_ast_new_return_statement_noarena(_mem, _node) (kefir_ast_new_return_statement((_mem), NULL, (_node)))
#define kefir_ast_new_function_definition_noarena(_mem, _decl, _body) (kefir_ast_new_function_definition((_mem), NULL, (_decl), (_body)))
#define kefir_ast_new_translation_unit_noarena(_mem) (kefir_ast_new_translation_unit((_mem), NULL))
#define kefir_ast_new_builtin_noarena(_mem, _op) (kefir_ast_new_builtin((_mem), NULL, (_op)))
#define kefir_ast_new_extension_node_noarena(_mem, _klass, _payload) (kefir_ast_new_extension_node((_mem), NULL, (_klass), (_payload)))
#define kefir_ast_new_label_address_noarena(_mem, _strings, _label) (kefir_ast_new_label_address((_mem), NULL, (_strings), (_label)))
#define kefir_ast_new_statement_expression_noarena(_mem) (kefir_ast_new_statement_expression((_mem), NULL))
#define kefir_ast_new_inline_assembly_noarena(_mem, _qual, _templ) (kefir_ast_new_inline_assembly((_mem), NULL, (_qual), (_templ)))

#define kefir_ast_declarator_identifier_noarena(_mem, _strings, _id) (kefir_ast_declarator_identifier((_mem), NULL, (_strings), (_id)))
#define kefir_ast_declarator_pointer_noarena(_mem, _decl) (kefir_ast_declarator_pointer((_mem), NULL, (_decl)))
#define kefir_ast_declarator_array_noarena(_mem, _type, _len, _decl) (kefir_ast_declarator_array((_mem), NULL, (_type), (_len), (_decl)))
#define kefir_ast_declarator_function_noarena(_mem, _decl) (kefir_ast_declarator_function((_mem), NULL, (_decl)))

#define kefir_ast_enum_specifier_init_noarena(_mem, _strings, _id, _complete, _type_spec) (kefir_ast_enum_specifier_init((_mem), NULL, (_strings), (_id), (_complete), (_type_spec)))
#define kefir_ast_structure_specifier_init_noarena(_mem, _strings, _id, _complete) (kefir_ast_structure_specifier_init((_mem), NULL, (_strings), (_id), (_complete)))
#define kefir_ast_structure_declaration_entry_alloc_noarena(_mem) (kefir_ast_structure_declaration_entry_alloc((_mem), NULL))

#define kefir_ast_new_initializer_member_designation_noarena(_mem, _strings, _label, _designation) (kefir_ast_new_initializer_member_designation((_mem), NULL, (_strings), (_label), (_designation)))
#define kefir_ast_new_initializer_index_designation_noarena(_mem, _index, _designation) (kefir_ast_new_initializer_index_designation((_mem), NULL, (_index), (_designation)))

#define kefir_ast_new_expression_initializer_noarena(_mem, _expr) (kefir_ast_new_expression_initializer((_mem), NULL, (_expr)))
#define kefir_ast_new_list_initializer_noarena(_mem) (kefir_ast_new_list_initializer((_mem), NULL))

#endif
