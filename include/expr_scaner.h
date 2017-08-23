/*
    File:    expr_scaner.h
    Created: 13 December 2015 at 09:05 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef EXPR_SCANER_H
#define EXPR_SCANER_H

#include <string>
#include <memory>
#include <set>
#include "../include/location.h"
#include "../include/error_count.h"
#include "../include/trie.h"
#include "../include/trie_for_set.h"
#include "../include/expr_lexem_info.h"
#include "../include/aux_expr_scaner.h"
#include "../include/aux_expr_lexem.h"
#include "../include/errors_and_tries.h"

class Expr_scaner{
public:
    Expr_scaner()                        = default;
    Expr_scaner(const Location_ptr&              location,
                const Errors_and_tries&          et,
                const Trie_for_set_of_char32ptr& trie_for_complement_of_set) :
        compl_set_trie(trie_for_complement_of_set),
        aux_scaner(std::make_unique<Aux_expr_scaner>(location, et)),
        et_(et), loc(location)
        {}
    Expr_scaner(const Expr_scaner& orig) = default;
    ~Expr_scaner()                       = default;

    Expr_lexem_info current_lexem();
    size_t          lexem_begin_line_number() const;
    void            back();
private:
    Trie_for_set_of_char32ptr compl_set_trie;
    Aux_expr_scaner_ptr       aux_scaner;
    Errors_and_tries          et_;
    Location_ptr              loc;

    size_t                    lexem_begin_line;
    char32_t*                 lexem_begin;

    enum class State{
        Begin_class_complement, First_char,
        Body_chars,             End_class_complement
    };

/*
 * The lexeme 'character class complement' can be descripted as the following regular
 * expression:
 *          ab+c
 * where
 *      a is the lexeme 'Begin_char_class_complement',
 *      b is the lexeme 'Character' or 'Character class',
 *      c is the lexeme 'End_char_class_complement'.
 *
 * If we construct a non-deterministic finite automaton by this regexp, next we build
 * a corresponding deterministic finite automaton, and, finally, we minimize the
 * deterministic automaton, then we obtain a finite automaton with the following
 * transition table:
 *
 * |-------|---|---|---|--------------|
 * | State | a | b | c |    Remark    |
 * |-------|---|---|---|--------------|
 * |   A   | B |   |   | Begin state. |
 * |-------|---|---|---|--------------|
 * |   B   |   | C |   |              |
 * |-------|---|---|---|--------------|
 * |   C   |   | C | E |              |
 * |-------|---|---|---|--------------|
 * |   E   |   |   |   | End state.   |
 * |-------|---|---|---|--------------|
 *
 * But for ease of writing, we need to introduce more meaningful names for states of
 * a finite automaton. The following table shows the matching state names from the
 * previous table and meaningful names. Meaningful names are collected in the enumeration
 * State.
 *
 * |---|------------------------|
 * |   |    Meaningful name     |
 * |---|------------------------|
 * | A | Begin_class_complement |
 * |---|------------------------|
 * | B | First_char             |
 * |---|------------------------|
 * | C | Body_chars             |
 * |---|------------------------|
 * | E | End_class_complement   |
 * |---|------------------------|
 *
 */

    State               state;
    size_t              set_idx = 0;

    Aux_expr_lexem_info aeli;
    Aux_expr_lexem_code aelic;

    size_t get_set_complement();

    using State_proc = void (Expr_scaner::*)();


    std::set<char32_t>  curr_set;

    static State_proc procs[];

    void begin_class_complement_proc(); void first_char_proc();
    void body_chars_proc();             void end_class_complement_proc();

    Expr_lexem_info convert_lexeme(Aux_expr_lexem_info);
};

using Expr_scaner_ptr = std::shared_ptr<Expr_scaner>;
#endif