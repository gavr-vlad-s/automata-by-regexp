/*
    File:    errors_and_tries.h
    Created: 06 November 2016 at 12:20 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef ERRORS_AND_TRIES_H
#define ERRORS_AND_TRIES_H

#include "../include/error_count.h"
#include "../include/char_trie.h"
#include <memory>
struct Errors_and_tries{
    std::shared_ptr<Error_count>  ec;
    std::shared_ptr<Char_trie>    ids_trie;
    std::shared_ptr<Char_trie>    strs_trie;

    Errors_and_tries()  = default;
    ~Errors_and_tries() = default;
};
#endif