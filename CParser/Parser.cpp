#include "stdafx.h"
#include "Parser.h"
#include "Lexer.h"

/**
 * 递归下降分析器代码参考自：
 *     https://github.com/lotabout/write-a-C-interpreter
 * 作者文章：
 *     http://lotabout.me/2015/write-a-C-interpreter-0
 */

CParser::CParser(string_t str)
: lexer(str)
{
    init();
}

CParser::~CParser()
{
}

void CParser::init()
{
    program();
    gen.eval();
}

void CParser::next()
{
    lexer_t token;
    do
    {
        token = lexer.next();
    } while (token == l_newline || token == l_space || token == l_comment);
    assert(token != l_error);
}

void CParser::program()
{
    next();
    while (!lexer.is_type(l_end))
    {
        global_declaration();
    }
}

void CParser::expression(operator_t level)
{
    // expressions have various format.
    // but majorly can be divided into two parts: unit and operator
    // for example `(char) *a[10] = (int *) func(b > 0 ? 10 : 20);
    // `a[10]` is an unit while `*` is an operator.
    // `func(...)` in total is an unit.
    // so we should first parse those unit and unary operators
    // and then the binary ones
    //
    // also the expression can be in the following types:
    //
    // 1. unit_unary ::= unit | unit unary_op | unary_op unit
    // 2. expr ::= unit_unary (bin_op unit_unary ...)

    // unit_unary()
    {
        if (lexer.is_type(l_end))
        {
            printf("%d: unexpected token EOF of expression\n", lexer.get_line());
            assert(0);
        }
        if (lexer.is_integer())
        {
            auto tmp = lexer.get_integer();
            match_number();

            // emit code
            gen.emit(IMM);
            gen.emit(tmp);
            expr_type = lexer.get_type();
        }
        else if (lexer.is_type(l_string))
        {
            // continous string "abc" "abc"

            // emit code
            gen.emit(IMM);
            gen.emit(gen.save_string(lexer.get_string()));
            expr_type = l_ptr;
        }
        else if (lexer.is_keyword(k_sizeof))
        {
            // sizeof is actually an unary operator
            // now only `sizeof(int)`, `sizeof(char)` and `sizeof(*...)` are
            // supported.
            match_keyword(k_sizeof);
            match_operator(op_lparan);
            expr_type = l_int;

            if (lexer.is_keyword(k_unsigned))
            {
                match_keyword(k_unsigned);
            }

            auto size = lexer.get_sizeof();

            while (lexer.is_operator(op_times))
            {
                match_operator(op_times);
                if (expr_type != l_ptr)
                {
                    size = LEX_SIZEOF(ptr);
                    expr_type = l_ptr;
                }
            }

            match_operator(op_rparan);

            // emit code
            gen.emit(IMM);
            gen.emit(size);

            expr_type = l_int;
        }
        else if (lexer.is_type(l_identifier))
        {
            // there are several type when occurs to Id
            // but this is unit, so it can only be
            // 1. function call
            // 2. Enum variable
            // 3. global/local variable
            match_type(l_identifier);

            auto func_id = id;

            if (lexer.is_operator(op_lparan))
            {
                // function call
                match_operator(op_lparan);

                // pass in arguments
                auto tmp = 0; // number of arguments
                while (!lexer.is_operator(op_rparan))
                {
                    expression(op_assign);
                    gen.emit(PUSH);
                    tmp++;

                    if (lexer.is_operator(op_comma))
                    {
                        match_operator(op_comma);
                    }
                }
                match_operator(op_rparan);;

                id = func_id;

                // emit code
                if (id->cls == Sys)
                {
                    // system functions
                    gen.emit(*id);
                }
                else if (id->cls == Fun)
                {
                    // function call
                    gen.emit(CALL);
                    gen.emit(*id);
                }
                else
                {
                    printf("%d: bad function call\n", lexer.get_line());
                    assert(0);;
                }

                // clean the stack for arguments
                if (tmp > 0)
                {
                    gen.emit(ADJ);
                    gen.emit(tmp);
                }
                expr_type = id->type;
            }
            else if (id->cls == Num)
            {

                // enum variable
                gen.emit(IMM);
                gen.emit(*id);
                expr_type = l_int;
            }
            else
            {
                // variable
                if (id->cls == Loc)
                {
                    gen.emit(LEA);
                    gen.emit(*id, ebp);
                }
                else if (id->cls == Glo)
                {
                    gen.emit(IMM);
                    gen.emit(*id);
                }
                else
                {
                    printf("%d: undefined variable\n", lexer.get_line());
                    assert(0);;
                }
                // emit code, default behaviour is to load the value of the
                // address which is stored in `ax`
                expr_type = id->type;
                gen.emitl(expr_type);
            }
        }
        else if (lexer.is_operator(op_lparan))
        {
            // cast or parenthesis
            match_operator(op_lparan);
            if (lexer.is_type(l_keyword))
            {
                auto tmp = parse_type();
                match_type(l_keyword);

                while (lexer.is_operator(op_times))
                {
                    match_operator(op_times);
                    ptr_level++;
                }
                match_operator(op_rparan);

                expression((operator_t)202); // cast has precedence as Inc(++)
                expr_type = tmp;
            }
            else
            {
                // normal parenthesis
                expression(op_assign);
                match_operator(op_rparan);
            }
        }
        else if (lexer.is_operator(op_times))
        {
            // dereference *<addr>
            match_operator(op_times);
            expression((operator_t)205);
            if (ptr_level > 0)
            {
                ptr_level--;
            }
            else
            {
                printf("%d: bad dereference\n", lexer.get_line());
                assert(0);;
            }

            gen.emitl(expr_type);
        }
        else if (lexer.is_operator(op_bit_and))
        {
            // get the address of
            match_operator(op_bit_and);
            expression((operator_t)206); // get the address of
            if (gen.top() == LC || gen.top() == LI)
            {
                gen.pop();
            }
            else
            {
                printf("%d: bad address of\n", lexer.get_line());
                assert(0);;
            }

            ptr_level++;
        }
        else if (lexer.is_operator(op_logical_not))
        {
            // not
            match_operator(op_logical_not);
            expression(op_plus_plus);

            // emit code, use <expr> == 0
            gen.emit(PUSH);
            gen.emit(IMM);
            gen.emit(0);
            gen.emit(EQ);

            expr_type = l_int;
        }
        else if (lexer.is_operator(op_bit_not))
        {
            // bitwise not
            match_operator(op_bit_not);
            expression(op_plus_plus);

            // emit code, use <expr> XOR -1
            gen.emit(PUSH);
            gen.emit(IMM);
            gen.emit(-1);
            gen.emit(XOR);

            expr_type = l_int;
        }
        else if (lexer.is_operator(op_plus))
        {
            // +var, do nothing
            match_operator(op_plus);
            expression((operator_t)201);

            expr_type = l_int;
        }
        else if (lexer.is_operator(op_minus))
        {
            // -var
            match_operator(op_minus);

            if (lexer.is_integer())
            {
                gen.emit(IMM);
                gen.emit(lexer.get_integer());
                match_integer();
            }
            else
            {
                gen.emit(IMM);
                gen.emit(-1);
                gen.emit(PUSH);
                expression((operator_t)201);
                gen.emit(MUL);
            }

            expr_type = l_int;
        }
        else if (lexer.is_operator(op_plus_plus, op_minus_minus))
        {
            auto tmp = lexer.get_operator();
            match_type(l_operator);
            expression((operator_t)tmp);
            if (gen.top() == LC)
            {
                gen.top(PUSH);  // to duplicate the address
                gen.emit(LC);
            }
            else if (gen.top() == LI)
            {
                gen.top(PUSH);
                gen.emit(LI);
            }
            else
            {
                printf("%d: bad lvalue of pre-increment\n", lexer.get_line());
                assert(0);;
            }
            gen.emit(PUSH);
            gen.emit(IMM);
            gen.emit(ptr_level > 0 ? LEX_SIZEOF(ptr) : 1);
            gen.emit(tmp == op_plus_plus ? ADD : SUB);
            gen.emits(expr_type);
        }
        else
        {
            printf("%d: bad expression\n", lexer.get_line());
            assert(0);;
        }
    }

    // binary operator and postfix operators.
    {
        while (lexer.is_type(l_operator) && OPERATOR_PRED(lexer.get_operator()) <= OPERATOR_PRED(level))
        {
            // handle according to current operator's precedence
            auto tmp = expr_type;
            if (lexer.is_operator(op_rparan))
            {
                break;
            }
            else if (lexer.is_operator(op_assign))
            {
                // var = expr;
                match_operator(op_assign);
                if (gen.top() == LC || gen.top() == LI)
                {
                    gen.top(PUSH); // save the lvalue's pointer
                }
                else
                {
                    printf("%d: bad lvalue in assignment\n", lexer.get_line());
                    assert(0);;
                }
                expression(op_assign);
                expr_type = tmp;
                gen.emits(expr_type);
            }
            else if (lexer.is_operator(op_query))
            {
                // expr ? a : b;
                match_operator(op_query);
                gen.emit(JZ);
                auto addr = gen.index();
                expression(op_assign);

                if (lexer.is_operator(op_colon))
                {
                    match_operator(op_colon);
                }
                else
                {
                    printf("%d: missing colon in conditional\n", lexer.get_line());
                    assert(0);;
                }

                gen.emit(gen.index() + 3, addr);
                gen.emit(JMP);
                addr = gen.index();
                expression(op_query);
                gen.emit(gen.index() + 1, addr);
            }
#define MATCH_BINOP(op, inc) \
    else if (lexer.is_operator(op)) { \
        match_operator(op); \
        gen.emit(PUSH); \
        expression(op); \
        gen.emit(inc); \
        expr_type = l_int; \
    }

MATCH_BINOP(op_logical_or, OR)
MATCH_BINOP(op_bit_xor, XOR)
MATCH_BINOP(op_logical_and, AND)
MATCH_BINOP(op_equal, EQ)
MATCH_BINOP(op_not_equal, NE)
MATCH_BINOP(op_less_than, LT)
MATCH_BINOP(op_less_than_or_equal, LE)
MATCH_BINOP(op_greater_than, GT)
MATCH_BINOP(op_greater_than_or_equal, GE)
MATCH_BINOP(op_left_shift, SHL)
MATCH_BINOP(op_right_shift, SHR)
MATCH_BINOP(op_plus, ADD)
MATCH_BINOP(op_minus, SUB)
MATCH_BINOP(op_times, MUL)
MATCH_BINOP(op_divide, DIV)
MATCH_BINOP(op_mod, MOD)
#undef MATCH_BINOP
            else if (lexer.is_operator(op_logical_or))
            {
                // logic or
                match_operator(op_logical_or);
                gen.emit(JNZ);
                auto addr = gen.index();
                expression(op_logical_or);
                gen.emit(gen.index() + 1, addr);
                expr_type = l_int;
            }
            else if (lexer.is_operator(op_logical_and))
            {
                // logic and
                match_operator(op_logical_and);
                gen.emit(JZ);
                auto addr = gen.index();
                expression(op_logical_and);
                gen.emit(gen.index() + 1, addr);
                expr_type = l_int;
            }
            else if (lexer.is_operator(op_plus_plus, op_minus_minus))
            {
                auto tmp2 = lexer.get_operator();
                match_type(l_operator);
                // postfix inc(++) and dec(--)
                // we will increase the value to the variable and decrease it
                // on `ax` to get its original value.
                if (gen.top() == LI)
                {
                    gen.top(PUSH);
                    gen.emit(LI);
                }
                else if (gen.top() == LC)
                {
                    gen.top(PUSH);
                    gen.emit(LC);
                }
                else
                {
                    printf("%d: bad value in increment\n", lexer.get_line());
                    assert(0);;
                }

                gen.emit(PUSH);
                gen.emit(IMM);
                gen.emit(ptr_level > 0 ? LEX_SIZEOF(ptr) : 1);
                gen.emit(tmp2 == op_plus_plus ? ADD : SUB);
                gen.emits(expr_type);
                gen.emit(PUSH);
                gen.emit(IMM);
                gen.emit(ptr_level > 0 ? LEX_SIZEOF(ptr) : 1);
                gen.emit(tmp2 == op_plus_plus ? SUB : ADD);
            }
            else if (lexer.is_operator(op_lsquare))
            {
                // array access var[xx]
                match_operator(op_lsquare);
                gen.emit(PUSH);
                expression(op_assign);
                match_operator(op_rsquare);

                if (ptr_level > 0)
                {
                    // pointer, `not char *`
                    gen.emit(PUSH);
                    gen.emit(IMM);
                    gen.emit(LEX_SIZEOF(int));
                    gen.emit(MUL);
                }
                else
                {
                    printf("%d: pointer type expected\n", lexer.get_line());
                    assert(0);;
                }
                ptr_level--;
                gen.emit(ADD);
                gen.emitl(expr_type);
            }
            else
            {
                printf("%d: compiler error, token = %s\n", lexer.get_line(), lexer.current().c_str());
                assert(0);;
            }
        }
    }
}

