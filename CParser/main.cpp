// CParser.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Lexer.h"

#define TEST(l, t, v) assert(l.next() == l_##t); assert(lexer.get_##t() == v);

int main()
{
    auto str = "ABC \n\n0.2e8  a_\n";
    CLexer lexer(str);

    TEST(lexer, identifier, "ABC");
    TEST(lexer, space, 1);
    TEST(lexer, newline, 2);
    TEST(lexer, double, 0.2e8);
    TEST(lexer, space, 2);
    TEST(lexer, identifier, "a_");
    TEST(lexer, newline, 1);

    return 0;
}

