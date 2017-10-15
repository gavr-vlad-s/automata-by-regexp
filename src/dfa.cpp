/*
    File:    dfa.cpp
    Created: 13 December 2015 at 09:05 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <string>
#include <stack>
#include <list>
#include <cstdio>
#include "../include/ndfa.h"
#include "../include/dfa.h"
#include "../include/trie_for_set.h"
#include "../include/operations_with_sets.h"
#include "../include/print_size_t.h"

// #include "../include/size_t_trie.h"
// #include "../include/operations_with_sets.h"
//
using namespace operations_with_sets;

// static std::set<Symbol> non_detalized_symbols(const NDFA& a, const std::set<size_t>& s)
// {
//     std::set<Symbol> result;
//     for(size_t st : s){
//         auto& st_jumps = a.jumps[st];
//         for(auto m : st_jumps){
//             /* loop on all transitions of the state st of
//              * the non-deterministic finite automaton a */
//             Symbol symb = m.first;
//             if(symb.kind != Symbol_kind::Epsilon){
//                 result.insert(symb);
//             }
//         }
//     }
//     return result;
// }

// static std::set<Symbol> detalize_symbols(const std::set<Symbol>& s,
//                                          const Trie_for_set_of_char32ptr& t)
// {
//     std::set<Symbol> result;
//     for(const auto& symb : s){
//         if(symb.kind != Symbol_kind::Char_class){
//             result.insert(symb);
//         }else{
//             auto detalized_symbol = t->get_set(symb.idx_of_set);
//             for(char32_t c : detalized_symbol){
//                 result.insert(char32_to_symbol(c));
//             }
//         }
//     }
//     return result;
// }

// // // // // // /* This function writes a set consisting of elements of type size_t into the prefix
// // // // // //  * tree of such sets, and returns the resulting index. */
// // // // // // size_t write_set_into_trie(Size_t_trie& trie, const std::set<size_t>& s){
// // // // // //     std::basic_string<size_t> str;
// // // // // //     for(size_t x : s){
// // // // // //         str.push_back(x);
// // // // // //     }
// // // // // //     size_t set_idx = trie.insert(str);
// // // // // //     return set_idx;
// // // // // // }

// static NDFA_state_jumps detalized_jumps(const NDFA_state_jumps& jmps,
//                                         const Trie_for_set_of_char32ptr& tr)
// {
//     NDFA_state_jumps result;
//     for(const auto& j : jmps){
//         auto& symb = j.first;
//         auto& sa   = j.second;
//         if(symb.kind != Symbol_kind::Char_class){
//             result[symb] = sa;
//         }else{
//             auto detalized_symbol = tr->get_set(symb.idx_of_set);
//             for(char32_t c : detalized_symbol){
//                 result[char32_to_symbol(c)] = sa;
//             }
//         }
//     }
//     return result;
// }

static Symbol char32_to_symbol(char32_t ch)
{
    Symbol s;
    s.kind = Symbol_kind::Char;
    s.c    = ch;
    return s;
}

static std::set<Symbol> expand_symbol(const Symbol                     symb,
                                      const Trie_for_set_of_char32ptr& tr)
{
    std::set<Symbol> result;
    if(symb.kind != Symbol_kind::Char_class){
        result.insert(symb);
    }else{
        auto symb_contents = tr->get_set(symb.idx_of_set);
        for(const char32_t c : symb_contents){
            result.insert(char32_to_symbol(c));
        }
    }
    return result;
}

static NDFA_state_jumps expand_state_jumps(const NDFA_state_jumps&          jumps,
                                           const Trie_for_set_of_char32ptr& tr)
{
    NDFA_state_jumps result;
    for(const auto& jump : jumps){
        auto& symb          = jump.first;
        auto& sa            = jump.second;
        auto  expanded_symb = expand_symbol(symb, tr);
        for(const auto& s : expanded_symb){
            auto  it            = result.find(s);
            if(it != result.end()){
                auto   old_sa     = it->second;
                auto&  old_states = old_sa.first;
                size_t old_act    = old_sa.second;
                size_t new_act    = old_act ? old_act : sa.second;
                result[s]         = std::make_pair(sa.first + old_states, new_act);
            }else{
                result[s]         = sa;
            }
        }
    }
    return result;
}

