#include "stdafx.h"
#include "Parser.h"


CParser::CParser(string_t str)
: lexer(str)
{
    lexstr = lexer.store_start();
    init();
}

CParser::~CParser()
{
}

long CParser::eval_op(long x, char* op, long y) {
    if (strcmp(op, "+") == 0) { return x + y; }
    if (strcmp(op, "-") == 0) { return x - y; }
    if (strcmp(op, "*") == 0) { return x * y; }
    if (strcmp(op, "/") == 0) { return x / y; }
    return 0;
}

long CParser::eval_exp(mpc_ast_t* t) {

    /* If tagged as number return it directly. */
    if (strstr(t->tag, "number")) {
        return lexer.get_store_int(atoi(t->contents+1));
    }

    /* The operator is always second child. */
    auto op = t->children[1]->contents;

    /* We store the third child in `x` */
    auto x = eval_exp(t->children[2]);

    /* Iterate the remaining children and combining. */
    auto i = 3;
    while (strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval_exp(t->children[i]));
        i++;
    }

    return x;
}
void CParser::init()
{
    auto Number = mpc_new("number");
    auto Operator = mpc_new("operator");
    auto Expr = mpc_new("expr");
    auto Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT, "\
      number   : /i\\d+/ ; \
      operator : '+' | '-' | '*' | '/' ; \
      expr     : <number> | '(' <operator> <expr>+ ')' ; \
      lispy    : /^/ <operator> <expr>+ /$/ ; \
    ", Number, Operator, Expr, Lispy);

    mpc_result_t r;
    if (mpc_parse("<stdin>", lexstr.c_str(), Lispy, &r)) {
        auto result = eval_exp(r.output);
        printf("%li\n", result);
        mpc_ast_delete(r.output);
    }
    else {
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
    }

    mpc_cleanup(4, Number, Operator, Expr, Lispy);
}
