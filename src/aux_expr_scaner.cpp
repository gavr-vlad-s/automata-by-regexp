/*
    File:    aux_expr_scaner.cpp
    Created: 20 July 2017 at 12:14 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include "../include/aux_expr_scaner.h"
#include "../include/aux_expr_lexem.h"
#include "../include/belongs.h"
#include <cstdlib>
#include <cstdio>
#include <utility>
#include <cstddef>
#include "../include/search_char.h"
#include "../include/get_init_state.h"

// #define DEBUG_MODE
#ifdef DEBUG_MODE
#include <string>
#endif

enum Category : uint16_t {
    Spaces,            Other,             Action_name_begin,
    Action_name_body,  Delimiters,        Dollar,
    Backslash,         Opened_square_br,  After_colon,
    After_backslash,   Begin_expr,        End_expr,
    Hat
};

#ifdef DEBUG_MODE
static const std::string category_string[] = {
    "Spaces",            "Other",             "Action_name_begin",
    "Action_name_body",  "Delimiters",        "Dollar",
    "Backslash",         "Opened_square_br",  "After_colon",
    "After_backslash",   "Begin_expr",        "End_expr",
    "Hat"
};

static std::string show_categories_set(uint64_t cs)
{
    std::string result;
    for(uint64_t i = Spaces; i <= Hat; ++i)
    {
        if(belongs(i, cs)){
            result += category_string[i] + ", ";
        }
    }
    if(!result.empty()){
        result.pop_back();
        result.pop_back();
    }
    result = "{" + result + "}";
    return result;
}

static void print_categories_set(uint64_t cs)
{
    std::string s = show_categories_set(cs);
    printf("%s\n", s.c_str());
}
#endif

/*
 * It happens that in std::map<K,V> the key type is integer, and a lot of keys with the same corresponding values.
 * If such a map must be a generated constant, then this map can be optimized. Namely, iterating through a map using
 * range-based for, we will build a std::vector<std::pair<K, V>>.
 * Then we group pairs std::pair<K, V> in pairs in the form (segment, a value of type V), where 'segment' is a struct
 * consisting of lower bound and upper bound. Next, we permute the grouped pair in the such way that in order to search
 * for in the array of the resulting values we can use the algorithm from the answer to exercise 6.2.24 of the book
 * Knuth D.E. The art of computer programming. Volume 3. Sorting and search. --- 2nd ed. --- Addison-Wesley, 1998.
 */

#define RandomAccessIterator typename
#define Callable             typename
#define Integral             typename
template<typename T>
struct Segment{
    T lower_bound;
    T upper_bound;

    Segment()               = default;
    Segment(const Segment&) = default;
    ~Segment()              = default;
};

template<typename T, typename V>
struct Segment_with_value{
    Segment<T> bounds;
    V          value;

    Segment_with_value()                          = default;
    Segment_with_value(const Segment_with_value&) = default;
    ~Segment_with_value()                         = default;
};

/* This function uses algorithm from the answer to the exercise 6.2.24 of the monography
 *  Knuth D.E. The art of computer programming. Volume 3. Sorting and search. --- 2nd ed.
 *  --- Addison-Wesley, 1998.
 */
template<RandomAccessIterator I, typename K>
std::pair<bool, size_t> knuth_find(I it_begin, I it_end, K key)
{
    std::pair<bool, size_t> result = {false, 0};
    size_t                  i      = 1;
    size_t                  n      = it_end - it_begin;
    while (i <= n)
    {
        const auto& curr        = it_begin[i - 1];
        const auto& curr_bounds = curr.bounds;
        if(key < curr_bounds.lower_bound){
            i = 2 * i;
        }else if(key > curr_bounds.upper_bound){
            i = 2 * i + 1;
        }else{
            result.first = true; result.second = i - 1;
            break;
        }
    }
    return result;
}

