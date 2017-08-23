/*
    File:    act_expr_parser.cpp
    Created: 13 December 2015 at 09:05 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include "../include/act_expr_parser.h"
#include "../include/belongs.h"
#include "../include/expr_lexem_info.h"
#include <map>
#include <cstdio>
#include <cstdlib>

Act_expr_parser::Act_expr_parser(Expr_scaner_ptr&        esc,
                                 const Errors_and_tries& et,
                                 std::shared_ptr<Scope>& scope)
{
    esc_         = esc;
    scope_       = scope;
    et_          = et;
    parser_stack = Multipop_stack<Stack_elem>();
}

//     Nothing,             UnknownLexem,       Action,
//     Opened_round_brack,  Closed_round_brack, Or,
//     Kleene_closure,      Positive_closure,   Optional_member,
//     Character,           Begin_expression,   End_expression,
//     Class_complement,    Character_class

Terminal lexem2terminal_map[] = {
    End_of_text, End_of_text, Term_a,
    Term_LP,     Term_RP,     Term_b,
    Term_c,      Term_c,      Term_c,
    Term_d,      Term_p,      Term_q,
    Term_d,      Term_d
};

inline Terminal lexem2terminal(const Expr_lexem_info& l)
{
    return lexem2terminal_map[static_cast<uint16_t>(l.code)];
}

// #define DEBUG_MODE
#ifdef DEBUG_MODE
static const char* terminal_strings[] = {
    "End_of_text", "Term_a",  "Term_b",
    "Term_c",      "Term_d",  "Term_p",
    "Term_q",      "Term_LP", "Term_RP"
};

void print_terminal(Terminal t)
{
    printf("Current terminal: %s.\n",terminal_strings[t]);
}
#endif

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

Act_expr_parser::Rule_info Act_expr_parser::rules[] = {
    {Nt_S, 3}, {Nt_T, 3}, {Nt_T, 1}, {Nt_E, 2}, {Nt_E, 1}, {Nt_F, 2},
    {Nt_F, 1}, {Nt_G, 2}, {Nt_G, 1}, {Nt_H, 1}, {Nt_H, 3}
};

#define ANY ((uint8_t)(-1))
struct GOTO_entry{
    uint8_t from;
    uint8_t to;
};

GOTO_entry goto_S[] = {
    {ANY, 1}
};

GOTO_entry goto_T[] = {
    {9, 15}, {ANY, 3}
};

GOTO_entry goto_E[] = {
    {10, 16}, {ANY, 4}
};

GOTO_entry goto_F[] = {
    {4, 12}, {16, 12}, {ANY, 5}
};

GOTO_entry goto_G[] = {
    {ANY, 6}
};

GOTO_entry goto_H[] = {
    {ANY, 7}
};

GOTO_entry* goto_table[] = {
    goto_S, goto_T, goto_E, goto_F, goto_G, goto_H
};

void Act_expr_parser::shift(size_t shifted_state, Expr_lexem_info e){
    Stack_elem selem;
    selem.st_num   = shifted_state;
    selem.attr.eli = e;
    parser_stack.push(selem);
    (this->*checker)(e);
}

void Act_expr_parser::reduce(Rule r){
    reduce_without_back(r);
    esc_->back();
}

size_t next_state(size_t s, Non_terminal n){
    size_t cs;
    GOTO_entry  current_entry;
    GOTO_entry* goto_for_n = goto_table[n];
    while((cs = (current_entry = *goto_for_n++).from) != ANY){
        if(cs == s){
            return current_entry.to;
        }
    }
    goto_for_n--;
    return goto_for_n -> to;
}

void Act_expr_parser::reduce_without_back(Rule r){
    size_t rule_len = rules[r].len;
    parser_stack.get_elems_from_top(rule_body, rule_len);
    generate_command(r);

    Stack_elem se;
    se.attr    = (this->*attrib_calc[r])();
    parser_stack.multi_pop(rule_len);
    Stack_elem top_elem = parser_stack.top();
    se.st_num           = next_state(top_elem.st_num, rules[r].nt);
    parser_stack.push(se);
}

#define ERROR     {Act_error, 0}
#define SHIFT(t)  {Act_shift, t}
#define REDUCE(r) {Act_reduce, r}
#define ACCESS    {Act_OK, 0}

using State_and_terminal  = std::pair<size_t, Terminal>;
using Parser_action_table = std::map<State_and_terminal, Parser_action_info>;

const Parser_action_table action_table = {
    {{0,Term_p},SHIFT(2)},              {{1,End_of_text},ACCESS},
    {{2,Term_d},SHIFT(8)},              {{2,Term_LP},SHIFT(9)},
    {{3,Term_b},SHIFT(10)},             {{3,Term_q},SHIFT(11)},
    {{4,Term_d},SHIFT(8)},              {{4,Term_LP},SHIFT(9)},
    {{4,Term_b},REDUCE(T_is_E)},        {{4,Term_q},REDUCE(T_is_E)},
    {{4,Term_RP},REDUCE(T_is_E)},       {{5,Term_b},REDUCE(E_is_F)},
    {{5,Term_d},REDUCE(E_is_F)},        {{5,Term_q},REDUCE(E_is_F)},
    {{5,Term_LP},REDUCE(E_is_F)},       {{5,Term_RP},REDUCE(E_is_F)},
    {{6,Term_b},REDUCE(F_is_G)},        {{6,Term_d},REDUCE(F_is_G)},
    {{6,Term_q},REDUCE(F_is_G)},        {{6,Term_LP},REDUCE(F_is_G)},
    {{6,Term_RP},REDUCE(F_is_G)},       {{6,Term_c},SHIFT(13)},
    {{7,Term_a},SHIFT(14)},             {{7,Term_b},REDUCE(G_is_H)},
    {{7,Term_c},REDUCE(G_is_H)},        {{7,Term_d},REDUCE(G_is_H)},
    {{7,Term_q},REDUCE(G_is_H)},        {{7,Term_LP},REDUCE(G_is_H)},
    {{7,Term_RP},REDUCE(G_is_H)},       {{8,Term_a},REDUCE(H_is_d)},
    {{8,Term_b},REDUCE(H_is_d)},        {{8,Term_c},REDUCE(H_is_d)},
    {{8,Term_d},REDUCE(H_is_d)},        {{8,Term_q},REDUCE(H_is_d)},
    {{8,Term_LP},REDUCE(H_is_d)},       {{8,Term_RP},REDUCE(H_is_d)},
    {{9,Term_d},SHIFT(8)},              {{9,Term_LP},SHIFT(9)},
    {{10,Term_d},SHIFT(8)},             {{10,Term_LP},SHIFT(9)},
    {{11,End_of_text},REDUCE(S_is_pTq)},{{12,Term_b},REDUCE(E_is_EF)},
    {{12,Term_d},REDUCE(E_is_EF)},      {{12,Term_q},REDUCE(E_is_EF)},
    {{12,Term_LP},REDUCE(E_is_EF)},     {{12,Term_RP},REDUCE(E_is_EF)},
    {{13,Term_b},REDUCE(F_is_Gc)},      {{13,Term_d},REDUCE(F_is_Gc)},
    {{13,Term_q},REDUCE(F_is_Gc)},      {{13,Term_LP},REDUCE(F_is_Gc)},
    {{13,Term_RP},REDUCE(F_is_Gc)},     {{14,Term_b},REDUCE(G_is_Ha)},
    {{14,Term_d},REDUCE(G_is_Ha)},      {{14,Term_q},REDUCE(G_is_Ha)},
    {{14,Term_LP},REDUCE(G_is_Ha)},     {{14,Term_RP},REDUCE(G_is_Ha)},
    {{14,Term_c},REDUCE(G_is_Ha)},      {{15,Term_b},SHIFT(10)},
    {{15,Term_RP},SHIFT(17)},           {{16,Term_d},SHIFT(8)},
    {{16,Term_LP},SHIFT(9)},            {{16,Term_b},REDUCE(T_is_TbE)},
    {{16,Term_q},REDUCE(T_is_TbE)},     {{16,Term_RP},REDUCE(T_is_TbE)},
    {{17,Term_a},REDUCE(H_is_LP_T_RP)}, {{17,Term_b},REDUCE(H_is_LP_T_RP)},
    {{17,Term_c},REDUCE(H_is_LP_T_RP)}, {{17,Term_d},REDUCE(H_is_LP_T_RP)},
    {{17,Term_q},REDUCE(H_is_LP_T_RP)}, {{17,Term_LP},REDUCE(H_is_LP_T_RP)},
    {{17,Term_RP},REDUCE(H_is_LP_T_RP)}
};

static const char* character_class_is_not_allowed =
    "Error at line %zu: in the regular expression for numbers, a "
    "character classs complement is not allowed.\n";

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
void Act_expr_parser::checker_for_number_expr(Expr_lexem_info e)
{
    if(Expr_lexem_code::Class_complement == e.code){
        printf(character_class_is_not_allowed, esc_->lexem_begin_line_number());
        et_.ec->increment_number_of_errors();
    }
}

void Act_expr_parser::checker_for_string_expr(Expr_lexem_info e){
}

void Act_expr_parser::compile(Command_buffer& buf, Number_or_string kind_of_expr)
{
    checker =
        (kind_of_expr == Number_expr) ? &Act_expr_parser::checker_for_number_expr :
        &Act_expr_parser::checker_for_string_expr;
    buf_ = buf;

    Stack_elem initial_elem;
    initial_elem.st_num                   = 0;
    initial_elem.attr.indeces.begin_index = 0;
    initial_elem.attr.indeces.end_index   = 0;
    parser_stack.push(initial_elem);

    for( ; ; ){
        eli_ = esc_->current_lexem();
        t = lexem2terminal(eli_);
#ifdef DEBUG_MODE
        print_terminal(t);
#endif
        current_state = parser_stack.top().st_num;
        auto it = action_table.find({current_state, t});
        Parser_action_info pai;
        if(it != action_table.end()){
            pai = it->second;
        }else{
            pai = (this->*error_hadler[current_state])();
        }
        switch(pai.kind){
            case Act_reduce:
                reduce(static_cast<Rule>(pai.arg));
                break;
            case Act_shift:
                shift(pai.arg, eli_);
                break;
            case Act_reduce_without_back:
                reduce_without_back(static_cast<Rule>(pai.arg));
                break;
            case Act_OK:
                buf = buf_;
                esc_->back();
                return;
        }
    }
}

Act_expr_parser::Attrib_calculator Act_expr_parser::attrib_calc[] = {
    &Act_expr_parser::attrib_by_S_is_pTq,
    &Act_expr_parser::attrib_by_T_is_TbE,
    &Act_expr_parser::attrib_by_T_is_E,
    &Act_expr_parser::attrib_by_E_is_EF,
    &Act_expr_parser::attrib_by_E_is_F,
    &Act_expr_parser::attrib_by_F_is_Gc,
    &Act_expr_parser::attrib_by_F_is_G,
    &Act_expr_parser::attrib_by_G_is_Ha,
    &Act_expr_parser::attrib_by_G_is_H,
    &Act_expr_parser::attrib_by_H_is_d,
    &Act_expr_parser::attrib_by_H_is_LP_T_RP
};

void Act_expr_parser::generate_E_is_EF()
{
    Command com;
    com.name        = Command_name::Concat;
    com.args.first  = rule_body[0].attr.indeces.end_index;
    com.args.second = rule_body[1].attr.indeces.end_index;
    com.action_name = 0;
    buf_.push_back(com);
}

void Act_expr_parser::generate_by_F_is_Gc()
{
    Command com;
    switch(rule_body[1].attr.eli.code){
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

void Act_expr_parser::generate_by_H_is_d()
{
    Command com;
    switch(rule_body[0].attr.eli.code){
        case Expr_lexem_code::Character:
            com.name        = Command_name::Char;
            com.c           = rule_body[0].attr.eli.c;
            break;
        case Expr_lexem_code::Class_complement:
            com.name        = Command_name::Char_class_complement;
            com.idx_of_set  = rule_body[0].attr.eli.set_of_char_index;
            break;
        case Expr_lexem_code::Character_class:
            com.name        = Command_name::Char_class;
            com.idx_of_set  = rule_body[0].attr.eli.set_of_char_index;
            break;
        default:
            ;
    }
    com.action_name = 0;
    buf_.push_back(com);
}

void Act_expr_parser::generate_by_G_is_Ha()
{
    Id_scope::iterator it;
    size_t             act_index;
    size_t             min_index;
    size_t             max_index;
    /* If the action a is not yet defined, then we display an error message and
        * assume that no action is specified. Otherwise, write down the index of
        * the action name. */
    act_index = rule_body[1].attr.eli.action_name_index;
    it        = scope_->idsc.find(act_index);
    if(it == scope_->idsc.end()){
        printf("The action ");
        et_.ids_trie->print(act_index);
        printf(" is not defined at line %zu.\n",
                esc_->lexem_begin_line_number());
        et_.ec -> increment_number_of_errors();
        return;
    } else if(it->second.kind != Action_name){
        printf("The identifier ");
        et_.ids_trie->print(act_index);
        printf(" is not action name at line %zu.\n",
                esc_->lexem_begin_line_number());
        et_.ec -> increment_number_of_errors();
        return;
    };
    min_index = rule_body[0].attr.indeces.begin_index;
    max_index = rule_body[0].attr.indeces.end_index + 1;
    for(size_t i = min_index; i < max_index; i++){
        buf_[i].action_name = act_index;
    }
}