static NDFA_jumps expand_jumps(const NDFA& a, const Trie_for_set_of_char32ptr& tr)
{
    auto&      old_jmps      = a.jumps;
    NDFA_jumps result        = NDFA_jumps(old_jmps.size());
    size_t     current_state = 0;
    for(const auto& state_jumps : old_jmps){
        result[current_state] = expand_state_jumps(state_jumps, tr);
        current_state++;
    }
    return result;
}

/* The next function using the set of s states of the NFA, constructs a set of symbols
 * from which at least one of the states belonging to the set s has a transition.
 * Epsilon transitions are not taken into account. */
static std::set<Symbol> jump_chars_set(const NDFA_jumps& js, const std::set<size_t>& s)
{
    std::set<Symbol> result;
    for(size_t state : s){
        auto& st_js = js[state];
        for(const auto& st_j : st_js){
            auto symb = st_j.first;
            if(symb.kind != Symbol_kind::Epsilon){
                result.insert(symb);
            }
        }
    }
    return result;
}

static const Symbol eps = {.kind = Symbol_kind::Epsilon};

static std::set<size_t> epsilon_closure(const NDFA_jumps& jumps,
                                        const std::set<size_t>& s)
{
    std::stack<size_t> stack_of_states;
    std::set<size_t>   eps_clos = s;
    for(size_t x : s){
        stack_of_states.push(x);
    }
    while(!stack_of_states.empty()){
        size_t t = stack_of_states.top();
        stack_of_states.pop();
        auto& t_jumps = jumps[t];
        auto  iter    = t_jumps.find(eps);
        if (iter != t_jumps.end()){
            auto eps_jumps = (iter->second).first;
            for(size_t st : eps_jumps){
                auto it = eps_clos.find(st);
                if(it == eps_clos.end()){
                    eps_clos.insert(st);
                    stack_of_states.push(st);
                }
            }
        }
    }
    return eps_clos;
}

/* The following function computes the set of states to which it will pass a set of
 * states, denoted by states, by the symbol (or class of characters) gc and returns
 * the resulting set as a container std :: set <size_t>. */
static std::set<size_t> move(const NDFA_jumps&       jumps,
                             const std::set<size_t>& states,
                             const Symbol            gc)
{
    std::set<size_t> move_set;
    for(size_t st : states){
        auto& st_jumps = jumps[st];
        auto  it       = st_jumps.find(gc);
        if(it != st_jumps.end()){
            /* The states in which a transition is made from the state st
             * by the symbol gc */
            auto& s = (it->second).first;
            move_set.insert(s.begin(), s.end());
        }
    }
    return move_set;
}

/* The contains_final_state function checks whether the set of states s of the NFA
 * contains a finite state of this automaton. */
static bool contains_final_state(const NDFA& a, const std::set<size_t>& s)
{
    return s.find(a.final_state) != s.end();
}

/* The following function calculates the action to be taken when passing from the DFA
 * state represented by the set of s states of the NFA, by the symbol or class of
 *  characters gc.  */
static size_t action_for_dfa_jump(const NDFA_jumps&       jumps,
                                  const std::set<size_t>& s,
                                  Symbol                  gc)
{
    for(size_t st : s){
        auto&  st_jmp = jumps[st];
        auto   it     = st_jmp.find(gc);
        if(it != st_jmp.end()){
            auto   target = it->second;
            size_t act    = target.second;
            if(act){
                return act;
            }
        }
    }
    return 0;
}

/* This function for NFA (nondeterministic finite state machine)
 * ndfa builds the corresponding DFA a. */
