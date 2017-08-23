/*
    File:    aux_expr_lexem.h
    Created: 20 July 2017 at 12:19 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/
#ifndef AUX_EXPR_LEXEM_H
#define AUX_EXPR_LEXEM_H
#include <cstddef>
enum class Aux_expr_lexem_code : uint16_t {
    Nothing,                     UnknownLexem,              Action,
    Opened_round_brack,          Closed_round_brack,        Or,
    Kleene_closure,              Positive_closure,          Optional_member,
    Character,                   Begin_expression,          End_expression,
    Class_Latin,                 Class_Letter,              Class_Russian,
    Class_bdigits,               Class_digits,              Class_latin,
    Class_letter,                Class_odigits,             Class_russian,
    Class_xdigits,               Class_ndq,                 Class_nsq,
    Begin_char_class_complement, End_char_class_complement, M_Class_Latin,
    M_Class_Letter,              M_Class_Russian,           M_Class_bdigits,
    M_Class_digits,              M_Class_latin,             M_Class_letter,
    M_Class_odigits,             M_Class_russian,           M_Class_xdigits,
    M_Class_ndq,                 M_Class_nsq
};

struct Aux_expr_lexem_info{
    Aux_expr_lexem_code code;
    union{
        size_t      action_name_index;
        char32_t    c;
    };
};
#endif