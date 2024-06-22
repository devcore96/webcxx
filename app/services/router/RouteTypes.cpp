#include "RouteTypes.hpp"
#include "../tools/Container.hpp"

extern std::vector<route_data> routes;
extern route_verb parse_verb(std::string request_method);

namespace Route {
    group::group(std::string uri) : base_uri(uri) { }



    void group::Get    (std::string uri,
                        std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        Route::Get    (base_uri + "/" + uri, callback);
    }

    void group::Post   (std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        
        Route::Post   (base_uri + "/" + uri, callback);
    }

    void group::Put    (std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        
        Route::Put    (base_uri + "/" + uri, callback);
    }

    void group::Patch  (std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        
        Route::Patch  (base_uri + "/" + uri, callback);
    }

    void group::Delete (std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        
        Route::Delete (base_uri + "/" + uri, callback);
    }

    void group::Options(std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        
        Route::Options(base_uri + "/" + uri, callback);
    }



    void group::Match  (std::initializer_list<route_verb>  verbs,
                 std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        Route::Match  (verbs, base_uri + "/" + uri, callback);
    }

    void group::Match  (std::initializer_list<std::string> verbs,
                 std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        Route::Match  (verbs, base_uri + "/" + uri, callback);
    }



    void group::Any    (std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        Route::Any    (base_uri + "/" + uri, callback);
    }



    group group::Group (std::string uri,
                        std::function<void(group)> routes) {
        return Route::Group(base_uri + "/" + uri, routes);
    }



    void Get    (std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        routes.push_back({ { route_verb::verb_get     }, uri, callback });
    }

    void Post   (std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        routes.push_back({ { route_verb::verb_post    }, uri, callback });
    }

    void Put    (std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        routes.push_back({ { route_verb::verb_put     }, uri, callback });
    }

    void Patch  (std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        routes.push_back({ { route_verb::verb_patch   }, uri, callback });
    }

    void Delete (std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        routes.push_back({ { route_verb::verb_delete  }, uri, callback });
    }

    void Options(std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        routes.push_back({ { route_verb::verb_options }, uri, callback });
    }



    void Match  (std::initializer_list<route_verb>  verbs,
                 std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        routes.push_back({ verbs, uri, callback });
    }

    void Match  (std::initializer_list<std::string> verbs,
                 std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        routes.push_back({ map_to(verbs, parse_verb), uri, callback });
    }



    void Any    (std::string uri,
                 std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback) {
        routes.push_back({ {
            route_verb::verb_get,
            route_verb::verb_post,
            route_verb::verb_put,
            route_verb::verb_patch,
            route_verb::verb_delete,
            route_verb::verb_options
        }, uri, callback });
    }



    group Group (std::string uri,
                 std::function<void(group)> routes) {
        group g { uri };

        routes(g);

        return g;
    }
}