void CParser::statement()
{
    // there are 8 kinds of statements here:
    // 1. if (...) <statement> [else <statement>]
    // 2. while (...) <statement>
    // 3. { <statement> }
    // 4. return xxx;
    // 5. <empty statement>;
    // 6. expression; (expression end with semicolon)

    if (lexer.is_keyword(k_if))
    {
        // if (...) <statement> [else <statement>]
        //
        //   if (...)           <cond>
        //                      JZ a
        //     <statement>      <statement>
        //   else:              JMP b
        // a:
        //     <statement>      <statement>
        // b:                   b:
        //
        //
        match_keyword(k_if);
        match_operator(op_lparan);
        expression(op_assign);  // parse condition
        match_operator(op_rparan);

        // emit code for if
        gen.emit(JZ);
        auto b = gen.index(); // bess for branch control
        gen.emit(-1);

        statement();         // parse statement
        if (lexer.is_keyword(k_else)) { // parse else
            match_keyword(k_else);

            // emit code for JMP B
            gen.emit(gen.index() + 2, b);
            gen.emit(JMP);
            b = gen.index();
            gen.emit(-1);

            statement();
        }

        gen.emit(gen.index(), b);
    }
    else if (lexer.is_keyword(k_while))
    {
        //
        // a:                     a:
        //    while (<cond>)        <cond>
        //                          JZ b
        //     <statement>          <statement>
        //                          JMP a
        // b:                     b:
        match_keyword(k_while);

        auto a = gen.index();

        match_operator(op_lparan);
        expression(op_assign);
        match_operator(op_rparan);

        gen.emit(JZ);
        auto b = gen.index();
        gen.emit(-1);

        statement();

        gen.emit(JMP);
        gen.emit(a);
        gen.emit(gen.index(), b);
    }
    else if (lexer.is_operator(op_lbrace))
    {
        // { <statement> ... }
        match_operator(op_lbrace);

        while (!lexer.is_operator(op_rbrace))
        {
            statement();
        }

        match_operator(op_rbrace);
    }
    else if (lexer.is_keyword(k_return))
    {
        // return [expression];
        match_keyword(k_return);

        if (!lexer.is_operator(op_semi))
        {
            expression(op_assign);
        }

        match_operator(op_semi);

        // emit code for return
        gen.emit(LEV);
    }
    else if (lexer.is_operator(op_semi))
    {
        // empty statement
        match_operator(op_semi);
    }
    else
    {
        // a = b; or function_call();
        expression(op_assign);
        match_operator(op_semi);
    }
}

