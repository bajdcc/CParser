#ifndef __LEXER_H
#define __LEXER_H

#include "types.h"

// 词法分析
class CLexer
{
public:
    explicit CLexer(string_t str);
    ~CLexer();

    // 外部接口
#define DEFINE_LEXER_GETTER(t) LEX_T(t) get_##t();
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

public:
    lexer_t next();

    lexer_t get_type() const;
    int get_line() const;
    int get_column() const;
    int get_last_line() const;
    int get_last_column() const;
    string_t current() const;

private:

    void move(int idx, int inc = -1, bool newline = false);

    // 内部解析
    lexer_t next_digit();
    lexer_t next_alpha();
    lexer_t next_space();
    lexer_t next_char();
    lexer_t next_string();
    lexer_t next_comment();
    lexer_t next_operator();

    int local();
    int local(int offset);

private:
    string_t str;
    int index{ 0 };
    int last_index{ 0 };
    int length{ 0 };

    lexer_t type { l_none };
    int line{ 1 };
    int column{ 1 };
    int last_line{ 1 };
    int last_column{ 1 };

    struct
    {
#define DEFINE_LEXER_GETTER(t) LEX_T(t) _##t;
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
    } bags;

    // 正则表达式
    smatch_t sm;
    regex_t r_string{ R"(([^\\])|(?:\\(?:([bfnrtv'"\\])|(?:0(\d{1,2}))|(\d)|(?:x([[:xdigit:]]{1,2})))))", std::regex::ECMAScript | std::regex::optimize };
    regex_t r_char{ R"('(?:([^'"\\])|(?:\\(?:([bfnrtv'"\\])|(?:0(\d{1,2}))|(\d)|(?:x([[:xdigit:]]{1,2})))))')", std::regex::ECMAScript | std::regex::optimize };
    regex_t r_digit{ R"(((?:\d+(\.)?\d*)(?:[eE][+-]?\d+)?)([uU])?([fFdDiIlL])?)", std::regex::ECMAScript | std::regex::optimize };
    regex_t r_alpha{ R"([[:alpha:]_]\w*)", std::regex::ECMAScript | std::regex::optimize };
    regex_t r_space{ R"(([ ]+)|((?:\r\n)+)|(\n+))", std::regex::ECMAScript | std::regex::optimize };
    regex_t r_comment{ R"((?://([^\r\n]*))|(?:/\*([[:print:]\n]*?)\*/))", std::regex::ECMAScript | std::regex::optimize };
    regex_t r_hex{ R"(^0x([[:xdigit:]]{1,8}))", std::regex::ECMAScript | std::regex::optimize };
    regex_t r_keyword{ lexer_keyword_regex(), std::regex::ECMAScript | std::regex::optimize };
    regex_t r_operator[3] =
    {
        regex_t{ lexer_operator_regex(1), std::regex::ECMAScript | std::regex::optimize },
        regex_t{ lexer_operator_regex(2), std::regex::ECMAScript | std::regex::optimize },
        regex_t{ lexer_operator_regex(3), std::regex::ECMAScript | std::regex::optimize }
    };
};

#endif
