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
enum class Symbol_kind{
    Epsilon, Char, Char_class, Class_complement
};

struct Symbol{
    Symbol_kind kind;
    union{
        char32_t c;
        size_t   idx_of_set;
    };
};
#endif