void CParser::enum_declaration()
{
    // parse enum [id] { a = 1, b = 3, ...}
    int i;
    i = 0;
    while (lexer.is_operator(op_rbrace))
    {
        if (!lexer.is_type(l_identifier))
        {
            printf("%d: bad enum identifier %s\n", lexer.get_line(), lexer.current().c_str());
            assert(0);;
        }
        next();
        if (lexer.is_operator(op_assign))
        {
            // like {a=10}
            next();
            if (!lexer.is_integer())
            {
                printf("%d: bad enum initializer\n", lexer.get_line());
                assert(0);;
            }
            i = lexer.get_integer();
            next();
        }

        id->cls = Num;
        id->type = l_int;
        id->value._int = i++;

        if (lexer.is_operator(op_comma))
        {
            next();
        }
    }
}

void CParser::function_parameter()
{
    auto params = 0;
    while (!lexer.is_operator(op_rparan))
    {
        // int name, ...
        auto type = parse_type();

        // pointer type
        while (lexer.is_operator(op_times))
        {
            match_operator(op_times);
            ptr_level++;
        }

        // parameter name
        if (!lexer.is_type(l_identifier))
        {
            printf("%d: bad parameter declaration\n", lexer.get_line());
            assert(0);;
        }
        if (id->cls == Loc)
        {
            printf("%d: duplicate parameter declaration\n", lexer.get_line());
            assert(0);;
        }

        match_type(l_identifier);
        // store the local variable
        id->_cls = id->cls; id->cls = Loc;
        id->_type = id->type; id->type = type;
        id->_value._int = id->value._int; id->value._int = params++;   // index of current parameter

        if (lexer.is_operator(op_comma))
        {
            match_operator(op_comma);
        }
    }
    ebp = params + 1;
}

