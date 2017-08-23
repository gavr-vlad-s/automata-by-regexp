/*
    File:    print_commands.cpp
    Created: 04 August 2017 at 11:28 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <cstdio>
#include "../include/print_commands.h"
#include "../include/operations_with_sets.h"
#include "../include/print_char32.h"

// enum class Command_name{
//     Or,                    Concat,  Kleene,     Positive,
//     Optional,              Char,    Char_class, Unknown,
//     Char_class_complement, Multior, Multiconcat
// };
//
// struct Command{
//     size_t action_name; /* The index of the identifier that is the name of
//                          * the action, in the prefix tree of identifiers. */
//     Command_name name;
//     union{
//         struct {
//             size_t first, second;
//         } args;
//         char32_t   c;
//         size_t     idx_of_set;
//     };
// };

static const char* command_names[] = {
    "Or",                    "Concat",  "Kleene",     "Positive",
    "Optional",              "Char",    "Char_class", "Unknown",
    "Char_class_complement", "Multior", "Multiconcat"
};

void print_command(const Command& e, const Trie_for_set_of_char32ptr& t)
{
    printf("%s, ", command_names[static_cast<size_t>(e.name)]);
    printf("action_name: %zu ",e.action_name);
    switch(e.name){
        case Command_name::Or:         case Command_name::Concat:
        case Command_name::Multior:    case Command_name::Multiconcat:
        case Command_name::Kleene:     case Command_name::Positive:
        case Command_name::Optional:
            {
                auto args = e.args;
                printf("{first_arg: %zu, second_arg: %zu}", args.first, args.second);
            }
            break;
        case Command_name::Char:
            print_char32(e.c);
            break;
        case Command_name::Char_class: case Command_name::Char_class_complement:
            {
                const auto& s = t->get_set(e.idx_of_set);
                operations_with_sets::print_set(s, print_char32);
            }
            break;
        default:
            ;
    }
}

void print_commands(const Command_buffer& buf, const Trie_for_set_of_char32ptr& t)
{
    size_t i = 0;
    for(const auto& e : buf){
        printf("%04zu: ", i++);
        print_command(e, t);
        putchar('\n');
    }
}