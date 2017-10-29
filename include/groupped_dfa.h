/*
    File:    groupped_dfa.h
    Created: 14 January 2016 at 18:19 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef GROUPPED_DFA_H
#define GROUPPED_DFA_H

#include <vector>
#include <map>
#include <set>
#include <utility>
#include "../include/minimal_dfa.h"
#include "../include/categories.h"

using G_DFA_state_jumps = std::map<Category, DFA_state_with_action>;
using G_DFA_jumps       = std::vector<G_DFA_state_jumps>;

struct G_DFA{
    G_DFA_jumps      jumps;
    size_t           begin_state   = 0;
    std::set<size_t> final_states;

    G_DFA()                        = default;
    ~G_DFA()                       = default;
    G_DFA(const G_DFA& orig)       = default;
};

void print_grouped_DFA(const G_DFA& gdfa);
#endif