void CParser::function_body()
{
    // type func_name (...) {...}
    //                   -->|   |<--

    // ...
    {
        // 1. local declarations
        // 2. statements
        // }

        auto pos_local = ebp; // position of local variables on the stack.

        while (lexer.is_basetype())
        {
            // local variable declaration, just like global ones.
            base_type = parse_type();

            while (!lexer.is_operator(op_semi))
            {
                auto type = base_type;
                while (lexer.is_operator(op_times))
                {
                    match_operator(op_times);
                    ptr_level++;
                }

                if (!lexer.is_type(l_identifier))
                {
                    // invalid declaration
                    printf("%d: bad local declaration\n", lexer.get_line());
                    assert(0);;
                }
                if (id->cls == Loc)
                {
                    // identifier exists
                    printf("%d: duplicate local declaration\n", lexer.get_line());
                    assert(0);;
                }
                match_type(l_identifier);

                // store the local variable
                id->_cls = id->cls; id->cls = Loc;
                id->_type = id->type; id->type = type;
                id->_value._int = id->value._int; id->value._int = ++pos_local;   // index of current parameter

                if (lexer.is_operator(op_comma))
                {
                    match_operator(op_comma);
                }
            }
            match_operator(op_semi);
        }

        // save the stack size for local variables
        gen.emit(ENT);
        gen.emit(pos_local - ebp);

        // statements
        while (!lexer.is_operator(op_rbrace))
        {
            statement();
        }

        // emit code for leaving the sub function
        gen.emit(LEV);
    }
}

