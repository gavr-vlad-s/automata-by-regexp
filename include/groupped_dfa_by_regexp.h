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
/**
 * \param [out] gdfa         resulting deterministic finite automaton
 *                           with groupped transitions
 *
 * \param [in]  buf          buffer containing commands
 * \param [in]  optimize_com if this argument is true, then adjacent commands Or,
 *                           the arguments of which are either Multior, or Char_def,
 *                           are glued together in single command.
 *                           Otherwise, gluing is not performed
 */
void grouped_DFA_by_regexp(G_DFA& gdfa, const Command_buffer& buf,
                           bool optimize_com);
#endif