static const Segment_with_value<char32_t, uint64_t> categories_table[] = {
    {{U'b', U'b'},  268},  {{U'R', U'R'},  268},  {{U'p', U'q'},   12},
    {{U'?', U'?'},  528},  {{U']', U']'},  512},  {{U'l', U'l'},  268},
    {{U'y', U'z'},   12},  {{U'(', U'+'},  528},  {{U'L', U'L'},  268},
    {{U'[', U'['},  640},  {{U'_', U'_'},   12},  {{U'd', U'd'},  268},
    {{U'n', U'n'},  780},  {{U's', U'w'},   12},  {{U'|', U'|'},  528},
    {{U'"', U'"'},  512},  {{U'0', U'9'},    8},  {{U'A', U'K'},   12},
    {{U'M', U'Q'},   12},  {{U'S', U'Z'},   12},  {{U'\\', U'\\'},  576},
    {{U'^', U'^'}, 4608},  {{U'a', U'a'},   12},  {{U'c', U'c'},   12},
    {{U'e', U'k'},   12},  {{U'm', U'm'},   12},  {{U'o', U'o'},  268},
    {{U'r', U'r'},  268},  {{U'x', U'x'},  268},  {{U'{', U'{'}, 1552},
    {{U'}', U'}'}, 2576},  {{   1,   32},    1},  {{U'$', U'$'},  544}
};

static const size_t num_of_elems_in_categories_table = 33;

uint64_t get_categories_set(char32_t c)
{
    auto t = knuth_find(categories_table,
                        categories_table + num_of_elems_in_categories_table,
                        c);
    return t.first ? categories_table[t.second].value : (1ULL << Other);
}

/**
 * Element of the transitions table of the character class processing automaton.
 */
struct Elem {
    /** A pointer to a string of characters that can be passed to any state. */
    char32_t*           symbols;
    /** lexeme code */
    Aux_expr_lexem_code code;
    /** If the current character matches symbols [0], then
     *  the transition to the state first_state;
     *  if the current character matches symbols [1], then
     *  the transition to the state first_state + 1;
     *  if the current character matches symbols [2], then
     *  the transition to the state first_state + 2, and so on. */
    uint16_t            first_state;
};

/* For the character classes handler, the state member of the Aux_expr_scaner
 * class is the index of the element in the navigation table, denoted below
 * as a_classes_jump_table. */
