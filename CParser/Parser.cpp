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
#if 0
    if (token != l_end)
    {
        printf("[%04d:%03d] %-12s - %s\n", \
            lexer.get_last_line(), \
            lexer.get_last_column(), \
            LEX_STRING(lexer.get_type()).c_str(), \
            lexer.current().c_str());
    }
#endif
}

void CParser::program()
{
    next();
    while (!lexer.is_type(l_end))
    {
        global_declaration();
    }
}

// 表达式
void CParser::expression(operator_t level)
{
    // 表达式有多种类型，像 `(char) *a[10] = (int *) func(b > 0 ? 10 : 20);
    //
    // 1. unit_unary ::= unit | unit unary_op | unary_op unit
    // 2. expr ::= unit_unary (bin_op unit_unary ...)

    // unit_unary()
    {
        if (lexer.is_type(l_end)) // 结尾
        {
            error("unexpected token EOF of expression");
            assert(0);
        }
        if (lexer.is_integer()) // 数字
        {
            auto tmp = lexer.get_integer();
            match_number();

            // emit code
            gen.emit(IMM);
            gen.emit(tmp);
            expr_type = lexer.get_type();
        }
        else if (lexer.is_type(l_string)) // 字符串
        {
            auto idx = gen.save_string(lexer.get_string());
#if 1
            printf("[%04d:%03d] String> %04X '%s'\n", lexer.get_line(), lexer.get_column(), idx, lexer.get_string().c_str());
#endif

            // emit code
            gen.emit(IMM);
            gen.emit(idx);
            gen.emit(LOAD);
            match_type(l_string);

            while (lexer.is_type(l_string))
            {
                idx = gen.save_string(lexer.get_string());
#if 0
                printf("[%04d:%03d] String> %04X '%s'\n", lexer.get_line(), lexer.get_column(), idx, lexer.get_string().c_str());
#endif
                match_type(l_string);
            }

            expr_type = l_ptr;
            ptr_level = 1;
        }
        else if (lexer.is_keyword(k_sizeof)) // sizeof
        {
            // 支持 `sizeof(int)`, `sizeof(char)` and `sizeof(*...)`
            match_keyword(k_sizeof);
            match_operator(op_lparan);
            expr_type = l_int;
            ptr_level = 0;

            if (lexer.is_keyword(k_unsigned))
            {
                match_keyword(k_unsigned); // 有符号或无符号大小相同
            }

            auto size = lexer.get_sizeof();
            next();

            while (lexer.is_operator(op_times))
            {
                match_operator(op_times);
                if (expr_type != l_ptr)
                {
                    size = LEX_SIZEOF(ptr); // 指针大小
                    expr_type = l_ptr;
                }
            }

            match_operator(op_rparan);

            // emit code
            gen.emit(IMM);
            gen.emit(size);

            expr_type = l_int;
            ptr_level = 0;
        }
        else if (lexer.is_type(l_identifier)) // 变量
        {
            // 三种可能
            // 1. function call 函数名调用
            // 2. Enum variable 枚举值
            // 3. global/local variable 全局/局部变量名
            match_type(l_identifier);

            auto func_id = id; // 保存当前的变量名(因为如果是函数调用，id会被覆盖)

            if (lexer.is_operator(op_lparan)) // 函数调用
            {
                // function call
                match_operator(op_lparan);

                // pass in arguments
                auto tmp = 0; // number of arguments
                while (!lexer.is_operator(op_rparan)) // 参数数量
                {
                    expression(op_assign);
                    gen.emit(PUSH);
                    tmp++;

                    if (lexer.is_operator(op_comma))
                    {
                        match_operator(op_comma);
                    }
                }
                match_operator(op_rparan);

                id = func_id;

                // emit code
                if (id->cls == Sys) // 内建函数
                {
                    // system functions
                    gen.emit(*id);
                }
                else if (id->cls == Fun) // 普通函数
                {
                    // function call
                    gen.emit(CALL);
                    gen.emit(*id);
                }
                else
                {
                    error("bad function call");
                }

                // 清除栈上参数
                if (tmp > 0)
                {
                    gen.emit(ADJ);
                    gen.emit(tmp);
                }
                expr_type = id->type;
                ptr_level = id->ptr;
            }
            else if (id->cls == Num)
            {
                // enum variable
                gen.emit(IMM);
                gen.emit(*id);
                expr_type = l_int;
                ptr_level = 0;
            }
            else
            {
                // variable
                if (id->cls == Loc) // 本地变量
                {
                    gen.emit(LEA);
                    gen.emit(*id, ebp);
                }
                else if (id->cls == Glo) // 全局变量
                {
                    gen.emit(IMM);
                    gen.emit(*id);
                    gen.emit(LOAD);
                }
                else
                {
                    error("undefined variable");
                }
                // emit code
                // 读取值到ax寄存器中
                expr_type = id->type;
                ptr_level = id->ptr;
                gen.emitl(id->ptr > 0 ? l_int : expr_type);
            }
        }
        else if (lexer.is_operator(op_lparan)) // 强制转换
        {
            // cast or parenthesis
            match_operator(op_lparan);
            if (lexer.is_type(l_keyword))
            {
                auto tmp = parse_type();
                auto ptr = 0;

                while (lexer.is_operator(op_times))
                {
                    match_operator(op_times);
                    ptr++;
                }
                match_operator(op_rparan);

                expression(op_plus_plus);
                expr_type = tmp;
                ptr_level = ptr;
            }
            else
            {
                // 普通括号嵌套
                expression(op_assign);
                match_operator(op_rparan);
            }
        }
        else if (lexer.is_operator(op_times)) // 解引用
        {
            // dereference *<addr>
            match_operator(op_times);
            expression(op_plus_plus);
            ptr_level--;

            gen.emitl(id->ptr > 0 ? l_int : expr_type);
        }
        else if (lexer.is_operator(op_bit_and)) // 取地址
        {
            // get the address of
            match_operator(op_bit_and);
            expression(op_plus_plus);
            if (gen.top() == LI || gen.top() == LC)
            {
                gen.pop();
            }
            else
            {
                error("bad address of");
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
            ptr_level = 0;
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
            ptr_level = 0;
        }
        else if (lexer.is_operator(op_plus))
        {
            // +var, do nothing
            match_operator(op_plus);
            expression(op_plus_plus);

            expr_type = l_int;
            ptr_level = 0;
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
                expression(op_plus_plus);
                gen.emit(MUL);
            }

            expr_type = l_int;
            ptr_level = 0;
        }
        else if (lexer.is_operator(op_plus_plus, op_minus_minus))
        {
            auto tmp = lexer.get_operator();
            match_type(l_operator);
            expression(op_plus_plus);
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
                error("bad lvalue of pre-increment");
            }
            gen.emit(PUSH);
            gen.emit(IMM);
            gen.emit(ptr_level > 0 ? LEX_SIZEOF(ptr) : 1);
            gen.emit(tmp == op_plus_plus ? ADD : SUB);
            gen.emits(ptr_level > 0 ? l_int : expr_type);
        }
        else
        {
            error("bad expression");
        }
    }

    // 二元表达式以及后缀操作符
    {
        while (lexer.is_type(l_operator) && OPERATOR_PRED(lexer.get_operator()) <= OPERATOR_PRED(level)) // 优先级判断
        {
            auto tmp = expr_type;
            auto ptr = ptr_level;
            if (lexer.is_operator(op_rparan) || lexer.is_operator(op_rsquare) || lexer.is_operator(op_colon))
            {
                break;
            }
            if (lexer.is_operator(op_assign))
            {
                // var = expr;
                match_operator(op_assign);
                if (gen.top() == LI || gen.top() == LC)
                {
                    gen.top(PUSH); // save the lvalue's pointer
                }
                else
                {
                    error("bad lvalue in assignment");
                }
                expression(op_assign);
                expr_type = tmp;
                ptr_level = ptr;
                gen.emits(ptr_level > 0 ? l_int : expr_type);
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
                    error("missing colon in conditional");
                }

                gen.emit(gen.index() + 3, addr);
                gen.emit(JMP);
                addr = gen.index();
                expression(op_query);
                gen.emit(gen.index() + 1, addr);
            }
#define MATCH_BINOP(op, inc, pred) \
    else if (lexer.is_operator(op)) { \
        match_operator(op); \
        gen.emit(PUSH); \
        expression(pred); \
        gen.emit(inc); \
        expr_type = l_int; \
        ptr_level = 0; \
    }

MATCH_BINOP(op_bit_or, OR, op_bit_xor)
MATCH_BINOP(op_bit_xor, XOR, op_bit_and)
MATCH_BINOP(op_bit_and, AND, op_equal)
MATCH_BINOP(op_equal, EQ, op_not_equal)
MATCH_BINOP(op_not_equal, NE, op_less_than)
MATCH_BINOP(op_less_than, LT, op_left_shift)
MATCH_BINOP(op_less_than_or_equal, LE, op_left_shift)
MATCH_BINOP(op_greater_than, GT, op_left_shift)
MATCH_BINOP(op_greater_than_or_equal, GE, op_left_shift)
MATCH_BINOP(op_left_shift, SHL, op_plus)
MATCH_BINOP(op_right_shift, SHR, op_plus)
MATCH_BINOP(op_times, MUL, op_plus_plus)
MATCH_BINOP(op_divide, DIV, op_plus_plus)
MATCH_BINOP(op_mod, MOD, op_plus_plus)
#undef MATCH_BINOP
            else if (lexer.is_operator(op_plus))
            {
                // add
                match_operator(op_plus);
                gen.emit(PUSH);
                expression(op_times);
                expr_type = l_int;
                ptr_level = ptr;
                expr_type = tmp;
                if (ptr > 0) {
                    gen.emit(PUSH);
                    gen.emit(IMM);
                    gen.emit(LEX_SIZEOF(ptr));
                    gen.emit(MUL);
                }
                gen.emit(ADD);
            }
            else if (lexer.is_operator(op_minus))
            {
                // sub
                match_operator(op_minus);
                gen.emit(PUSH);
                expression(op_times);
                expr_type = l_int;
                ptr_level = ptr;
                expr_type = tmp;
                if (ptr > 0) {
                    gen.emit(PUSH);
                    gen.emit(IMM);
                    gen.emit(LEX_SIZEOF(ptr));
                    gen.emit(MUL);
                }
                gen.emit(SUB);
            }
            else if (lexer.is_operator(op_logical_or))
            {
                // logic or
                match_operator(op_logical_or);
                gen.emit(JNZ);
                auto addr = gen.index();
                gen.emit(-1);
                expression(op_logical_and);
                gen.emit(gen.index(), addr);
                expr_type = l_int;
                ptr_level = 0;
            }
            else if (lexer.is_operator(op_logical_and))
            {
                // logic and
                match_operator(op_logical_and);
                gen.emit(JZ);
                auto addr = gen.index();
                gen.emit(-1);
                expression(op_bit_or);
                gen.emit(gen.index(), addr);
                expr_type = l_int;
                ptr_level = 0;
            }
            else if (lexer.is_operator(op_plus_plus, op_minus_minus))
            {
                auto tmp2 = lexer.get_operator();
                match_type(l_operator);
                // postfix inc(++) and dec(--)
                // we will increase the value to the variable and decrease it
                // on `ax` to get its original value.
                // 构建副本
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
                    error("bad value in increment");
                }

                gen.emit(PUSH);
                gen.emit(IMM);
                gen.emit(ptr_level > 0 ? LEX_SIZEOF(ptr) : 1);
                gen.emit(tmp2 == op_plus_plus ? ADD : SUB);
                gen.emits(ptr_level > 0 ? l_int : expr_type);

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

                if (ptr > 0)
                {
                    // pointer, `not char *`
                    if (id->type != l_char)
                    {
                        gen.emit(PUSH);
                        gen.emit(IMM);
                        gen.emit(LEX_SIZEOF(int));
                        gen.emit(MUL);
                    }
                    gen.emit(ADD);
                    gen.emit(LI);
                    expr_type = tmp;
                    ptr_level = ptr - 1;
                }
                else
                {
                    error("pointer type expected");
                }
            }
            else
            {
                error("compiler error, token = " + lexer.current());
            }
        }
    }
}

