/*
SmartPtr.h
*/

#pragma once

#define _SMARTPTR_H_
#ifndef _SMARTPTR_H_

#include <iostream>
using namespace std;

// A generic smart pointer class
template <class T>
class SmartPtr {
    T* ptr; // Actual pointer
public:
    // Constructor
    explicit SmartPtr(T* p = NULL) { ptr = p; }

    // Destructor
    ~SmartPtr() { delete (ptr); }

    // Overloading dereferencing operator
    T& operator*() { return *ptr; }

    // Overloading arrow operator so that
    // members of T can be accessed
    // like a pointer (useful if T represents
    // a class or struct or union type)
    T* operator->() { return ptr; }
};


#endif // _SMARTPTR_H_