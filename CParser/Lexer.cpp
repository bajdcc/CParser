#include "stdafx.h"
#include "Lexer.h"


CLexer::CLexer(string_t str): str(str)
{
    length = str.length();
    assert(length > 0);
}

CLexer::~CLexer()
{
}

#define DEFINE_LEXER_GETTER(t) \
LEX_T(t) CLexer::get_##t() \
{ \
    return bags._##t; \
}
DEFINE_LEXER_GETTER(char)
DEFINE_LEXER_GETTER(uchar)
DEFINE_LEXER_GETTER(short)
DEFINE_LEXER_GETTER(ushort)
DEFINE_LEXER_GETTER(int)
DEFINE_LEXER_GETTER(uint)
DEFINE_LEXER_GETTER(long)
DEFINE_LEXER_GETTER(ulong)
DEFINE_LEXER_GETTER(float)
DEFINE_LEXER_GETTER(double)
DEFINE_LEXER_GETTER(operator)
DEFINE_LEXER_GETTER(keyword)
DEFINE_LEXER_GETTER(identifier)
DEFINE_LEXER_GETTER(string)
DEFINE_LEXER_GETTER(comment)
DEFINE_LEXER_GETTER(space)
DEFINE_LEXER_GETTER(newline)

#undef DEFINE_LEXER_GETTER

lexer_t CLexer::next()
{
    auto c = local();
    type = l_error;
    if (isalpha(c))
    {
        type = next_alpha();
    }
    else if (isdigit(c))
    {
        type = next_digit();
    }
    else if (isspace(c))
    {
        type = next_space();
    }
    return type;
}

lexer_t CLexer::get_type() const
{
    return type;
}

int CLexer::get_line() const
{
    return line;
}

int CLexer::get_column() const
{
    return column;
}

int CLexer::get_last_line() const
{
    return last_line;
}

int CLexer::get_last_column() const
{
    return last_column;
}

string_t CLexer::current() const
{
    switch (type)
    {
    case l_space:
    case l_newline:
        return "... [" + LEX_STRING(type) + "]";
    default:
        break;
    }
    return str.substr(last_index, index - last_index);
}

void CLexer::move(int idx, int inc, bool newline)
{
    last_index = index;
    last_line = line;
    last_column = column;
    if (newline)
    {
        column = 1;
        line += inc;
    }
    else
    {
        if (inc < 0)
            column += idx;
        else
            column += inc;
    }
    index += idx;
}

lexer_t CLexer::next_digit()
{
    if (std::regex_search(str.cbegin() + index, str.cend(), sm, r_digit))
    {
        auto s = sm[0].str();
        bags._double = std::atof(s.c_str());
        move(s.length());
        return l_double;
    }
    assert(!"digit not match");
    return l_error;
}

lexer_t CLexer::next_alpha()
{
    if (std::regex_search(str.cbegin() + index, str.cend(), sm, r_alpha))
    {
        auto s = sm[0].str();
        bags._identifier = s.c_str();
        move(s.length());
        return l_identifier;
    }
    assert(!"alpha not match");
    return l_error;
}

lexer_t CLexer::next_space()
{
    if (std::regex_search(str.cbegin() + index, str.cend(), sm, r_space))
    {
        auto m = std::find_if(sm.begin(), sm.end(), [](auto sm) {return 1; });
        if (m == sm.end()) assert(!"space not match");
        auto ms = m->str();
        auto ml = ms.length();
        if (ms[0] == ' ')
        {
            bags._space = ml;
            move(ml);
            return l_space;
        }
        else if (ms[0] == '\r')
        {
            bags._newline = ml / 2;
            move(ml, bags._newline, true);
            return l_newline;
        }
        else if (ms[0] == '\n')
        {
            bags._newline = ml;
            move(ml, bags._newline, true);
            return l_newline;
        }
        assert(!"space not match");
        return l_error;
    }
    assert(!"space not match");
    return l_error;
}

int CLexer::local()
{
    if (index < length)
        return str[index];
    return -1;
}

void CLexer::match(int ch)
{
    assert(local() == ch);
}
