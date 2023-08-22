// stack-struct.h - 1.0.0 - 2022-12-11
//
// Copyright (c) 2022 J.Vovk
//
// This file is part of VovkPLCRuntime.
//
// VovkPLCRuntime is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// VovkPLCRuntime is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with VovkPLCRuntime.  If not, see <https://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../runtime-tools.h"

template <typename T> struct Stack {
    T* _data = nullptr;
    uint32_t MAX_STACK_SIZE = 0;
    uint32_t _size = 0;
    Stack(uint32_t max_size = 0);
    ~Stack();
    void format(uint32_t size);
    // Pushes a value to the top of the stack
    void push(T value);
    // Pops the top value from the stack
    T pop();
    void pop(uint32_t size);
    // Returns the top value from the stack
    T peek(uint32_t depth = 0);
    // Returns the value at the specified index like an array
    // T value = stack[0];
    T operator [](uint32_t index);
    bool set(uint32_t index, T value);
    bool get(uint32_t index, T& value);
    bool setBit(uint32_t index, uint8_t bit, bool value);
    bool getBit(uint32_t index, uint8_t bit, bool& value);
    // Returns the number of elements in the stack
    uint32_t size();
    // Returns true if the stack is empty
    bool empty();
    // Removes all elements from the stack
    void clear();
    // Prints the stack to the serial port, where the tail is on the left and the head is on the right.
    int print();
    void println();
};


template <typename T> struct LinkedList {
    struct Node {
        T value;
        Node* next = nullptr;
    };
    Node* head = nullptr;
    Node* tail = nullptr;
    uint32_t _size = 0;
    // Pushes a value to the end of the list
    void push(T value);
    // Pops the first value from the list
    T pop();
    // Returns the first value from the list
    T peek(uint32_t depth = 0);
    // Returns the number of elements in the list
    uint32_t size();
    // Returns true if the list is empty
    bool empty();
    // Removes all elements from the list
    void clear();
    // Prints the list to the serial port, where the head is on the left and the tail is on the right.
    int print();
};

#include "stack-struct-impl.h"