// 基本语句
void CParser::statement()
{
    // there are 8 kinds of statements here:
    // 1. if (...) <statement> [else <statement>]
    // 2. while (...) <statement>
    // 3. { <statement> }
    // 4. return xxx;
    // 5. <empty statement>;
    // 6. expression; (expression end with semicolon)

    if (lexer.is_keyword(k_if)) // if判断
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
        expression(op_assign);  // if判断的条件
        match_operator(op_rparan);

        // emit code for if
        gen.emit(JZ);
        auto b = gen.index(); // 分支判断，这里要回写到b(即if结尾)
        gen.emit(-1);

        statement();  // 处理if执行体
        if (lexer.is_keyword(k_else)) { // 处理else
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
    else if (lexer.is_keyword(k_while)) // while循环
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
        expression(op_assign); // 条件
        match_operator(op_rparan);

        gen.emit(JZ);
        auto b = gen.index(); // 退出的地方
        gen.emit(-1);

        statement();

        gen.emit(JMP);
        gen.emit(a); // 重复循环
        gen.emit(gen.index(), b);
    }
    else if (lexer.is_operator(op_lbrace)) // 语句
    {
        // { <statement> ... }
        match_operator(op_lbrace);

        while (!lexer.is_operator(op_rbrace))
        {
            statement();
        }

        match_operator(op_rbrace);
    }
    else if (lexer.is_keyword(k_return)) // 返回
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
    else if (lexer.is_operator(op_semi)) // 空语句
    {
        // empty statement
        match_operator(op_semi);
    }
    else // 表达式
    {
        // a = b; or function_call();
        expression(op_assign);
        match_operator(op_semi);
    }
}

