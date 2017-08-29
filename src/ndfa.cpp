/*
    File:    ndfa.cpp
    Created: 28 August 2017 at 14:22 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include "../include/ndfa.h"
#include "../include/operations_with_sets.h"

static const Symbol eps = {.kind = Symbol_kind::Epsilon};

using namespace operations_with_sets;

using NDFA_builder = void (*)(NDFA&, const Command_buffer&, size_t, size_t);

//     Or,                    Concat,  Kleene,     Positive,
//     Optional,              Char,    Char_class, Unknown,
//     Char_class_complement, Multior, Multiconcat

static void or_builder(NDFA&  a,           const  Command_buffer& commands,
                       size_t command_nom, size_t first_state_nom);

static void concat_builder(NDFA&  a,           const  Command_buffer& commands,
                           size_t command_nom, size_t first_state_nom);

static void kleene_builder(NDFA&  a,           const  Command_buffer& commands,
                           size_t command_nom, size_t first_state_nom);

static void positive_builder(NDFA&  a,           const  Command_buffer& commands,
                             size_t command_nom, size_t first_state_nom);

static void optional_builder(NDFA&  a,           const  Command_buffer& commands,
                             size_t command_nom, size_t first_state_nom);

static void leaf_builder(NDFA&  a,           const  Command_buffer& commands,
                         size_t command_nom, size_t first_state_nom);

static void unknown_builder(NDFA&  a,           const  Command_buffer& commands,
                            size_t command_nom, size_t first_state_nom);

static void multior_builder(NDFA&  a,           const  Command_buffer& commands,
                            size_t command_nom, size_t first_state_nom);

static void multiconcat_builder(NDFA&  a,           const  Command_buffer& commands,
                                size_t command_nom, size_t first_state_nom);

static const NDFA_builder ndfa_builders[] = {
    or_builder,               concat_builder,  kleene_builder,     positive_builder,
    optional_builder,         leaf_builder,    leaf_builder, unknown_builder,
    leaf_builder, multior_builder, multiconcat_builder
};

/* This function builds a nondeterministic finite automaton a for a command whose
 * index in the list of commands is specified by the command_nom parameter.
 * The smallest of the state numbers of the machine being built is indicated by
 * the parameter first_state_nom. */
static void generate_NDFA_for_command(NDFA&  a,       const  Command_buffer& commands,
                                      size_t cmd_nom, size_t first_state_nom)
{
    auto& com = commands[cmd_nom];
    (ndfa_builders[static_cast<size_t>(com.kind)])(a, commands, cmd_nom, first_state_nom);
}

void build_NDFA(NDFA& a, const Command_buffer& commands)
{
    if(!commands.empty()){
        size_t last_command_index = commands.size() - 1;
        generate_NDFA_for_command(a, commands, last_command_index, 0);
    }
}

static constexpr size_t join_actions(const States_with_action& sa1,
                                     const States_with_action& sa2)
{
    return sa1.second ? sa1.second : sa2.second;
}

static void add_state_jumps(NDFA_state_jumps& sj,     Symbol c,
                            const States_with_action& added_states)
{
    auto it = sj.find(c);
    if(it != sj.end()){
        auto   sa     = it -> second;
        size_t action = join_actions(sa.second, added_states.second);
        sj[c] = std::make_pair(added_states.first + sa.first, action);
    }else{
        sj[c] = added_states;
    }
}

static void or_builder(NDFA&  a,           const  Command_buffer& commands,
                       size_t command_nom, size_t first_state_nom)
{
    auto& com        = commands[command_nom];
    NDFA a1, a2;
    NDFA_state_jumps state_jumps;
    generate_NDFA_for_command(a1, commands, com.args.first, first_state_nom + 1);
    generate_NDFA_for_command(a2, commands, com.args.second, a1.final_state + 1);
    Set_of_states s {a1.begin_state, a2.begin_state};
    state_jumps[eps] = std::make_pair(s, 0);

    /* Next, we glue a1 and a2 with the addition of states to a. */
    size_t final_st  = a2.final_state + 1;
    auto last_state = std::make_pair(single_elem(final_st),0);
    add_state_jumps(a1.jumps.back(), eps, last_state);
    add_state_jumps(a2.jumps.back(), eps, last_state);
    a.jumps.push_back(state_jumps);
    a.jumps.insert(a.jumps.end(), a1.jumps.begin(), a1.jumps.end());
    a.jumps.insert(a.jumps.end(), a2.jumps.begin(), a2.jumps.end());
    a.jumps.push_back(NDFA_state_jumps());
    a.begin_state = first_state_nom;
    a.final_state = final_st;
}

