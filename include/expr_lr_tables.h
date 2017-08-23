/*
    File:    expr_lr_tables.h
    Created: 09 August 2017 at 14:40 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef EXPR_SLR_TABLES_H
#define EXPR_SLR_TABLES_H
#include "../include/types_for_lr_tables.h"
#include "../include/expr_traits.h"
extern const LR_parser_tables<Expr_grammar_traits> expr_slr_tables;
#endif