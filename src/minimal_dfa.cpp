/*
    File:    minimal_dfa.cpp
    Created: 30 September 2017 at 10:44 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <algorithm>
#include <cstdio>
#include <list>
#include "../include/minimal_dfa.h"
#include "../include/operations_with_sets.h"
#include "../include/trie_for_set.h"
#include "../include/print_size_t.h"

using namespace operations_with_sets;

/* The following is an implementation of the function that builds a minimized DFA,
 * as well as the auxiliary functions and data types necessary for this. */

using Partition_as_vector = std::vector<size_t>;
/* This type describes the partition of the set of states of DFA into groups of states.
 * Namely, the value of the element with the index i is the number of the group to
 * which the state with the number i belongs. The group numbers are assumed to be
 * non-negative integers, and the group of states with the number 0 is fictitious, in
 * the following sense: if from the state i there is no transition over some symbol gc,
 * then we assume that there is a transition from i over the symbol gc to this
 * fictitious group. */

using Partition_as_sets = std::list<std::set<size_t>>;
/* This type also describes the partitioning of the set of states of DFA into groups of
 * states, but describes somewhat differently. Namely, the element with the index i is
 * a group of states with the number i + 1. In this case, the fictitious group with the
 * number 0 is not included in this list of groups. */

/* Using the DFA transition table, this function constructs the same table,
 * but in a different format. */
static Min_DFA_jumps  convert_jumps(const DFA& a)
{
    Min_DFA_jumps conv_j = std::vector<Min_DFA_state_jumps>(a.number_of_states);
    for(const auto x : a.jumps){
        auto state_and_symb = x.first;
        auto target         = x.second;
        conv_j[state_and_symb.first][state_and_symb.second] = target;
    }
    return conv_j;
}

/* The build_initial_partition function builds an initial partition of states of
 * the DFA a: into states that are finite, and states that are not finite. */
static Partition_as_sets build_initial_partition(const DFA& a)
{
    Partition_as_sets initial;
    auto             final_states_group = a.final_states;
    std::set<size_t> non_final_states_group;
    size_t           num_of_states      = a.number_of_states;
    for(size_t st = 0; st < num_of_states; st++){
        if(!is_elem(st, final_states_group)){
            non_final_states_group.insert(st);
        }
    }
    initial.push_back(final_states_group);
    if(!non_final_states_group.empty()){
        initial.push_back(non_final_states_group);
    }
    return initial;
}

// // void test_initial_partition(const DFA& a){
// //     puts("Initial partition of DFA states:");
// //     auto init_partition = build_initial_partition(a);
// //     for(const auto& g: init_partition){
// //         print_set(g, print_size_t);
// //     }
// //     putchar('\n');
// // }

static Partition_as_vector convert_partition_form(const Partition_as_sets& ps,
                                                  const DFA& a)
{
    Partition_as_vector pv = std::vector<size_t>(a.number_of_states);
    size_t i = 1;
    for(const auto& s : ps){
        for(size_t x : s){
            pv[x] = i;
        }
        i++;
    }
    return pv;
}

using Group = std::set<size_t>;

/* The next function over the set s of DFA states with a jump table j constructs
 * a set of symbols from which at least one of the states belonging to the set s
 * has a transition. */
static std::set<Symbol> jump_chars_for_group(const Min_DFA_jumps& j, const Group& s)
{
    std::set<Symbol> jump_chars;
    for(size_t st : s){
        auto& st_jumps = j[st];
        for(auto m : st_jumps){
            /* This cycle is over all transitions for the state st of
             * the deterministic finite automaton */
            Symbol gc = m.first;
            jump_chars.insert(gc);
        }
    }
    return jump_chars;
}

// void print_jump_chars_for_group(const Min_DFA_jumps& j, const Group& s){
//     auto jc = jump_chars_for_group(j, s);
//     print_set(jc, print_generalized_char);
// }

using State_and_group_numb = std::pair<size_t, size_t>;
/* The first element of the given pair is a state, and the second is the
 * number of the group of states into which the given state passes */

using States_classification = std::vector<State_and_group_numb>;

/* The following function classifies the states that are in the group 'group'.
 * Namely, a vector is returned consisting of pairs of the form
 *    (state; the number of the group of states into which the state passes through gc)
 * In this case, all the first elements of these pairs belong to the group 'group'.
 */