static void concat_builder(NDFA&  a,           const  Command_buffer& commands,
                           size_t command_nom, size_t first_state_nom)
{
    auto& com = commands[command_nom];
    NDFA a1, a2;
    NDFA_state_jumps state_jumps1, state_jumps2;
    generate_NDFA_for_command(a1, commands, com.args.first, first_state_nom);
    generate_NDFA_for_command(a2, commands, com.args.second, a1.final_state);
    /* Next, we glue a1 and a2 with the addition of states to a. */
    /* Let us glue the begin state of the automaton a2 with the final
     * state of the automaton a1. */
    state_jumps1 = a1.jumps.back();
    state_jumps2 = a2.jumps[0];
    for(auto sj : state_jumps2){
        add_state_jumps(state_jumps1, sj.first, sj.second);
    }
    /* Then we add the states of the automaton a1 (except the final one). */
    a.jumps.insert(a.jumps.end(), a1.jumps.begin(), a1.jumps.end() - 1);
    /* Add the glued state. */
    a.jumps.push_back(state_jumps1);
    /* Finally, we add other states of the automaton a2. */
    a.jumps.insert(a.jumps.end(), a2.jumps.begin() + 1, a2.jumps.end());
    a.begin_state = first_state_nom;
    a.final_state = a2.final_state;
}

static void kleene_builder(NDFA&  a,           const  Command_buffer& commands,
                           size_t command_nom, size_t first_state_nom)
{
    auto& com = commands[command_nom];
    NDFA              a_without_clos;
    NDFA_state_jumps  state_jumps;

    generate_NDFA_for_command(a_without_clos, commands, com.args.first,
                              first_state_nom + 1);

    size_t             final_st = a_without_clos.final_state + 1;
    Set_of_states s {a_without_clos.begin_state, final_st};
    States_with_action temp_jumps = std::make_pair(s,0);

    add_state_jumps(a_without_clos.jumps.back(), eps, temp_jumps);

    state_jumps[eps] = temp_jumps;

    a.jumps.push_back(state_jumps);
    a.jumps.insert(a.jumps.end(), a_without_clos.jumps.begin(),
                   a_without_clos.jumps.end());
    a.jumps.push_back(NDFA_state_jumps());
    a.begin_state = first_state_nom;
    a.final_state = final_st;
}

static void positive_builder(NDFA&  a,           const  Command_buffer& commands,
                             size_t command_nom, size_t first_state_nom)
{
    auto& com = commands[command_nom];
    NDFA a1, a2;
    NDFA_state_jumps state_jumps;
    generate_NDFA_for_command(a1, commands, com.args.first, first_state_nom);
    a2 = a1;
    size_t number_of_states_in_a1 = a1.jumps.size();

    /* The cycle below increases the number of states for the second copy of the
     * automaton for the expression under the positive closure sign. */
    /* The outer cycle is a cycle over the states of the automaton a2. */
    for(auto& state_jmps : a2.jumps){
        /* The following loop is the loop on transitions of the current state. */
        for(auto& jump : state_jmps){
            Symbol             c  = jump.first;
            States_with_action sa = jump.second;
            Set_of_states      new_set_of_states;
            for(const auto s : sa.first){
                new_set_of_states.insert(s + number_of_states_in_a1);
            }
            state_jmps[c]         = std::make_pair(new_set_of_states, sa.second);
        }
    }
    a2.begin_state += number_of_states_in_a1;
    a2.final_state += number_of_states_in_a1;

    size_t final_st = a2.final_state + 1;

    Set_of_states s {a2.begin_state, final_st};
    States_with_action temp_jumps = std::make_pair(s,0);

    add_state_jumps(a1.jumps.back(), eps, temp_jumps);
    add_state_jumps(a2.jumps.back(), eps, temp_jumps);

    a.jumps.insert(a.jumps.end(), a1.jumps.begin(), a1.jumps.end());
    a.jumps.insert(a.jumps.end(), a2.jumps.begin(), a2.jumps.end());
    a.jumps.push_back(NDFA_state_jumps());
    a.begin_state = first_state_nom;
    a.final_state = final_st;
}

