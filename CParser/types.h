#ifndef __TYPES_H
#define __TYPES_H

#include <string>

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
DEFINE_BASE_TYPE(l_identifier, std::string)
DEFINE_BASE_TYPE(l_string, std::string)
DEFINE_BASE_TYPE(l_comment, std::string)

#undef DEFINE_BASE_TYPE

#define LEX_T(t) base_t<l_##t>::type

#endif