void Act_expr_parser::generate_by_T_is_TbE()
{
    Command com;
    com.name        = Command_name::Or;
    com.args.first  = rule_body[0].attr.indeces.end_index;
    com.args.second = rule_body[2].attr.indeces.end_index;
    com.action_name = 0;
    buf_.push_back(com);
}

void Act_expr_parser::generate_command(Rule r)
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

Attributes Act_expr_parser::attrib_by_S_is_pTq()
{
    return rule_body[1].attr;
}

Attributes Act_expr_parser::attrib_by_T_is_TbE()
{
    Attributes s = rule_body[0].attr;
    s.indeces.end_index = buf_.size() - 1;
    return s;
}

Attributes Act_expr_parser::attrib_by_T_is_E()
{
    return rule_body[0].attr;
}

Attributes Act_expr_parser::attrib_by_E_is_EF()
{
    Attributes s = rule_body[0].attr;
    s.indeces.end_index = buf_.size() - 1;
    return s;
}

Attributes Act_expr_parser::attrib_by_E_is_F()
{
    return rule_body[0].attr;
}

Attributes Act_expr_parser::attrib_by_F_is_Gc()
{
    Attributes s = rule_body[0].attr;
    s.indeces.end_index = buf_.size() - 1;
    return s;
}

Attributes Act_expr_parser::attrib_by_F_is_G()
{
    return rule_body[0].attr;
}