static const Elem a_classes_jump_table[] = {
    {const_cast<char32_t*>(U"ae"),Aux_expr_lexem_code::M_Class_Latin,  1},  // 0:   [:L...
    {const_cast<char32_t*>(U"t"), Aux_expr_lexem_code::M_Class_Latin,  3},  // 1:   [:La...
    {const_cast<char32_t*>(U"t"), Aux_expr_lexem_code::M_Class_Letter, 4},  // 2:   [:Le...
    {const_cast<char32_t*>(U"i"), Aux_expr_lexem_code::M_Class_Latin,  5},  // 3:   [:Lat...
    {const_cast<char32_t*>(U"t"), Aux_expr_lexem_code::M_Class_Letter, 6},  // 4:   [:Let...
    {const_cast<char32_t*>(U"n"), Aux_expr_lexem_code::M_Class_Latin,  7},  // 5:   [:Lati...
    {const_cast<char32_t*>(U"e"), Aux_expr_lexem_code::M_Class_Letter, 8},  // 6:   [:Lett...
    {const_cast<char32_t*>(U":"), Aux_expr_lexem_code::M_Class_Latin,  9},  // 7:   [:Latin...
    {const_cast<char32_t*>(U"r"), Aux_expr_lexem_code::M_Class_Letter, 10}, // 8:   [:Lette...
    {const_cast<char32_t*>(U"]"), Aux_expr_lexem_code::M_Class_Latin,  11}, // 9:   [:Latin:...
    {const_cast<char32_t*>(U":"), Aux_expr_lexem_code::M_Class_Letter, 12}, // 10:  [:Letter...
    {const_cast<char32_t*>(U""),  Aux_expr_lexem_code::Class_Latin,    0},  // 11:  [:Latin:]
    {const_cast<char32_t*>(U"]"), Aux_expr_lexem_code::M_Class_Letter, 13}, // 12:  [:Letter:...
    {const_cast<char32_t*>(U""),  Aux_expr_lexem_code::Class_Letter,   0},  // 13:  [:Letter:]

    {const_cast<char32_t*>(U"u"), Aux_expr_lexem_code::M_Class_Russian,15}, // 14:  [:R...
    {const_cast<char32_t*>(U"s"), Aux_expr_lexem_code::M_Class_Russian,16}, // 15:  [:Ru...
    {const_cast<char32_t*>(U"s"), Aux_expr_lexem_code::M_Class_Russian,17}, // 16:  [:Rus...
    {const_cast<char32_t*>(U"i"), Aux_expr_lexem_code::M_Class_Russian,18}, // 17:  [:Russ...
    {const_cast<char32_t*>(U"a"), Aux_expr_lexem_code::M_Class_Russian,19}, // 18:  [:Russi...
    {const_cast<char32_t*>(U"n"), Aux_expr_lexem_code::M_Class_Russian,20}, // 19:  [:Russia...
    {const_cast<char32_t*>(U":"), Aux_expr_lexem_code::M_Class_Russian,21}, // 20:  [:Russian...
    {const_cast<char32_t*>(U"]"), Aux_expr_lexem_code::M_Class_Russian,22}, // 21:  [:Russian:...
    {const_cast<char32_t*>(U""),  Aux_expr_lexem_code::Class_Russian,  0},  // 22:  [:Russian:]

    {const_cast<char32_t*>(U"d"), Aux_expr_lexem_code::M_Class_bdigits,24}, // 23:  [:b...
    {const_cast<char32_t*>(U"i"), Aux_expr_lexem_code::M_Class_bdigits,25}, // 24:  [:bd...
    {const_cast<char32_t*>(U"g"), Aux_expr_lexem_code::M_Class_bdigits,26}, // 25:  [:bdi...
    {const_cast<char32_t*>(U"i"), Aux_expr_lexem_code::M_Class_bdigits,27}, // 26:  [:bdig...
    {const_cast<char32_t*>(U"t"), Aux_expr_lexem_code::M_Class_bdigits,28}, // 27:  [:bdigi...
    {const_cast<char32_t*>(U"s"), Aux_expr_lexem_code::M_Class_bdigits,29}, // 28:  [:bdigit...
    {const_cast<char32_t*>(U":"), Aux_expr_lexem_code::M_Class_bdigits,30}, // 29:  [:bdigits...
    {const_cast<char32_t*>(U"]"), Aux_expr_lexem_code::M_Class_bdigits,31}, // 30:  [:bdigits:...
    {const_cast<char32_t*>(U""),  Aux_expr_lexem_code::Class_bdigits,  0},  // 31:  [:bdigits:]

    {const_cast<char32_t*>(U"i"), Aux_expr_lexem_code::M_Class_digits, 33}, // 32:  [:d...
    {const_cast<char32_t*>(U"g"), Aux_expr_lexem_code::M_Class_digits, 34}, // 33:  [:di...
    {const_cast<char32_t*>(U"i"), Aux_expr_lexem_code::M_Class_digits, 35}, // 34:  [:dig...
    {const_cast<char32_t*>(U"t"), Aux_expr_lexem_code::M_Class_digits, 36}, // 35:  [:digi...
    {const_cast<char32_t*>(U"s"), Aux_expr_lexem_code::M_Class_digits, 37}, // 36:  [:digit...
    {const_cast<char32_t*>(U":"), Aux_expr_lexem_code::M_Class_digits, 38}, // 37:  [:digits...
    {const_cast<char32_t*>(U"]"), Aux_expr_lexem_code::M_Class_digits, 39}, // 38:  [:digits:...
    {const_cast<char32_t*>(U""),  Aux_expr_lexem_code::Class_digits,   0},  // 39:  [:digits:]

    {const_cast<char32_t*>(U"ae"),Aux_expr_lexem_code::M_Class_latin,  41}, // 40:  [:l...
    {const_cast<char32_t*>(U"t"), Aux_expr_lexem_code::M_Class_latin,  43}, // 41:  [:la...
    {const_cast<char32_t*>(U"t"), Aux_expr_lexem_code::M_Class_letter, 44}, // 42:  [:le...
    {const_cast<char32_t*>(U"i"), Aux_expr_lexem_code::M_Class_latin,  45}, // 43:  [:lat...
    {const_cast<char32_t*>(U"t"), Aux_expr_lexem_code::M_Class_letter, 46}, // 44:  [:let...
    {const_cast<char32_t*>(U"n"), Aux_expr_lexem_code::M_Class_latin,  47}, // 45:  [:lati...
    {const_cast<char32_t*>(U"e"), Aux_expr_lexem_code::M_Class_letter, 48}, // 46:  [:lett...
    {const_cast<char32_t*>(U":"), Aux_expr_lexem_code::M_Class_latin,  49}, // 47:  [:latin...
    {const_cast<char32_t*>(U"r"), Aux_expr_lexem_code::M_Class_letter, 50}, // 48:  [:lette...
    {const_cast<char32_t*>(U"]"), Aux_expr_lexem_code::M_Class_latin,  51}, // 49:  [:latin:...
    {const_cast<char32_t*>(U":"), Aux_expr_lexem_code::M_Class_letter, 52}, // 50:  [:letter...
    {const_cast<char32_t*>(U""),  Aux_expr_lexem_code::Class_latin,    0},  // 51:  [:latin:]
    {const_cast<char32_t*>(U"]"), Aux_expr_lexem_code::M_Class_letter, 53}, // 52:  [:letter:...
    {const_cast<char32_t*>(U""),  Aux_expr_lexem_code::Class_letter,   0},  // 53:  [:letter:]

    {const_cast<char32_t*>(U"ds"),Aux_expr_lexem_code::M_Class_ndq,    55}, // 54:  [:n...
    {const_cast<char32_t*>(U"q"), Aux_expr_lexem_code::M_Class_ndq,    57}, // 55:  [:nd...
    {const_cast<char32_t*>(U"q"), Aux_expr_lexem_code::M_Class_nsq,    58}, // 56:  [:ns...
    {const_cast<char32_t*>(U":"), Aux_expr_lexem_code::M_Class_ndq,    59}, // 57:  [:ndq...
    {const_cast<char32_t*>(U":"), Aux_expr_lexem_code::M_Class_nsq,    60}, // 58:  [:nsq...
    {const_cast<char32_t*>(U"]"), Aux_expr_lexem_code::M_Class_ndq,    61}, // 59:  [:ndq:...
    {const_cast<char32_t*>(U"]"), Aux_expr_lexem_code::M_Class_nsq,    62}, // 60:  [:nsq:...
    {const_cast<char32_t*>(U""),  Aux_expr_lexem_code::Class_ndq,      0},  // 61:  [:ndq:]
    {const_cast<char32_t*>(U""),  Aux_expr_lexem_code::Class_nsq,      0},  // 62:  [:nsq:]

    {const_cast<char32_t*>(U"d"), Aux_expr_lexem_code::M_Class_odigits,64}, // 63:  [:o...
    {const_cast<char32_t*>(U"i"), Aux_expr_lexem_code::M_Class_odigits,65}, // 64:  [:od...
    {const_cast<char32_t*>(U"g"), Aux_expr_lexem_code::M_Class_odigits,66}, // 65:  [:odi...
    {const_cast<char32_t*>(U"i"), Aux_expr_lexem_code::M_Class_odigits,67}, // 66:  [:odig...
    {const_cast<char32_t*>(U"t"), Aux_expr_lexem_code::M_Class_odigits,68}, // 67:  [:odigi...
    {const_cast<char32_t*>(U"s"), Aux_expr_lexem_code::M_Class_odigits,69}, // 68:  [:odigit...
    {const_cast<char32_t*>(U":"), Aux_expr_lexem_code::M_Class_odigits,70}, // 69:  [:odigits...
    {const_cast<char32_t*>(U"]"), Aux_expr_lexem_code::M_Class_odigits,71}, // 70:  [:odigits:...
    {const_cast<char32_t*>(U""),  Aux_expr_lexem_code::Class_odigits,  0,}, // 71:  [:odigits:]

    {const_cast<char32_t*>(U"u"), Aux_expr_lexem_code::M_Class_russian,73}, // 72:  [:r...
    {const_cast<char32_t*>(U"s"), Aux_expr_lexem_code::M_Class_russian,74}, // 73:  [:ru...
    {const_cast<char32_t*>(U"s"), Aux_expr_lexem_code::M_Class_russian,75}, // 74:  [:rus...
    {const_cast<char32_t*>(U"i"), Aux_expr_lexem_code::M_Class_russian,76}, // 75:  [:russ...
    {const_cast<char32_t*>(U"a"), Aux_expr_lexem_code::M_Class_russian,77}, // 76:  [:russi...
    {const_cast<char32_t*>(U"n"), Aux_expr_lexem_code::M_Class_russian,78}, // 77:  [:russia...
    {const_cast<char32_t*>(U":"), Aux_expr_lexem_code::M_Class_russian,79}, // 78:  [:russian...
    {const_cast<char32_t*>(U"]"), Aux_expr_lexem_code::M_Class_russian,80}, // 79:  [:russian:...
    {const_cast<char32_t*>(U""),  Aux_expr_lexem_code::Class_russian,  0},  // 80:  [:russian:]

    {const_cast<char32_t*>(U"d"), Aux_expr_lexem_code::M_Class_xdigits,82}, // 81:  [:x...
    {const_cast<char32_t*>(U"i"), Aux_expr_lexem_code::M_Class_xdigits,83}, // 82:  [:xd...
    {const_cast<char32_t*>(U"g"), Aux_expr_lexem_code::M_Class_xdigits,84}, // 83:  [:xdi...
    {const_cast<char32_t*>(U"i"), Aux_expr_lexem_code::M_Class_xdigits,85}, // 84:  [:xdig...
    {const_cast<char32_t*>(U"t"), Aux_expr_lexem_code::M_Class_xdigits,86}, // 85:  [:xdigi...
    {const_cast<char32_t*>(U"s"), Aux_expr_lexem_code::M_Class_xdigits,87}, // 86:  [:xdigit...
    {const_cast<char32_t*>(U":"), Aux_expr_lexem_code::M_Class_xdigits,88}, // 87:  [:xdigits...
    {const_cast<char32_t*>(U"]"), Aux_expr_lexem_code::M_Class_xdigits,89}, // 88:  [:xdigits:...
    {const_cast<char32_t*>(U""),  Aux_expr_lexem_code::Class_xdigits,  0}   // 89:  [:xdigits:]
};

