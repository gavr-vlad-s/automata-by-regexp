/*
    File:    symbol.cpp
    Created: 28 August 2017 at 16:06 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <cstdio>
#include <set>
#include "../include/symbol.h"
#include "../include/print_size_t.h"
#include "../include/print_char32.h"
#include "../include/operations_with_sets.h"

using operations_with_sets::print_set;

bool operator < (const Symbol s1, const Symbol s2)
{
    if(s1.kind != s2.kind){
        return s1.kind < s2.kind;
    }

    switch(s1.kind){
        case Symbol_kind::Char:
            return s1.c < s2.c;

        case Symbol_kind::Char_class: case Symbol_kind::Class_complement:
            return s1.idx_of_set < s2.idx_of_set;

        default:
            return false;
    }
}

static const char* kind_strs[] = {
    "epsilon", "Char", "Char_class", "Class_complement", "All"
};

void print_symbol(const Symbol& symb, const Trie_for_set_of_char32ptr& t)
{
    std::set<char32_t> s;
    Symbol_kind        k = symb.kind;
    printf("%s ", kind_strs[static_cast<size_t>(k)]);
    switch(k){
        case Symbol_kind::Char:
            print_char32(symb.c);
            break;
        case Symbol_kind::Char_class: case Symbol_kind::Class_complement:
            s = t->get_set(symb.idx_of_set);
            print_set(s, print_char32);
            break;
        default:
            ;
    }
}