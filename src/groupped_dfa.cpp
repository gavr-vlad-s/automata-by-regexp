/*
    File:    groupped_dfa.cpp
    Created: 29 October 2017 at 11:15 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include "../include/groupped_dfa.h"
#include "../include/operations_with_sets.h"
#include "../include/print_size_t.h"
#include "../include/categories.h"
#include <cstdio>

using namespace operations_with_sets;

static const char* num_of_states_format_str =
    "Number of states of minimized DFA with grouped transitions: %zu.\n";

static const char* begin_state_format_str   =
    "Begin state of minimized DFA:                               %zu.\n";

static const char* final_states_format_str  =
    "Final states of minimized DFA: ";


void print_grouped_DFA(const G_DFA& gdfa)
{
    printf(num_of_states_format_str, gdfa.jumps.size());
    printf(begin_state_format_str, gdfa.begin_state);

    printf(final_states_format_str);
    print_set(gdfa.final_states, print_size_t);
    putchar('\n');

    size_t current_state = 0;
    for(const auto& state_jumps : gdfa.jumps){
        for(const auto& j : state_jumps){
            Category cat = j.first;
            auto     sa  = j.second;
//             Symbol sym = j.first;
            printf("delta(%zu, ", current_state);
            print_category(cat);
            printf(") = %zu with action having index %zu\n", sa.st, sa.action_idx);
        }
        current_state++;
    }
}