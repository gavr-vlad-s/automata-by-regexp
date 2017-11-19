/*
     File:    test-simple-parser.cpp
     Created: 03 August 2017 at 15:33 Moscow time.
     Author:  Гаврилов Владимир Сергеевич
     E-mails: vladimir.s.gavrilov@gmail.com
              gavrilov.vladimir.s@mail.ru
              gavvs1977@yandex.ru
*/

#include <cstdlib>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
// #include "../include/fsize.h"
// #include "../include/error_count.h"
#include "../include/location.h"
// #include "../include/trie.h"
// #include "../include/aux_expr_scaner.h"
#include "../include/char_conv.h"
// #include "../include/test_aux_expr_scaner.h"
#include "../include/errors_and_tries.h"
#include "../include/file_contents.h"
#include "../include/char_trie.h"
#include "../include/expr_scaner.h"
// #include "../include/simple_regex_parser.h"
#include "../include/command.h"
#include "../include/print_commands.h"
#include "../include/scope.h"
#include "../include/expr_lr_tables.h"
#include "../include/slr_act_expr_parser.h"
#include "../include/fuse_commands.h"
#include "../include/ndfa.h"
#include "../include/dfa.h"
#include "../include/minimal_dfa.h"
#include "../include/groupped_dfa.h"
#include "../include/groupped_dfa_by_regexp.h"
#include "../include/match.h"
// #include "../include/act_expr_parser.h"

// #include "../include/test_expr_scaner.h"

/* This function opens a file with test text, and it returns string containing text,
 * if file is successfully opened and the file size is not equal to zero, and the
 * empty string otherwise. */
std::u32string init_testing(const char* name)
{
    auto contents = get_contents(name);
    auto& str      = contents.second;
    switch(contents.first){
        case Get_contents_return_code::Normal:
            if(!str.length()){
                puts("File length is zero.");
                return U"";
            }else{
                return utf8_to_u32string(str.c_str());
            }
            break;

        case Get_contents_return_code::Impossible_open:
            puts("It is impossible to open the file.");
            return U"";

        case Get_contents_return_code::Read_error:
            puts("File reading failed.");
            return U"";
    }
    return U"";
}

static const char32_t* write_act_name = U"write";
static const char32_t* write_act_body = U"buffer += ch;";

static const char32_t* add_dec_digit_act_name = U"add_dec_digit";
static const char32_t* add_dec_digit_act_body = U"token.value = token.value * 10 + digit2int(ch);";

static const char32_t* add_hex_digit_act_name = U"add_hex_digit";
static const char32_t* add_hex_digit_act_body = U"token.value = token.value << 4 + digit2int(ch);";

static const char32_t* add_bin_digit_act_name = U"add_bin_digit";
static const char32_t* add_bin_digit_act_body = U"token.value = token.value << 1 + digit2int(ch);";

static const char* before_fusing =
    R"~(
***********************************************************
*                     Before fusing:                      *
***********************************************************)~";

static const char* after_fusing =
    R"~(
***********************************************************
*                     After fusing:                       *
***********************************************************)~";

static const char* non_deterministic_aut =
    R"~(
***********************************************************
*            Non-deterministic automaton:                 *
***********************************************************)~";

static const char* deterministic_aut =
    R"~(
***********************************************************
*                Deterministic automaton:                 *
***********************************************************)~";

static const char* minimized_deterministic_aut =
    R"~(
***********************************************************
*          Minimized deterministic automaton:             *
***********************************************************)~";

static const char* minimized_groupped_deterministic_aut =
    R"~(
***********************************************************
*          Minimized deterministic automaton              *
*              with groupped transitions:                 *
***********************************************************)~";

void add_action(Errors_and_tries&       etr,
                std::shared_ptr<Scope>& scope,
                const std::u32string&   name,
                const std::u32string&   body)
{
    Id_attributes iattr;
    iattr.kind             = Action_name;
    size_t idx             = etr.ids_trie -> insert(name);
    size_t body_idx        = etr.strs_trie-> insert(body);
    iattr.act_string       = body_idx;
    scope->idsc[idx]       = iattr;

    Str_attributes sattr;
    sattr.kind             = Action_definition;
    sattr.code             = 0;
    scope->strsc[body_idx] = sattr;

    auto name_in_utf8 = u32string_to_utf8(name);
    printf("Index of action with name %s is %zu.\n",
           name_in_utf8.c_str(), idx);
}

