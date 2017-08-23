/*
    File:    stack_elem.h
    Created: 13 December 2015 at 09:05 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef STACK_ELEM_H
#define STACK_ELEM_H

#include "../include/expr_lexem_info.h"

struct Attributes{
    union{
        Expr_lexem_info eli;
        struct{
            size_t begin_index;
            size_t end_index;
        } indeces;
    };
};

struct Stack_elem{
    size_t     st_num; ///< State number of the SLR(1)-parser.
    Attributes attr;   ///< Attributes of the grammar symbol corresponding to the state.
};
#endif