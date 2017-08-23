/*
    File:    expr_lr_tables.cpp
    Created: 09 August 2017 at 15:22 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include "../include/expr_lr_tables.h"
#include <utility>

static const Rule_info<Non_terminal> rules[] = {
    {Non_terminal::Nt_S, 3}, {Non_terminal::Nt_T, 3}, {Non_terminal::Nt_T, 1},
    {Non_terminal::Nt_E, 2}, {Non_terminal::Nt_E, 1}, {Non_terminal::Nt_F, 2},
    {Non_terminal::Nt_F, 1}, {Non_terminal::Nt_G, 2}, {Non_terminal::Nt_G, 1},
    {Non_terminal::Nt_H, 1}, {Non_terminal::Nt_H, 3}
};

static const GOTO_entry goto_S[] = {
    {ANY, 1}
};

static const GOTO_entry goto_T[] = {
    {9, 15}, {ANY, 3}
};

static const GOTO_entry goto_E[] = {
    {10, 16}, {ANY, 4}
};

static const GOTO_entry goto_F[] = {
    {4, 12}, {16, 12}, {ANY, 5}
};

static const GOTO_entry goto_G[] = {
    {ANY, 6}
};

static const GOTO_entry goto_H[] = {
    {ANY, 7}
};

static const GOTO_entry* goto_table[] = {
    goto_S, goto_T, goto_E, goto_F, goto_G, goto_H
};

static const Parser_action_table<Terminal> action_table = {
    {{0,Terminal::Term_p},SHIFT(2)},              {{1,Terminal::End_of_text},ACCESS},
    {{2,Terminal::Term_d},SHIFT(8)},              {{2,Terminal::Term_LP},SHIFT(9)},
    {{3,Terminal::Term_b},SHIFT(10)},             {{3,Terminal::Term_q},SHIFT(11)},
    {{4,Terminal::Term_d},SHIFT(8)},              {{4,Terminal::Term_LP},SHIFT(9)},
    {{4,Terminal::Term_b},REDUCE(T_is_E)},        {{4,Terminal::Term_q},REDUCE(T_is_E)},
    {{4,Terminal::Term_RP},REDUCE(T_is_E)},       {{5,Terminal::Term_b},REDUCE(E_is_F)},
    {{5,Terminal::Term_d},REDUCE(E_is_F)},        {{5,Terminal::Term_q},REDUCE(E_is_F)},
    {{5,Terminal::Term_LP},REDUCE(E_is_F)},       {{5,Terminal::Term_RP},REDUCE(E_is_F)},
    {{6,Terminal::Term_b},REDUCE(F_is_G)},        {{6,Terminal::Term_d},REDUCE(F_is_G)},
    {{6,Terminal::Term_q},REDUCE(F_is_G)},        {{6,Terminal::Term_LP},REDUCE(F_is_G)},
    {{6,Terminal::Term_RP},REDUCE(F_is_G)},       {{6,Terminal::Term_c},SHIFT(13)},
    {{7,Terminal::Term_a},SHIFT(14)},             {{7,Terminal::Term_b},REDUCE(G_is_H)},
    {{7,Terminal::Term_c},REDUCE(G_is_H)},        {{7,Terminal::Term_d},REDUCE(G_is_H)},
    {{7,Terminal::Term_q},REDUCE(G_is_H)},        {{7,Terminal::Term_LP},REDUCE(G_is_H)},
    {{7,Terminal::Term_RP},REDUCE(G_is_H)},       {{8,Terminal::Term_a},REDUCE(H_is_d)},
    {{8,Terminal::Term_b},REDUCE(H_is_d)},        {{8,Terminal::Term_c},REDUCE(H_is_d)},
    {{8,Terminal::Term_d},REDUCE(H_is_d)},        {{8,Terminal::Term_q},REDUCE(H_is_d)},
    {{8,Terminal::Term_LP},REDUCE(H_is_d)},       {{8,Terminal::Term_RP},REDUCE(H_is_d)},
    {{9,Terminal::Term_d},SHIFT(8)},              {{9,Terminal::Term_LP},SHIFT(9)},
    {{10,Terminal::Term_d},SHIFT(8)},             {{10,Terminal::Term_LP},SHIFT(9)},
    {{11,Terminal::End_of_text},REDUCE(S_is_pTq)},{{12,Terminal::Term_b},REDUCE(E_is_EF)},
    {{12,Terminal::Term_d},REDUCE(E_is_EF)},      {{12,Terminal::Term_q},REDUCE(E_is_EF)},
    {{12,Terminal::Term_LP},REDUCE(E_is_EF)},     {{12,Terminal::Term_RP},REDUCE(E_is_EF)},
    {{13,Terminal::Term_b},REDUCE(F_is_Gc)},      {{13,Terminal::Term_d},REDUCE(F_is_Gc)},
    {{13,Terminal::Term_q},REDUCE(F_is_Gc)},      {{13,Terminal::Term_LP},REDUCE(F_is_Gc)},
    {{13,Terminal::Term_RP},REDUCE(F_is_Gc)},     {{14,Terminal::Term_b},REDUCE(G_is_Ha)},
    {{14,Terminal::Term_d},REDUCE(G_is_Ha)},      {{14,Terminal::Term_q},REDUCE(G_is_Ha)},
    {{14,Terminal::Term_LP},REDUCE(G_is_Ha)},     {{14,Terminal::Term_RP},REDUCE(G_is_Ha)},
    {{14,Terminal::Term_c},REDUCE(G_is_Ha)},      {{15,Terminal::Term_b},SHIFT(10)},
    {{15,Terminal::Term_RP},SHIFT(17)},           {{16,Terminal::Term_d},SHIFT(8)},
    {{16,Terminal::Term_LP},SHIFT(9)},            {{16,Terminal::Term_b},REDUCE(T_is_TbE)},
    {{16,Terminal::Term_q},REDUCE(T_is_TbE)},     {{16,Terminal::Term_RP},REDUCE(T_is_TbE)},
    {{17,Terminal::Term_a},REDUCE(H_is_LP_T_RP)}, {{17,Terminal::Term_b},REDUCE(H_is_LP_T_RP)},
    {{17,Terminal::Term_c},REDUCE(H_is_LP_T_RP)}, {{17,Terminal::Term_d},REDUCE(H_is_LP_T_RP)},
    {{17,Terminal::Term_q},REDUCE(H_is_LP_T_RP)}, {{17,Terminal::Term_LP},REDUCE(H_is_LP_T_RP)},
    {{17,Terminal::Term_RP},REDUCE(H_is_LP_T_RP)}
};

const LR_parser_tables<Expr_grammar_traits> expr_slr_tables = {
    const_cast<Rule_info<Non_terminal>*>(rules),
    const_cast<GOTO_entry**>(goto_table),
    const_cast<Parser_action_table<Terminal>*>(&action_table)
};