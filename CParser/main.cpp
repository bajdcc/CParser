// CParser.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Lexer.h"

#define OUTPUT(l, t) printf("[%03d:%03d] %-12s - %s\n", \
    l.get_last_line(), \
    l.get_last_column(), \
    LEX_STRING(l.get_type()).c_str(), \
    l.current().c_str());

#define TEST(l, t, v) \
    assert(l.next() == l_##t); \
    assert(l.get_##t() == v); \
    OUTPUT(l, t);

int main()
{
    auto str = "ABC \n\n0.2e8  a_\n";

    printf("# 输入 \n----[[[\n%s\n----]]]\n", str);

    CLexer lexer(str);

    printf("\n# 解析 \n");

    TEST(lexer, identifier, "ABC");
    TEST(lexer, space, 1);
    TEST(lexer, newline, 2);
    TEST(lexer, double, 0.2e8);
    TEST(lexer, space, 2);
    TEST(lexer, identifier, "a_");
    TEST(lexer, newline, 1);

    return 0;
}