States_classification group_states_classificate(const Partition_as_vector& pv,
                                                const Min_DFA_jumps&       j,
                                                const Group&               group,
                                                const Symbol               gc)
{
    States_classification sc;
    for(size_t state : group){
        auto& state_jumps = j[state]; // We get a map in which state transitions are stored,
                                      // the number of which is contained in the variable
                                      // state, and the keys are of type Symbol.
        auto it = state_jumps.find(gc);
        if(it != state_jumps.end()){
          /* If there is a transition on the symbol gc, then we write down the pair
           * (state; the number of the group of states into which the state passes through gc)
           */
            sc.push_back(std::make_pair(state, pv[(it->second).st]));
        }else{
            /* Otherwise we assume that the transition to gc is performed in the fictitious
             * group with the number 0. */
            sc.push_back(std::make_pair(state, 0));
        }
    }
    /* Now we sort the given vector by the second component of each pair, i.e. by the number
     * of the group into which the state passes through gc */
    sort(sc.begin(), sc.end(),
        [](const State_and_group_numb& sg1, const State_and_group_numb& sg2){
            return sg1.second < sg2.second;
        });
    return sc;
}

static Partition_as_sets split_group_by_gc(const Partition_as_vector& pv,
                                           const Min_DFA_jumps&       j,
                                           const Group&               group,
                                           const Symbol gc)
{
    Partition_as_sets partition;
    auto states_classification = group_states_classificate(pv, j, group, gc);

    /* And now we actually build a partition of the group 'group',
     * using the sorted vector of pairs */
    std::pair<size_t, size_t> current_elem  = states_classification[0];
    Group                     current_group = {current_elem.first};
    for(const auto p : states_classification){
        if(current_elem.second == p.second){
            current_group.insert(p.first);
        }else{
            partition.push_back(current_group);
            current_group = {p.first};
        }
        current_elem = p;
    }
    partition.push_back(current_group);
    return partition;
}

static Partition_as_sets split_group_partition_by_gc(const Partition_as_vector& pv,
                                                     const Min_DFA_jumps&       j,
                                                     const Partition_as_sets&   group_partition,
                                                     const Symbol               gc)
{
    Partition_as_sets partition;
    for(const auto& g : group_partition){
        auto splitted_g = split_group_by_gc(pv, j, g, gc);
        partition.insert(partition.end(), splitted_g.begin(), splitted_g.end());
    }
    return partition;
}

static Partition_as_sets split_group(const Partition_as_vector& pv, const Min_DFA_jumps& j,
                                     const Group& group)
{
    /* For group of states 'group' we get symbols and character classes, on which
     * transitions from the states of this group are possible */
    auto jump_chars = jump_chars_for_group(j, group);
    Partition_as_sets partition_of_group = {group};
    /* Next, for each possible transition symbol, let us find out whether
     * any states are distinguishable by this symbol. */
    for(auto gc : jump_chars){
        partition_of_group = split_group_partition_by_gc(pv, j, partition_of_group, gc);
    }
    return partition_of_group;
}

static Partition_as_sets split_partition(const Partition_as_sets& old, const Min_DFA_jumps& j,
                                         const DFA& a)
{
    Partition_as_sets   result;
    Partition_as_vector pv = convert_partition_form(old, a);
    for(const auto& g :old){
        auto splitted_g = split_group(pv, j, g);
        result.insert(result.end(), splitted_g.begin(), splitted_g.end());
    }
    return result;
}

static Partition_as_sets split_states_into_equivalence_classes(const Min_DFA_jumps& j,
                                                               const DFA& a)
{
    Partition_as_sets old_partition, new_partition;
    new_partition = build_initial_partition(a);
    while(old_partition != new_partition){
        old_partition = new_partition;
        new_partition = split_partition(old_partition, j, a);
    }
    return new_partition;
}

static size_t action_for_group(const Min_DFA_jumps& j, const Group& g,
                               const Symbol& gc)
{
    for(const size_t state : g){
        auto& state_j = j[state];
        auto  it      = state_j.find(gc);
        if(it != state_j.end()){
            auto target = it->second;
            size_t act  = target.action_idx;
            if(act){
                return act;
            }
        }
    }
    return 0;
}

