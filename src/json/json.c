#include "gitstat/json/json.h"
#include "gitstat/util/memory.h"
#include "gitstat/util/string.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct json_parser {
    const char *src;
    size_t len;
    size_t pos;
    gitstat_error_t error;
} json_parser_t;

static void json_parser_skip_whitespace(json_parser_t *p) {
    while (p->pos < p->len && isspace((unsigned char)p->src[p->pos])) {
        p->pos++;
    }
}

static char json_parser_peek(json_parser_t *p) {
    json_parser_skip_whitespace(p);
    if (p->pos >= p->len) {
        return '\0';
    }
    return p->src[p->pos];
}

static char json_parser_get(json_parser_t *p) {
    json_parser_skip_whitespace(p);
    if (p->pos >= p->len) {
        return '\0';
    }
    return p->src[p->pos++];
}

static json_value_t *json_value_alloc(json_type_t type) {
    json_value_t *v = (json_value_t *)gitstat_calloc(1, sizeof(json_value_t));
    if (v != nullptr) {
        v->type = type;
    }
    return v;
}

static json_value_t *json_parse_value(json_parser_t *p);

static char *json_parse_raw_string(json_parser_t *p) {
    json_parser_skip_whitespace(p);
    if (p->pos >= p->len || p->src[p->pos] != '"') {
        p->error = GITSTAT_ERR_JSON_PARSE;
        return nullptr;
    }
    p->pos++;

    size_t start = p->pos;
    size_t out_len = 0;
    while (p->pos < p->len && p->src[p->pos] != '"') {
        if (p->src[p->pos] == '\\') {
            p->pos++;
            if (p->pos >= p->len) {
                p->error = GITSTAT_ERR_JSON_PARSE;
                return nullptr;
            }
        }
        p->pos++;
        out_len++;
    }

    if (p->pos >= p->len || p->src[p->pos] != '"') {
        p->error = GITSTAT_ERR_JSON_PARSE;
        return nullptr;
    }

    char *result = (char *)gitstat_alloc(out_len + 1);
    if (result == nullptr) {
        p->error = GITSTAT_ERR_NOMEM;
        return nullptr;
    }

    size_t src_idx = start;
    size_t dst_idx = 0;
    while (src_idx < p->pos) {
        if (p->src[src_idx] == '\\') {
            src_idx++;
            switch (p->src[src_idx]) {
                case '"': result[dst_idx++] = '"'; break;
                case '\\': result[dst_idx++] = '\\'; break;
                case '/': result[dst_idx++] = '/'; break;
                case 'b': result[dst_idx++] = '\b'; break;
                case 'f': result[dst_idx++] = '\f'; break;
                case 'n': result[dst_idx++] = '\n'; break;
                case 'r': result[dst_idx++] = '\r'; break;
                case 't': result[dst_idx++] = '\t'; break;
                default: result[dst_idx++] = p->src[src_idx]; break;
            }
            src_idx++;
        } else {
            result[dst_idx++] = p->src[src_idx++];
        }
    }
    result[dst_idx] = '\0';
    p->pos++;
    return result;
}

static json_value_t *json_parse_string(json_parser_t *p) {
    char *str = json_parse_raw_string(p);
    if (str == nullptr) {
        return nullptr;
    }
    json_value_t *val = json_value_alloc(JSON_TYPE_STRING);
    if (val == nullptr) {
        gitstat_free(str);
        p->error = GITSTAT_ERR_NOMEM;
        return nullptr;
    }
    val->as.string_value = str;
    return val;
}

static json_value_t *json_parse_number(json_parser_t *p) {
    json_parser_skip_whitespace(p);
    size_t start = p->pos;
    if (p->pos < p->len && (p->src[p->pos] == '-' || p->src[p->pos] == '+')) {
        p->pos++;
    }
    while (p->pos < p->len && (isdigit((unsigned char)p->src[p->pos]) || p->src[p->pos] == '.' || p->src[p->pos] == 'e' || p->src[p->pos] == 'E' || p->src[p->pos] == '-' || p->src[p->pos] == '+')) {
        p->pos++;
    }

    size_t len = p->pos - start;
    char *num_str = gitstat_strndup(p->src + start, len);
    if (num_str == nullptr) {
        p->error = GITSTAT_ERR_NOMEM;
        return nullptr;
    }

    char *endptr = nullptr;
    double dval = strtod(num_str, &endptr);
    gitstat_free(num_str);

    json_value_t *val = json_value_alloc(JSON_TYPE_NUMBER);
    if (val == nullptr) {
        p->error = GITSTAT_ERR_NOMEM;
        return nullptr;
    }
    val->as.number_value = dval;
    return val;
}

