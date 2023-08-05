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

template <typename T> struct Stack {
    T* _data = nullptr;
    uint32_t MAX_STACK_SIZE = 0;
    uint32_t _size = 0;

    Stack(uint32_t max_size = 0) {
        format(max_size);
    }
    ~Stack() {
        if (_data != nullptr) delete [] _data;
        _data = nullptr;
    }

    void format(uint32_t size) {
        if (_data != nullptr) delete [] _data;
        _data = new T[size];
        MAX_STACK_SIZE = size;
        _size = MAX_STACK_SIZE;
    }

    // Pushes a value to the top of the stack
    void push(T value) {
        if (_size >= MAX_STACK_SIZE) {
            Serial.println(F("Stack overflow!"));
            Serial.print(F("Trying to push ")); Serial.print(value, HEX); Serial.print(F(" to stack at index ")); Serial.println(_size);
            Serial.print(F("MAX_STACK_SIZE = ")); Serial.println(MAX_STACK_SIZE);
            return;
        }
        _data[_size] = value;
        _size++;
    }

    // Pops the top value from the stack
    T pop() {
        if (_size == 0) return T();
        _size--;
        return _data[_size];
    }
    void pop(uint32_t size) {
        if (size > _size) size = _size;
        _size -= size;
    }

    // Returns the top value from the stack
    T peek(uint32_t depth = 0) {
        if (_size == 0) return T();
        if (depth >= _size) return T();
        return _data[_size - depth - 1];
    }

    // Returns the value at the specified index like an array
    // T value = stack[0];
    T operator [](uint32_t index) {
        if (index >= _size) return T();
        return _data[index];
    }

    bool set(uint32_t index, T value) {
        if (index >= _size) return true;
        _data[index] = value;
        return false;
    }

    bool get(uint32_t index, T& value) {
        if (index >= _size) return true;
        value = _data[index];
        return false;
    }

    bool setBit(uint32_t index, uint8_t bit, bool value) {
        if (index >= _size) return true;
        if (bit >= 8) return true;
        if (value) _data[index] |= 1 << bit;
        else _data[index] &= ~(1 << bit);
        return false;
    }
    
    bool getBit(uint32_t index, uint8_t bit, bool& value) {
        if (index >= _size) return true;
        if (bit >= 8) return true;
        value = _data[index] & (1 << bit);
        return false;
    }


    // Returns the number of elements in the stack
    uint32_t size() { return _size; }

    // Returns true if the stack is empty
    bool empty() { return _size == 0; }

    // Removes all elements from the stack
    void clear() { _size = 0; }

    // Prints the stack to the serial port, where the tail is on the left and the head is on the right.
    int print() {
        int length = Serial.print(F("Stack("));
        if (_size < 10) length += Serial.print(' ');
        length += Serial.print(_size);
        length += Serial.print(F(") ["));
        if (_size > 16) {
            uint32_t hidden = _size - 16;
            length += Serial.print(' ');
            if (hidden < 10) length += Serial.print(' ');
            length += Serial.print(hidden);
            length += Serial.print(F(" more bytes... "));
            for (uint32_t i = 0; i < 16; i++) {
                T value = _data[hidden + i];
                if (value < 0x10) length += Serial.print('0');
                length += Serial.print(value, HEX);
                length += Serial.print(' ');
            }
            length += Serial.print(']');
            return length;
        }
        for (uint32_t i = 0; i < _size; i++) {
            T value = _data[i];
            if (value < 0x10) length += Serial.print('0');
            length += Serial.print(value, HEX);
            if (i < _size - 1) length += Serial.print(' ');
        }
        length += Serial.print(']');
        return length;
    }
    void println() {
        print();
        Serial.println();
    }
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
    void push(T value) {
        Node* node = new Node();
        node->value = value;
        if (head == nullptr) {
            head = node;
            tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
        _size++;
    }

    // Pops the first value from the list
    T pop() {
        if (head == nullptr) return T();
        Node* node = head;
        head = head->next;
        T value = node->value;
        delete node;
        node = nullptr;
        _size--;
        return value;
    }

    // Returns the first value from the list
    T peek(uint32_t depth = 0) {
        if (head == nullptr) return T();
        Node* node = head;
        for (uint32_t i = 0; i < depth; i++) {
            if (node->next == nullptr) return T();
            node = node->next;
        }
        return node->value;
    }

    // Returns the number of elements in the list
    uint32_t size() { return _size; }

    // Returns true if the list is empty
    bool empty() { return _size == 0; }

    // Removes all elements from the list
    void clear() {
        Node* node = head;
        while (node != nullptr) {
            Node* next = node->next;
            delete node;
            node = next;
        }
        head = nullptr;
        tail = nullptr;
        _size = 0;
    }

    // Prints the list to the serial port, where the head is on the left and the tail is on the right.
    int print() {
        int length = Serial.print(F("List("));
        length += Serial.print(_size);
        length += Serial.print(F(") ["));
        if (_size > 16) {
            uint32_t hidden = _size - 16;
            length += Serial.print(' ');
            if (hidden < 10) length += Serial.print(' ');
            length += Serial.print(hidden);
            length += Serial.print(F(" more bytes... "));
            Node* node = head;
            for (uint32_t i = 0; i < hidden; i++) {
                node = node->next;
            }
            for (uint32_t i = 0; i < 16; i++) {
                T value = node->value;
                if (value < 0x10) length += Serial.print('0');
                length += Serial.print(value, HEX);
                length += Serial.print(' ');
                node = node->next;
            }
            length += Serial.print(']');
            return length;
        }
        Node* node = head;
        for (uint32_t i = 0; i < _size; i++) {
            T value = node->value;
            if (value < 0x10) length += Serial.print('0');
            length += Serial.print(value, HEX);
            if (i < _size - 1) length += Serial.print(' ');
            node = node->next;
        }
        length += Serial.print(']');
        return length;
    }
};