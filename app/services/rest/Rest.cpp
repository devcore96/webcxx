#include "Rest.hpp"

namespace rest {
    void swap      (url& lhs, url& rhs) noexcept { return lhs.swap(rhs); }
    bool operator==(url& lhs, url& rhs) noexcept { return lhs.native() == rhs.native(); }
    bool operator!=(url& lhs, url& rhs) noexcept { return lhs.native() != rhs.native(); }
     url operator /(url& lhs, url& rhs) noexcept { return url(lhs) /= rhs; }

    curl_initializer _initializer { };

    int write_callback(char* data, size_t size, size_t nmemb,
                       std::string* buffer) {
        if (buffer == nullptr)
            return 0;

        buffer->append(data, size * nmemb);

        return size * nmemb;
    }

    response get(const url& u, std::vector<header> headers) {
        CURL* curl = curl_easy_init();

        if (!curl) {
            throw rest_error("Failed to initialize CURL connection.");
        }

        response res;

        curl_easy_setopt(curl, CURLOPT_URL, u.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res.data);

        curl_slist* headers_chunk = nullptr;

        for (header& h : headers)
            headers_chunk = curl_slist_append(headers_chunk, h.c_str());

        if (headers_chunk)
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_chunk);

        CURLcode ec = curl_easy_perform(curl);

        if (ec) {
            curl_easy_cleanup(curl);
            throw rest_error("Failed to send GET request: " + std::string(curl_easy_strerror(ec)), u);
        }

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.status_code);
        curl_easy_cleanup(curl);

        return res;
    }

    response post(const url& u, std::string data, std::vector<header> headers) {
        CURL* curl = curl_easy_init();

        if (!curl) {
            throw rest_error("Failed to initialize CURL connection.");
        }

        response res;

        curl_easy_setopt(curl, CURLOPT_URL, u.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res.data);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

        curl_slist* headers_chunk = nullptr;

        for (header& h : headers)
            headers_chunk = curl_slist_append(headers_chunk, h.c_str());

        if (headers_chunk)
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_chunk);

        CURLcode ec = curl_easy_perform(curl);

        if (ec) {
            curl_easy_cleanup(curl);
            std::string str = std::string(curl_easy_strerror(ec));
            throw rest_error("Failed to send POST request: " + str, u);
        }

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.status_code);
        curl_easy_cleanup(curl);

        return res;
    }

    response post(const url& u, json data, std::vector<header> headers) {
        return post(u, (std::string)data, headers);
    }

    response post(const url& u, const char* data, std::vector<header> headers) {
        return post(u, std::string(data), headers);
    }

    response do_delete(const url& u, std::vector<header> headers) {
        CURL* curl = curl_easy_init();

        if (!curl) {
            throw rest_error("Failed to initialize CURL connection.");
        }

        response res;

        curl_easy_setopt(curl, CURLOPT_URL, u.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res.data);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

        curl_slist* headers_chunk = nullptr;

        for (header& h : headers)
            headers_chunk = curl_slist_append(headers_chunk, h.c_str());

        if (headers_chunk)
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_chunk);

        CURLcode ec = curl_easy_perform(curl);

        if (ec) {
            curl_easy_cleanup(curl);
            throw rest_error("Failed to send DELETE request: " + std::string(curl_easy_strerror(ec)), u);
        }

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.status_code);
        curl_easy_cleanup(curl);

        return res;
    }
}
