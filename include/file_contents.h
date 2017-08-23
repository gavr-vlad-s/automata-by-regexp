/*
    File:    file_contents.h
    Author:  Гаврилов Владимир Сергеевич
    Created: 4 February 2016, 13:10
    e-mails: vladimir.s.gavrilov@gmail.com,
             gavrilov.vladimir.s@mail.ru,
             gavvs1977@yandex.ru
*/

#ifndef FILE_CONTENTS_H
#define FILE_CONTENTS_H
#include <string>
#include <utility>

/** Return codes from the function get_contents. */
enum class Get_contents_return_code{
    Normal,           ///< This code means success.
    Impossible_open,  ///< This code means that file could not open.
    Read_error        ///< This code means that an error occurred while reading the file.
};

using Contents  = std::pair<Get_contents_return_code, std::string>;

/**
   Returns: the contents of the file with the specified name
   \param [in] name file name
   \returns The pair (return code, value), here value is of type std::string.
   If an error occured, then the second component of this pair is an empty string.
*/
Contents get_contents(const char* name);
#endif