void CParser::function_declaration()
{
    // type func_name (...) {...}
    //               | this part

    match_operator(op_lparan);
    function_parameter();
    match_operator(op_rparan);
    match_operator(op_lbrace);
    function_body();
    //match('}');

    gen.unwind();
}

// int [*]id [; | (...) {...}]
void CParser::global_declaration()
{
    base_type = l_int;

    // parse enum, this should be treated alone.
    if (lexer.is_keyword(k_enum))
    {
        // enum [id] { a = 10, b = 20, ... }
        match_keyword(k_enum);
        if (!lexer.is_operator(op_lbrace))
        {
            match_type(l_identifier); // skip the [id] part
        }
        if (lexer.is_operator(op_lbrace))
        {
            // parse the assign part
            match_operator(op_lbrace);
            enum_declaration();
            match_operator(op_rbrace);
        }

        match_operator(op_semi);
        return;
    }

    // parse type information
    parse_type();

    // parse the comma seperated variable declaration.
    while (!lexer.is_operator(op_semi, op_rbrace))
    {
        auto type = base_type;
        // parse pointer type, note that there may exist `int ****x;`
        while (lexer.is_operator(op_times))
        {
            match_operator(op_times);
            ptr_level++;
        }

        if (!lexer.is_type(l_identifier))
        {
            // invalid declaration
            printf("%d: bad global declaration\n", lexer.get_line());
            assert(0);;
        }
        match_type(l_identifier);
        if (id->cls)
        {
            // identifier exists
            printf("%d: duplicate global declaration\n", lexer.get_line());
            assert(0);;
        }
        id->type = type;

        if (lexer.is_operator(op_lparan))
        {
            id->cls = Fun;
            id->value._int = gen.index(); // the memory address of function
            function_declaration();
        }
        else
        {
            // variable declaration
            id->cls = Glo; // global variable
            id->value._int = gen.get_data(); // assign memory address
        }

        if (lexer.is_operator(op_comma))
        {
            match_operator(op_comma);
        }
    }
    next();
}

void CParser::match_keyword(keyword_t type)
{
    assert(lexer.is_keyword(type));
    next();
}

void CParser::match_operator(operator_t type)
{
    assert(lexer.is_operator(type));
    next();
}

void CParser::match_type(lexer_t type)
{
    assert(lexer.is_type(type));
    if (type == l_identifier)
        save_identifier();
    next();
}

void CParser::match_number()
{
    assert(lexer.is_number());
    next();
}

void CParser::match_integer()
{
    assert(lexer.is_integer());
    next();
}

lexer_t CParser::parse_type()
{
    auto type = l_int;
    if (lexer.is_type(l_keyword))
    {
        auto _unsigned = false;
        if (lexer.is_keyword(k_unsigned))
        {
            _unsigned = true;
            match_keyword(k_unsigned);
        }
        type = lexer.get_typeof(_unsigned);
        match_type(l_keyword);
    }
    return type;
}

void CParser::save_identifier()
{
    id = gen.add_sym(lexer.get_identifier());
}