// 枚举声明
void CParser::enum_declaration()
{
    // parse enum [id] { a = 1, b = 3, ...}
    int i = 0;
    while (!lexer.is_operator(op_rbrace))
    {
        if (!lexer.is_type(l_identifier))
        {
            error("bad enum identifier " + lexer.current());
        }
        match_type(l_identifier);
        if (lexer.is_operator(op_assign)) // 赋值
        {
            // like { a = 10 }
            next();
            if (!lexer.is_integer())
            {
                error("bad enum initializer");
            }
            i = lexer.get_integer();
            next();
        }

        // 保存值到变量中
        id->cls = Num;
        id->type = l_int;
        id->value._int = i++;

        if (lexer.is_operator(op_comma))
        {
            next();
        }
    }
}

// 函数参数
void CParser::function_parameter()
{
    auto params = 0;
    while (!lexer.is_operator(op_rparan)) // 判断参数右括号结尾
    {
        // int name, ...
        auto type = parse_type(); // 基本类型
        auto ptr = 0;

        // pointer type
        while (lexer.is_operator(op_times)) // 指针
        {
            match_operator(op_times);
            ptr++;
        }

        // parameter name
        if (!lexer.is_type(l_identifier))
        {
            error("bad parameter declaration");
        }

        match_type(l_identifier);
        if (id->cls == Loc) // 与变量声明冲突
        {
            error("duplicate parameter declaration");
        }

        // 保存本地变量
        // 这里为什么要多设个地方保存之前的值，是因为变量有域(大括号划分)的限制
        // 进入一个函数体时，全局变量需要保存，退出函数体时恢复
        id->_cls = id->cls; id->cls = Loc;
        id->_type = id->type; id->type = type;
        id->_value._int = id->value._int; id->value._int = params; // 变量在栈上地址
        id->_ptr = id->ptr; id->ptr = ptr;

        params += 4;

        if (lexer.is_operator(op_comma))
        {
            match_operator(op_comma);
        }
    }
    ebp = params + 4;
}

