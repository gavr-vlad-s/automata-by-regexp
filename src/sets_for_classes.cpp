/*
    File:    sets_for_classes.cpp
    Created: 12 February 2017 at 20:55 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include "../include/sets_for_classes.h"
#include <string>
static const std::u32string latin_upper_letters   = U"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const std::u32string latin_lower_letters   = U"abcdefghijklmnopqrstuvwxyz";
static const std::u32string russian_upper_letters = U"АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
static const std::u32string russian_lower_letters = U"абвгдеёжзийклмнопрстуфхцчшщъыьэюя";
static const std::u32string binary_digits         = U"01";
static const std::u32string octal_digits          = U"01234567";
static const std::u32string decimal_digits        = U"0123456789";
static const std::u32string hexadecimal_digits    = U"0123456789ABCDEFabcdef";
static const std::u32string upper_letters         =
    latin_upper_letters + russian_upper_letters;
static const std::u32string lower_letters =
    latin_lower_letters + russian_lower_letters;

/* This function builds a set consisting of str string characters. The representation
 * of the set is std :: set <char32_t>. */
static std::set<char32_t> u32string2set(const std::u32string& s)
{
    std::set<char32_t> result;
    for(const char32_t c : s){
        result.insert(c);
    }
    return result;
}

const std::set<char32_t> sets_for_char_classes[] = {
    u32string2set(latin_upper_letters),    u32string2set(upper_letters),
    u32string2set(russian_upper_letters),  u32string2set(binary_digits),
    u32string2set(decimal_digits),         u32string2set(latin_lower_letters),
    u32string2set(lower_letters),          u32string2set(octal_digits),
    u32string2set(russian_lower_letters),  u32string2set(hexadecimal_digits),
    std::set<char32_t>(),                  std::set<char32_t>()
};