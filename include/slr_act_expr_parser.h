/*
    File:    slr_act_expr_parser.h
    Created: 19 August 2017 at 10:12 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef SLR_ACT_EXPR_PARSER_H
#define SLR_ACT_EXPR_PARSER_H

#include <memory>
#include "../include/command.h"
#include "../include/expr_scaner.h"
#include "../include/expr_lexem_info.h"
#include "../include/scope.h"
#include "../include/errors_and_tries.h"
#include "../include/lr_parser.h"
#include "../include/expr_traits.h"
#include "../include/expr_lr_tables.h"
#include "../include/types_for_lr_tables.h"

using Concrete_LR_parser =
    LR_parser<Expr_grammar_traits, Expr_scaner_traits, Command_buffer>;

class SLR_act_expr_parser : public Concrete_LR_parser{
public:
    SLR_act_expr_parser()                                = default;
    virtual ~SLR_act_expr_parser()                       = default;
    SLR_act_expr_parser(const SLR_act_expr_parser& orig) = default;

    SLR_act_expr_parser(const Expr_scaner_ptr&                       esc,
                        const Errors_and_tries&                      et,
                        const std::shared_ptr<Scope>&                scope,
                        const LR_parser_tables<Expr_grammar_traits>& tables) :
       Concrete_LR_parser(esc, tables), scope_(scope),  et_(et) {};

private:
    std::shared_ptr<Scope>       scope_;
    Errors_and_tries             et_;

    using Error_handler = Parser_action_info (SLR_act_expr_parser::*)();
    static Error_handler error_hadler[];

    using Attrib_calculator = Attributes<Lexem_type> (SLR_act_expr_parser::*)();
    static Attrib_calculator attrib_calculator[];

    Attributes<Lexem_type> attrib_by_S_is_pTq();
    Attributes<Lexem_type> attrib_by_T_is_TbE();
    Attributes<Lexem_type> attrib_by_T_is_E();
    Attributes<Lexem_type> attrib_by_E_is_EF();
    Attributes<Lexem_type> attrib_by_E_is_F();
    Attributes<Lexem_type> attrib_by_F_is_Gc();
    Attributes<Lexem_type> attrib_by_F_is_G();
    Attributes<Lexem_type> attrib_by_G_is_Ha();
    Attributes<Lexem_type> attrib_by_G_is_H();
    Attributes<Lexem_type> attrib_by_H_is_d();
    Attributes<Lexem_type> attrib_by_H_is_LP_T_RP();

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

protected:
    virtual void                   generate_command(Rule_type r);
    virtual Attributes<Lexem_type> attrib_calc(Rule_type r);
    virtual Terminal_type          lexem2terminal(const Lexem_type& l);
    virtual Parser_action_info     error_hadling(size_t s);
};
#endif