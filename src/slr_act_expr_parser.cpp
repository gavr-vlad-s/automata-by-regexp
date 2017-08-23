/*
    File:    slr_act_expr_parser.cpp
    Created: 20 August 2017 at 12:30 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <map>
#include <cstdio>
#include <cstdlib>
#include "../include/slr_act_expr_parser.h"
#include "../include/belongs.h"
#include "../include/expr_lexem_info.h"
#include "../include/expr_traits.h"

static const Terminal lexem2terminal_map[] = {
    Terminal::End_of_text, Terminal::End_of_text, Terminal::Term_a,
    Terminal::Term_LP,     Terminal::Term_RP,     Terminal::Term_b,
    Terminal::Term_c,      Terminal::Term_c,      Terminal::Term_c,
    Terminal::Term_d,      Terminal::Term_p,      Terminal::Term_q,
    Terminal::Term_d,      Terminal::Term_d
};

Terminal SLR_act_expr_parser::lexem2terminal(const Expr_lexem_info& l)
{
    return lexem2terminal_map[static_cast<uint16_t>(l.code)];
}

/* Grammar rules:
 *
 * -------------------------------------------
 * | Rule number | Rule       | Rule name    |
 * |------------------------------------------
 * | (0)         | S -> pTq   | S_is_pTq     |
 * | (1)         | T -> TbE   | T_is_TbE     |
 * | (2)         | T -> E     | T_is_E       |
 * | (3)         | E -> EF    | E_is_EF      |
 * | (4)         | E -> F     | E_is_F       |
 * | (5)         | F -> Gc    | F_is_Gc      |
 * | (6)         | F -> G     | F_is_G       |
 * | (7)         | G -> Ha    | G_is_Ha      |
 * | (8)         | G -> H     | G_is_H       |
 * | (9)         | H -> d     | H_is_d       |
 * | (10)        | H -> (T)   | H_is_LP_T_RP |
 * ---------------------------------------------
 *
 * In this grammar, a means $action_name, b means the operator |, c means unary
 * operators ? * +, d means a character or a character class, p means { (opening
 * curly bracket), q means } (closing curly bracket).
 */

static const char* opening_curly_brace_is_expected =
    "An opening curly brace is expected at line %zu.\n";

static const char* char_or_char_class_expected =
    "A character, a character class, or an opening parenthesis are "
    "expected at line %zu.\n";

static const char* or_operator_or_brace_expected =
    "An operator | or closing brace are expected at line %zu.\n";

static const char* unexpected_action =
    "Unexpected action at line %zu.\n";

static const char* unexpected_postfix_operator =
    "Unexpected postfix operator at line %zu.\n";

static const char* unexpected_end_of_text =
    "Unexpected end of text at line %zu.\n";

static const char* unexpected_opening_brace =
    "Unexpected opening brace at line %zu.\n";

static const char* or_operator_or_round_br_closed =
    "An operator | or closing parenthesis are expected at line %zu.\n";

SLR_act_expr_parser::Attrib_calculator SLR_act_expr_parser::attrib_calculator[] = {
    &SLR_act_expr_parser::attrib_by_S_is_pTq,
    &SLR_act_expr_parser::attrib_by_T_is_TbE,
    &SLR_act_expr_parser::attrib_by_T_is_E,
    &SLR_act_expr_parser::attrib_by_E_is_EF,
    &SLR_act_expr_parser::attrib_by_E_is_F,
    &SLR_act_expr_parser::attrib_by_F_is_Gc,
    &SLR_act_expr_parser::attrib_by_F_is_G,
    &SLR_act_expr_parser::attrib_by_G_is_Ha,
    &SLR_act_expr_parser::attrib_by_G_is_H,
    &SLR_act_expr_parser::attrib_by_H_is_d,
    &SLR_act_expr_parser::attrib_by_H_is_LP_T_RP
};