// 函数体
void CParser::function_body()
{
    // type func_name (...) {...}
    //                   -->|   |<--

    // ...
    {
        // 1. local declarations
        // 2. statements
        // }

        auto pos_local = ebp; // 变量在栈上地址

        while (lexer.is_basetype())
        {
            // 处理基本类型
            base_type = parse_type();

            while (!lexer.is_operator(op_semi)) // 判断语句结束
            {
                auto ptr = 0;
                auto type = base_type;
                while (lexer.is_operator(op_times)) // 处理指针
                {
                    match_operator(op_times);
                    ptr++;
                }

                if (!lexer.is_type(l_identifier))
                {
                    // invalid declaration
                    error("bad local declaration");
                }
                match_type(l_identifier);
                if (id->cls == Loc) // 变量重复声明
                {
                    // identifier exists
                    error("duplicate local declaration");
                }

                // 保存本地变量
                // 这里为什么要多设个地方保存之前的值，是因为变量有域(大括号划分)的限制
                // 进入一个函数体时，全局变量需要保存，退出函数体时恢复
                id->_cls = id->cls; id->cls = Loc;
                id->_type = id->type; id->type = type;
                id->_value._int = id->value._int; id->value._int = pos_local;  // 参数在栈上地址
                id->_ptr = id->ptr; id->ptr = ptr;

                pos_local += 4;

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

// 函数声明
void CParser::function_declaration()
{
    // type func_name (...) {...}
    //               | this part

    match_operator(op_lparan);
    function_parameter();
    match_operator(op_rparan);
    match_operator(op_lbrace);
    function_body();
    // match('}'); 这里不处理右括号是为了上层函数判断结尾

    gen.unwind();
}

// 变量声明语句(全局或函数体内)
// int [*]id [; | (...) {...}]
void CParser::global_declaration()
{
    base_type = l_int;

    // 处理enum枚举
    if (lexer.is_keyword(k_enum))
    {
        // enum [id] { a = 10, b = 20, ... }
        match_keyword(k_enum);
        if (!lexer.is_operator(op_lbrace))
        {
            match_type(l_identifier); // 省略了[id]枚举名
        }
        if (lexer.is_operator(op_lbrace))
        {
            // 处理枚举体
            match_operator(op_lbrace);
            enum_declaration(); // 枚举的变量声明部分，即 a = 10, b = 20, ...
            match_operator(op_rbrace);
        }

        match_operator(op_semi);
        return;
    }

    // 解析基本类型，即变量声明时的类型
    base_type = parse_type();
    if (base_type == l_none)
        base_type = l_int;

    // 处理逗号分隔的变量声明
    while (!lexer.is_operator(op_semi, op_rbrace))
    {
        auto type = base_type; // 以先声明的类型为基础
        auto ptr = 0;
        // 处理指针, 像`int ****x;`
        while (lexer.is_operator(op_times))
        {
            match_operator(op_times);
            ptr++;
        }

        if (!lexer.is_type(l_identifier)) // 不存在变量名则报错
        {
            // invalid declaration
            error("bad global declaration");
        }
        match_type(l_identifier);
        if (id->cls) // 变量名已经声明，则报重复声明错误
        {
            // identifier exists
            error("duplicate global declaration");
        }
        id->type = type;
        id->ptr = ptr;

        if (lexer.is_operator(op_lparan)) // 有左括号则应判定是函数声明
        {
            id->cls = Fun;
            id->value._int = gen.index(); // 记录函数地址
#if 1
            printf("[%04d:%03d] Function> %04X '%s'\n", lexer.get_line(), lexer.get_column(), id->value._int * 4, id->name.c_str());
#endif
            function_declaration();
#if 1
            printf("[%04d:%03d] Function> %04X '%s'\n", lexer.get_line(), lexer.get_column(), id->value._int * 4, id->name.c_str());
#endif
        }
        else
        {
            // 处理变量声明
            id->cls = Glo; // 全局变量
            id->value._int = gen.get_data(); // 记录变量地址
#if 1
            printf("[%04d:%03d] Global> %04X '%s'\n", lexer.get_line(), lexer.get_column(), id->value._int, id->name.c_str());
#endif
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

// 处理基本类型
// 分char,short,int,long以及相应的无符号类型(无符号暂时不支持)
// 以及float和double(暂时不支持)
lexer_t CParser::parse_type()
{
    auto type = l_int;
    if (lexer.is_type(l_keyword))
    {
        auto _unsigned = false;
        if (lexer.is_keyword(k_unsigned)) // 判定是否带有unsigned前缀
        {
            _unsigned = true;
            match_keyword(k_unsigned);
        }
        type = lexer.get_typeof(_unsigned); // 根据keyword得到lexer_t
        match_type(l_keyword);
    }
    return type;
}

// 保存刚刚识别的变量名
void CParser::save_identifier()
{
    id = gen.add_sym(lexer.get_identifier());
    if (id->ptr == 0)
        id->ptr = ptr_level;
}

void CParser::error(string_t info)
{
    printf("[%04d:%03d] ERROR: %s\n", lexer.get_line(), lexer.get_column(), info.c_str());
    assert(0);
}