Aux_expr_scaner::Automaton_proc Aux_expr_scaner::procs[] = {
    &Aux_expr_scaner::start_proc,     &Aux_expr_scaner::unknown_proc,
    &Aux_expr_scaner::action_proc,    &Aux_expr_scaner::delimiter_proc,
    &Aux_expr_scaner::classes_proc,   &Aux_expr_scaner::char_proc,
    &Aux_expr_scaner::hat_proc
};

Aux_expr_scaner::Final_proc Aux_expr_scaner::finals[] = {
    &Aux_expr_scaner::none_final_proc,
    &Aux_expr_scaner::unknown_final_proc,
    &Aux_expr_scaner::action_final_proc,
    &Aux_expr_scaner::delimiter_final_proc,
    &Aux_expr_scaner::classes_final_proc,
    &Aux_expr_scaner::char_final_proc,
    &Aux_expr_scaner::hat_final_proc
};

bool Aux_expr_scaner::start_proc()
{
    bool t = true;
    state = -1;
    /* For an automaton that processes a lexeme, the state with the number (-1)
     * is the state in which this machine is initialized. */
    if(belongs(Spaces, char_categories)){
        loc->current_line += U'\n' == ch;
        return t;
    }
    lexem_begin_line = loc->current_line;
    if(belongs(Delimiters, char_categories)){
        automaton = A_delimiter; token.code = Aux_expr_lexem_code::UnknownLexem;
        (loc->pcurrent_char)--;
    }else if(belongs(Dollar, char_categories)){
        automaton = A_action;    token.code = Aux_expr_lexem_code::Action;
        buffer.clear();
    }else if(belongs(Opened_square_br, char_categories)){
        automaton = A_class,     token.code = Aux_expr_lexem_code::Character;
        token.c = U'[';
    }else if(belongs(Backslash, char_categories)){
         automaton = A_char;     token.code = Aux_expr_lexem_code::Character;
    }else if(belongs(Begin_expr, char_categories)){
        token.code = Aux_expr_lexem_code::Begin_expression; t = false;
        (loc->pcurrent_char)++;
    }else if(belongs(End_expr, char_categories)){
        token.code = Aux_expr_lexem_code::End_expression; t = false;
        (loc->pcurrent_char)++;
    }else if(belongs(Hat, char_categories)){
        automaton = A_hat,     token.code = Aux_expr_lexem_code::Character;
        token.c = U'^';
    }else{
        token.code = Aux_expr_lexem_code::Character; token.c = ch; t = false;
        (loc->pcurrent_char)++;
    }
    return t;
}

