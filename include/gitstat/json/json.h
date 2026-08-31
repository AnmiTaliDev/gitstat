#ifndef GITSTAT_JSON_JSON_H
#define GITSTAT_JSON_JSON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "gitstat/error.h"

typedef enum json_type {
    JSON_TYPE_NULL = 0,
    JSON_TYPE_BOOL,
    JSON_TYPE_NUMBER,
    JSON_TYPE_STRING,
    JSON_TYPE_ARRAY,
    JSON_TYPE_OBJECT
} json_type_t;

typedef struct json_value json_value_t;

typedef struct json_member {
    char *key;
    json_value_t *value;
    struct json_member *next;
} json_member_t;

typedef struct json_array_node {
    json_value_t *value;
    struct json_array_node *next;
} json_array_node_t;

struct json_value {
    json_type_t type;
    union {
        bool bool_value;
        double number_value;
        char *string_value;
        struct {
            json_array_node_t *head;
            size_t count;
        } array_value;
        struct {
            json_member_t *head;
            size_t count;
        } object_value;
    } as;
};

[[nodiscard]] json_value_t *json_parse(const char *text, size_t length, gitstat_error_t *err);
void json_value_free(json_value_t *val);

[[nodiscard]] json_type_t json_get_type(const json_value_t *val);
[[nodiscard]] bool json_is_null(const json_value_t *val);
[[nodiscard]] bool json_get_bool(const json_value_t *val);
[[nodiscard]] double json_get_number(const json_value_t *val);
[[nodiscard]] int64_t json_get_int64(const json_value_t *val);
[[nodiscard]] const char *json_get_string(const json_value_t *val);

[[nodiscard]] size_t json_array_length(const json_value_t *arr);
[[nodiscard]] json_value_t *json_array_get(const json_value_t *arr, size_t index);

[[nodiscard]] json_value_t *json_object_get(const json_value_t *obj, const char *key);
[[nodiscard]] const char *json_object_get_string(const json_value_t *obj, const char *key);
[[nodiscard]] int64_t json_object_get_int64(const json_value_t *obj, const char *key);
[[nodiscard]] bool json_object_get_bool(const json_value_t *obj, const char *key);

#endif
