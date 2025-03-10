// jvmath.h - 1.0.0 - 2022-12-11
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

#include "jvint.h"

#define abs(x) ((x) < 0 ? -(x) : (x))
#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define sign(x) ((x) < 0 ? -1 : 1)
#define clamp(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

#define PI 3.14159265358979323846264338
#define E 2.71828182845904523536028747
#define PHI 1.61803398874989484820458683
#define SQRT2 1.41421356237309504880168872
#define SQRT3 1.73205080756887729352744634

float sqrt(float x);
float exp(float x);
float factorial(float x);
float log(float x);
float sin(float x);
float cos(float x);
float tan(float x);
float asin(float x);
float acos(float x);
float atan(float x);
float atan2(float y, float x);
float sinh(float x);
float pow(float x, float y);
float fmod(float x, float y);
float fabs(float x);


float sqrt(float x) {
    float xhalf = 0.5f * x;
    int i = *(int*) &x; // get bits for floating value
    i = 0x5f375a86 - (i >> 1); // gives initial guess y0
    x = *(float*) &i; // convert bits back to float
    x = x * (1.5f - xhalf * x * x); // Newton step, repeating increases accuracy
    x = x * (1.5f - xhalf * x * x); // Newton step, repeating increases accuracy
    x = x * (1.5f - xhalf * x * x); // Newton step, repeating increases accuracy
    return 1 / x;
}

float exp(float x) {
    float result = 1;
    for (int i = 0; i < 100; i++) {
        result += pow(x, i) / factorial(i);
    }
    return result;
}

float factorial(float x) {
    float result = 1;
    for (int i = 1; i < x; i++) {
        result *= i;
    }
    return result;
}

float log(float x) {
    float result = 0;
    for (int i = 1; i < 100; i++) {
        result += pow(-1, i + 1) * pow(x - 1, i) / i;
    }
    return result;
}

float sin(float x) {
    float result = 0;
    for (int i = 0; i < 100; i++) {
        result += pow(-1, i) * pow(x, 2 * i + 1) / factorial(2 * i + 1);
    }
    return result;
}

float cos(float x) {
    float result = 0;
    for (int i = 0; i < 100; i++) {
        result += pow(-1, i) * pow(x, 2 * i) / factorial(2 * i);
    }
    return result;
}

float tan(float x) {
    return sin(x) / cos(x);
}

float asin(float x) {
    float result = 0;
    for (int i = 0; i < 100; i++) {
        result += factorial(2 * i) * pow(x, 2 * i + 1) / (pow(4, i) * pow(factorial(i), 2) * (2 * i + 1));
    }
    return result;
}

float acos(float x) {
    return PI / 2 - asin(x);
}

float atan(float x) {
    float result = 0;
    for (int i = 0; i < 100; i++) {
        result += pow(-1, i) * pow(x, 2 * i + 1) / (2 * i + 1);
    }
    return result;
}

float atan2(float y, float x) {
    return atan(y / x);
}

float sinh(float x) {
    float result = 0;
    for (int i = 0; i < 100; i++) {
        result += pow(x, 2 * i + 1) / factorial(2 * i + 1);
    }
    return result;
}

float pow(float x, float y) {
    return exp(y * log(x));
}

float fmod(float x, float y) {
    return x - y * (int) (x / y);
}

float fabs(float x) {
    return x < 0 ? -x : x;
}