// runtime-stack.h - 1.0.0 - 2022-12-11
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
    T* stack = nullptr;
    uint16_t MAX_STACK_SIZE = 0;
    uint16_t _size = 0;

    Stack(uint16_t max_size) {
        MAX_STACK_SIZE = max_size;
        stack = new T[MAX_STACK_SIZE];
    }

    // Pushes a value to the top of the stack
    void push(T value) {
        if (_size >= MAX_STACK_SIZE) return;
        stack[_size] = value;
        _size++;
    }

    // Pops the top value from the stack
    T pop() {
        if (_size == 0) return T();
        _size--;
        return stack[_size];
    }

    // Returns the top value from the stack
    T peek(uint16_t depth = 0) {
        if (_size == 0) return T();
        if (depth >= _size) return T();
        return stack[_size - depth - 1];
    }

    // Returns the number of elements in the stack
    uint16_t size() { return _size; }

    // Returns true if the stack is empty
    bool empty() { return _size == 0; }

    // Removes all elements from the stack
    void clear() { _size = 0; }

    // Prints the stack to the serial port, where the tail is on the left and the head is on the right.
    //  Example: "Stack <Buffer 01 02 03 04> "
    // The buffer is printed in hexadecimal format.
    // If the buffer is empty, the string "Empty" is printed.
    //  Example: "Stack <Empty> "
    // If the buffer is bigger than 16 bytes, only the last 16 bytes are printed and the string "..." is added in front of the buffer.
    void print() {
        if (_size == 0) {
            Serial.print(F("Stack <Empty> "));
            return;
        }
        Serial.print(F("Stack[")); Serial.print(_size); Serial.print(F("] <"));
        Serial.print(F("Buffer "));
        if (_size > 16) {
            uint16_t hidden = _size - 16;
            Serial.print(hidden);
            Serial.print(F(" more bytes ... "));
            for (uint16_t i = 0; i < 16; i++) {
                T value = stack[hidden + i];
                if (value < 0x10) Serial.print('0');
                Serial.print(value, HEX);
                Serial.print(' ');
            }
            Serial.print('>');
            return;
        }
        for (uint16_t i = 0; i < _size; i++) {
            T value = stack[i];
            if (value < 0x10) Serial.print('0');
            Serial.print(value, HEX);
            if (i < _size - 1) Serial.print(' ');
        }
        Serial.print('>');
    }
    void println() {
        print();
        Serial.println();
    }

};