Attributes Act_expr_parser::attrib_by_G_is_Ha()
{
    return rule_body[0].attr;
}

Attributes Act_expr_parser::attrib_by_G_is_H()
{
    return rule_body[0].attr;
}

Attributes Act_expr_parser::attrib_by_H_is_d()
{
    Attributes s;
    s.indeces.begin_index = s.indeces.end_index = buf_.size() - 1;
    return s;
}

Attributes Act_expr_parser::attrib_by_H_is_LP_T_RP(){
    return rule_body[1].attr;
}

Act_expr_parser::Error_handler Act_expr_parser::error_hadler[] = {
    &Act_expr_parser::state00_error_handler, // 0  +
    &Act_expr_parser::state01_error_handler, // 1  +
    &Act_expr_parser::state02_error_handler, // 2  +
    &Act_expr_parser::state03_error_handler, // 3  +
    &Act_expr_parser::state04_error_handler, // 4  +
    &Act_expr_parser::state04_error_handler, // 5  +
    &Act_expr_parser::state06_error_handler, // 6  +
    &Act_expr_parser::state07_error_handler, // 7  +
    &Act_expr_parser::state07_error_handler, // 8  +
    &Act_expr_parser::state02_error_handler, // 9  +
    &Act_expr_parser::state02_error_handler, // 10 +
    &Act_expr_parser::state11_error_handler, // 11 +
    &Act_expr_parser::state04_error_handler, // 12 +
    &Act_expr_parser::state04_error_handler, // 13 +
    &Act_expr_parser::state06_error_handler, // 14 +
    &Act_expr_parser::state15_error_handler, // 15 +
    &Act_expr_parser::state04_error_handler, // 16 +
    &Act_expr_parser::state07_error_handler  // 17 +
};

