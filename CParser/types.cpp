#include "stdafx.h"
#include "types.h"

string_t lexer_string_list[] =
{
    "none",
    "error",
    "char",
    "uchar",
    "short",
    "ushort",
    "int",
    "uint",
    "long",
    "ulong",
    "float",
    "double",
    "operator",
    "keyword",
    "identifier",
    "string",
    "comment",
    "space",
    "newline",
    "END"
};

const string_t& lexer_typestr(lexer_t type)
{
    assert(type >= l_none && type < l_end);
    return lexer_string_list[type];
}

string_t keyword_string_list[] =
{
    "@START",
    "auto",
    "bool",
    "break",
    "case",
    "char",
    "const",
    "continue",
    "default",
    "do",
    "double",
    "else",
    "enum",
    "extern",
    "false",
    "float",
    "for",
    "goto",
    "if",
    "int",
    "long",
    "register",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "struct",
    "switch",
    "true",
    "typedef",
    "union",
    "unsigned",
    "void",
    "volatile",
    "while",
    "@END",
};

const string_t& lexer_keywordstr(keyword_t type)
{
    assert(type > k__start && type < k__end);
    return keyword_string_list[type - k__start - 1];
}

string_t lexer_keyword_regex()
{
    string_t str;
    for (auto i = k__start + 1; i < k__end; i++)
    {
        str += "(";
        str += keyword_string_list[i];
        str += ")|";
    }
    str.erase(str.length() - 1);
    return str;
}

string_t operator_string_list[] =
{
    "@START",
    "=",
    "+",
    "-",
    "*",
    "/",
    "\\",
    "?",
    "%",
    "&",
    "|",
    "~",
    "^",
    "!",
    "<",
    ">",
    "",
    ")",
    "{",
    "}",
    "[",
    "]",
    "",
    ".",
    ";",
    ":",
    "==",
    "!=",
    "++",
    "--",
    "+=",
    "-=",
    "*=",
    "/=",
    "&=",
    "|=",
    "^=",
    "%=",
    "<=",
    ">=",
    "&&",
    "||",
    "->",
    "<<",
    ">>",
    "<<=",
    ">>=",
    "...",
    "@END",
};

string_t opname_string_list[] =
{
    "@START",
    "assign",
    "plus",
    "minus",
    "times",
    "divide",
    "escape",
    "query",
    "mod",
    "bit_and",
    "bit_or",
    "bit_not",
    "bit_xor",
    "logical_not",
    "less_than",
    "greater_than",
    "lparan",
    "rparan",
    "lbrace",
    "rbrace",
    "lsquare",
    "rsquare",
    "comma",
    "dot",
    "semi",
    "colon",
    "equal",
    "not_equal",
    "plus_plus",
    "minus_minus",
    "plus_assign",
    "minus_assign",
    "times_assign",
    "div_assign",
    "and_assign",
    "or_assign",
    "xor_assign",
    "mod_assign",
    "less_than_or_equal",
    "greater_than_or_equal",
    "logical_and",
    "logical_or",
    "pointer",
    "left_shift",
    "right_shift",
    "left_shift_assign",
    "right_shift_assign",
    "ellipsis",
    "@END",
};

string_t operator_esc_string_list[] =
{
    "@START",
    "=",
    "\\+",
    "-",
    "\\*",
    "/",
    "\\\\",
    "\\?",
    "%",
    "&",
    "\\|",
    "~",
    "\\^",
    "!",
    "<",
    ">",
    "\\(",
    "\\)",
    "\\{",
    "\\}",
    "\\[",
    "\\]",
    ",",
    "\\.",
    ";",
    ":",
    "==",
    "!=",
    "\\+\\+",
    "--",
    "\\+=",
    "-=",
    "\\*=",
    "/=",
    "&=",
    "\\|=",
    "\\^=",
    "%=",
    "\\<=",
    "\\>=",
    "&&",
    "\\|\\|",
    "->",
    "<<",
    ">>",
    "<<=",
    ">>=",
    "\\.\\.\\.",
    "@END",
};

const string_t& lexer_opstr(operator_t type)
{
    assert(type > op__start && type < op__end);
    return operator_string_list[type];
}

const string_t& lexer_opnamestr(operator_t type)
{
    assert(type > op__start && type < op__end);
    return opname_string_list[type];
}

int op_len_start_idx[] =
{
    op__start,
    op_assign,
    op_equal,
    op_left_shift_assign,
    op__end
};

int lexer_operator_start_idx(int len)
{
    assert(len >= 0 && len <= 4);
    return op_len_start_idx[len];
}

string_t lexer_operator_regex(int len)
{
    assert(len >= 1 && len <= 3);
    string_t str;
    for (auto i = op_len_start_idx[len]; i < op_len_start_idx[len + 1]; i++)
    {
        str += "(";
        str += operator_esc_string_list[i];
        str += ")|";
    }
    str.erase(str.length() - 1);
    return str;
}