static const char* class_strings[] = {
    "[:Latin:]",   "[:Letter:]",  "[:Russian:]",
    "[:bdigits:]", "[:digits:]",  "[:latin:]",
    "[:letter:]",  "[:odigits:]", "[:russian:]",
    "[:xdigits:]", "[:ndq:]",     "[:nsq:]"
};

static const char* line_expects = "Line %zu expects %s.\n";

void Aux_expr_scaner::correct_class()
{
    /* This function corrects the code of the token, most likely a character class,
     * and displays the necessary diagnostics. */
    if(token.code >= Aux_expr_lexem_code::M_Class_Latin){
        int y = static_cast<int>(token.code) -
                static_cast<int>(Aux_expr_lexem_code::M_Class_Latin);
        printf(line_expects, loc->current_line,class_strings[y]);
        token.code = static_cast<Aux_expr_lexem_code>(y +
                        static_cast<int>(Aux_expr_lexem_code::Class_Latin));
        en -> increment_number_of_errors();
    }
}

Aux_expr_lexem_info Aux_expr_scaner::current_lexem()
{
    automaton   = A_start;
    token.code  = Aux_expr_lexem_code::Nothing;
    lexem_begin = loc->pcurrent_char;
    bool t = true;
    while((ch = *(loc->pcurrent_char)++)){
        char_categories = get_categories_set(ch);
#ifdef DEBUG_MODE
        print_categories_set(char_categories);
#endif
        t = (this->*procs[automaton])();
        if(!t){
            /* We get here only if the lexeme has already been read. At the same time,
             * the current automaton has already read the character that follows
             * immediately after the end of the lexeme read, based on this symbol, it was
             * decided that the lexeme was read and the transition to the next character
             * was made. Therefore, in order to not miss the first character of the next
             * lexeme, you need to decrease the pcurrent_char pointer by one. */
            (loc->pcurrent_char)--;
            if(Aux_expr_lexem_code::Action == token.code){
                /* If the current lexeme is an identifier, then this identifier must be
                 * written to the identifier table. */
                token.action_name_index = ids -> insert(buffer);
            }else if(A_class == automaton){
                /* If you have finished processing the class of characters, you need to
                 * adjust its code, and, possibly, output diagnostics. */
                correct_class();
            }
            return token;
        }
    }
    /* Here we can be, only if we have already read all the processed text. In this
     * case, the pointer to the current symbol points to a character that is immediately
     * after the null character, which is a sign of the end of the text. To avoid entering
     * subsequent calls outside the text, you need to go back to the null character.*/
    (loc->pcurrent_char)--;
    /* Further, since we are here, the end of the current token (perhaps unexpected) has
     * not yet been processed. It is necessary to perform this processing, and, probably,
     * to display some kind of diagnostics.*/
    (this->*finals[automaton])();
    return token;
}

