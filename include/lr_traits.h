/*
    File:    lr_traits.h
    Created: 19 August 2017 at 09:52 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef LR_TRAITS_H
#define LR_TRAITS_H
#include "../include/myconcepts.h"
template<typename Lexem_type>
struct Attributes{
    union{
        Lexem_type li;
        struct{
            size_t begin_index;
            size_t end_index;
        } indeces;
    };
};

template<typename Lexem_type>
struct Stack_elem{
    //! State number of the SLR(1)-parser.
    size_t                 st_num;
    //! Attributes of the grammar symbol corresponding to the state.
    Attributes<Lexem_type> attr;
};

template<Integral T, Integral NT, typename R, size_t N>
struct Grammar_traits{
    using Terminal_t                 = T;
    using Non_terminal_t             = NT;
    using Rule_t                     = R;
    static const size_t rule_max_len = N;
};

template<typename S, typename Lexem_type>
struct Scaner_traits{
    using Scaner_t                   = S;
    using Lexem_t                    = Lexem_type;
};
#endif