SLR_act_expr_parser::Error_handler SLR_act_expr_parser::error_hadler[] = {
    &SLR_act_expr_parser::state00_error_handler, // 0  +
    &SLR_act_expr_parser::state01_error_handler, // 1  +
    &SLR_act_expr_parser::state02_error_handler, // 2  +
    &SLR_act_expr_parser::state03_error_handler, // 3  +
    &SLR_act_expr_parser::state04_error_handler, // 4  +
    &SLR_act_expr_parser::state04_error_handler, // 5  +
    &SLR_act_expr_parser::state06_error_handler, // 6  +
    &SLR_act_expr_parser::state07_error_handler, // 7  +
    &SLR_act_expr_parser::state07_error_handler, // 8  +
    &SLR_act_expr_parser::state02_error_handler, // 9  +
    &SLR_act_expr_parser::state02_error_handler, // 10 +
    &SLR_act_expr_parser::state11_error_handler, // 11 +
    &SLR_act_expr_parser::state04_error_handler, // 12 +
    &SLR_act_expr_parser::state04_error_handler, // 13 +
    &SLR_act_expr_parser::state06_error_handler, // 14 +
    &SLR_act_expr_parser::state15_error_handler, // 15 +
    &SLR_act_expr_parser::state04_error_handler, // 16 +
    &SLR_act_expr_parser::state07_error_handler  // 17 +
};

/* In this array, the rules are collected for which reduce is performed in
 * error-handling functions. The number of the array element is the number of the
 * current state of the parser. If a state in the corresponding error-handling
 * function is not reduced, then the element of this array with the corresponding
 * index is (-1). */
static const char reduce_rules[] = {
    -1,       -1,          -1,      -1,
    T_is_E,   E_is_F,      F_is_G,  G_is_H,
    H_is_d,   -1,          -1,      S_is_pTq,
    E_is_EF,  F_is_Gc,     G_is_Ha, -1,
    T_is_TbE, H_is_LP_T_RP
};

void SLR_act_expr_parser::generate_E_is_EF()
{
    Command com;
    com.name        = Command_name::Concat;
    com.args.first  = rule_body[0].attr.indeces.end_index;
    com.args.second = rule_body[1].attr.indeces.end_index;
    com.action_name = 0;
    buf_.push_back(com);
}

void SLR_act_expr_parser::generate_by_F_is_Gc()
{
    Command com;
    switch(rule_body[1].attr.li.code){
        case Expr_lexem_code::Kleene_closure:
            com.name = Command_name::Kleene;
            break;
        case Expr_lexem_code::Positive_closure:
            com.name = Command_name::Positive;
            break;
        case Expr_lexem_code::Optional_member:
            com.name = Command_name::Optional;
            break;
        default:
            ;
    }
    com.args.first  = rule_body[0].attr.indeces.end_index;
    com.args.second = 0;
    com.action_name = 0;
    buf_.push_back(com);
}

void SLR_act_expr_parser::generate_by_H_is_d()
{
    Command com;
    switch(rule_body[0].attr.li.code){
        case Expr_lexem_code::Character:
            com.name        = Command_name::Char;
            com.c           = rule_body[0].attr.li.c;
            break;
        case Expr_lexem_code::Class_complement:
            com.name        = Command_name::Char_class_complement;
            com.idx_of_set  = rule_body[0].attr.li.set_of_char_index;
            break;
        case Expr_lexem_code::Character_class:
            com.name        = Command_name::Char_class;
            com.idx_of_set  = rule_body[0].attr.li.set_of_char_index;
            break;
        default:
            ;
    }
    com.action_name = 0;
    buf_.push_back(com);
}

void SLR_act_expr_parser::generate_by_G_is_Ha()
{
    Id_scope::iterator it;
    size_t             act_index;
    size_t             min_index;
    size_t             max_index;
    /* If the action a is not yet defined, then we display an error message and
        * assume that no action is specified. Otherwise, write down the index of
        * the action name. */
    act_index = rule_body[1].attr.li.action_name_index;
    it        = scope_->idsc.find(act_index);
    if(it == scope_->idsc.end()){
        printf("The action ");
        et_.ids_trie->print(act_index);
        printf(" is not defined at line %zu.\n",
                scaner->lexem_begin_line_number());
        et_.ec -> increment_number_of_errors();
        return;
    } else if(it->second.kind != Action_name){
        printf("The identifier ");
        et_.ids_trie->print(act_index);
        printf(" is not action name at line %zu.\n",
                scaner->lexem_begin_line_number());
        et_.ec -> increment_number_of_errors();
        return;
    };
    min_index = rule_body[0].attr.indeces.begin_index;
    max_index = rule_body[0].attr.indeces.end_index + 1;
    for(size_t i = min_index; i < max_index; i++){
        buf_[i].action_name = act_index;
    }
}

