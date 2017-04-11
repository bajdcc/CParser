#ifndef __TYPES_H
#define __TYPES_H

#include <string>

using string_t = std::string;
using smatch_t = std::smatch;
using regex_t = std::regex;

enum lexer_t
{
    l_none,
    l_error,
    l_char,
    l_uchar,
    l_short,
    l_ushort,
    l_int,
    l_uint,
    l_long,
    l_ulong,
    l_float,
    l_double,
    l_operator,
    l_keyword,
    l_identifier,
    l_string,
    l_comment,
    l_space,
    l_newline,
    l_end,
};

enum keyword_t
{
    k__start,
    k_auto,
    k_bool,
    k_break,
    k_case,
    k_char,
    k_const,
    k_continue,
    k_default,
    k_do,
    k_double,
    k_else,
    k_enum,
    k_extern,
    k_false,
    k_float,
    k_for,
    k_goto,
    k_if,
    k_int,
    k_long,
    k_register,
    k_return,
    k_short,
    k_signed,
    k_sizeof,
    k_static,
    k_struct,
    k_switch,
    k_true,
    k_typedef,
    k_union,
    k_unsigned,
    k_void,
    k_volatile,
    k_while,
    k__end
};

enum operator_t
{
    op__start,
    op_assign,
    op_plus,
    op_minus,
    op_times,
    op_divide,
    op_escape,
    op_query,
    op_mod,
    op_bit_and,
    op_bit_or,
    op_bit_not,
    op_bit_xor,
    op_logical_not,
    op_less_than,
    op_greater_than,
    op_lparan,
    op_rparan,
    op_lbrace,
    op_rbrace,
    op_lsquare,
    op_rsquare,
    op_comma,
    op_dot,
    op_semi,
    op_colon,
    op_equal,
    op_not_equal,
    op_plus_plus,
    op_minus_minus,
    op_plus_assign,
    op_minus_assign,
    op_times_assign,
    op_div_assign,
    op_and_assign,
    op_or_assign,
    op_xor_assign,
    op_mod_assign,
    op_less_than_or_equal,
    op_greater_than_or_equal,
    op_logical_and,
    op_logical_or,
    op_pointer,
    op_left_shift,
    op_right_shift,
    op_left_shift_assign,
    op_right_shift_assign,
    op_ellipsis,
    op__end
};

template<lexer_t>
struct base_t
{
    using type = void*;
};

#define DEFINE_BASE_TYPE(t, obj) \
template<> \
struct base_t<t> \
{ \
    using type = obj; \
};

DEFINE_BASE_TYPE(l_char, char)
DEFINE_BASE_TYPE(l_uchar, unsigned char)
DEFINE_BASE_TYPE(l_short, short)
DEFINE_BASE_TYPE(l_ushort, unsigned short)
DEFINE_BASE_TYPE(l_int, int)
DEFINE_BASE_TYPE(l_uint, unsigned int)
DEFINE_BASE_TYPE(l_long, long)
DEFINE_BASE_TYPE(l_ulong, unsigned long)
DEFINE_BASE_TYPE(l_float, float)
DEFINE_BASE_TYPE(l_double, double)
DEFINE_BASE_TYPE(l_operator, operator_t)
DEFINE_BASE_TYPE(l_keyword, keyword_t)
DEFINE_BASE_TYPE(l_identifier, string_t)
DEFINE_BASE_TYPE(l_string, string_t)
DEFINE_BASE_TYPE(l_comment, string_t)
DEFINE_BASE_TYPE(l_space, int)
DEFINE_BASE_TYPE(l_newline, int)

#undef DEFINE_BASE_TYPE

const string_t& lexer_typestr(lexer_t);
const string_t& lexer_keywordstr(keyword_t);
const string_t& lexer_opstr(operator_t);
const string_t& lexer_opnamestr(operator_t);

string_t lexer_keyword_regex();
string_t lexer_operator_regex(int);

int lexer_operator_start_idx(int);

#define LEX_T(t) base_t<l_##t>::type
#define LEX_STRING(t) lexer_typestr(t)

#define KEYWORD_STRING(t) lexer_keywordstr(t)
#define OPERATOR_STRING(t) lexer_opnamestr(t)

#endif