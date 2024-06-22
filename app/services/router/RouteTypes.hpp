#pragma once

#include "Router.hpp"

namespace Route {
    class group {
    private:
        std::string base_uri;
    public:
        group(std::string uri);
        
        group(const group& ) = default;
        group(      group&&) = default;

        group& operator=(const group& ) = default;
        group& operator=(      group&&) = default;

        void  Get    (                                          std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);
        void  Post   (                                          std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);
        void  Put    (                                          std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);
        void  Patch  (                                          std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);
        void  Delete (                                          std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);
        void  Options(                                          std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);

        void  Match  (std::initializer_list<route_verb>  verbs, std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);
        void  Match  (std::initializer_list<std::string> verbs, std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);

        void  Any    (                                          std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);

        group Group  (                                          std::string uri, std::function<void(group)> routes);
    };

    extern  void Get    (                                          std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);
    extern  void Post   (                                          std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);
    extern  void Put    (                                          std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);
    extern  void Patch  (                                          std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);
    extern  void Delete (                                          std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);
    extern  void Options(                                          std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);

    extern  void Match  (std::initializer_list<route_verb>  verbs, std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);
    extern  void Match  (std::initializer_list<std::string> verbs, std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);

    extern  void Any    (                                          std::string uri, std::function<std::unique_ptr<response>(std::map<std::string, std::any>)> callback);

    extern group Group  (                                          std::string uri, std::function<void(group)> routes);
}
