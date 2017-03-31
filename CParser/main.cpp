// CParser.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Lexer.h"


int main()
{
    auto str = "ABC 0.2e8 a_";
    CLexer lexer(str);

    assert(lexer.next() == l_identifier); assert(lexer.get_identifier() == "ABC");
    assert(lexer.next() == l_none);
    assert(lexer.next() == l_double); assert(lexer.get_double() == 0.2e8);
    assert(lexer.next() == l_none);
    assert(lexer.next() == l_identifier); assert(lexer.get_identifier() == "a_");

    return 0;
}

