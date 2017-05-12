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
        str += "(^";
        str += keyword_string_list[i];
        str += "$)|";
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
    "(",
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
        str += "(^";
        str += operator_esc_string_list[i];
        str += "$)|";
    }
    str.erase(str.length() - 1);
    return str;
}

string_t err_string_list[] =
{
    "@START",
    "invalid character",
    "invalid operator",
    "invalid comment",
    "invalid digit",
    "invalid string",
    "@END",
};

const string_t& lexer_errstr(error_t type)
{
    assert(type > e__start && type < e__end);
    return err_string_list[type];
}

int op_pred[] =
{
    9999, // op__start,
    1401, // op_assign,
     401, // op_plus,
     402, // op_minus,
     302, // op_times,
     301, // op_divide,
    9000, // op_escape,
    1301, // op_query,
     303, // op_mod,
     801, // op_bit_and,
    1001, // op_bit_or,
     208, // op_bit_not,
     901, // op_bit_xor,
     207, // op_logical_not,
     603, // op_less_than,
     601, // op_greater_than,
     102, // op_lparan,
     102, // op_rparan,
    9000, // op_lbrace,
    9000, // op_rbrace,
     101, // op_lsquare,
     101, // op_rsquare,
    1501, // op_comma,
     103, // op_dot,
    9000, // op_semi,
    1302, // op_colon,
     701, // op_equal,
     702, // op_not_equal,
     203, // op_plus_plus,
     204, // op_minus_minus,
    1405, // op_plus_assign,
    1406, // op_minus_assign,
    1403, // op_times_assign,
    1402, // op_div_assign,
    1409, // op_and_assign,
    1411, // op_or_assign,
    1410, // op_xor_assign,
    1404, // op_mod_assign,
     604, // op_less_than_or_equal,
     602, // op_greater_than_or_equal,
    1101, // op_logical_and,
    1201, // op_logical_or,
     104, // op_pointer,
     501, // op_left_shift,
     502, // op_right_shift,
    1407, // op_left_shift_assign,
    1408, // op_right_shift_assign,
    9000, // op_ellipsis,
    9999, // op__end
};

int lexer_operatorpred(operator_t type)
{
    assert(type > op__start && type < op__end);
    return op_pred[type];
}
