/*
    File:    symbol.h
    Created: 28 August 2017 at 13:47 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef SYMBOL_H
#define SYMBOL_H
#include <cstddef>
#include "../include/trie_for_set.h"
enum class Symbol_kind{
    Epsilon, Char, Char_class, Class_complement, All
};

struct Symbol{
    Symbol_kind kind;
    union{
        char32_t c;
        size_t   idx_of_set;
    };
};

bool operator < (const Symbol s1, const Symbol s2);

void print_symbol(const Symbol& s, const Trie_for_set_of_char32ptr& t);
#endif