bool Aux_expr_scaner::unknown_proc()
{
    return belongs(Other, char_categories);
}

/* This array consists of pairs of the form (state, character) and is used to initialize
 * the character class processing automaton. The sense of the element of the array is this:
 * if the current character in the initialization state coincides with the second component
 * of the element, the work begins with the state that is the first component of the element.
 * Consider, for example, the element {54, U'n '}. If the current character coincides with
 * the second component of this element, then work begins with the state being the first
 * component, i.e. from state 54. The array must be sorted in ascending order of the
 * second component.*/
static const State_for_char init_table_for_classes[] = {
    {0, U'L'},  {14, U'R'}, {23, U'b'}, {32, U'd'}, {40, U'l'},
    {54, U'n'}, {63, U'o'}, {72, U'r'}, {81, U'x'}
};

static const char* expects_LRbdlnorx =
    "The line %zu expects one of the following characters: L, R, b, d, l, n, o, r, x.\n";

static const char* latin_letter_expected =
    "A Latin letter or an underscore is expected on the %zu line.\n";

bool Aux_expr_scaner::classes_proc()
{
    bool t = false;
    switch(state){
        case -1:
            if(U':' == ch){
                state = -2; t = true;
            }else if(U'^' == ch){
                token.code = Aux_expr_lexem_code::Begin_char_class_complement;
                (loc->pcurrent_char)++;
            }
            break;
        case -2:
            if(belongs(After_colon, char_categories)){
                state = get_init_state(ch, init_table_for_classes,
                                       sizeof(init_table_for_classes)/
                                       sizeof(State_for_char));
                token.code = a_classes_jump_table[state].code;
                t = true;
            }else{
                printf(expects_LRbdlnorx, loc->current_line);
                en -> increment_number_of_errors();
            }
            break;
        default:
            Elem elem = a_classes_jump_table[state];
            token.code = elem.code;
            int y = search_char(ch, elem.symbols);
            if(y != THERE_IS_NO_CHAR){
                state = elem.first_state + y; t = true;
            }
    }
    return t;
}

