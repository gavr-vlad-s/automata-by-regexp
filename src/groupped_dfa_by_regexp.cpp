/*
    File:    grouped_DFA_by_regexp.cpp
    Created: 29 October 2017 at 09:27 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include "../include/groupped_dfa_by_regexp.h"
#include "../include/ndfa.h"
#include "../include/dfa.h"
#include "../include/minimal_dfa.h"
#include "../include/fuse_commands.h"
#include "../include/categories.h"
#include <algorithm>
#include "../include/operations_with_sets.h"

using namespace operations_with_sets;

static void fuse_DFA_jumps(G_DFA& gdfa,
                           const Min_DFA& min_dfa,
                           const Trie_for_set_of_char32ptr& t){
    auto& mjs = min_dfa.jumps;

    G_DFA_jumps groupped_jumps;

    for(const auto& mj : mjs){
        /* mj contains transitions for the current state of the automaton min_dfa
         * (recall that mjs is a vector of transitions for states, i.e. the element
         * in this vector having the index 0 is the transitions for the state 0;
         * element with index 1 --- transitions for state 1, etc.) */
        if(mj.empty()){
            /* If there are no transitions in the source automaton for the current
             * state, then there are no transitions for this state in the automaton
             * with grouped transitions, so no grouping actions are necessary. */
            groupped_jumps.push_back(G_DFA_state_jumps());
            continue;
        }
        using Symb_DFA_state_action = std::pair<Symbol, DFA_state_with_action>;
        std::vector<Symb_DFA_state_action> j;
        j.insert(j.end(), mj.begin(), mj.end());
        std::sort(j.begin(), j.end(),
                  [](const Symb_DFA_state_action& a, const Symb_DFA_state_action& b)->bool{
                      return (a.second.st != b.second.st) ? (a.second.st < b.second.st) :
                             (a.second.action_idx < b.second.action_idx);
                  });
        /* Now the vector j is sorted so that the pairs with the same second
         * component  are arranged in series. */
        /* We now group these pairs. */
        G_DFA_state_jumps     groupped_sj;
        Category              current_category = symbol2category(j[0].first, t);
        DFA_state_with_action sa               = j[0].second;
        for(auto p : j){
            auto psa = p.second;
            if((psa.st == sa.st) && (psa.action_idx == sa.action_idx)){
                current_category = current_category + symbol2category(p.first, t);
            }else{
                groupped_sj[current_category] = sa;
                current_category              = symbol2category(p.first, t);
                sa                            = p.second;
            }
        }
        groupped_sj[current_category] = sa;
        groupped_jumps.push_back(groupped_sj);
    }
    gdfa.jumps        = groupped_jumps;
    gdfa.begin_state  = min_dfa.begin_state;
    gdfa.final_states = min_dfa.final_states;
}

void grouped_DFA_by_regexp(G_DFA&                           gdfa,
                           const Command_buffer&            buf,
                           const Trie_for_set_of_char32ptr& t)
{
    auto               buf_  = fuse_commands(buf);
    NDFA               ndfa;
    DFA                dfa;
    Min_DFA            mdfa;
    build_NDFA(ndfa, buf_);
    convert_NDFA_to_DFA(dfa, ndfa, t);
    minimize_DFA(mdfa, dfa);
    fuse_DFA_jumps(gdfa, mdfa, t);
}