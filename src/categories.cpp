/*
    File:    categories.cpp
    Created: 10 January 2015 at 10:47 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <string>
#include <map>
#include <utility>
#include "../include/categories.h"
#include "../include/operations_with_sets.h"
#include "../include/sets_for_classes.h"

#define DEBUG_MODE
#ifdef DEBUG_MODE
#  include <cstdio>
#  include "../include/char_conv.h"    // For debug printing.
#  include "../include/print_char32.h" // For debug printing.
#endif

using operations_with_sets::operator+;
using operations_with_sets::operator-;
using operations_with_sets::operator*;
using operations_with_sets::is_elem;
using operations_with_sets::is_subseteq;

#ifdef DEBUG_MODE
using operations_with_sets::print_set; // For debug printing.

static const char* category_kind_as_str[] = {
    "All_chars", "Set_of_cs", "Set_of_cs_complement"
};

void print_category(const Category& c){
    auto k = c.kind;
    printf("%s", category_kind_as_str[static_cast<unsigned>(k)]);
    switch(k){
        case Category_kind::Set_of_cs:
        case Category_kind::Set_of_cs_complement:
            putchar(' ');
            print_set(c.s, print_char32);
            break;
        default:
            ;
    }
}
#endif

Category symbol2category(const Symbol& symb, const Trie_for_set_of_char32ptr& t){
    Category categ;
    Symbol_kind k = symb.kind;
    switch(k){
        case Symbol_kind::Char:
            categ.kind = Category_kind::Set_of_cs;
            categ.s    = Set_of_char({symb.c});
            break;
        case Symbol_kind::Char_class:
            categ.kind = Category_kind::Set_of_cs;
            categ.s    = t->get_set(symb.idx_of_set);
            break;
        case Symbol_kind::Class_complement:
            categ.kind = Category_kind::Set_of_cs_complement;
            categ.s = t->get_set(symb.idx_of_set);
            break;
        case Symbol_kind::All:
            categ.kind = Category_kind::All_chars;
            categ.s    = Set_of_char();
            break;
        default:
            ;
    }
    return categ;
}

/* The operation of comparing character categories to equality is defined as follows:
   1) if c1.kind != c1.kind, then categories are not equal;
   2) if (c1.kind == c2.kind) && (c1.kind != Set_of_cs), then categories are equal;
   3) in other cases, the categories are equal if and only if (c1.s == c2.s).
*/
bool operator == (const Category& c1, const Category& c2){
    return (c1.kind == c2.kind) && ((c1.kind != Category_kind::Set_of_cs) || (c1.s == c2.s));
}

/* The order relation (<) on the set of character categories is defined as follows:
   1) if (c1.kind != c2.kind), then c1 < c2 is equivalent to (c1.kind < c2.kind);
   2) if (c1.kind == c2.kind) && ((c1.kind == Set_of_cs) || (c1.kind == Set_of_cs_complement)),
      then is not true that c1 < c2;
   3) in other cases, c1 < c2 if and only if (c1.s < c2.s)
      (in the sense of STL-container std::set).
*/
bool operator < (const Category& c1, const Category& c2){
    if(c1.kind != c2.kind){
        return c1.kind < c2.kind;
    }else{
        return ((c1.kind == Category_kind::Set_of_cs) ||
                (c1.kind == Category_kind::Set_of_cs_complement)) &&
               (c1.s < c2.s);
    }
}

using Categories_pair = std::pair<Category_kind, Category_kind>;

static const std::map<Categories_pair, Category_kind> addition_table = {
    {{Category_kind::All_chars, Category_kind::All_chars},
     Category_kind::All_chars},
    {{Category_kind::All_chars, Category_kind::Set_of_cs},
     Category_kind::All_chars},
    {{Category_kind::All_chars, Category_kind::Set_of_cs_complement},
     Category_kind::All_chars},
    {{Category_kind::Set_of_cs, Category_kind::All_chars},
     Category_kind::All_chars},
    {{Category_kind::Set_of_cs_complement, Category_kind::All_chars},
     Category_kind::All_chars},
};