bool Aux_expr_scaner::char_proc()
{
    if(belongs(After_backslash, char_categories)){
        token.c = (U'n' == ch) ? U'\n' : ch;
        (loc->pcurrent_char)++;
    }else{
        token.c = U'\\';
    }
    return false;
}

bool Aux_expr_scaner::delimiter_proc()
{
    bool t = false;
    switch(ch){
        case U'{':
            token.code = Aux_expr_lexem_code::Begin_expression;
            break;
        case U'}':
            token.code = Aux_expr_lexem_code::End_expression;
            break;
        case U'(':
            token.code = Aux_expr_lexem_code::Opened_round_brack;
            break;
        case U')':
            token.code = Aux_expr_lexem_code::Closed_round_brack;
            break;
        case U'|':
            token.code = Aux_expr_lexem_code::Or;
            break;
        case U'*':
            token.code = Aux_expr_lexem_code::Kleene_closure;
            break;
        case U'+':
            token.code = Aux_expr_lexem_code::Positive_closure;
            break;
        case U'?':
            token.code = Aux_expr_lexem_code::Optional_member;
            break;
    }
    (loc->pcurrent_char)++;
    return t;
}

bool Aux_expr_scaner::action_proc()
{
    bool t = true;
    /* The variable t is true if the action name has not yet
     * been fully read, and false otherwise. */
    if(-1 == state){
        if(belongs(Action_name_begin, char_categories)){
            buffer += ch; state = 0;
        }else{
            printf(latin_letter_expected, loc->current_line);
            en -> increment_number_of_errors();
            t = false;
        }
        return t;
    }
    t = belongs(Action_name_body, char_categories);
    if(t){
        buffer += ch;
    }
    return t;
}

bool Aux_expr_scaner::hat_proc()
{
    bool t = false;
    if(ch == U']'){
        token.code = Aux_expr_lexem_code::End_char_class_complement;
        (loc->pcurrent_char)++;
    }
    return t;
}

void Aux_expr_scaner::none_final_proc()
{
    /* This subroutine will be called if, after reading the input text, it turned out
     * to be in the A_start automaton. Then you do not need to do anything. */
}

void Aux_expr_scaner::unknown_final_proc()
{
    /* This subroutine will be called if, after reading the input text, it turned out
     * to be in the A_unknown automaton. Then you do not need to do anything. */
}

void Aux_expr_scaner::action_final_proc()
{
    /* This function will be called if, after reading the input stream, they were
     * in the action names processing automaton, the A_action automaton. Then this
     * name should be written in the prefix tree of identifiers. */
    token.action_name_index = ids -> insert(buffer);
}

void Aux_expr_scaner::delimiter_final_proc()
{
}

void Aux_expr_scaner::classes_final_proc()
{
    switch(state){
        case -1:
            token.code = Aux_expr_lexem_code::Character;  token.c = U'[';
            break;
        case -2:
            token.code = Aux_expr_lexem_code::UnknownLexem;
            break;
        default:
            token.code = a_classes_jump_table[state].code;
            correct_class();
    }
}

void Aux_expr_scaner::char_final_proc()
{
    token.c = U'\\';
}

void Aux_expr_scaner::hat_final_proc()
{
    token.code = Aux_expr_lexem_code::Character;
    token.c    = U'^';
}