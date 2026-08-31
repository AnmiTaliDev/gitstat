#include "gitstat/http/client.h"
#include "gitstat/http/http.h"
#include <curl/curl.h>
#include <string.h>

static size_t gitstat_curl_write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    gitstat_buffer_t *buf = (gitstat_buffer_t *)userp;
    if (gitstat_buffer_append(buf, (const char *)contents, total_size) != GITSTAT_OK) {
        return 0;
    }
    return total_size;
}

gitstat_error_t gitstat_http_global_init(void) {
    CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
    return (code == CURLE_OK) ? GITSTAT_OK : GITSTAT_ERR_HTTP;
}

void gitstat_http_global_cleanup(void) {
    curl_global_cleanup();
}

gitstat_error_t gitstat_http_execute(const gitstat_http_request_t *req, gitstat_http_response_t *res) {
    if (req == nullptr || req->url == nullptr || res == nullptr) {
        return GITSTAT_ERR_INVALID_ARG;
    }

    CURL *curl = curl_easy_init();
    if (curl == nullptr) {
        return GITSTAT_ERR_HTTP;
    }

    gitstat_buffer_clear(&res->body);
    res->status_code = 0;

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/vnd.github.v3+json, application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");

    if (req->user_agent != nullptr) {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, req->user_agent);
    } else {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "gitstat/1.0");
    }

    if (req->token != nullptr && req->token[0] != '\0') {
        char auth_header[512];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", req->token);
        headers = curl_slist_append(headers, auth_header);
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, req->url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, gitstat_curl_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&res->body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, req->timeout_seconds > 0 ? req->timeout_seconds : 10L);

    if (req->method != nullptr && strcmp(req->method, "POST") == 0) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (req->post_data != nullptr) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->post_data);
        }
    }

    CURLcode code = curl_easy_perform(curl);
    if (code != CURLE_OK) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return GITSTAT_ERR_HTTP;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res->status_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return GITSTAT_OK;
}
