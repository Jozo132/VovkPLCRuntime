

#include "common.h"

char* str = nullptr;

WASM_EXPORT void doSomething() {
    if (!str) {
        str = (char*) malloc(14);
        const char* hello = "Hello, World!";
        memcpy(str, hello, 14);
    }
    char* c = (char*) str;
    while (*c != '\0') {
        printf("%c", *c);
        c++;
    }
    printf("\n");
    if (str[12] == '!') str[12] = '?';
    else str[12] = '!';
    // delete[] str;
}