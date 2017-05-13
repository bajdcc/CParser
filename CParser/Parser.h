#ifndef __PARSER_H
#define __PARSER_H

#include "types.h"
#include "Lexer.h"
#include "Gen.h"

class CParser
{
public:
    explicit CParser(string_t str);
    ~CParser();

private:
    void init();
    void next();

    void program();
    void expression(operator_t level);
    void statement();
    void enum_declaration();
    void function_parameter();
    void function_body();
    void function_declaration();
    void global_declaration();

private:
    void match_keyword(keyword_t);
    void match_operator(operator_t);
    void match_type(lexer_t);
    void match_number();
    void match_integer();

    lexer_t parse_type();
    void save_identifier();

    void error(string_t);

private:
    lexer_t base_type{ l_none };
    lexer_t expr_type{ l_none };
    int ptr_level{ 0 };
    int ebp{ 0 };
    std::shared_ptr<sym_t> id;

private:
    CLexer lexer;
    CGen gen;
};

#endif