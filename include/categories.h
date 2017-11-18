/*
    File:    categories.h
    Created: 10 January 2015 at 10:32 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/
#ifndef CATEGORIES_H
#define CATEGORIES_H

#include "../include/symbol.h"
#include "../include/trie_for_set.h"
#include <set>

using Set_of_char = std::set<char32_t>;

enum class Category_kind{
    All_chars, Set_of_cs, Set_of_cs_complement
};

struct Category{
    Category_kind kind = Category_kind::Set_of_cs;
    Set_of_char   s;
};

Category symbol2category(const Symbol& symb, const Trie_for_set_of_char32ptr& t);

bool operator < (const Category& c1, const Category& c2);

bool operator == (const Category& c1, const Category& c2);

/* Calculation of the union of character categories. */
Category operator + (const Category& c1, const Category& c2);

/* If categories c1 and c2 intersect, then the returned value is true;
 * else the returned value is false. */
bool operator * (const Category& c1, const Category& c2);

/* If the category c1 is contained in the category c2, then the returned value
 * is true, else the returned value is false.
 */
bool is_subcategory(const Category& c1, const Category& c2);

void print_category(const Category& c);
#endif