Category operator + (const Category& c1, const Category& c2){
    Category result;
    Category_kind k1 = c1.kind;
    Category_kind k2 = c2.kind;
    auto ait = addition_table.find(std::make_pair(k1, k2));
    if(ait != addition_table.end()){
        result.kind = ait->second;
        result.s    = Set_of_char();
    }else{
        enum Branch{
            Not_set_and_not_set, Not_set_and_set,
            Set_and_not_set,     Set_and_set
        };
        bool   k1_is_set = Category_kind::Set_of_cs == k1;
        bool   k2_is_set = Category_kind::Set_of_cs == k2;
        Branch b         = static_cast<Branch>(k1_is_set * 2 + k2_is_set);
        switch(b){
            case Not_set_and_not_set:
                {
                    auto z      = c1.s * c2.s;
                    result.s    = z;
                    result.kind = (z.empty()) ? Category_kind::All_chars :
                                                Category_kind::Set_of_cs_complement;
                }
                break;
            case Not_set_and_set:
                {
                    auto z      = c1.s - c2.s;
                    result.s    = z;
                    result.kind = (z.empty()) ? Category_kind::All_chars :
                                                Category_kind::Set_of_cs_complement;
                }
                break;
            case Set_and_not_set:
                {
                    auto z      = c2.s - c1.s;
                    result.s    = z;
                    result.kind = (z.empty()) ? Category_kind::All_chars :
                                                Category_kind::Set_of_cs_complement;
                }
                break;
            case Set_and_set:
                result.kind = Category_kind::Set_of_cs;
                result.s    = c1.s + c2.s;
                break;
            default:
                ;
        }
    }
    return result;
}

bool operator * (const Category& c1, const Category& c2){
    bool t = true;
    Category_kind k1 = c1.kind;
    Category_kind k2 = c2.kind;

    enum class Branch{
        All_and_all,   All_and_set,   All_and_compl,
        Set_and_all,   Set_and_set,   Set_and_compl,
        Compl_and_all, Compl_and_set, Compl_and_compl
    };

    Branch b = static_cast<Branch>(3 * static_cast<unsigned>(k1) +
                                   static_cast<unsigned>(k2));

    switch(b){
        case Branch::All_and_set:
            t = !(c2.s.empty());
            break;
        case Branch::Set_and_all:
            t = !(c1.s.empty());
            break;
        case Branch::Set_and_set:
            t = !((c1.s * c2.s).empty());
            break;
        case Branch::Set_and_compl:
            t = !((c1.s - c2.s).empty());
            break;
        case Branch::Compl_and_set:
            t = !((c2.s - c1.s).empty());
            break;
        case Branch::Compl_and_compl:
            {
                auto z = c1 + c2;
                t      = z.kind != Category_kind::All_chars;
            }
            break;
        default:
            return t;
    }
    return t;
}

static const std::map<Categories_pair, bool> subcategory_table = {
    {{Category_kind::All_chars,            Category_kind::All_chars},            true},
    {{Category_kind::All_chars,            Category_kind::Set_of_cs},            false},
    {{Category_kind::All_chars,            Category_kind::Set_of_cs_complement}, false},
    {{Category_kind::Set_of_cs_complement, Category_kind::All_chars},            true},
    {{Category_kind::Set_of_cs,            Category_kind::All_chars},            true}
};

bool is_subcategory(const Category& c1, const Category& c2){
    bool t = true;
    Category_kind k1 = c1.kind;
    Category_kind k2 = c2.kind;

    auto it = subcategory_table.find(std::make_pair(k1, k2));
    if(it != subcategory_table.end()){
        t = it->second;
    }else{
        enum Branch{
            Not_set_and_not_set, Not_set_and_set,
            Set_and_not_set,     Set_and_set
        };
        bool   k1_is_set = Category_kind::Set_of_cs == k1;
        bool   k2_is_set = Category_kind::Set_of_cs == k2;
        Branch b         = static_cast<Branch>(k1_is_set * 2 + k2_is_set);
        switch(b){
            case Not_set_and_not_set:
                is_subseteq(c2.s, c1.s);
                break;
            case Not_set_and_set:      case Set_and_not_set:
                t = (c1.s * c2.s).empty();
                break;
            case Set_and_set:
                t = is_subseteq(c1.s, c2.s);
                break;
            default:
                ;
        }
    }
    return t;
}