static json_value_t *json_parse_array(json_parser_t *p) {
    json_parser_get(p);
    json_value_t *val = json_value_alloc(JSON_TYPE_ARRAY);
    if (val == nullptr) {
        p->error = GITSTAT_ERR_NOMEM;
        return nullptr;
    }

    if (json_parser_peek(p) == ']') {
        json_parser_get(p);
        return val;
    }

    json_array_node_t **tail = &val->as.array_value.head;
    while (true) {
        json_value_t *element = json_parse_value(p);
        if (element == nullptr) {
            json_value_free(val);
            return nullptr;
        }

        json_array_node_t *node = (json_array_node_t *)gitstat_calloc(1, sizeof(json_array_node_t));
        if (node == nullptr) {
            json_value_free(element);
            json_value_free(val);
            p->error = GITSTAT_ERR_NOMEM;
            return nullptr;
        }
        node->value = element;
        *tail = node;
        tail = &node->next;
        val->as.array_value.count++;

        char next = json_parser_peek(p);
        if (next == ',') {
            json_parser_get(p);
        } else if (next == ']') {
            json_parser_get(p);
            break;
        } else {
            p->error = GITSTAT_ERR_JSON_PARSE;
            json_value_free(val);
            return nullptr;
        }
    }

    return val;
}

static json_value_t *json_parse_object(json_parser_t *p) {
    json_parser_get(p);
    json_value_t *val = json_value_alloc(JSON_TYPE_OBJECT);
    if (val == nullptr) {
        p->error = GITSTAT_ERR_NOMEM;
        return nullptr;
    }

    if (json_parser_peek(p) == '}') {
        json_parser_get(p);
        return val;
    }

    json_member_t **tail = &val->as.object_value.head;
    while (true) {
        char *key = json_parse_raw_string(p);
        if (key == nullptr) {
            json_value_free(val);
            return nullptr;
        }

        if (json_parser_get(p) != ':') {
            gitstat_free(key);
            p->error = GITSTAT_ERR_JSON_PARSE;
            json_value_free(val);
            return nullptr;
        }

        json_value_t *member_val = json_parse_value(p);
        if (member_val == nullptr) {
            gitstat_free(key);
            json_value_free(val);
            return nullptr;
        }

        json_member_t *member = (json_member_t *)gitstat_calloc(1, sizeof(json_member_t));
        if (member == nullptr) {
            gitstat_free(key);
            json_value_free(member_val);
            json_value_free(val);
            p->error = GITSTAT_ERR_NOMEM;
            return nullptr;
        }
        member->key = key;
        member->value = member_val;
        *tail = member;
        tail = &member->next;
        val->as.object_value.count++;

        char next = json_parser_peek(p);
        if (next == ',') {
            json_parser_get(p);
        } else if (next == '}') {
            json_parser_get(p);
            break;
        } else {
            p->error = GITSTAT_ERR_JSON_PARSE;
            json_value_free(val);
            return nullptr;
        }
    }

    return val;
}

static json_value_t *json_parse_value(json_parser_t *p) {
    json_parser_skip_whitespace(p);
    if (p->pos >= p->len) {
        p->error = GITSTAT_ERR_JSON_PARSE;
        return nullptr;
    }

    char c = p->src[p->pos];
    if (c == '{') {
        return json_parse_object(p);
    }
    if (c == '[') {
        return json_parse_array(p);
    }
    if (c == '"') {
        return json_parse_string(p);
    }
    if (isdigit((unsigned char)c) || c == '-') {
        return json_parse_number(p);
    }
    if (strncmp(p->src + p->pos, "true", 4) == 0) {
        p->pos += 4;
        json_value_t *val = json_value_alloc(JSON_TYPE_BOOL);
        if (val) val->as.bool_value = true;
        return val;
    }
    if (strncmp(p->src + p->pos, "false", 5) == 0) {
        p->pos += 5;
        json_value_t *val = json_value_alloc(JSON_TYPE_BOOL);
        if (val) val->as.bool_value = false;
        return val;
    }
    if (strncmp(p->src + p->pos, "null", 4) == 0) {
        p->pos += 4;
        return json_value_alloc(JSON_TYPE_NULL);
    }

    p->error = GITSTAT_ERR_JSON_PARSE;
    return nullptr;
}

