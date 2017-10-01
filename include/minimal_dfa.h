/*
    File:    minimal_dfa.h
    Created: 30 September 2017 at 10:38 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/
#ifndef MINIMAL_DFA_H
#define MINIMAL_DFA_H

#include "../include/dfa.h"
#include "../include/symbol.h"
#include "../include/trie_for_set.h"
#include <vector>
#include <map>
#include <set>
#include <utility>

using Min_DFA_state_jumps = std::map<Symbol, DFA_state_with_action>;
using Min_DFA_jumps       = std::vector<Min_DFA_state_jumps>;
/* In Min_DFA_jumps, an element having the index j is transitions
 * for the state having number j. */

struct Min_DFA{
    Min_DFA_jumps    jumps;
    size_t           begin_state = 0;
    std::set<size_t> final_states;

    Min_DFA()                    = default;
    ~Min_DFA()                   = default;
    Min_DFA(const Min_DFA& orig) = default;
};

void minimize_DFA(Min_DFA& minimized, const DFA& minimizing);

void print_minimal_DFA(const Min_DFA& a, const Trie_for_set_of_char32ptr& t);
#endif