static Min_DFA_jumps minimal_DFA_jumps(const Partition_as_sets& equivalence_classes,
                                       const Min_DFA_jumps&     j,
                                       const DFA&               a)
{
    Min_DFA_jumps result;
    auto eq_classes_as_vector = convert_partition_form(equivalence_classes, a);
    for(const auto& eq_class : equivalence_classes){
        size_t representative = *(eq_class.begin()); // Chose the representative of
                                                     // the current equivalence
                                                     // class of states
        /* After gluing the states in the same equivalence class into one state of the
         * new automaton, it is sufficient to consider only characters for the transition
         * characters from the state of representation. */
        auto&  representative_jumps = j[representative];
        Min_DFA_state_jumps current_jumps;
        for(const auto& x : representative_jumps){
            auto   jump_char  = x.first;
            auto   target     = x.second;
            target.action_idx = action_for_group(j, eq_class, jump_char);
            target.st         = eq_classes_as_vector[target.st] - 1;
            current_jumps[jump_char] = target;
        }
        result.push_back(current_jumps);
    }
    return result;
}

using Permutation = std::vector<size_t>;
static Permutation build_state_permutation(const Min_DFA_jumps& j, size_t begin_state)
{
    size_t        state_idx        = 1;
    size_t        number_of_states = j.size();
    auto          permutation      = Permutation(number_of_states);
    permutation[begin_state]       = 0;
    for(size_t i = 0; i < number_of_states; i++){
        if(i != begin_state){
            permutation[i] = state_idx++;
        }
    }
    return permutation;
}

/* According to the transition table of a DFA with a minimum number of states,
 * a transition table is constructed in which the initial state has the number 0. */
static Min_DFA_jumps reorder_states_in_jt(const Min_DFA_jumps& j, const Permutation& p)
{
    Min_DFA_jumps result;
    size_t        number_of_states = j.size();
    size_t        state_idx = 0;
    result    = std::vector<Min_DFA_state_jumps>(number_of_states);
    for(const auto& m : j){
        Min_DFA_state_jumps new_jumps_for_state;
        for(const auto x : m){
            auto jump_char                 = x.first;
            auto target                    = x.second;
            target.st                      = p[target.st];
            new_jumps_for_state[jump_char] = target;
        }
        result[p[state_idx]] = new_jumps_for_state;
        state_idx++;
    }
    return result;
}

static Min_DFA reordered_DFA(const Min_DFA& source)
{
    Min_DFA result;
    auto& sj          = source.jumps;
    auto  permutation = build_state_permutation(sj, source.begin_state);
    auto  new_jumps   = reorder_states_in_jt(sj, permutation);
    auto& sf          = source.final_states;

    std::set<size_t> fs;
    for(const size_t s : sf){
        fs.insert(permutation[s]);
    }

    result.jumps        = new_jumps;
    result.begin_state  = 0;
    result.final_states = fs;
    return result;
}

void minimize_DFA(Min_DFA& minimal, const DFA& source)
{
    auto sj                   = convert_jumps(source);
    auto equivalence_classes  = split_states_into_equivalence_classes(sj, source);
    auto minimal_jumps        = minimal_DFA_jumps(equivalence_classes, sj, source);
    auto eq_classes_as_vector = convert_partition_form(equivalence_classes, source);

    /* We determine which states are final states, and which
     * state is the initial state */
    size_t       mb = source.begin_state;
    auto         mf = source.final_states;
    size_t       bs = 0;
    decltype(mf) fs;
    for(auto g : equivalence_classes){
        if(is_elem(mb, g)){
            bs = eq_classes_as_vector[mb] - 1;
        }
        auto temp = g * mf;
        if(!temp.empty()){
            size_t s = eq_classes_as_vector[*(g.begin())] - 1;
            fs.insert(s);
        }
    }
    minimal.jumps        = minimal_jumps;
    minimal.begin_state  = bs;
    minimal.final_states = fs;
    minimal              = reordered_DFA(minimal);
}

void print_minimal_DFA(const Min_DFA& a, const Trie_for_set_of_char32ptr& tr)
{
    printf("Number of states of minimized DFA: %zu.\n", a.jumps.size());
    printf("Begin state of minimized DFA:      %zu.\n", a.begin_state);

    printf("Final states of minimized DFA: ");
    print_set(a.final_states, print_size_t);
    putchar('\n');

    size_t current_state = 0;
    for(const auto& state_jumps : a.jumps){
        for(const auto& j : state_jumps){
            Symbol sym = j.first;
            auto   sa  = j.second;
            printf("delta(%zu, ", current_state);
            print_symbol(sym, tr);
            printf(") = %zu with action having index %zu\n", sa.st, sa.action_idx);
        }
        current_state++;
    }
}