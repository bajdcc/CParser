#include "stdafx.h"
#include "Gen.h"
#include "VirtualMachine.h"


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
    case l_char: emit(LI); break;
    case l_int: emit(LI); break;
    default: assert(!"unsupported type"); break;
    }
}

void CGen::emits(lexer_t lexer)
{
    switch (lexer)
    {
    case l_char: emit(SI); break;
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
        s->ptr = 0;
        s->cls = CLASS_NULL;
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
    auto entry = symbols.find("main");
    if (entry == symbols.end())
    {
        printf("main() not defined\n");
        assert(0);
    }
    CVirtualMachine vm(text, data);
    vm.exec(entry->second->value._int);
}

void CGen::builtin()
{
    builtin_add("printf", Sys, l_int, PRTF);
    builtin_add("memcmp", Sys, l_int, MCMP);
    builtin_add("exit", Sys, l_int, EXIT);
    builtin_add("memset", Sys, l_int, MSET);
    builtin_add("open", Sys, l_int, OPEN);
    builtin_add("read", Sys, l_int, READ);
    builtin_add("close", Sys, l_int, CLOS);
    builtin_add("malloc", Sys, l_int, MALC);
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
