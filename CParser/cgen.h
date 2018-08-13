//
// Project: CMiniLang
// Author: bajdcc
//

#ifndef CMINILANG_GEN_H
#define CMINILANG_GEN_H

#include <map>
#include <memory>
#include <vector>
#include "types.h"
#include "memory.h"

namespace clib {

    class cgen {
    public:
        cgen();
        ~cgen();

        void emit(LEX_T(int));
        void emit(LEX_T(int), int index);
        void emit(const sym_t &);
        void emit(const sym_t &, int ebp);
        void emitl(lexer_t lexer);
        void emits(lexer_t lexer);

        int index() const;
        void pop();
        void unwind();

        LEX_T(int) top() const;
        void top(LEX_T(int));
        int save_string(const LEX_T(string) &);

        int get_data();

        std::shared_ptr<sym_t> add_sym(LEX_T(string));

        void eval();

    private:
        void builtin();
        void builtin_add(string_t name, class_t cls, lexer_t type, LEX_T(int) value);

    private:
        std::vector<LEX_T(int)> text;
        std::vector<LEX_T(int)> stack;
        std::vector<LEX_T(char)> data;
        std::map<LEX_T(string), std::shared_ptr<sym_t>> symbols;
    };
}

#endif //CMINILANG_GEN_H
