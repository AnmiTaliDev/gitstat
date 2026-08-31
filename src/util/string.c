#include "gitstat/util/string.h"
#include "gitstat/util/memory.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

char *gitstat_strdup(const char *str) {
    if (str == nullptr) {
        return nullptr;
    }
    size_t len = strlen(str);
    char *copy = (char *)gitstat_alloc(len + 1);
    if (copy == nullptr) {
        return nullptr;
    }
    memcpy(copy, str, len + 1);
    return copy;
}

char *gitstat_strndup(const char *str, size_t n) {
    if (str == nullptr) {
        return nullptr;
    }
    size_t len = 0;
    while (len < n && str[len] != '\0') {
        len++;
    }
    char *copy = (char *)gitstat_alloc(len + 1);
    if (copy == nullptr) {
        return nullptr;
    }
    memcpy(copy, str, len);
    copy[len] = '\0';
    return copy;
}

bool gitstat_str_is_empty(const char *str) {
    return str == nullptr || str[0] == '\0';
}

bool gitstat_str_equals_ignore_case(const char *a, const char *b) {
    if (a == nullptr && b == nullptr) {
        return true;
    }
    if (a == nullptr || b == nullptr) {
        return false;
    }
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return false;
        }
        a++;
        b++;
    }
    return *a == *b;
}

void gitstat_str_trim(char *str) {
    if (str == nullptr || str[0] == '\0') {
        return;
    }
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    char *end = str + strlen(str) - 1;
    while (end >= start && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

char *gitstat_str_escape_json(const char *input) {
    if (input == nullptr) {
        return gitstat_strdup("");
    }
    size_t len = strlen(input);
    size_t extra = 0;
    for (size_t i = 0; i < len; ++i) {
        switch (input[i]) {
            case '"':
            case '\\':
            case '\b':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
                extra += 1;
                break;
            default:
                break;
        }
    }
    char *out = (char *)gitstat_alloc(len + extra + 1);
    if (out == nullptr) {
        return nullptr;
    }
    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        switch (input[i]) {
            case '"':
                out[j++] = '\\';
                out[j++] = '"';
                break;
            case '\\':
                out[j++] = '\\';
                out[j++] = '\\';
                break;
            case '\b':
                out[j++] = '\\';
                out[j++] = 'b';
                break;
            case '\f':
                out[j++] = '\\';
                out[j++] = 'f';
                break;
            case '\n':
                out[j++] = '\\';
                out[j++] = 'n';
                break;
            case '\r':
                out[j++] = '\\';
                out[j++] = 'r';
                break;
            case '\t':
                out[j++] = '\\';
                out[j++] = 't';
                break;
            default:
                out[j++] = input[i];
                break;
        }
    }
    out[j] = '\0';
    return out;
}
