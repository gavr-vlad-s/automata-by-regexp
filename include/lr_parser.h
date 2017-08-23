/*
    File:    lr_parser.h
    Created: 18 August 2017 at 17:42 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef LR_PARSER_H
#define LR_PARSER_H

#include <memory>
#include <map>
#include <utility>
#include <cstddef>
#include "../include/myconcepts.h"
#include "../include/multipop_stack.h"
#include "../include/types_for_lr_tables.h"
#include "../include/lr_traits.h"

template<typename G, typename S, typename Container>
class LR_parser{
public:
    using Lexem_type        = typename S::Lexem_t;
    using Non_terminal_type = typename G::Non_terminal_t;
    using Terminal_type     = typename G::Terminal_t;
    using Scaner_type       = typename S::Scaner_t;

    LR_parser<G, S, Container>()                                       = default;
    LR_parser<G, S, Container>(const LR_parser<G, S, Container>& orig) = default;

    LR_parser<G, S, Container>(const std::shared_ptr<Scaner_type>& scaner_,
                               const LR_parser_tables<G>&       tables) :
        scaner(scaner_)
    {
        rules        = tables.rules;
        goto_table   = tables.goto_table;
        action_table = tables.action_table;
    }

    virtual ~LR_parser<G, S, Container>()                              = default;

    void compile(Container& buf);

protected:
    using Rule_type         = typename G::Rule_t;
    using SE                = Stack_elem<Lexem_type>;

    size_t                              current_state;
    Lexem_type                          li;
    Terminal_type                       t;
    Multipop_stack<SE>                  parser_stack;
    std::shared_ptr<Scaner_type>        scaner;
    SE                                  rule_body[G::rule_max_len];
    Container                           buf_;

    virtual void                   generate_command(Rule_type r)       = 0;
    virtual Attributes<Lexem_type> attrib_calc(Rule_type r)            = 0;
    virtual Terminal_type          lexem2terminal(const Lexem_type& l) = 0;
    virtual Parser_action_info     error_hadling(size_t s)             = 0;

private:
    void shift(size_t shifted_state, Lexem_type e);
    void reduce_without_back(Rule_type r);
    void reduce(Rule_type r);

    Rule_info<Non_terminal_type>*       rules;
    GOTO_entry**                        goto_table;
    Parser_action_table<Terminal_type>* action_table;

    size_t next_state(size_t s, Non_terminal_type n);
};

template<typename G, typename S, typename Container>
size_t LR_parser<G, S, Container>::next_state(size_t s, Non_terminal_type n)
{
    size_t cs;
    GOTO_entry  current_entry;
    GOTO_entry* goto_for_n = goto_table[static_cast<size_t>(n)];
    while((cs = (current_entry = *goto_for_n++).from) != ANY){
        if(cs == s){
            return current_entry.to;
        }
    }
    goto_for_n--;
    return goto_for_n -> to;
}

template<typename G, typename S, typename Container>
void LR_parser<G, S, Container>::shift(size_t shifted_state, Lexem_type e)
{
    SE selem;
    selem.st_num  = shifted_state;
    selem.attr.li = e;
    parser_stack.push(selem);
}

template<typename G, typename S, typename Container>
void LR_parser<G, S, Container>::reduce_without_back(Rule_type r)
{
    auto   r_info   = rules[static_cast<size_t>(r)];
    size_t rule_len = r_info.len;
    parser_stack.get_elems_from_top(rule_body, rule_len);
    generate_command(r);

    SE se;
    se.attr         = attrib_calc(r);
    parser_stack.multi_pop(rule_len);
    SE top_elem     = parser_stack.top();
    se.st_num       = next_state(top_elem.st_num, r_info.nt);
    parser_stack.push(se);
}

template<typename G, typename S, typename Container>
void LR_parser<G, S, Container>::reduce(Rule_type r)
{
    reduce_without_back(r);
    scaner->back();
}

template<typename G, typename S, typename Container>
void LR_parser<G, S, Container>::compile(Container& buf)
{
    buf_ = buf;

    parser_stack.clear();

    SE initial_elem;
    initial_elem.st_num                   = 0;
    initial_elem.attr.indeces.begin_index = 0;
    initial_elem.attr.indeces.end_index   = 0;
    parser_stack.push(initial_elem);

    for( ; ; ){
        li = scaner->current_lexem();
        t = lexem2terminal(li);
        current_state = parser_stack.top().st_num;
        auto it = action_table->find({current_state, t});
        Parser_action_info pai;
        if(it != action_table->end()){
            pai = it->second;
        }else{
            pai = error_hadling(current_state);
        }
        switch(static_cast<Parser_action_name>(pai.kind)){
            case Parser_action_name::Reduce:
                reduce(static_cast<Rule_type>(pai.arg));
                break;
            case Parser_action_name::Shift:
                shift(pai.arg, li);
                break;
            case Parser_action_name::Reduce_without_back:
                reduce_without_back(static_cast<Rule_type>(pai.arg));
                break;
            case Parser_action_name::OK:
                buf = buf_;
                scaner->back();
                return;
        }
    }
}
#endif