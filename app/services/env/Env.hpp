#pragma once

#include <string>
#include <memory>

#include <cgicc/Cgicc.h>
#include <cgicc/CgiEnvironment.h>

#include "../rest/Rest.hpp"

class env_data {
private:
    cgicc::Cgicc cgi;
    rest::json   post_data;

public:
    env_data();

    // Retrieves post data from form inputs
    std::string get_form_post(const std::string& name) const;
    
    // Retrieves JSON post data
     rest::json get_json_post() const;

    // Retrieves post data from form input or JSON
    rest::json& operator[](const std::string& name);

    // Retrieves posted files
    std::vector<cgicc::FormFile>::iterator get_file(const std::string& name);
    std::vector<cgicc::FormFile>::const_iterator get_file(const std::string& name) const;
    std::vector<cgicc::FormFile> get_files();

    std::string request_method;
    std::string query_string;
    
    std::string content_type;
         size_t content_length;
    std::string content_data;
    
    std::string auth_type;
    std::string remote_host;
    std::string remote_address;
    std::string remote_user;
    std::string remote_identification;
    std::string user_agent;
    
    std::string server_name;
    std::string server_port;
    
           bool https;
         
    std::string url;
};

extern std::shared_ptr<env_data> env;
