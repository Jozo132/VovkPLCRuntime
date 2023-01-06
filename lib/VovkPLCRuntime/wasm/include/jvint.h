// jvint.h - J.Vovk stdint.h alternative for standalone WASM

#pragma once

typedef unsigned char byte;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef unsigned long uintptr_t;

typedef long intptr_t;
typedef unsigned int size_t;

#define UINT8_MAX 255
#define UINT16_MAX 65535
#define UINT32_MAX 4294967295
#define UINT64_MAX 18446744073709551615
#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807
#define INT8_MIN -128
#define INT16_MIN -32768
#define INT32_MIN -2147483648
#define INT64_MIN -9223372036854775808

#define intmax_t long long
#define uintmax_t unsigned long long

#define ptrdiff_t intptr_t

#define bool char
#define boolean char
#define true 1
#define false 0

#define NULL 0
#define nullptr 0