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