void SLR_act_expr_parser::generate_by_T_is_TbE()
{
    Command com;
    com.name        = Command_name::Or;
    com.args.first  = rule_body[0].attr.indeces.end_index;
    com.args.second = rule_body[2].attr.indeces.end_index;
    com.action_name = 0;
    buf_.push_back(com);
}

void SLR_act_expr_parser::generate_command(Rule r)
{
    switch(r){
        case T_is_TbE:
            generate_by_T_is_TbE();
            break;

        case E_is_EF:
            generate_E_is_EF();
            break;

        case F_is_Gc:
            generate_by_F_is_Gc();
            break;

        case H_is_d:
            generate_by_H_is_d();
            break;

        case G_is_Ha:
            generate_by_G_is_Ha();
            break;

        default:
            ;
    }
}

/* Functions for calculating of attributes: */
Attributes<Expr_lexem_info> SLR_act_expr_parser::attrib_by_S_is_pTq()
{
    return rule_body[1].attr;
}

Attributes<Expr_lexem_info> SLR_act_expr_parser::attrib_by_T_is_TbE()
{
    Attributes<Expr_lexem_info> s = rule_body[0].attr;
    s.indeces.end_index = buf_.size() - 1;
    return s;
}

Attributes<Expr_lexem_info> SLR_act_expr_parser::attrib_by_T_is_E()
{
    return rule_body[0].attr;
}

Attributes<Expr_lexem_info> SLR_act_expr_parser::attrib_by_E_is_EF()
{
    Attributes<Expr_lexem_info> s = rule_body[0].attr;
    s.indeces.end_index = buf_.size() - 1;
    return s;
}

Attributes<Expr_lexem_info> SLR_act_expr_parser::attrib_by_E_is_F()
{
    return rule_body[0].attr;
}

Attributes<Expr_lexem_info> SLR_act_expr_parser::attrib_by_F_is_Gc()
{
    Attributes<Expr_lexem_info> s = rule_body[0].attr;
    s.indeces.end_index = buf_.size() - 1;
    return s;
}

Attributes<Expr_lexem_info> SLR_act_expr_parser::attrib_by_F_is_G()
{
    return rule_body[0].attr;
}

Attributes<Expr_lexem_info> SLR_act_expr_parser::attrib_by_G_is_Ha()
{
    return rule_body[0].attr;
}

Attributes<Expr_lexem_info> SLR_act_expr_parser::attrib_by_G_is_H()
{
    return rule_body[0].attr;
}

Attributes<Expr_lexem_info> SLR_act_expr_parser::attrib_by_H_is_d()
{
    Attributes<Expr_lexem_info> s;
    s.indeces.begin_index = s.indeces.end_index = buf_.size() - 1;
    return s;
}

Attributes<Expr_lexem_info> SLR_act_expr_parser::attrib_by_H_is_LP_T_RP(){
    return rule_body[1].attr;
}

Attributes<Expr_lexem_info> SLR_act_expr_parser::attrib_calc(Rule r)
{
    return (this->*attrib_calculator[r])();
}

/* Functions for error handling: */
Parser_action_info SLR_act_expr_parser::state00_error_handler()
{
    printf(opening_curly_brace_is_expected, scaner->lexem_begin_line_number());
    et_.ec->increment_number_of_errors();
    if(li.code != Expr_lexem_code::Closed_round_brack){
        scaner->back();
    }
    li.code = Expr_lexem_code::Begin_expression;
    Parser_action_info pa;
    pa.kind   = static_cast<uint16_t>(Parser_action_name::Shift); pa.arg = 2;
    return pa;
}

Parser_action_info SLR_act_expr_parser::state01_error_handler()
{
    Parser_action_info pa;
    pa.kind = static_cast<uint16_t>(Parser_action_name::OK); pa.arg = 0;
    return pa;
}

Parser_action_info SLR_act_expr_parser::state02_error_handler()
{
    printf(char_or_char_class_expected, scaner->lexem_begin_line_number());
    et_.ec->increment_number_of_errors();
    scaner->back();
    li.code = Expr_lexem_code::Character;
    li.c    = 'a';
    Parser_action_info pa;
    pa.kind = static_cast<uint16_t>(Parser_action_name::Shift); pa.arg = 8;
    return pa;
}

