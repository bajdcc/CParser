#include "stdafx.h"
#include "Gen.h"


CGen::CGen()
{
    builtin();
}


CGen::~CGen()
{
}

void CGen::emit(LEX_T(int) ins)
{
    text.push_back(ins);
}

void CGen::emit(LEX_T(int) ins, int index)
{
    text[index] = ins;
}

void CGen::emit(const sym_t& sym)
{
    switch (sym.type)
    {
#define DEFINE_LEXER_STORAGE(t) case l_##t: emit(sym.value._##t); break;
        DEFINE_LEXER_STORAGE(char)
        DEFINE_LEXER_STORAGE(uchar)
        DEFINE_LEXER_STORAGE(short)
        DEFINE_LEXER_STORAGE(ushort)
        DEFINE_LEXER_STORAGE(int)
        DEFINE_LEXER_STORAGE(uint)
        DEFINE_LEXER_STORAGE(long)
        DEFINE_LEXER_STORAGE(ulong)
#undef DEFINE_LEXER_STORAGE
    default: assert(!"unsupported type"); break;
    }
}

void CGen::emit(const sym_t& sym, int ebp)
{
    switch (sym.type)
    {
#define DEFINE_LEXER_STORAGE(t) case l_##t: emit(ebp - sym.value._##t); break;
        DEFINE_LEXER_STORAGE(char)
        DEFINE_LEXER_STORAGE(uchar)
        DEFINE_LEXER_STORAGE(short)
        DEFINE_LEXER_STORAGE(ushort)
        DEFINE_LEXER_STORAGE(int)
        DEFINE_LEXER_STORAGE(uint)
        DEFINE_LEXER_STORAGE(long)
        DEFINE_LEXER_STORAGE(ulong)
#undef DEFINE_LEXER_STORAGE
    default: assert(!"unsupported type"); break;
    }
}

void CGen::emitl(lexer_t lexer)
{
    switch (lexer)
    {
    case l_char: emit(LC); break;
    case l_int: emit(LI); break;
    default: assert(!"unsupported type"); break;
    }
}

void CGen::emits(lexer_t lexer)
{
    switch (lexer)
    {
    case l_char: emit(SC); break;
    case l_int: emit(SI); break;
    default: assert(!"unsupported type"); break;
    }
}

int CGen::index() const
{
    return text.size();
}

void CGen::pop()
{
    text.pop_back();
}

void CGen::unwind()
{
    // unwind local variable declarations for all local variables.
    for (auto& sym : symbols)
    {
        auto id = sym.second;
        if (id->cls == Loc) {
            id->cls = id->_cls;
            id->type = id->_type;
            id->value._int = id->_value._int;
        }
    }
}

int CGen::save_string(const LEX_T(string)& str)
{
    auto idx = data.size();
    for (auto &c : str)
    {
        data.push_back(c);
    }
    data.push_back(0);
    return idx;
}

int CGen::get_data()
{
    data.push_back(0);
    return data.size() - 1;
}

std::shared_ptr<sym_t> CGen::add_sym(LEX_T(string) name)
{
    auto sym = symbols.find(name);
    if (sym == symbols.end())
    {
        auto s = std::make_shared<sym_t>();
        s->name = name;
        symbols[name] = s;
        return s;
    }
    return sym->second;
}

LEX_T(int) CGen::top() const
{
    return *text.rbegin();
}

void CGen::top(LEX_T(int) ins)
{
    *text.rbegin() = ins;
}

void CGen::eval()
{
    // setup stack
    auto poolsize = 512;
    stack.resize(512);
    auto sp = (int *)((int)stack.data() + poolsize*LEX_SIZEOF(int));
    *--sp = -1;

    //--------------------------------------------------

    auto entry = symbols.find("main");
    if (entry == symbols.end())
    {
        printf("main() not defined\n");
        assert(0);
    }

    auto pc = entry->second->value._int;
    auto ax = 0;
    auto bp = (int *)0;

    auto cycle = 0;
    while (pc != -1)
    {
        cycle++;
        auto op = text[pc++]; // get next operation code

        // print debug info
        if (false)
        {
            printf("%03d> [%02d] %.4s", cycle, pc,
                &"LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,"
                "OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"
                "OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT"[op * 5]);
            if (op <= ADJ)
                printf(" %d\n", text[pc]);
            else
                printf("\n");
        }
        if (op == IMM) { ax = text[pc++]; }                                     // load immediate value to ax
        else if (op == LC) { ax = *(char *)ax; }                                // load character to ax, address in ax
        else if (op == LI) { ax = *(int *)ax; }                                 // load integer to ax, address in ax
        else if (op == SC) { ax = *(char *)*sp++ = ax; }                        // save character to address, value in ax, address on stack
        else if (op == SI) { *(int *)*sp++ = ax; }                              // save integer to address, value in ax, address on stack
        else if (op == PUSH) { *--sp = ax; }                                    // push the value of ax onto the stack
        else if (op == JMP) { pc = text[pc]; }                                  // jump to the address
        else if (op == JZ) { pc = ax ? pc + 1 : text[pc]; }                     // jump if ax is zero
        else if (op == JNZ) { pc = ax ? text[pc] : pc + 1; }                    // jump if ax is zero
        else if (op == CALL) { *--sp = pc + 1; pc = text[pc]; }                 // call subroutine
                                                                                // else if (op == RET)  {pc = (int *)*sp++;}                              // return from subroutine;
        else if (op == ENT) { *--sp = (int)bp; bp = sp; sp = sp - text[pc++]; } // make new stack frame
        else if (op == ADJ) { sp = sp + text[pc++]; }                           // add esp, <size>
        else if (op == LEV) { sp = bp; bp = (int *)*sp++; pc = *sp++; }         // restore call frame and PC
        else if (op == LEA) { ax = (int)(bp + text[pc++]); }                    // load address for arguments.

        else if (op == PRF) {
            auto tmp = sp + text[pc + 1]; // 利用之后的ADJ清栈指令知道函数调用的参数个数
            ax = printf((char *)(data.data() + tmp[-1]), tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); } // load address for arguments.

        else if (op == OR)  ax = *sp++ | ax;
        else if (op == XOR) ax = *sp++ ^ ax;
        else if (op == AND) ax = *sp++ & ax;
        else if (op == EQ)  ax = *sp++ == ax;
        else if (op == NE)  ax = *sp++ != ax;
        else if (op == LT)  ax = *sp++ < ax;
        else if (op == LE)  ax = *sp++ <= ax;
        else if (op == GT)  ax = *sp++ >  ax;
        else if (op == GE)  ax = *sp++ >= ax;
        else if (op == SHL) ax = *sp++ << ax;
        else if (op == SHR) ax = *sp++ >> ax;
        else if (op == ADD) ax = *sp++ + ax;
        else if (op == SUB) ax = *sp++ - ax;
        else if (op == MUL) ax = *sp++ * ax;
        else if (op == DIV) ax = *sp++ / ax;
        else if (op == MOD) ax = *sp++ % ax;
        else
        {
            printf("unknown instruction:%d\n", op);
            assert(0);
        }
    }
}

void CGen::builtin()
{
    builtin_add("printf", Sys, l_int, PRF);
}

void CGen::builtin_add(string_t name, class_t cls, lexer_t type, LEX_T(int) value)
{
    auto sym = std::make_shared<sym_t>();
    sym->name = name;
    sym->cls = cls;
    sym->type = type;
    sym->value._int = value;
    symbols.insert(std::make_pair(name, sym));
}
