#ifndef __PARSER_H
#define __PARSER_H

#include "types.h"
#include "Lexer.h"
extern "C" {
    #include "mpc.h"
}

class CParser
{
public:
    explicit CParser(string_t str);
    ~CParser();

private:
    void init();

    long eval_exp(mpc_ast_t* t);
    long eval_op(long x, char* op, long y);

private:
    CLexer lexer;
    std::string lexstr;
};

#endif