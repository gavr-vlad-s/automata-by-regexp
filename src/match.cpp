/*
    File:    match.cpp
    Created: 18 November 2017 at 15:51 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <utility>
#include "../include/match.h"
#include "../include/categories.h"
#include "../include/operations_with_sets.h"

bool is_category_elem(char32_t c, const Category& cat)
{
    bool ret_val;
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
    }
    return ret_val;
}

std::pair<bool, size_t> next_state(char32_t c, const G_DFA_state_jumps& js)
{
    std::pair<bool, size_t> ret_val = {false, 0};
    for(const auto& j : js){
        auto& cat = j.first;
        if(is_category_elem(c, cat)){
            ret_val.first  = true;
            ret_val.second = j.second.st;
            break;
        }
    }
    return ret_val;
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
    using operations_with_sets::is_elem;
    bool   ret_val      = true;

    size_t begin_state  = gdfa.begin_state;
    auto&  final_states = gdfa.final_states;
    auto&  jumps        = gdfa.jumps;

    size_t current_state = begin_state;
    for(const char32_t c : str){
        auto& js = jumps[current_state];
        auto  p  = next_state(c, js);
        if(p.first){
            current_state = p.second;
        }else{
            ret_val = is_elem(current_state, final_states);
            break;
        }
    }
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
    bool ret_val = true;
    for(const auto& s : strs){
        ret_val = match(gdfa, s);
        if(!ret_val){
            break;
        }
    }
    return ret_val;
}