/* In this array, the rules are collected for which convolution is performed in
 * error-handling functions. The number of the array element is the number of the
 * current state of the parser. If a state in the corresponding error-handling
 * function is not convoluted, then the element of this array with the corresponding
 * index is (-1). */
char reduce_rules[] = {
    -1,       -1,          -1,      -1,
    T_is_E,   E_is_F,      F_is_G,  G_is_H,
    H_is_d,   -1,          -1,      S_is_pTq,
    E_is_EF,  F_is_Gc,     G_is_Ha, -1,
    T_is_TbE, H_is_LP_T_RP
};

Parser_action_info Act_expr_parser::state00_error_handler()
{
    printf(opening_curly_brace_is_expected, esc_->lexem_begin_line_number());
    et_.ec->increment_number_of_errors();
    if(eli_.code != Expr_lexem_code::Closed_round_brack){
        esc_->back();
    }
    eli_.code = Expr_lexem_code::Begin_expression;
    Parser_action_info pa;
    pa.kind = Act_shift; pa.arg = 2;
    return pa;
}

Parser_action_info Act_expr_parser::state01_error_handler()
{
    Parser_action_info pa;
    pa.kind = Act_OK; pa.arg = 0;
    return pa;
}

Parser_action_info Act_expr_parser::state02_error_handler()
{
    printf(char_or_char_class_expected, esc_->lexem_begin_line_number());
    et_.ec->increment_number_of_errors();
    esc_->back();
    eli_.code = Expr_lexem_code::Character;
    eli_.c    = 'a';
    Parser_action_info pa;
    pa.kind = Act_shift; pa.arg = 8;
    return pa;
}

