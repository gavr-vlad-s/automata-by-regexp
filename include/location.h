/*
    File:    location.h
    Created: 13 December 2015 at 09:05 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef LOCATION_H
#define LOCATION_H

#include <memory>
/* The following structure describes the current position in the processed text.
 * This is due to the fact that, due to the conflict of the lexem 'identifier'
 * and the lexem 'character', instead of one scanner, two must be done: the main
 * scanner and the scanner for regular expressions. In this case, the main parser
 * will call parsers for regular expressions, which, in turn, will call the scanner
 * for regular expressions. Therefore, both of these scanners need to know the
 * current position in the current text and be able to manipulate this position,
 * and each of these scanners must continue to work from the place in the processed
 * text, on which the previous scanner finished work. As a consequence, the information
 * about the current position in the processed text should be shared by both scanners,
 * so that a smart pointer to the shared information about the current location should
 * be sent to the constructor of each of the scanners.
 */

struct Location {
    char32_t* pcurrent_char; ///< pointer to the current character
    size_t    current_line;  ///< number of current line

    Location() : pcurrent_char(nullptr), current_line(1) {};
    Location(char32_t* txt) : pcurrent_char(txt), current_line(1) {};
};

using Location_ptr = std::shared_ptr<Location>;
#endif