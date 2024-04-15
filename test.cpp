#include "mbed.h"
#include <cstring>
#include <cstdio>

int main() {
    const char* str1 = "hello";
    const char* str2 = "hello";

    if (strcmp(str1, str2) == 0) {
        printf("Strings are equal\n");
    } else {
        printf("Strings are not equal\n");
    }
}