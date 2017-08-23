/*
    File:    expr_lexem_info.h
    Created: 28 July 2017 at 15:15 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef EXPR_LEXEM_INFO_H
#define EXPR_LEXEM_INFO_H
#include <cstddef>
enum class Expr_lexem_code : uint16_t {
    Nothing,             UnknownLexem,       Action,
    Opened_round_brack,  Closed_round_brack, Or,
    Kleene_closure,      Positive_closure,   Optional_member,
    Character,           Begin_expression,   End_expression,
    Class_complement,    Character_class
};

struct Expr_lexem_info{
    Expr_lexem_code code;
    union{
        size_t      action_name_index;
        size_t      set_of_char_index;
        char32_t    c;
    };
};
#endif