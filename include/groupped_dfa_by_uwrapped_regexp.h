/*
    File:    grouped_DFA_by_uwrapped_regexp.h
    Created: 2 February 2017 at 10:14 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef GROUPED_DFA_BY_UWRAPPED_REGEXP_H
#define GROUPED_DFA_BY_UWRAPPED_REGEXP_H
#include "../include/unwrapped_command.h"
#include "../include/groupped_dfa.h"
/**
 * \param [out] gdfa         resulting deterministic finite automaton
 *                           with groupped transitions
 *
 * \param [in]  buf          buffer containing unwrapped commands
 * \param [in]  optimize_com if this argument is true, then adjacent commands Or,
 *                           the arguments of which are either Multior, or Char_def,
 *                           are glued together in single command.
 *                           Otherwise, gluing is not performed
 */
void grouped_DFA_by_uwrapped_regexp(G_DFA& gdfa, const Unwrapped_commands& buf,
                                    bool optimize_com);
#endif