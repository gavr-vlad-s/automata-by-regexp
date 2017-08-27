/*
    File:    fuse_commands.cpp
    Created: 24 August 2017 at 09:38 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <cstddef>
#include "../include/fuse_commands.h"
#include "../include/belongs.h"

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

static const size_t leaves =
    (1ULL << static_cast<size_t>(Command_name::Char))                  |
    (1ULL << static_cast<size_t>(Command_name::Unknown))               |
    (1ULL << static_cast<size_t>(Command_name::Char_class))            |
    (1ULL << static_cast<size_t>(Command_name::Char_class_complement));

static inline bool is_leaf(const Command& c)
{
    return belongs(static_cast<uint64_t>(c.name), leaves);
}

using Combuf = Command_buffer;

static void fuse_or(Combuf& out, const Combuf& inp, size_t curr_idx);

static void fuse_concat(Combuf& out, const Combuf& inp, size_t curr_idx);

static void  fuse_commandsR(Combuf& out, const Combuf& inp, size_t curr_idx)
{
    Command      com = inp[curr_idx];
    Command_name n   = com.name;
    size_t       fst_idx;
    switch(n){
        case Command_name::Or:      case Command_name::Multior:
            fuse_or(out, inp, curr_idx);
            break;
        case Command_name::Concat:  case Command_name::Multiconcat:
            fuse_concat(out, inp, curr_idx);
            break;
        case Command_name::Kleene:  case Command_name::Positive:
        case Command_name::Optional:
            fuse_commandsR(out, inp, com.args.first);
            fst_idx        = out.size() - 1;
            com.args.first = fst_idx;
            out.push_back(com);
            break;
        case Command_name::Char:    case Command_name::Char_class:
        case Command_name::Unknown: case Command_name::Char_class_complement:
            out.push_back(com);
            break;
    }
}

static void fuse_or(Combuf& out, const Combuf& inp, size_t curr_idx)
{
    Command      com           = inp[curr_idx];
    auto         args          = com.args;
    size_t       fst_idx       = args.first;
    size_t       snd_idx       = args.second;
    bool         left_is_leaf  = is_leaf(inp[fst_idx]);
    bool         right_is_leaf = is_leaf(inp[snd_idx]);

    if(left_is_leaf){
        size_t       s         = out.size();
        fuse_commandsR(out, inp, fst_idx);
        fuse_commandsR(out, inp, snd_idx);
        if(right_is_leaf){
            com.args.first   = s;
            com.args.second  = s + 1;
            com.name         = Command_name::Multior;
            out.push_back(com);
            return;
        }
        Command snd_arg        = out.back();
        if(Command_name::Multior == snd_arg.name){
            out.back().args.first = s;
            return;
        }
        com.args.first  = s;
        com.args.second = out.size() - 1;
        com.name        = Command_name::Or;
        out.push_back(com);
        return;
    }

    fuse_commandsR(out, inp, fst_idx);
    Command fst_arg         = out.back();
    if(Command_name::Multior == fst_arg.name){
        if(right_is_leaf){
            com.args        = fst_arg.args;
            com.name        = Command_name::Multior;
            com.args.second = out.size() - 1;
            out.back()      = inp[snd_idx];
            out.push_back(com);
            return;
        }
        size_t s             = out.size() - 1;
        fuse_commandsR(out, inp, snd_idx);
        Command      snd_arg = out.back();
        if(Command_name::Multior == snd_arg.name){
            com.args        = out[s].args;
            out.erase(out.begin() + s);
            com.args.second = snd_arg.args.second - 1;
            com.name        = Command_name::Multior;
            out.push_back(com);
            return;
        }
        com.args.first  = s;
        fuse_commandsR(out, inp, snd_idx);
        com.args.second = out.size() - 1;
        com.name        = Command_name::Or;
        out.push_back(com);
        return;
    }

    com.args.first  = out.size() - 1;
    fuse_commandsR(out, inp, snd_idx);
    com.args.second = out.size() - 1;
    com.name        = Command_name::Or;
    out.push_back(com);
}

static void fuse_concat(Combuf& out, const Combuf& inp, size_t curr_idx)
{
    Command      com           = inp[curr_idx];
    auto         args          = com.args;
    size_t       fst_idx       = args.first;
    size_t       snd_idx       = args.second;
    bool         left_is_leaf  = is_leaf(inp[fst_idx]);
    bool         right_is_leaf = is_leaf(inp[snd_idx]);

    if(left_is_leaf){
        size_t       s         = out.size();
        fuse_commandsR(out, inp, fst_idx);
        fuse_commandsR(out, inp, snd_idx);
        if(right_is_leaf){
            com.args.first   = s;
            com.args.second  = s + 1;
            com.name         = Command_name::Multiconcat;
            out.push_back(com);
            return;
        }
        Command      snd_arg = out.back();
        if(Command_name::Multiconcat == snd_arg.name){
            out.back().args.first = s;
            return;
        }
        com.args.first  = s;
        com.args.second = out.size() - 1;
        com.name        = Command_name::Concat;
        out.push_back(com);
        return;
    }

    fuse_commandsR(out, inp, fst_idx);
    Command         fst_arg = out.back();
    if(Command_name::Multiconcat == fst_arg.name){
        if(right_is_leaf){
            com.args        = fst_arg.args;
            com.name        = Command_name::Multiconcat;
            com.args.second = out.size() - 1;
            out.back()      = inp[snd_idx];
            out.push_back(com);
            return;
        }
        size_t s             = out.size() - 1;
        fuse_commandsR(out, inp, snd_idx);
        Command      snd_arg = out.back();
        if(Command_name::Multiconcat == snd_arg.name){
            com.args        = out[s].args;
            out.erase(out.begin() + s);
            com.args.second = snd_arg.args.second - 1;
            com.name        = Command_name::Multiconcat;
            out.push_back(com);
            return;
        }
        com.args.first  = s;
        fuse_commandsR(out, inp, snd_idx);
        com.args.second = out.size() - 1;
        com.name        = Command_name::Concat;
        out.push_back(com);
        return;
    }

    com.args.first  = out.size() - 1;
    fuse_commandsR(out, inp, snd_idx);
    com.args.second = out.size() - 1;
    com.name        = Command_name::Concat;
    out.push_back(com);
}

Command_buffer fuse_commands(const Command_buffer& buf)
{
    Command_buffer result;
    if(!buf.empty()){
        fuse_commandsR(result, buf, buf.size() - 1);
    }
    return result;
}