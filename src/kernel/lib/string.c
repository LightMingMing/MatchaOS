//
// Created by 赵明明 on 2019/11/19.
//

#include "string.h"

size_t strlen(const char *s) {
    size_t cnt = 0;
    while (*s++ != '\0') {
        cnt++;
    }
    return cnt;
}

size_t strnlen(const char *s, size_t len) {
    size_t cnt = 0;
    while (*s++ != '\0' && cnt < len) {
        cnt++;
    }
    return cnt;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 != '\0' && *s1 == *s2) {
        s1++, s2++;
    }
    return (int) ((unsigned char) *s1 - (unsigned char) *s2);
}