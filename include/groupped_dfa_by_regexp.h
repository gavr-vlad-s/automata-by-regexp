/*
    File:    grouped_DFA_by_regexp.h
    Created: 29 October 2017 at 09:20 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef GROUPED_DFA_BY_REGEXP_H
#define GROUPED_DFA_BY_REGEXP_H
#include "../include/command.h"
#include "../include/groupped_dfa.h"
#include "../include/trie_for_set.h"
/**
 * \param [out] gdfa         resulting deterministic finite automaton
 *                           with groupped transitions
 *
 * \param [in]  buf          buffer containing commands
 *
 * \param [in]  t            trie stores sets having type std::set< char32_t >
 */
void grouped_DFA_by_regexp(G_DFA&                           gdfa,
                           const Command_buffer&            buf,
                           const Trie_for_set_of_char32ptr& t);
#endif