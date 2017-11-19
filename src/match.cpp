/*
    File:    match.cpp
    Created: 18 November 2017 at 15:51 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#define DEBUG_MODE

#include <utility>
#include <algorithm>

#ifdef DEBUG_MODE
#    include <cstdio>
#    include "../include/char_conv.h"
#endif

#include "../include/match.h"
#include "../include/categories.h"
#include "../include/operations_with_sets.h"

static bool is_category_elem(char32_t c, const Category& cat)
{
#ifdef DEBUG_MODE
    printf("Current character: %s\n", char32_to_utf8(c).c_str());
    printf("Current category: ");
    print_category(cat);
    putchar('\n');
#endif
    bool ret_val = false;
    switch(cat.kind){
        case Category_kind::All_chars:
            ret_val = true;
            break;
        case Category_kind::Set_of_cs:
            ret_val = operations_with_sets::is_elem(c, cat.s);
            break;
        case Category_kind::Set_of_cs_complement:
            ret_val = !operations_with_sets::is_elem(c, cat.s);
            break;
        default:
            ;
    }
#ifdef DEBUG_MODE
    printf("returned value: %s\n", ret_val ? "true" : "false");
#endif
    return ret_val;
}

static std::pair<bool, size_t> next_state(char32_t c, const G_DFA_state_jumps& js)
{
    for(const auto& j : js){
        auto& cat    = j.first;
        auto  target = j.second;
        if(is_category_elem(c, cat)){
            return {true, target.st};
        }
    }
    return {false, 0};
}

/**
 * \param [in] gdfa  a deterministic finite automaton with grouped transitions
 * \param [in] str   matched string
 * \result
 *      true, if str matches the regular expression on which the deterministic
 *            finite automaton was built
 *      false, otherwise
 */
bool match(const G_DFA& gdfa, const std::u32string& str)
{
#ifdef DEBUG_MODE
    printf("**********************************************\n");
    printf("Matched string: '%s'\n", u32string_to_utf8(str).c_str());
#endif
    using operations_with_sets::is_elem;
    bool   ret_val      = false;

    size_t begin_state  = gdfa.begin_state;
    auto&  final_states = gdfa.final_states;
    auto&  jumps        = gdfa.jumps;

    size_t current_state = begin_state;
    for(const char32_t c : str){
        auto& js = jumps[current_state];
        auto  p  = next_state(c, js);
        if(!p.first){break;}
        current_state = p.second;
    }
    ret_val = is_elem(current_state, final_states);
    return ret_val;
}

/**
 * \param [in] gdfa  a deterministic finite automaton with grouped transitions
 * \param [in] strs  matched strings
 * \result
 *      true, if any string of the vector strs matches the regular expression on
 *            which the deterministic finite automaton was built
 *      false, otherwise
 */
bool match_all(const G_DFA& gdfa, const std::vector<std::u32string>& strs)
{
    return std::all_of(strs.begin(), strs.end(),
                       [&gdfa](const std::u32string& s){return match(gdfa, s);});
}
