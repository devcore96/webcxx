#include <exception>
#include <algorithm>
#include <version>

#ifdef __cpp_lib_stacktrace
# include <stacktrace>
#endif

#include <signal.h>

#include "../env/Env.hpp"
#include "../tools/Container.hpp"
#include "Router.hpp"

struct url_info {
    std::vector<std::string> path;
    std::vector<std::string> query;
};

std::vector<route_data> routes;
std::map<std::string, std::function<std::any(std::string)>> value_mappers;
std::map<unsigned int, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)>> error_routes;

route_verb parse_verb(std::string request_method) {
    std::transform(request_method.begin(),
                   request_method.end(),
                   request_method.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    if (request_method ==     "get") return route_verb::verb_get;
    if (request_method ==    "post") return route_verb::verb_post;
    if (request_method ==     "put") return route_verb::verb_put;
    if (request_method ==   "patch") return route_verb::verb_patch;
    if (request_method ==  "delete") return route_verb::verb_delete;
    if (request_method == "options") return route_verb::verb_options;

    return route_verb::verb_unknown;
}

std::vector<std::string> split_string(std::string str, char delim) {
    std::vector<std::string> ret;

    std::istringstream ss(str);
    std::string cur;

    while(std::getline(ss, cur, delim))
        ret.push_back(cur);

    return ret;
}

url_info split_url(const std::string& url) {
    std::string url_string, query_string;
    std::istringstream ss(url);

    std::getline(ss, url_string, '?');
    std::getline(ss, query_string);

    return {
        split_string(  url_string, '/'),
        split_string(query_string, '&')
    };
}

// Parse route parameters
bool complete_route(std::string route, std::string url, std::map<std::string, std::any>& params) {
    url_info       info = split_url(url);
    url_info route_info = split_url(route);

    if(info.path.size() != route_info.path.size()) return false;

    size_t id = 0;
    for(std::string& slug : route_info.path) {
        if(slug.size() == 0) continue;

        while(info.path.size() > id && info.path[id].size() == 0) id++;

        if(info.path.size() == id) {
            params.clear();
            return false;
        }

        if(slug[0] == '{') {
            std::string type, name;

            try {
                std::vector<std::string> values = split_string(slug.substr(1, slug.size() - 2), ' ');

                type = values[0];
                name = values[1];    
            } catch(std::exception &e) {
                throw std::runtime_error("Route " + route + " is ill-formed.");
            }

            std::any value;

            if (type != "string") {
                std::function<std::any(std::string)> conversion_function;

                try {
                    conversion_function = value_mappers[type];
                } catch(std::exception &e) {
                    throw std::runtime_error("Route " + route + " is ill-formed: Unknown value type \"" + type + "\".");
                }

                try {
                    value = conversion_function(info.path[id]);
                } catch(std::bad_cast &e) { 
                    params.clear();
                    return false;
                }
            } else value = info.path[id];

            params.insert({ name, value });
        } else if(slug != info.path[id]) {
            params.clear();
            return false;
        }

        id++;
    }

    return true;
}

std::string clean_url(std::string url) {
    while (*(url.end() - 1) == '/') url.pop_back();

    for (size_t i = 0; i < url.size(); i++) {
        if (url[i] == '/' && url[i + 1] == '/') {
            url.erase(url.begin() + i);
            i--;
        }
    }

    return url;
}

void router() {
    try {
        std::map<std::string, std::any> params;

        route_verb verb = parse_verb(env->request_method);

        std::string url = clean_url(env->url);

        for(auto& route : routes) {
            std::string route_url = clean_url(route.uri);

            if(contains(route.verb, verb) && (route_url == url || complete_route(route_url, url, params))) {
                std::cout << route.callback(params)->render() << std::flush;

                return;
            }
        }

        std::cout << error_routes[404]({})->render() << std::flush;
    } catch(std::exception& e) {
        std::map<std::string, std::any> params;
        params.insert({ "e", &e });

        std::cout << error_routes[500](params)->render() << std::flush;
    }
}

// Segfault handler to display debug information
void segfault_sigaction(int signal, siginfo_t *si, void *arg) {
    std::map<std::string, std::any> params;
    std::runtime_error e = std::runtime_error("Segmentation fault.");

    params.insert({ "e", static_cast<std::exception*>(&e) });

    std::cout << error_routes[500](params)->render() << std::flush;

    exit(0);
}

void init_router() {
    // Init default value mappers
    map_value<ptrdiff_t>(   "int");
    map_value<   size_t>(  "uint");
    map_value<   double>("double");

    // Init error routes
    error_routes.insert({ 404,
        #include <pages/errors/404.cpphtml>
    });

    error_routes.insert({ 500,
        #include <pages/errors/500.cpphtml>
    });

    // Set up segfault handler
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_sigaction;
    sa.sa_flags     = SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);
}
