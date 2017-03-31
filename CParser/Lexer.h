#ifndef __LEXER_H
#define __LEXER_H

#include "types.h"

// 词法分析
class CLexer
{
public:
    explicit CLexer(std::string str);
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
        DEFINE_LEXER_GETTER(string)
        DEFINE_LEXER_GETTER(comment)
#undef DEFINE_LEXER_GETTER

private:
    // 内部解析

private:
    std::string str;
    int index{ 0 };
    int length{ 0 };
};

#endif