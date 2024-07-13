#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <any>
#include <functional>

#include "Rest.hpp"

constexpr const char* get_response_message(unsigned int response_code) {
    switch(response_code) {
        case 200: return "OK";
        case 201: return "Created";
        case 202: return "Accepted";
        case 204: return "No Content";
        case 205: return "Reset Content";
        case 300: return "Multiple Choice";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 307: return "Temporary Redirect";
        case 308: return "Permanent Redirect";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 408: return "Request Timeout";
        case 500: return "Internal Server Error";
    }

    return "Unknown";
}

struct response {
    unsigned int             response_code;
    std::vector<std::string> headers;

    std::string render_headers() {
        std::string output;

        for(auto& header : headers)
            output += header + "\n";
        
        output += "Status: " + std::to_string(response_code) + " " + get_response_message(response_code) + "\n";
        output += "\n";

        return output;
    }

    virtual std::string render() = 0;
};

class html_response : public response {
public:
    std::string html;

    std::string render() {
        return render_headers() + html;
    }
};

class data_response : public response {
public:
    std::string data;

    std::string render() {
        return render_headers() + data;
    }
};

[[deprecated("Use view(rest::response) instead")]]
inline std::unique_ptr<html_response> view(std::string html, unsigned int response_code = 200) {
    std::unique_ptr<html_response> res { new html_response };

    res->html          = html;
    res->response_code = response_code;

    res->headers.push_back("Content-Type: text/html");

    return res;
}

[[deprecated("Use reply(rest::response) instead")]]
inline std::unique_ptr<data_response> reply(std::string data, std::string content_type, unsigned int response_code = 200) {
    std::unique_ptr<data_response> res { new data_response };

    res->data          = data;
    res->response_code = response_code;

    res->headers.push_back("Content-Type: " + content_type);

    return res;
}

inline std::unique_ptr<html_response> view(rest::response response) {
    std::unique_ptr<html_response> res { new html_response };

    res->html          = response.data;
    res->response_code = response.status_code;

    res->headers.push_back("Content-Type: text/html");

    return res;
}

inline std::unique_ptr<data_response> reply(std::string content_type, rest::response response) {
    std::unique_ptr<data_response> res { new data_response };

    res->data          = response.data;
    res->response_code = response.status_code;

    res->headers.push_back("Content-Type: " + content_type);

    return res;
}

inline std::unique_ptr<data_response> reply(rest::json json, int response_code = 200) {
    return reply("application/json", {
        .status_code = response_code,
        .data = json
    });
}
