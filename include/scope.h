/*
    File:    scope.h
    Created: 13 December 2015 at 09:05 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef SCOPE_H
#define SCOPE_H

#include <map>
enum Id_kind{
    Scaner_name,   Codes_type_name, Lexem_ident_name,
    Code_of_lexem, Action_name
};

enum Str_kind {
    Keyword_repres,             Delimiter_repres,
    Added_members,              Added_token_fiels,
    Action_definition,          Single_lined_comment_begin,
    Multi_lined_comment_begin,  Multi_lined_comment_end
};

struct Id_attributes{
    /**
     * The kind of identifier, that is, whether this identifier is the name of the
     * scanner, the name of the type of lexeme codes, the lexeme code, or the name
     * of the action.
     */
    Id_kind kind;
    union {
        /**
         * The numerical value of the lexeme code,
         * if the identifier is the lexem code.
         */
        size_t code;
        /**
         * The index of the string literal, which is the definition of the action,
         * in the prefix tree of string literals, if the identifier is the name of
         * the action.
         */
        size_t act_string;
    };
};

struct Str_attributes{
    /**
     * The type of string literal, that is, whether it is a string representation of
     * the keyword, a string representation of the delimiter, a list of members added
     * to the scanner class, an action definition, the beginning of a single-line
     * comment, the beginning of a multi-line comment, or the end of a multi-line
     * comment.
     */
    Str_kind kind;
    /**
     * The lexeme code, if the string literal is a string representation of
     * a keyword or a delimiter.
     */
    size_t code;
};

using Id_scope = std::map<size_t, Id_attributes>;

using Str_scope = std::map<size_t, Str_attributes>;

class Scope {
public:
    /// Mapping of indeces of identifiers in the attributes of identifiers.
    Id_scope  idsc;
    /// Mapping of indeces of string literals in the attributes of string literals.
    Str_scope strsc;

    Scope()                  = default;
    ~Scope()                 = default;
    Scope(const Scope& orig) = default;
};
#endif