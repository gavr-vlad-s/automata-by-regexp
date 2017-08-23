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

            Id_attributes iattr;
            iattr.kind             = Action_name;
            size_t idx             = etr.ids_trie -> insert(write_act_name);
            size_t body_idx        = etr.strs_trie-> insert(write_act_body);
            iattr.act_string       = body_idx;
            scope->idsc[idx]       = iattr;

            Str_attributes sattr;
            sattr.kind             = Action_definition;
            sattr.code             = 0;
            scope->strsc[body_idx] = sattr;

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
            print_commands(commands, trie_for_sets);
        }
    }
    return 0;
}