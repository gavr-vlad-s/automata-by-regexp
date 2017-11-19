/*
    File:    match.h
    Created: 18 November 2017 at 15:37 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef MATCH_H
#define MATCH_H
#include <string>
#include <vector>
#include <algorithm>
#include "../include/groupped_dfa.h"

/**
 * \param [in] gdfa  a deterministic finite automaton with grouped transitions
 * \param [in] str   matched string
 * \result
 *      true, if str matches the regular expression on which the deterministic
 *            finite automaton was built
 *      else, otherwise
 */
bool match(const G_DFA& gdfa, const std::u32string& str);

/**
 * \param [in] gdfa  a deterministic finite automaton with grouped transitions
 * \param [in] strs  matched strings
 * \result
 *      true, if any string of strs matches the regular expression on
 *            which the deterministic finite automaton was built
 *      else, otherwise
 */
template<class InputIt>
bool match_all(const G_DFA& gdfa, InputIt first, InputIt last)
{
    return std::all_of(first, last,
                       [&gdfa](const std::u32string& s){return match(gdfa, s);});
}

#endif