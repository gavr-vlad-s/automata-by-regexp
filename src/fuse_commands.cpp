/*
    File:    fuse_commands.cpp
    Created: 24 August 2017 at 09:38 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include "../include/fuse_commands.h"
#include <cstddef>

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

void  fuse_ors(Command_buffer& out, const Command_buffer& inp, size_t curr_idx);

void  fuse_concats(Command_buffer& out, const Command_buffer& inp, size_t curr_idx);

void  fuse_multiors(Command_buffer& out, const Command_buffer& inp, size_t curr_idx);

void  fuse_multiconcats(Command_buffer& out, const Command_buffer& inp, size_t curr_idx);

void  fuse_commandsR(Command_buffer& out, const Command_buffer& inp, size_t curr_idx)
{
    Command      com = inp[curr_idx];
    Command_name n   = com.name;
    size_t       fst_idx, snd_idx;
    switch(n){
        case Command_name::Or:
            fuse_ors(out, inp, curr_idx);
            break;
        case Command_name::Concat:
            fuse_concats(out, inp, curr_idx);
            break;
        case Command_name::Kleene:   case Command_name::Positive:
        case Command_name::Optional:
            fuse_commandsR(out, inp, com.args.first);
            fst_idx        = out.size() - 1;
            com.args.first = fst_idx;
            out.push_back(com);
            break;
        case Command_name::Char:     case Command_name::Char_class:
        case Command_name::Unknown:  case Command_name::Char_class_complement:
            out.push_back(com);
            break;
        case Command_name::Multior:
            fuse_multiors(out, inp, curr_idx);
            break;
        case Command_name::Multiconcat:
            fuse_multiconcats(out, inp, curr_idx);
            break;
    }
}

void  fuse_ors(Command_buffer& out, const Command_buffer& inp, size_t curr_idx)
{}

void  fuse_concats(Command_buffer& out, const Command_buffer& inp, size_t curr_idx)
{}

void  fuse_multiors(Command_buffer& out, const Command_buffer& inp, size_t curr_idx)
{}

void  fuse_multiconcats(Command_buffer& out, const Command_buffer& inp, size_t curr_idx)
{}

Command_buffer fuse_commands(const Command_buffer& buf)
{
    Command_buffer result;
    if(!buf.empty()){
        fuse_commandsR(result, buf, buf.size() - 1);
    }
    return result;
}