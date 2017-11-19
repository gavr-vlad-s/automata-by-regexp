/*
    File:    match.cpp
    Created: 18 November 2017 at 15:51 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

// #define DEBUG_MODE

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

    size_t begin_state  = gdfa.begin_state;
    auto&  final_states = gdfa.final_states;
    auto&  jumps        = gdfa.jumps;

    size_t current_state = begin_state;

    size_t i             = 0;
    size_t len           = str.size();
    for( ; i < len; ++i){
        char32_t c = str[i];
        auto& js   = jumps[current_state];
        auto  pred = [c](auto& a)->bool{return is_category_elem(c, a.first);};
        auto  it   = std::find_if(js.begin(), js.end(), pred);
        if(it != js.end()){
            current_state = (it->second).st;
        }else{
            if(i + 1 < len){return false;}
        }
    }
    return is_elem(current_state, final_states);
}

// /**
//  * \param [in] gdfa  a deterministic finite automaton with grouped transitions
//  * \param [in] strs  matched strings
//  * \result
//  *      true, if any string of the vector strs matches the regular expression on
//  *            which the deterministic finite automaton was built
//  *      false, otherwise
//  */
// bool match_all(const G_DFA& gdfa, const std::vector<std::u32string>& strs)
// {
//     return std::all_of(strs.begin(), strs.end(),
//                        [&gdfa](const std::u32string& s){return match(gdfa, s);});
// }