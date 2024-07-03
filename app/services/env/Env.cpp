#include "Env.hpp"

std::shared_ptr<env_data> env;

env_data::env_data() {
    auto cgicc_env        = cgi.getEnvironment();

    request_method        = cgicc_env.getRequestMethod();
    query_string          = cgicc_env.getQueryString();

    content_type          = cgicc_env.getContentType();
    content_length        = cgicc_env.getContentLength();
    content_data          = cgicc_env.getPostData();

    auth_type             = cgicc_env.getAuthType();
    remote_host           = cgicc_env.getRemoteHost();
    remote_address        = cgicc_env.getRemoteAddr();
    remote_user           = cgicc_env.getRemoteUser();
    remote_identification = cgicc_env.getRemoteIdent();
    user_agent            = cgicc_env.getUserAgent();

    server_name           = cgicc_env.getServerName();
    server_port           = cgicc_env.getServerPort();

    https                 = cgicc_env.usingHTTPS();

    url                   = cgicc_env.getRedirectURL();

    try {
        post_data         = (json::parser { }).parse(content_data);
    } catch (std::exception& e) { }
}

std::string env_data::get_form_post(const std::string& name) const {
    return cgi(name);   
}

rest::json env_data::get_json_post() const {
    return post_data;
}
rest::json& env_data::operator[](const std::string& name) {
    if (!post_data[name].isset()) {
        post_data[name] = get_form_post(name);
    }

    return post_data[name];
}

std::vector<cgicc::FormFile>::iterator env_data::get_file(const std::string& name) {
    return cgi.getFile(name);
}

std::vector<cgicc::FormFile>::const_iterator env_data::get_file(const std::string& name) const {
    return cgi.getFile(name);
}

std::vector<cgicc::FormFile> env_data::get_files() {
    return cgi.getFiles();
}
