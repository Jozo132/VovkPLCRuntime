// stack-struct-impl.h - 2022-12-11
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

#include "stack-struct.h"

template <typename T> Stack<T>::Stack(u32 max_size) { format(); }

template <typename T> void Stack<T>::format(u32 fill_size) {
    _size = fill_size;
    for (u32 i = 0; i < _size; i++) _data[i] = 0;
}
template <typename T> bool Stack<T>::push(T value) {
    if (_size >= MAX_STACK_SIZE) {
        Serial.println(F("Stack overflow!"));
        Serial.print(F("Trying to push ")); Serial.print(value, HEX); Serial.print(F(" to stack at index ")); Serial.println(_size);
        Serial.print(F("MAX_STACK_SIZE = ")); Serial.println(MAX_STACK_SIZE);
        return true;
    }
    _data[_size] = value;
    _size++;
    return false;
}
template <typename T> T Stack<T>::pop() {
    if (_size == 0) return T();
    _size--;
    return _data[_size];
}
template <typename T> void Stack<T>::pop(u32 size) {
    if (size > _size) size = _size;
    _size -= size;
}
template <typename T> T Stack<T>::peek(u32 depth) {
    if (_size == 0) return T();
    if (depth >= _size) return T();
    return _data[_size - depth - 1];
}
template <typename T> T Stack<T>::operator [](u32 index) {
    if (index >= _size) return T();
    return _data[index];
}
template <typename T> bool Stack<T>::set(u32 index, T value) {
    if (index >= _size) return true;
    _data[index] = value;
    return false;
}
template <typename T> bool Stack<T>::get(u32 index, T& value) {
    if (index >= _size) return true;
    value = _data[index];
    return false;
}
template <typename T> bool Stack<T>::setBit(u32 index, uint8_t bit, bool value) {
    if (index >= _size) return true;
    if (bit >= 8) return true;
    if (value) _data[index] |= 1 << bit;
    else _data[index] &= ~(1 << bit);
    return false;
}
template <typename T> bool Stack<T>::getBit(u32 index, uint8_t bit, bool& value) {
    if (index >= _size) return true;
    if (bit >= 8) return true;
    value = _data[index] & (1 << bit);
    return false;
}
template <typename T> u32 Stack<T>::size() { return _size; }
template <typename T> bool Stack<T>::empty() { return _size == 0; }
template <typename T> void Stack<T>::clear() { _size = 0; }
template <typename T> int Stack<T>::print() {
    int length = Serial.print(F("Stack("));
    if (_size < 10) length += Serial.print(' ');
    length += Serial.print(_size);
    length += Serial.print(F(") ["));
    if (_size > 16) { // Show at most the last 16 bytes on the stack
        u32 hidden = _size - 16;
        length += Serial.print(' ');
        if (hidden < 10) length += Serial.print(' ');
        length += Serial.print(hidden);
        length += Serial.print(F(" more bytes... "));
        for (u32 i = hidden; i < _size; i++) {
            T value = _data[i];
            if (value < 0x10) length += Serial.print('0');
            length += Serial.print(value, HEX);
            length += Serial.print(' ');
        }
        length += Serial.print(']');
        return length;
    }
    for (u32 i = 0; i < _size; i++) {
        T value = _data[i];
        if (value < 0x10) length += Serial.print('0');
        length += Serial.print(value, HEX);
        if (i < _size - 1) length += Serial.print(' ');
    }
    length += Serial.print(']');
    return length;
}
template <typename T> void Stack<T>::println() {
    print();
    Serial.println();
}

template <typename T> bool Stack<T>::readArea(u32 offset, u8* value, u32 size) {
    bool error = false;
    u8 temp = 0;
    for (u32 i = 0; i < size; i++) {
        error = get(offset + i, temp);
        value[i] = temp;
        if (error) return error;
    }
    return false;
}

template <typename T> bool Stack<T>::writeArea(u32 offset, u8* value, u32 size) {
    bool error = false;
    for (u32 i = 0; i < size; i++) {
        error = set(offset + i, value[i]);
        if (error) return error;
    }
    return false;
}




template <typename T> void LinkedList<T>::push(T value) {
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
template <typename T> T LinkedList<T>::pop() {
    if (head == nullptr) return T();
    Node* node = head;
    head = head->next;
    T value = node->value;
    delete node;
    node = nullptr;
    _size--;
    return value;
}
template <typename T> T LinkedList<T>::peek(u32 depth) {
    if (head == nullptr) return T();
    Node* node = head;
    for (u32 i = 0; i < depth; i++) {
        if (node->next == nullptr) return T();
        node = node->next;
    }
    return node->value;
}
template <typename T> u32 LinkedList<T>::size() { return _size; }
template <typename T> bool LinkedList<T>::empty() { return _size == 0; }
template <typename T> void LinkedList<T>::clear() {
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
template <typename T> int LinkedList<T>::print() {
    int length = Serial.print(F("List("));
    length += Serial.print(_size);
    length += Serial.print(F(") ["));
    if (_size > 16) {
        u32 hidden = _size - 16;
        length += Serial.print(' ');
        if (hidden < 10) length += Serial.print(' ');
        length += Serial.print(hidden);
        length += Serial.print(F(" more bytes... "));
        Node* node = head;
        for (u32 i = 0; i < hidden; i++) {
            node = node->next;
        }
        for (u32 i = 0; i < 16; i++) {
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
    for (u32 i = 0; i < _size; i++) {
        T value = node->value;
        if (value < 0x10) length += Serial.print('0');
        length += Serial.print(value, HEX);
        if (i < _size - 1) length += Serial.print(' ');
        node = node->next;
    }
    length += Serial.print(']');
    return length;
}