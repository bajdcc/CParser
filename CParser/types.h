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

enum operator_t
{
    
};

enum keyword_t
{

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

#define LEX_T(t) base_t<l_##t>::type
#define LEX_STRING(t) lexer_typestr(t)

#endif