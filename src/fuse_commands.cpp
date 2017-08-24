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
    return belongs(c.name, leaves);
}

static void insert_command_args(Command_buffer& out, const Command_buffer& inp, size_t idx)
{
    Command com    = inp[idx];
    auto    args   = com.args;
    auto    inpb   = inp.begin();
    auto    fst_it = inpb + args.first;
    auto    snd_it = inpb + args.second + 1;
    out.insert(out.end(), fst_it, snd_it);
}

static void  fuse_oc(Command_buffer& out,      const Command_buffer& inp,
                     size_t          curr_idx, Command_name          cn);

static void  fuse_commandsR(Command_buffer& out, const Command_buffer& inp, size_t curr_idx)
{
    Command      com = inp[curr_idx];
    Command_name n   = com.name;
    size_t       fst_idx, snd_idx;
    switch(n){
        case Command_name::Or:      case Command_name::Multior:
            fuse_oc(out, inp, curr_idx, Command_name::Multior);
            break;
        case Command_name::Concat:  case Command_name::Multiconcat:
            fuse_oc(out, inp, curr_idx, Command_name::Multiconcat);
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

static void  fuse_oc(Command_buffer& out,      const Command_buffer& inp,
                     size_t          curr_idx, Command_name          cn)
{
    Command      com     = inp[curr_idx];
    Command_name n       = com.name;
    auto         args    = com.args;
    size_t       fst_idx = args.first;
    size_t       snd_idx = args.second;
    auto         fst_arg = inp[fst_idx];
    auto         snd_arg = inp[snd_idx];
    Command_name n1      = fst_arg.name;
    Command_name n2      = snd_arg.name;
    bool         m1      = cn == n1;
    bool         m2      = cn == n2;
    bool         t1      = is_leaf(fst_arg) || m1;
    bool         t2      = is_leaf(snd_arg) || m2;

    if(!(t1 && t2)){
        fuse_commandsR(out, inp, fst_idx);
        com.args.first   = out.size() - 1;
        fuse_commandsR(out, inp, snd_idx);
        com.args.second  = out.size() - 1;
        out.push_back(com);
        return;
    }

    enum class Args_kinds{
        Leaf_leaf, Leaf_m, M_leaf, M_m
    };

    Args_kinds k = static_cast<Args_kinds>(m1 * 2 + m2);
    switch(k){
        case Leaf_leaf:
            fuse_commandsR(out, inp, fst_idx);
            com.args.first   = out.size() - 1;
            fuse_commandsR(out, inp, snd_idx);
            break;
        case Leaf_m:
            com.args.first   = out.size();
            fuse_commandsR(out, inp, fst_idx);
            insert_command_args(out, inp, snd_idx);
            break;
        case M_leaf:
            com.args.first   = out.size();
            insert_command_args(out, inp, fst_idx);
            fuse_commandsR(out, inp, snd_idx);
            break;
        case M_m:
            com.args.first   = out.size();
            insert_command_args(out, inp, fst_idx);
            insert_command_args(out, inp, snd_idx);
            break;
    }
    com.args.second  = out.size() - 1;
    com.name         = cn;
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