static const std::vector<std::u32string> matched_strings = {
    U"0x123ABCDEF", U"0X127aAbBcCdDeEfF",
    U"0b11011011",  U"0B110111",
    U"0123789456",  U"76543218900",
    U"0Ae",         U"identifier"
};

int main(int argc, char** argv)
{
    if(1 == argc){
        puts("File is not given.");
    }else{
        std::u32string t = init_testing(argv[1]);
        if(t.length()){
            char32_t* p        = const_cast<char32_t*>(t.c_str());
            auto      loc      = std::make_shared<Location>(p);

            Errors_and_tries etr;
            etr.ec             = std::make_shared<Error_count>();
            etr.ids_trie       = std::make_shared<Char_trie>();
            etr.strs_trie      = std::make_shared<Char_trie>();
            auto trie_for_sets = std::make_shared<Trie_for_set_of_char32>();
            auto esc           = std::make_shared<Expr_scaner>(loc, etr, trie_for_sets);
            auto scope         = std::make_shared<Scope>();

            add_action(etr, scope, write_act_name, write_act_body);
            add_action(etr, scope, add_dec_digit_act_name, add_dec_digit_act_body);
            add_action(etr, scope, add_hex_digit_act_name, add_hex_digit_act_body);
            add_action(etr, scope, add_bin_digit_act_name, add_bin_digit_act_body);

//             Id_attributes iattr;
//             iattr.kind             = Action_name;
//             size_t idx             = etr.ids_trie -> insert(write_act_name);
//             size_t body_idx        = etr.strs_trie-> insert(write_act_body);
//             iattr.act_string       = body_idx;
//             scope->idsc[idx]       = iattr;
//
//             Str_attributes sattr;
//             sattr.kind             = Action_definition;
//             sattr.code             = 0;
//             scope->strsc[body_idx] = sattr;

            auto slr_arp           =
                std::make_unique<SLR_act_expr_parser>(esc, etr, scope, expr_slr_tables);

//             auto arp           = std::make_unique<Act_expr_parser>(esc, etr, scope);

            Command_buffer commands;

            slr_arp->compile(commands);

// #ifndef NUM_TEST
//             arp->compile(commands, String_expr);
// #else
//             arp->compile(commands, Number_expr);
// #endif
            puts(before_fusing);
            print_commands(commands, trie_for_sets);

            puts(after_fusing);
            auto fused = fuse_commands(commands);
            print_commands(fused, trie_for_sets);

            NDFA ndaut;
            puts(non_deterministic_aut);
            build_NDFA(ndaut, fused);
            print_NDFA(ndaut, trie_for_sets);

            DFA daut;
            puts(deterministic_aut);
            convert_NDFA_to_DFA(daut, ndaut, trie_for_sets);
            print_DFA(daut, trie_for_sets);

            Min_DFA min_daut;
            puts(minimized_deterministic_aut);
            minimize_DFA(min_daut, daut);
            print_minimal_DFA(min_daut, trie_for_sets);

            G_DFA gdfa;
            puts(minimized_groupped_deterministic_aut);
            grouped_DFA_by_regexp(gdfa, commands, trie_for_sets);
            print_grouped_DFA(gdfa);

            for(const auto& str : matched_strings){
                bool        is_matched  = match(gdfa, str);
                std::string str_as_utf8 = u32string_to_utf8(str);
                printf("string %s is %s\n", str_as_utf8.c_str(),
                                            is_matched ? "matched" : "not matched");
            }

            bool r = match_all(gdfa, matched_strings.begin(), matched_strings.end());
            printf("Result of the function match_all: %s.\n",
                   r ? "all strings are matched" : "not all strings are matched");

        }
    }
    return 0;
}