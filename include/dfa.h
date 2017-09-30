/*
    File:    dfa.h
    Created: 13 December 2015 at 09:05 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/
#ifndef DFA_H
#define DFA_H

#include "../include/ndfa.h"
#include "../include/symbol.h"
#include "../include/trie_for_set.h"
#include <vector>
#include <map>
#include <set>
#include <utility>

struct DFA_state_with_action{
    size_t st;
    size_t action_idx;
};

using State_and_symbol = std::pair<size_t, Symbol>;
using DFA_jumps = std::map<State_and_symbol, DFA_state_with_action>;

struct DFA{
    DFA_jumps        jumps;
    size_t           begin_state      = 0;
    size_t           number_of_states = 0;
    std::set<size_t> final_states;

    DFA()                             = default;
    ~DFA()                            = default;
    DFA(const DFA& orig)              = default;
};


/* This function builds the NFA (nondeterministic finite automaton) ndfa
 * by the DFA (deterministic finite automaton) a. */
void convert_NDFA_to_DFA(DFA& a, const NDFA& ndfa, const Trie_for_set_of_char32ptr& t);

void print_DFA(const DFA& a, const Trie_for_set_of_char32ptr& t);
#endif