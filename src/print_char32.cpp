/*
    File:    print_char32.cpp
    Created: 19 January 2017 at 10:40 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <string>
#include <map>
#include "../include/print_char32.h"
#include "../include/char_conv.h"

static const std::map<char32_t, std::string> esc_char_strings = {
    {U'\'', R"~(')~"},        {U'\"', R"~(")~"},        {U'\?', R"~('?')~"},
    {U'\\', R"~('\')~"},      {U'\0', R"~('\0')~"},     {U'\a', R"~('\a')~"},
    {U'\b', R"~('\b')~"},     {U'\f', R"~('\f')~"},     {U'\n', R"~('\n')~"},
    {U'\r', R"~('\r')~"},     {U'\t', R"~('\t')~"},     {U'\v', R"~('\v')~"},
    {U'\x01', R"~('\X01')~"}, {U'\x02', R"~('\X02')~"}, {U'\x03', R"~('\X03')~"},
    {U'\x04', R"~('\X04')~"}, {U'\x05', R"~('\X05')~"}, {U'\x06', R"~('\X06')~"},
    {U'\x0e', R"~('\X0e')~"}, {U'\x0f', R"~('\X0f')~"}, {U'\x10', R"~('\X10')~"},
    {U'\x11', R"~('\X11')~"}, {U'\x12', R"~('\X12')~"}, {U'\x13', R"~('\X13')~"},
    {U'\x14', R"~('\X14')~"}, {U'\x15', R"~('\X15')~"}, {U'\x16', R"~('\X16')~"},
    {U'\x17', R"~('\X17')~"}, {U'\x18', R"~('\X18')~"}, {U'\x19', R"~('\X19')~"},
    {U'\x1a', R"~('\X1a')~"}, {U'\x1b', R"~('\X1b')~"}, {U'\x1c', R"~('\X1c')~"},
    {U'\x1d', R"~('\X1d')~"}, {U'\x1e', R"~('\X1e')~"}, {U'\x1f', R"~('\X1f')~"}
};

std::string show_char32(const char32_t c)
{
    std::string result;
    auto it = esc_char_strings.find(c);
    if(it != esc_char_strings.end()){
        result = it->second;
    }else{
        result = '\'' + char32_to_utf8(c) + '\'';
    }
    return result;
}

void print_char32(const char32_t c)
{
    std::string s = show_char32(c);
    printf("%s", s.c_str());
}