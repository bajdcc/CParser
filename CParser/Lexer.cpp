#include "stdafx.h"
#include "Lexer.h"


CLexer::CLexer(std::string str): str(str)
{
    length = str.length();
    assert(length > 0);
}

LEX_T(char) CLexer::get_char()
{
}

LEX_T(uchar) CLexer::get_uchar()
{
}

LEX_T(short) CLexer::get_short()
{
}

LEX_T(ushort) CLexer::get_ushort()
{
}

LEX_T(int) CLexer::get_int()
{
}

LEX_T(uint) CLexer::get_uint()
{
}

LEX_T(long) CLexer::get_long()
{
}

LEX_T(ulong) CLexer::get_ulong()
{
}

LEX_T(float) CLexer::get_float()
{
}

LEX_T(double) CLexer::get_double()
{
}

LEX_T(operator) CLexer::get_operator()
{
}

LEX_T(keyword) CLexer::get_keyword()
{
}

LEX_T(string) CLexer::get_string()
{
}

LEX_T(comment) CLexer::get_comment()
{
}

CLexer::~CLexer()
{
}
