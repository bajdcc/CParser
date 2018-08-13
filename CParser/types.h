//
// Project: CMiniLang
// Author: bajdcc
//
#ifndef CMINILANG_TYPES_H
#define CMINILANG_TYPES_H

#include <string>
#include <unordered_map>

using string_t = std::string;
template<class K, class V> using map_t = std::unordered_map<K, V>;

namespace clib {
    using int8 = signed __int8;
    using uint8 = unsigned __int8;
    using int16 = signed __int16;
    using uint16 = unsigned __int16;
    using int32 = signed __int32;
    using uint32 = unsigned __int32;
    using int64 = signed __int64;
    using uint64 = unsigned __int64;

#ifdef WIN32
    using sint = int32;
    using uint = uint32;
    using slong = long long;
    using ulong = unsigned long long;
#else
    using sint = int64;
    using uint = uint64;
    using slong = long;
    using ulong = unsigned long;
#endif
    using byte = uint8;
    using size_t = uint;

    enum lexer_t {
        l_none,
        l_ptr,
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

    enum keyword_t {
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

    enum operator_t {
        op__start,
        op_assign,
        op_equal,
        op_plus,
        op_plus_assign,
        op_minus,
        op_minus_assign,
        op_times,
        op_times_assign,
        op_divide,
        op_div_assign,
        op_bit_and,
        op_and_assign,
        op_bit_or,
        op_or_assign,
        op_bit_xor,
        op_xor_assign,
        op_mod,
        op_mod_assign,
        op_less_than,
        op_less_than_or_equal,
        op_greater_than,
        op_greater_than_or_equal,
        op_logical_not,
        op_not_equal,
        op_escape,
        op_query,
        op_bit_not,
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
        op_plus_plus,
        op_minus_minus,
        op_logical_and,
        op_logical_or,
        op_pointer,
        op_left_shift,
        op_right_shift,
        op_left_shift_assign,
        op_right_shift_assign,
        op_ellipsis,
        op__end,
    };

    enum error_t {
        e__start,
        e_invalid_char,
        e_invalid_operator,
        e_invalid_comment,
        e_invalid_digit,
        e_invalid_string,
        e__end
    };

    template<lexer_t>
    struct base_t {
        using type = void *;
    };
    template<class T>
    struct base_lexer_t {
        static const lexer_t type = l_none;
    };

#define DEFINE_BASE_TYPE(t, obj) \
template<> \
struct base_t<t> \
{ \
    using type = obj; \
    static const int size = sizeof(obj); \
};

    DEFINE_BASE_TYPE(l_ptr, void*)
    DEFINE_BASE_TYPE(l_char, char)
    DEFINE_BASE_TYPE(l_uchar, unsigned char)
    DEFINE_BASE_TYPE(l_short, short)
    DEFINE_BASE_TYPE(l_ushort, unsigned short)
    DEFINE_BASE_TYPE(l_int, int)
    DEFINE_BASE_TYPE(l_uint, unsigned int)
    DEFINE_BASE_TYPE(l_long, slong)
    DEFINE_BASE_TYPE(l_ulong, ulong)
    DEFINE_BASE_TYPE(l_float, float)
    DEFINE_BASE_TYPE(l_double, double)
    DEFINE_BASE_TYPE(l_operator, operator_t)
    DEFINE_BASE_TYPE(l_keyword, keyword_t)
    DEFINE_BASE_TYPE(l_identifier, string_t)
    DEFINE_BASE_TYPE(l_string, string_t)
    DEFINE_BASE_TYPE(l_comment, string_t)
    DEFINE_BASE_TYPE(l_space, uint)
    DEFINE_BASE_TYPE(l_newline, uint)
    DEFINE_BASE_TYPE(l_error, error_t)
#undef DEFINE_BASE_TYPE

#define DEFINE_CONV_TYPE(t, obj) \
template<> \
struct base_lexer_t<obj> \
{ \
    static const lexer_t type = t; \
};

    DEFINE_CONV_TYPE(l_ptr, void*)
    DEFINE_CONV_TYPE(l_char, char)
    DEFINE_CONV_TYPE(l_uchar, unsigned char)
    DEFINE_CONV_TYPE(l_short, short)
    DEFINE_CONV_TYPE(l_ushort, unsigned short)
    DEFINE_CONV_TYPE(l_int, int)
    DEFINE_CONV_TYPE(l_uint, unsigned int)
    DEFINE_CONV_TYPE(l_long, slong)
    DEFINE_CONV_TYPE(l_ulong, ulong)
    DEFINE_CONV_TYPE(l_float, float)
    DEFINE_CONV_TYPE(l_double, double)
    DEFINE_CONV_TYPE(l_string, string_t)
    DEFINE_CONV_TYPE(l_error, error_t)
#undef DEFINE_CONV_TYPE

    const string_t &lexer_typestr(lexer_t);
    const string_t &lexer_keywordstr(keyword_t);
    const string_t &lexer_opstr(operator_t);
    const string_t &lexer_opnamestr(operator_t);
    const string_t &lexer_errstr(error_t);
    int lexer_operatorpred(operator_t);

    extern string_t keyword_string_list[];
    string_t lexer_operator_regex(int);

    int lexer_operator_start_idx(int);

#define LEX_T(t) base_t<l_##t>::type
#define LEX_CONV_T(t) base_lexer_t<t>::type
#define LEX_SIZEOF(t) base_t<l_##t>::size
#define LEX_STRING(t) lexer_typestr(t)

#define KEYWORD_STRING(t) lexer_keywordstr(t)
#define OPERATOR_STRING(t) lexer_opnamestr(t)
#define OP_STRING(t) lexer_opstr(t)
#define ERROR_STRING(t) lexer_errstr(t)

#define OPERATOR_PRED(t) lexer_operatorpred(t)

//----------------------------------------------------

// instructions

    enum ins_t {
        LEA, IMM, JMP, CALL, JZ, JNZ, ENT, ADJ, LEV, LI, SI, LC, SC, PUSH, LOAD,
        OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD,
        OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, TRAC, TRAN, EXIT
    };

    enum class_t {
        CLASS_NULL, Num, Fun, Sys, Glo, Loc, Id
    };

    union storage_t {
#define DEFINE_LEXER_STORAGE(t) LEX_T(t) _##t;
        DEFINE_LEXER_STORAGE(char)
        DEFINE_LEXER_STORAGE(uchar)
        DEFINE_LEXER_STORAGE(short)
        DEFINE_LEXER_STORAGE(ushort)
        DEFINE_LEXER_STORAGE(int)
        DEFINE_LEXER_STORAGE(uint)
        DEFINE_LEXER_STORAGE(long)
        DEFINE_LEXER_STORAGE(ulong)
        DEFINE_LEXER_STORAGE(float)
        DEFINE_LEXER_STORAGE(double)
#undef DEFINE_LEXER_STORAGE
    };

    struct sym_t {
        string_t name;
        class_t cls, _cls;
        lexer_t type, _type;
        storage_t value, _value;
        LEX_T(int) ptr, _ptr;
    };
}

#endif //CMINILANG_TYPES_H
