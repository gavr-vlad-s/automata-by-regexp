/*
    File:    multipop_stack.h
    Created: 13 December 2015 at 09:05 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef MULTIPOP_STACK_H
#define MULTIPOP_STACK_H

#include <vector>
#include <cstdio>

template<typename T>
class Multipop_stack {
public:
    Multipop_stack()                           = default;
    Multipop_stack(const Multipop_stack& orig) = default;
    ~Multipop_stack()                          = default;
    T    top();
    void pop();
    void push(const T& elem);
    void get_elems_from_top(T result[], size_t number_of_elems) const;
    void multi_pop(size_t number_of_elems);
    bool empty() const;
    size_t size() const;
    void print(void (*print_elem)(const T&)) const;
    void clear();
private:
    std::vector<T> s;
    size_t         top_index = 0;
};

template<typename T>
T Multipop_stack<T>::top()
{
    return s[top_index - 1];
}

template<typename T>
void Multipop_stack<T>::pop()
{
    s.pop_back();
    if(top_index != 0){top_index--;};
    return;
}

template<typename T>
void Multipop_stack<T>::push(const T& elem)
{
    s.push_back(elem);
    top_index++;
    return;
}

template<typename T>
void Multipop_stack<T>::get_elems_from_top(T result[], size_t number_of_elems) const
{
    if(top_index >= number_of_elems){
        size_t temp = top_index - number_of_elems;
            for(size_t i = temp; i < top_index; i++ ){
                result[i - temp] = s[i];
            }
    }
    return;
}

template<typename T>
void Multipop_stack<T>::multi_pop(size_t number_of_elems)
{
    s.erase(s.end() - number_of_elems,  s.end());
    if(top_index >= number_of_elems){
        top_index -= number_of_elems;
    }
    else{
        top_index = 0;
    }
    return;
}

template<typename T>
bool Multipop_stack<T>::empty() const
{
    return top_index == 0;
}

template<typename T>
size_t Multipop_stack<T>::size() const
{
    return top_index;
}

template<typename T>
void Multipop_stack<T>::print(void (*print_elem)(const T&)) const
{
    for(const T& e : s){
        print_elem(e); putchar(' ');
    }
}


template<typename T>
void Multipop_stack<T>::clear()
{
    s.clear();
    top_index = 0;
}
#endif