void convert_NDFA_to_DFA(DFA& a, const NDFA& ndfa, const Trie_for_set_of_char32ptr& tr)
{
    std::vector<size_t>      marked_states_of_dfa;
    std::vector<size_t>      unmarked_states_of_dfa;
    Trie_for_set_of_sizet    sets_of_ndfa_states;
    Set_of_states            finals;
    std::map<size_t, size_t> states_nums; /* This is a mapping of the indices of
                                             the sets of states of the NFA to the
                                             DFA state numbers. The numbering of the
                                             latter starts from zero, and they are
                                             numbered in the order of appearance in
                                             the calculations. */
    size_t current_nom_of_DFA_state = 0;

    auto expanded_jumps = expand_jumps(ndfa, tr);

    // We calculate the initial state of the DFA a.
    auto begin_state       = epsilon_closure(expanded_jumps,  {ndfa.begin_state});
    size_t begin_state_idx = sets_of_ndfa_states.insertSet(begin_state); // write_set_into_trie(sets_of_ndfa_states, begin_state);

    states_nums[begin_state_idx] = current_nom_of_DFA_state;
    if(contains_final_state(ndfa, begin_state)){
        finals.insert(current_nom_of_DFA_state);
    }
    current_nom_of_DFA_state++;

    unmarked_states_of_dfa.push_back(begin_state_idx);

    while(!unmarked_states_of_dfa.empty()){
        size_t t_idx = unmarked_states_of_dfa.back();
        marked_states_of_dfa.push_back(t_idx);
        unmarked_states_of_dfa.pop_back();
        // Unmarked state processing.
        /* To do this, we first get a list of all the symbols and symbol classes by
         * which the transition from the processed state is possible at all. */
        auto t          = sets_of_ndfa_states.get_set(t_idx);
        auto jump_chars = jump_chars_set(expanded_jumps, t);
        /* And now we calculate the transitions on these symbols from the
         * current state of the DFA. */
        for(Symbol gc : jump_chars){
            auto   u     = epsilon_closure(expanded_jumps, move(expanded_jumps, t, gc));
            size_t u_idx = sets_of_ndfa_states.insertSet(u); // write_set_into_trie(sets_of_ndfa_states, u);
            if(!u.empty()){
                /* Here we find ourselves, if the epsilon-closure is not empty, that
                 * is, there is a transition from t by gc. */
                DFA_state_with_action sa;
                sa.action_idx = action_for_dfa_jump(expanded_jumps, t, gc);
                auto it       = states_nums.find(u_idx);
                if(it == states_nums.end()){
                    unmarked_states_of_dfa.push_back(u_idx);
                    states_nums[u_idx] = current_nom_of_DFA_state;
                    if(contains_final_state(ndfa, u)){
                        finals.insert(current_nom_of_DFA_state);
                    }
                    sa.st = current_nom_of_DFA_state;
                    current_nom_of_DFA_state++;
                }else{
                    sa.st = it->second;
                }
                // Now add a transition entry.
                a.jumps[std::make_pair(states_nums[t_idx], gc)] = sa;
            }
        }
    }
    a.final_states     = finals;
    a.number_of_states = current_nom_of_DFA_state;
}

void print_DFA(const DFA& a, const Trie_for_set_of_char32ptr& t)
{
    printf("Number of states of DFA: %zu.\n", a.number_of_states);

    printf("Begin state of DFA: %zu.\n", a.begin_state);

    printf("Final states of DFA: ");
    print_set(a.final_states, print_size_t);
    putchar('\n');

    for(const auto& j : a.jumps){
        auto& st_and_symb = j.first;
        auto& sa          = j.second;
        printf("delta(%zu, ", st_and_symb.first);
        print_symbol(st_and_symb.second, t);
        printf(") = %zu with action having index %zu\n", sa.st, sa.action_idx);

    }
}