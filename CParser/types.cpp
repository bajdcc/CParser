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
