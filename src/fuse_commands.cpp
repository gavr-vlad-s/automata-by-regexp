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

// static void insert_command_args(Combuf& out, const Combuf& inp, size_t idx)
// {
//     Command com    = inp[idx];
//     auto    args   = com.args;
//     auto    inpb   = inp.begin();
//     auto    fst_it = inpb + args.first;
//     auto    snd_it = inpb + args.second + 1;
//     out.insert(out.end(), fst_it, snd_it);
// }

// static void  fuse_oc(Combuf& out,      const Combuf& inp,
//                      size_t          curr_idx, Command_name          cn);

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

static void fuse_bufs(Combuf& inp1, Combuf& inp2)
{
    size_t s1 = inp1.size();
    for(auto c : inp2){
        switch(c.name){
            case Command_name::Or:       case Command_name::Multior:
            case Command_name::Concat:   case Command_name::Multiconcat:
                c.args.first  += s1;
                c.args.second += s1;
                break;
            case Command_name::Kleene:   case Command_name::Positive:
            case Command_name::Optional:
                c.args.first  += s1;
                break;
            default:
                ;
        }
    }
    inp1.insert(inp1.end(), inp2.begin(), inp2.end());
}

static void fuse_or(Combuf& out, const Combuf& inp, size_t curr_idx)
{
    Command      com     = inp[curr_idx];
    auto         args    = com.args;
    size_t       fst_idx = args.first;
    size_t       snd_idx = args.second;

    Combuf       argv1;
    Combuf       argv2;

    fuse_commandsR(argv1, inp, fst_idx);
    fuse_commandsR(argv2, inp, snd_idx);

    auto         fst_arg = argv1.back();
    auto         snd_arg = argv2.back();
    bool         m1      = fst_arg.name == Command_name::Multior;
    bool         m2      = snd_arg.name == Command_name::Multior;
    bool         t1      = is_leaf(fst_arg) || m1;
    bool         t2      = is_leaf(snd_arg) || m2;

    if(!(t1 && t2)){
        size_t s         = out.size();
        fst_idx          = argv1.size() + s - 1;
        fuse_bufs(argv1, argv2);
        snd_idx          = argv1.size() + s - 1;
        fuse_bufs(out, argv1);
        com.name         = Command_name::Or;
        com.args.first   = fst_idx;
        com.args.second  = snd_idx;
        out.push_back(com);
        return;
    }

    if(m1){
        argv1.pop_back();
    }
    if(m2){
        argv2.pop_back();
    }
    size_t     s     = out.size();
    fst_idx          = argv1.size() + s - 1;
    fuse_bufs(argv1, argv2);
    snd_idx          = argv1.size() + s - 1;
    fuse_bufs(out, argv1);
    com.name         = Command_name::Multior;
    com.args.first   = fst_idx;
    com.args.second  = snd_idx;
    out.push_back(com);
}

static void fuse_concat(Combuf& out, const Combuf& inp, size_t curr_idx)
{
    Command      com     = inp[curr_idx];
    auto         args    = com.args;
    size_t       fst_idx = args.first;
    size_t       snd_idx = args.second;

    Combuf       argv1;
    Combuf       argv2;

    fuse_commandsR(argv1, inp, fst_idx);
    fuse_commandsR(argv2, inp, snd_idx);

    auto         fst_arg = argv1.back();
    auto         snd_arg = argv2.back();
    bool         m1      = fst_arg.name == Command_name::Multiconcat;
    bool         m2      = snd_arg.name == Command_name::Multiconcat;
    bool         t1      = is_leaf(fst_arg) || m1;
    bool         t2      = is_leaf(snd_arg) || m2;

    if(!(t1 && t2)){
        size_t s         = out.size();
        fst_idx          = argv1.size() + s - 1;
        fuse_bufs(argv1, argv2);
        snd_idx          = argv1.size() + s - 1;
        fuse_bufs(out, argv1);
        com.name         = Command_name::Concat;
        com.args.first   = fst_idx;
        com.args.second  = snd_idx;
        out.push_back(com);
        return;
    }

    if(m1){
        argv1.pop_back();
    }
    if(m2){
        argv2.pop_back();
    }
    size_t     s     = out.size();
    fst_idx          = argv1.size() + s - 1;
    fuse_bufs(argv1, argv2);
    snd_idx          = argv1.size() + s - 1;
    fuse_bufs(out, argv1);
    com.name         = Command_name::Multiconcat;
    com.args.first   = fst_idx;
    com.args.second  = snd_idx;
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