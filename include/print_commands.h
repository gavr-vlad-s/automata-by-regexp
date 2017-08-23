/*
    File:    print_commands.h
    Created: 04 August 2017 at 11:22 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef PRINT_COMMANDS_H
#define PRINT_COMMANDS_H
#include "../include/command.h"
#include "../include/trie_for_set.h"
void print_commands(const Command_buffer& buf, const Trie_for_set_of_char32ptr& t);
#endif