Parser_action_info SLR_act_expr_parser::state03_error_handler()
{
    printf(or_operator_or_brace_expected, scaner->lexem_begin_line_number());
    et_.ec->increment_number_of_errors();
    if(t != Terminal::Term_p){
        scaner->back();
    }
    li.code = Expr_lexem_code::Or;
    Parser_action_info pa;
    pa.kind = static_cast<uint16_t>(Parser_action_name::Shift); pa.arg = 10;
    return pa;
}

Parser_action_info SLR_act_expr_parser::state04_error_handler()
{
    Rule r = static_cast<Rule>(reduce_rules[current_state]);
    Parser_action_info pa;
    switch(t){
        case Terminal::Term_a:
            printf(unexpected_action, scaner->lexem_begin_line_number());
            pa.kind = static_cast<uint16_t>(Parser_action_name::Reduce); pa.arg = r;
            break;

        case Terminal::Term_c:
            printf(unexpected_postfix_operator, scaner->lexem_begin_line_number());
            pa.kind = static_cast<uint16_t>(Parser_action_name::Reduce); pa.arg = r;
            break;

        case Terminal::End_of_text:
            printf(unexpected_end_of_text, scaner->lexem_begin_line_number());
            pa.kind = static_cast<uint16_t>(Parser_action_name::Reduce); pa.arg = r;
            break;

        case Terminal::Term_p:
            printf(unexpected_opening_brace, scaner->lexem_begin_line_number());
            pa.kind = static_cast<uint16_t>(Parser_action_name::Reduce_without_back); pa.arg = r;
            break;

        default:
            ;
    }
    return pa;
}

Parser_action_info SLR_act_expr_parser::state06_error_handler()
{
    Rule r = static_cast<Rule>(reduce_rules[current_state]);
    Parser_action_info pa;
    switch(t){
        case Terminal::Term_a:
            printf(unexpected_action, scaner->lexem_begin_line_number());
            pa.kind = static_cast<uint16_t>(Parser_action_name::Reduce_without_back); pa.arg = r;
            break;

        case Terminal::Term_p:
            printf(unexpected_opening_brace, scaner->lexem_begin_line_number());
            pa.kind = static_cast<uint16_t>(Parser_action_name::Reduce_without_back); pa.arg = r;
            break;

        case Terminal::End_of_text:
            printf(unexpected_end_of_text, scaner->lexem_begin_line_number());
            pa.kind = static_cast<uint16_t>(Parser_action_name::Reduce_without_back); pa.arg = r;
            break;

        default:
            ;
    }
    return pa;
}

Parser_action_info SLR_act_expr_parser::state07_error_handler()
{
    Rule r = static_cast<Rule>(reduce_rules[current_state]);
    Parser_action_info pa;
    if(Terminal::Term_p == t){
        printf(unexpected_opening_brace, scaner->lexem_begin_line_number());
        pa.kind = static_cast<uint16_t>(Parser_action_name::Reduce_without_back); pa.arg = r;
    }else{
        printf(unexpected_end_of_text, scaner->lexem_begin_line_number());
        pa.kind = static_cast<uint16_t>(Parser_action_name::Reduce_without_back); pa.arg = r;
    }
    et_.ec->increment_number_of_errors();
    return pa;
}

Parser_action_info SLR_act_expr_parser::state11_error_handler()
{
    Parser_action_info pa;
    pa.kind = static_cast<uint16_t>(Parser_action_name::Reduce); pa.arg = S_is_pTq;
    return pa;
}

Parser_action_info SLR_act_expr_parser::state15_error_handler()
{
    printf(or_operator_or_round_br_closed, scaner->lexem_begin_line_number());
    et_.ec->increment_number_of_errors();
    if(t != Terminal::Term_p){
        scaner->back();
    }
    li.code = Expr_lexem_code::Or;
    Parser_action_info pa;
    pa.kind = static_cast<uint16_t>(Parser_action_name::Shift); pa.arg = 10;
    return pa;
}


Parser_action_info SLR_act_expr_parser::error_hadling(size_t s)
{
    return (this->*error_hadler[s])();
}