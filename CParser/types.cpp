//
// Project: CMiniLang
// Author: bajdcc
//

#include "stdafx.h"
#include "types.h"

namespace clib {
    string_t lexer_string_list[] = {
        "none",
        "ptr",
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

    const string_t &lexer_typestr(lexer_t type) {
        assert(type >= l_none && type < l_end);
        return lexer_string_list[type];
    }

    string_t keyword_string_list[] = {
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

    const string_t &lexer_keywordstr(keyword_t type) {
        assert(type > k__start && type < k__end);
        return keyword_string_list[type - k__start];
    }

    std::tuple<operator_t, string_t, string_t, int> operator_string_list[] = {
        std::make_tuple(op__start, "@START", "@START", 9999),
        std::make_tuple(op_assign, "=", "assign", 1401),
        std::make_tuple(op_equal, "==", "equal", 701),
        std::make_tuple(op_plus, "+", "plus", 401),
        std::make_tuple(op_plus_assign, "+=", "plus_assign", 1405),
        std::make_tuple(op_minus, "-", "minus", 402),
        std::make_tuple(op_minus_assign, "-=", "minus_assign", 1406),
        std::make_tuple(op_times, "*", "times", 302),
        std::make_tuple(op_times_assign, "*=", "times_assign", 1403),
        std::make_tuple(op_divide, "/", "divide", 301),
        std::make_tuple(op_div_assign, "/=", "div_assign", 1402),
        std::make_tuple(op_bit_and, "&", "bit_and", 801),
        std::make_tuple(op_and_assign, "&=", "and_assign", 1409),
        std::make_tuple(op_bit_or, "|", "bit_or", 1001),
        std::make_tuple(op_or_assign, "|=", "or_assign", 1411),
        std::make_tuple(op_bit_xor, "^", "bit_xor", 901),
        std::make_tuple(op_xor_assign, "^=", "xor_assign", 1410),
        std::make_tuple(op_mod, "%", "mod", 303),
        std::make_tuple(op_mod_assign, "%=", "mod_assign", 1404),
        std::make_tuple(op_less_than, "<", "less_than", 603),
        std::make_tuple(op_less_than_or_equal, "<=", "less_than_or_equal", 604),
        std::make_tuple(op_greater_than, ">", "greater_than", 601),
        std::make_tuple(op_greater_than_or_equal, ">=", "greater_than_or_equal", 602),
        std::make_tuple(op_logical_not, "!", "logical_not", 207),
        std::make_tuple(op_not_equal, "!=", "not_equal", 702),
        std::make_tuple(op_escape, "\\", "escape", 9000),
        std::make_tuple(op_query, "?", "query", 1301),
        std::make_tuple(op_bit_not, "~", "bit_not", 208),
        std::make_tuple(op_lparan, "(", "lparan", 102),
        std::make_tuple(op_rparan, ")", "rparan", 102),
        std::make_tuple(op_lbrace, "{", "lbrace", 9000),
        std::make_tuple(op_rbrace, "}", "rbrace", 9000),
        std::make_tuple(op_lsquare, "[", "lsquare", 101),
        std::make_tuple(op_rsquare, "]", "rsquare", 101),
        std::make_tuple(op_comma, ",", "comma", 1501),
        std::make_tuple(op_dot, ".", "dot", 103),
        std::make_tuple(op_semi, ";", "semi", 9000),
        std::make_tuple(op_colon, ":", "colon", 1302),
        std::make_tuple(op_plus_plus, "++", "plus_plus", 203),
        std::make_tuple(op_minus_minus, "--", "minus_minus", 204),
        std::make_tuple(op_logical_and, "&&", "logical_and", 1101),
        std::make_tuple(op_logical_or, "||", "logical_or", 1201),
        std::make_tuple(op_pointer, "->", "pointer", 104),
        std::make_tuple(op_left_shift, "<<", "left_shift", 501),
        std::make_tuple(op_right_shift, ">>", "right_shift", 502),
        std::make_tuple(op_left_shift_assign, "<<=", "left_shift_assign", 1407),
        std::make_tuple(op_right_shift_assign, ">>=", "right_shift_assign", 1408),
        std::make_tuple(op_ellipsis, "...", "ellipsis", 9000),
        std::make_tuple(op__end, "@END", "@END", 9999),
    };

    const string_t &lexer_opstr(operator_t type) {
        assert(type > op__start && type < op__end);
        return std::get<1>(operator_string_list[type]);
    }

    const string_t &lexer_opnamestr(operator_t type) {
        assert(type > op__start && type < op__end);
        return std::get<2>(operator_string_list[type]);
    }

    string_t err_string_list[] = {
        "@START",
        "#E !char!",
        "#E !operator!",
        "#E !comment!",
        "#E !digit!",
        "#E !string!",
        "@END",
    };

    const string_t &lexer_errstr(error_t type) {
        assert(type > e__start && type < e__end);
        return err_string_list[type];
    }

    int lexer_operatorpred(operator_t type) {
        assert(type > op__start && type < op__end);
        return std::get<3>(operator_string_list[type]);
    }
}