static void optional_builder(NDFA&  a,           const  Command_buffer& commands,
                             size_t command_nom, size_t first_state_nom)
{
    auto& com = commands[command_nom];
    NDFA a_without_opt;
    generate_NDFA_for_command(a_without_opt, commands, com.args.first, first_state_nom + 1);

    NDFA_state_jumps begin_st, begin_st_for_eps, end_st_for_eps;

    size_t final_st       = a_without_opt.final_state + 3;
    Set_of_states s {first_state_nom + 1, a_without_opt.final_state + 1};
    begin_st[eps]         = std::make_pair(s, 0);
    begin_st_for_eps[eps] = std::make_pair(single_elem(a_without_opt.final_state + 2), 0);
    end_st_for_eps[eps]   = std::make_pair(single_elem(final_st), 0);
    add_state_jumps(a_without_opt.jumps.back(), eps,
                    std::make_pair(single_elem(final_st),0));

    a.jumps.push_back(begin_st);
    a.jumps.insert(a.jumps.end(), a_without_opt.jumps.begin(), a_without_opt.jumps.end());
    a.jumps.push_back(begin_st_for_eps);
    a.jumps.push_back(end_st_for_eps);
    a.jumps.push_back(NDFA_state_jumps());

    a.begin_state = first_state_nom;
    a.final_state = final_st;
}

static Symbol command2symbol(const Command com)
{
    Symbol result;
    switch(com.name){
        case Command_name::Char:
            result.kind       = Symbol_kind::Char;
            result.c          = com.c;
            break;
        case Command_name::Char_class:
            result.kind       = Symbol_kind::Char_class;
            result.idx_of_set = com.idx_of_set;
            break;
        case Command_name::Char_class_complement:
            result.kind       = Symbol_kind::Class_complement;
            result.idx_of_set = com.idx_of_set;
            break;
        default:
            ;
    }
    return result;
}

static void leaf_builder(NDFA&  a,           const  Command_buffer& commands,
                         size_t command_nom, size_t first_state_nom)
{
    auto& com   = commands[command_nom];
    NDFA_state_jumps begin_st;
    Symbol symb = command2symbol(com);
    begin_st[symb] =
        std::make_pair(single_elem(first_state_nom + 1), com.action_name);
    a.jumps.push_back(begin_st);
    a.jumps.push_back(NDFA_state_jumps());

    a.begin_state = first_state_nom;
    a.final_state = first_state_nom + 1;
}

static void unknown_builder(NDFA&  a,           const  Command_buffer& commands,
                            size_t command_nom, size_t first_state_nom)
{}

static void multior_builder(NDFA&  a,           const  Command_buffer& commands,
                            size_t command_nom, size_t first_state_nom)
{
    auto&  com               = commands[command_nom];
    size_t fst               = com.args.first;
    size_t snd               = com.args.second;
    size_t num_of_commands   = snd - fst + 1;
    auto   jmps              = NDFA_jumps(2 * num_of_commands + 2);
//     auto&  com           = commands[command_nom];
//     auto   s             = com.s;
//     size_t num_of_chars  = s.size();
//     size_t dnum_of_chars = 2 * num_of_chars;
//     auto   jumps         = std::vector<NDFA_state_jumps>(dnum_of_chars + 2);
//     size_t y             = first_state_nom + num_of_chars;
//     size_t last_state    = first_state_nom + 2 * num_of_chars + 1;
//     auto   last_sa       = std::make_pair(single_elem(last_state), 0);
//     auto   sts           = range(first_state_nom + 1, first_state_nom + num_of_chars);
//     auto&  j0            = jumps[0];
//     j0[eps_gc]           = std::make_pair(sts, 0);
//
//     size_t i = 1;
//
//     NDFA_state_jumps last_jump;
//     last_jump[eps_gc]    = last_sa;
//
//     for(const auto c : s){
//         Generalized_char gc;
//         gc.kind                 = Char;
//         gc.c                    = c;
//         NDFA_state_jumps t;
//         t[gc]                   = std::make_pair(single_elem(y + i), com.action_name);
//         jumps[i]                = t;
//         jumps[num_of_chars + i] = last_jump;
//         i++;
//     }
//
//     a.jumps = jumps;
//     a.begin_state = first_state_nom;
//     a.final_state = last_state;
}

static void multiconcat_builder(NDFA&  a,           const  Command_buffer& commands,
                                size_t command_nom, size_t first_state_nom)
{
    NDFA   accumulator;
    auto&  com               = commands[command_nom];
    size_t fst               = com.args.first;
    size_t snd               = com.args.second;
    size_t num_of_commands   = snd - fst + 1;
    auto   jmps              = NDFA_jumps(num_of_commands + 1);
    accumulator.begin_state  = first_state_nom;
    accumulator.final_state  = first_state_nom + num_of_commands;
    size_t current_last_state = first_state_nom + 1;
    for(size_t i = 0; i < num_of_commands; ++i, ++current_last_state){
        NDFA_state_jumps j;
        auto& cmd  = commands[fst + i];
        auto  symb = command2symbol(cmd);
        j[symb]    = std::make_pair(single_elem(current_last_state), cmd.action_name);
        jmps[i]    = j;
    }
    accumulator.jumps        = jmps;
    a                        = accumulator;
}