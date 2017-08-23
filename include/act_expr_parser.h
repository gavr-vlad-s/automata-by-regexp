/*
    File:    act_expr_parser.h
    Created: 13 December 2015 at 09:05 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef ACT_EXPR_PARSER_H
#define ACT_EXPR_PARSER_H

#include <memory>
#include "../include/multipop_stack.h"
#include "../include/command.h"
#include "../include/expr_scaner.h"
#include "../include/scope.h"
#include "../include/errors_and_tries.h"
#include "../include/stack_elem.h"

enum Rule{
    S_is_pTq, T_is_TbE,     T_is_E,
    E_is_EF,  E_is_F,       F_is_Gc,
    F_is_G,   G_is_Ha,      G_is_H,
    H_is_d,   H_is_LP_T_RP
};

enum Non_terminal : uint8_t{
    Nt_S, Nt_T, Nt_E,
    Nt_F, Nt_G, Nt_H
};

enum Terminal{
    End_of_text, Term_a,  Term_b,
    Term_c,      Term_d,  Term_p,
    Term_q,      Term_LP, Term_RP
};

Terminal lexem2terminal(const Expr_lexem_info& l);

enum Parser_action_name{
    Act_OK, Act_shift, Act_reduce, Act_reduce_without_back
};

struct Parser_action_info{
    uint8_t kind;
    uint8_t arg;
};

enum Number_or_string {
    Number_expr, String_expr
};

#define MAXIMAL_LENGTH_OF_RULE 3
#define NUMBERS_OF_RULES (H_is_LP_T_RP + 1)
class Act_expr_parser{
public:
    Act_expr_parser()                            = default;
    ~Act_expr_parser()                           = default;
    Act_expr_parser(const Act_expr_parser& orig) = default;
    Act_expr_parser(Expr_scaner_ptr&        esc,
                    const Errors_and_tries& et,
                    std::shared_ptr<Scope>& scope);
    void compile(Command_buffer& buf, Number_or_string kind_of_expr);
private:
    Command_buffer buf_;

    Multipop_stack<Stack_elem> parser_stack;

    Expr_lexem_info eli_;
    Terminal        t;

    std::shared_ptr<Expr_scaner> esc_;
    std::shared_ptr<Scope>       scope_;
    Errors_and_tries             et_;

    size_t current_state;

    void shift(size_t shifted_state, Expr_lexem_info e);
    void reduce(Rule r);
    void reduce_without_back(Rule r);

    typedef Parser_action_info (Act_expr_parser::*Error_handler)();
    static Error_handler error_hadler[];

    struct Rule_info{
        Non_terminal nt;  /* target for reducing */
        uint8_t      len; /* rule length */
    };
    static Rule_info rules[];

    Stack_elem rule_body[MAXIMAL_LENGTH_OF_RULE];

    typedef Attributes (Act_expr_parser::*Attrib_calculator)();
    static Attrib_calculator attrib_calc[];

    Attributes attrib_by_S_is_pTq();
    Attributes attrib_by_T_is_TbE();
    Attributes attrib_by_T_is_E();
    Attributes attrib_by_E_is_EF();
    Attributes attrib_by_E_is_F();
    Attributes attrib_by_F_is_Gc();
    Attributes attrib_by_F_is_G();
    Attributes attrib_by_G_is_Ha();
    Attributes attrib_by_G_is_H();
    Attributes attrib_by_H_is_d();
    Attributes attrib_by_H_is_LP_T_RP();

    void generate_command(Rule r);
    void generate_by_G_is_Ha();
    void generate_by_H_is_d();
    void generate_by_F_is_Gc();
    void generate_E_is_EF();
    void generate_by_T_is_TbE();

    Parser_action_info state00_error_handler();
    Parser_action_info state01_error_handler();
    Parser_action_info state02_error_handler();
    Parser_action_info state03_error_handler();
    Parser_action_info state04_error_handler();
    Parser_action_info state06_error_handler();
    Parser_action_info state07_error_handler();
    Parser_action_info state11_error_handler();
    Parser_action_info state15_error_handler();

    typedef void (Act_expr_parser::*Checker)(Expr_lexem_info);
    Checker checker;

    void checker_for_number_expr(Expr_lexem_info e);
    void checker_for_string_expr(Expr_lexem_info e);
};
#endif