json_value_t *json_parse(const char *text, size_t length, gitstat_error_t *err) {
    if (text == nullptr) {
        if (err) *err = GITSTAT_ERR_INVALID_ARG;
        return nullptr;
    }
    if (length == 0) {
        length = strlen(text);
    }

    json_parser_t p = {
        .src = text,
        .len = length,
        .pos = 0,
        .error = GITSTAT_OK
    };

    json_value_t *val = json_parse_value(&p);
    if (err) {
        *err = p.error;
    }
    return val;
}

void json_value_free(json_value_t *val) {
    if (val == nullptr) {
        return;
    }
    switch (val->type) {
        case JSON_TYPE_STRING:
            gitstat_free(val->as.string_value);
            break;
        case JSON_TYPE_ARRAY: {
            json_array_node_t *node = val->as.array_value.head;
            while (node != nullptr) {
                json_array_node_t *next = node->next;
                json_value_free(node->value);
                gitstat_free(node);
                node = next;
            }
            break;
        }
        case JSON_TYPE_OBJECT: {
            json_member_t *m = val->as.object_value.head;
            while (m != nullptr) {
                json_member_t *next = m->next;
                gitstat_free(m->key);
                json_value_free(m->value);
                gitstat_free(m);
                m = next;
            }
            break;
        }
        default:
            break;
    }
    gitstat_free(val);
}

json_type_t json_get_type(const json_value_t *val) {
    return val != nullptr ? val->type : JSON_TYPE_NULL;
}

bool json_is_null(const json_value_t *val) {
    return val == nullptr || val->type == JSON_TYPE_NULL;
}

bool json_get_bool(const json_value_t *val) {
    if (val == nullptr || val->type != JSON_TYPE_BOOL) {
        return false;
    }
    return val->as.bool_value;
}

double json_get_number(const json_value_t *val) {
    if (val == nullptr || val->type != JSON_TYPE_NUMBER) {
        return 0.0;
    }
    return val->as.number_value;
}

int64_t json_get_int64(const json_value_t *val) {
    if (val == nullptr || val->type != JSON_TYPE_NUMBER) {
        return 0;
    }
    return (int64_t)val->as.number_value;
}

const char *json_get_string(const json_value_t *val) {
    if (val == nullptr || val->type != JSON_TYPE_STRING) {
        return nullptr;
    }
    return val->as.string_value;
}

size_t json_array_length(const json_value_t *arr) {
    if (arr == nullptr || arr->type != JSON_TYPE_ARRAY) {
        return 0;
    }
    return arr->as.array_value.count;
}

json_value_t *json_array_get(const json_value_t *arr, size_t index) {
    if (arr == nullptr || arr->type != JSON_TYPE_ARRAY) {
        return nullptr;
    }
    json_array_node_t *node = arr->as.array_value.head;
    size_t i = 0;
    while (node != nullptr) {
        if (i == index) {
            return node->value;
        }
        node = node->next;
        i++;
    }
    return nullptr;
}

json_value_t *json_object_get(const json_value_t *obj, const char *key) {
    if (obj == nullptr || obj->type != JSON_TYPE_OBJECT || key == nullptr) {
        return nullptr;
    }
    json_member_t *m = obj->as.object_value.head;
    while (m != nullptr) {
        if (m->key != nullptr && strcmp(m->key, key) == 0) {
            return m->value;
        }
        m = m->next;
    }
    return nullptr;
}

const char *json_object_get_string(const json_value_t *obj, const char *key) {
    json_value_t *val = json_object_get(obj, key);
    return json_get_string(val);
}

int64_t json_object_get_int64(const json_value_t *obj, const char *key) {
    json_value_t *val = json_object_get(obj, key);
    return json_get_int64(val);
}

bool json_object_get_bool(const json_value_t *obj, const char *key) {
    json_value_t *val = json_object_get(obj, key);
    return json_get_bool(val);
}
