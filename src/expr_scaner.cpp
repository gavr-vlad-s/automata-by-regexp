/*
    File:    expr_scaner.cpp
    Created: 13 December 2015 at 09:05 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include "../include/expr_scaner.h"
#include "../include/aux_expr_lexem.h"
#include "../include/belongs.h"
#include "../include/sets_for_classes.h"
#include <cstdlib>
#include <cstdio>

template<typename T>
bool is_in_segment(T value, T lower, T upper)
{
    return (lower <= value) && (value <= upper);
}

inline uint64_t belongs(Aux_expr_lexem_code e, uint64_t s)
{
    return belongs(static_cast<uint64_t>(e), s);
}

static const std::set<char32_t> single_quote = {U'\''};
static const std::set<char32_t> double_quote = {U'\"'};

static const uint64_t classes_of_chars_without_complement =
    (1ULL << static_cast<uint64_t>(Aux_expr_lexem_code::Class_Latin))   |
    (1ULL << static_cast<uint64_t>(Aux_expr_lexem_code::Class_Letter))  |
    (1ULL << static_cast<uint64_t>(Aux_expr_lexem_code::Class_Russian)) |
    (1ULL << static_cast<uint64_t>(Aux_expr_lexem_code::Class_bdigits)) |
    (1ULL << static_cast<uint64_t>(Aux_expr_lexem_code::Class_digits))  |
    (1ULL << static_cast<uint64_t>(Aux_expr_lexem_code::Class_latin))   |
    (1ULL << static_cast<uint64_t>(Aux_expr_lexem_code::Class_letter))  |
    (1ULL << static_cast<uint64_t>(Aux_expr_lexem_code::Class_odigits)) |
    (1ULL << static_cast<uint64_t>(Aux_expr_lexem_code::Class_russian)) |
    (1ULL << static_cast<uint64_t>(Aux_expr_lexem_code::Class_xdigits));

static const uint64_t classes_of_chars_with_complement =
    (1ULL << static_cast<uint64_t>(Aux_expr_lexem_code::Class_ndq)) |
    (1ULL << static_cast<uint64_t>(Aux_expr_lexem_code::Class_nsq));

static const char* not_admissible_nsq_ndq =
    "Error at line %zu: character classes [:ndq:] and [:nsq:] are not admissible "
    "in the character class complement.\n";

static const char* not_admissible_lexeme =
    "Error at line %zu: expected a character or character class, with the "
    "exception of [:nsq:] and [:ndq:].\n";

static const size_t first_code_of_char_class =
    static_cast<size_t>(Aux_expr_lexem_code::Class_Latin);

inline size_t char_class_to_array_index(Aux_expr_lexem_code e)
{
    return static_cast<uint64_t>(e) - first_code_of_char_class;
}

inline std::set<char32_t> char_class_set_by_lexeme(Aux_expr_lexem_code e)
{
    return sets_for_char_classes[char_class_to_array_index(e)];
}

Expr_lexem_info Expr_scaner::convert_lexeme(const Aux_expr_lexem_info aeli)
{
    Expr_lexem_info     eli;
    Aux_expr_lexem_code aelic = aeli.code;

    if(is_in_segment(aelic, Aux_expr_lexem_code::M_Class_Latin,
                     Aux_expr_lexem_code::M_Class_nsq))
    {
        int y = static_cast<int>(aelic) -
                static_cast<int>(Aux_expr_lexem_code::M_Class_Latin) +
                static_cast<int>(Aux_expr_lexem_code::Class_Latin);
        aelic = static_cast<Aux_expr_lexem_code>(y);
    }

    switch(aelic){
        case Aux_expr_lexem_code::Character:
            eli.c                 = aeli.c;
            eli.code              = Expr_lexem_code::Character;
            break;
        case Aux_expr_lexem_code::Action:
            eli.action_name_index = aeli.action_name_index;
            eli.code              = Expr_lexem_code::Action;
            break;
        case Aux_expr_lexem_code::Class_Latin ... Aux_expr_lexem_code::Class_xdigits:
            eli.set_of_char_index =
                compl_set_trie->insertSet(char_class_set_by_lexeme(aelic));
            eli.code              = Expr_lexem_code::Character_class;
            break;
        case Aux_expr_lexem_code::Class_ndq:
            eli.set_of_char_index = compl_set_trie->insertSet(double_quote);
            eli.code = Expr_lexem_code::Class_complement;
            break;
        case Aux_expr_lexem_code::Class_nsq:
            eli.set_of_char_index = compl_set_trie->insertSet(single_quote);
            eli.code              = Expr_lexem_code::Class_complement;
            break;
        default:
            eli.code              = static_cast<Expr_lexem_code>(aelic);
    }

    return eli;
}

Expr_scaner::State_proc Expr_scaner::procs[] = {
    &Expr_scaner::begin_class_complement_proc,
    &Expr_scaner::first_char_proc,
    &Expr_scaner::body_chars_proc,
    &Expr_scaner::end_class_complement_proc
};

void Expr_scaner::begin_class_complement_proc()
{
    state = State::First_char;
}

void Expr_scaner::first_char_proc()
{
    state = State::Body_chars;
    if(Aux_expr_lexem_code::Character == aelic){
        curr_set.insert(aeli.c);
    }else if(belongs(aelic, classes_of_chars_without_complement)){
        const auto& s = sets_for_char_classes[char_class_to_array_index(aelic)];
        curr_set.insert(s.begin(), s.end());
    }else if(belongs(aelic, classes_of_chars_with_complement)){
        printf(not_admissible_nsq_ndq, aux_scaner->lexem_begin_line_number());
        et_.ec->increment_number_of_errors();
    }else{
        printf(not_admissible_lexeme, aux_scaner->lexem_begin_line_number());
        et_.ec->increment_number_of_errors();
    }
}

void Expr_scaner::body_chars_proc()
{
    state = State::Body_chars;
    if(Aux_expr_lexem_code::Character == aelic){
        curr_set.insert(aeli.c);
    }else if(belongs(aelic, classes_of_chars_without_complement)){
        const auto& s = sets_for_char_classes[char_class_to_array_index(aelic)];
        curr_set.insert(s.begin(), s.end());
    }else if(belongs(aelic, classes_of_chars_with_complement)){
        printf(not_admissible_nsq_ndq, aux_scaner->lexem_begin_line_number());
        et_.ec->increment_number_of_errors();
    }else if(Aux_expr_lexem_code::End_char_class_complement == aelic){
        set_idx = compl_set_trie->insertSet(curr_set);
        state = State::End_class_complement;
    }else{
        printf(not_admissible_lexeme, aux_scaner->lexem_begin_line_number());
        et_.ec->increment_number_of_errors();
    }
}

void Expr_scaner::end_class_complement_proc(){}

size_t Expr_scaner::get_set_complement()
{
    set_idx = 0;
    state   = State::Begin_class_complement;

    curr_set.clear();

    while((aelic = (aeli = aux_scaner-> current_lexem()).code) !=
          Aux_expr_lexem_code::Nothing)
    {
        (this->*procs[static_cast<size_t>(state)])();
        if(State::End_class_complement == state){
            break;
        }
    }
    return set_idx;
}

Expr_lexem_info Expr_scaner::current_lexem()
{
    Expr_lexem_info     eli;

    aelic            = (aeli = aux_scaner-> current_lexem()).code;
    lexem_begin_line = aux_scaner->lexem_begin_line_number();;
    lexem_begin      = aux_scaner->lexem_begin_ptr();
    switch(aelic){
        case Aux_expr_lexem_code::Nothing       ... Aux_expr_lexem_code::Class_xdigits:
        case Aux_expr_lexem_code::Class_ndq:
        case Aux_expr_lexem_code::Class_nsq:
        case Aux_expr_lexem_code::M_Class_Latin ... Aux_expr_lexem_code::M_Class_nsq:
            eli = convert_lexeme(aeli);
            break;
        case Aux_expr_lexem_code::Begin_char_class_complement:
            aux_scaner->back();
            eli.code              = Expr_lexem_code::Class_complement;
            eli.set_of_char_index = get_set_complement();
            break;
        case Aux_expr_lexem_code::End_char_class_complement:
            eli.code = Expr_lexem_code::UnknownLexem;
    }
    return eli;
}

size_t Expr_scaner::lexem_begin_line_number() const
{
    return lexem_begin_line;
}

void Expr_scaner::back()
{
    loc->pcurrent_char = lexem_begin;
    loc->current_line  = lexem_begin_line;
}