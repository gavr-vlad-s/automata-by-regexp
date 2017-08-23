/*
    File:    trie_for_set.h
    Created: 20 July 2017 at 15:06 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/
#ifndef TRIE_FOR_SET_H
#define TRIE_FOR_SET_H

#include "../include/trie.h"
#include <set>
#include <string>
#include <memory>

template<typename T>
class Trie_for_set : public Trie<T>{
public:
    virtual ~Trie_for_set<T>() { };
    Trie_for_set<T>()                            = default;
    Trie_for_set<T>(const Trie_for_set<T>& orig) = default;

    /**
     *  \brief The function get_set on the index idx of the set of values of
     *         type T builds the same set, but already as std::set< T >.
     *  \param [in] idx The index of the set of states in the prefix tree of such sets.
     *  \return         The same set, but already as std::set < T >.
     */
    std::set<T> get_set(size_t idx);
    size_t insertSet(const std::set<T>& s);
private:
    virtual void post_action(const std::basic_string<T>& s, size_t n);
};

template<typename T>
std::set<T> Trie_for_set<T>::get_set(size_t idx){
    std::set<T> s;
    size_t current = idx;
    for( ; current; current = Trie<T>::node_buffer[current].parent){
        s.insert(Trie<T>::node_buffer[current].c);
    }
    return s;
}

template<typename T>
void Trie_for_set<T>::post_action(const std::basic_string<T>& s, size_t n){
}

template<typename T>
size_t Trie_for_set<T>::insertSet(const std::set<T>& s){
    std::basic_string<T> str;
    for(auto ch : s){
        str += ch;
    }
    size_t idx = this->insert(str);
    return idx;
}

using Trie_for_set_of_char32    = Trie_for_set<char32_t>;
using Trie_for_set_of_sizet     = Trie_for_set<size_t>;
using Trie_for_set_of_char32ptr = std::shared_ptr<Trie_for_set_of_char32>;
using Trie_for_set_of_sizetptr  = std::shared_ptr<Trie_for_set_of_sizet>;
#endif