Parser_action_info Act_expr_parser::state03_error_handler()
{
    printf(or_operator_or_brace_expected, esc_->lexem_begin_line_number());
    et_.ec->increment_number_of_errors();
    if(t != Term_p){
        esc_->back();
    }
    eli_.code = Expr_lexem_code::Or;
    Parser_action_info pa;
    pa.kind = Act_shift; pa.arg = 10;
    return pa;
}

Parser_action_info Act_expr_parser::state04_error_handler()
{
    Rule r = static_cast<Rule>(reduce_rules[current_state]);
    Parser_action_info pa;
    switch(t){
        case Term_a:
            printf(unexpected_action, esc_->lexem_begin_line_number());
            pa.kind = Act_reduce; pa.arg = r;
            break;

        case Term_c:
            printf(unexpected_postfix_operator, esc_->lexem_begin_line_number());
            pa.kind = Act_reduce; pa.arg = r;
            break;

        case End_of_text:
            printf(unexpected_end_of_text, esc_->lexem_begin_line_number());
            pa.kind = Act_reduce; pa.arg = r;
            break;

        case Term_p:
            printf(unexpected_opening_brace, esc_->lexem_begin_line_number());
            pa.kind = Act_reduce_without_back; pa.arg = r;
            break;

        default:
            ;
    }
    return pa;
}

Parser_action_info Act_expr_parser::state06_error_handler()
{
    Rule r = static_cast<Rule>(reduce_rules[current_state]);
    Parser_action_info pa;
    switch(t){
        case Term_a:
            printf(unexpected_action, esc_->lexem_begin_line_number());
            pa.kind = Act_reduce_without_back; pa.arg = r;
            break;

        case Term_p:
            printf(unexpected_opening_brace, esc_->lexem_begin_line_number());
            pa.kind = Act_reduce_without_back; pa.arg = r;
            break;

        case End_of_text:
            printf(unexpected_end_of_text, esc_->lexem_begin_line_number());
            pa.kind = Act_reduce_without_back; pa.arg = r;
            break;

        default:
            ;
    }
    return pa;
}

Parser_action_info Act_expr_parser::state07_error_handler()
{
    Rule r = static_cast<Rule>(reduce_rules[current_state]);
    Parser_action_info pa;
    if(Term_p == t){
        printf(unexpected_opening_brace, esc_->lexem_begin_line_number());
        pa.kind = Act_reduce_without_back; pa.arg = r;
    }else{
        printf(unexpected_end_of_text, esc_->lexem_begin_line_number());
        pa.kind = Act_reduce_without_back; pa.arg = r;
    }
    et_.ec->increment_number_of_errors();
    return pa;
}

Parser_action_info Act_expr_parser::state11_error_handler()
{
    Parser_action_info pa;
    pa.kind = Act_reduce; pa.arg = S_is_pTq;
    return pa;
}

Parser_action_info Act_expr_parser::state15_error_handler()
{
    printf(or_operator_or_round_br_closed, esc_->lexem_begin_line_number());
    et_.ec->increment_number_of_errors();
    if(t != Term_p){
        esc_->back();
    }
    eli_.code = Expr_lexem_code::Or;
    Parser_action_info pa;
    pa.kind = Act_shift; pa.arg = 10;
    return pa;
}