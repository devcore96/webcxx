#include "../app/services/router/Router.hpp"
#include "../app/services/env/Env.hpp"

auto api = Route::Group("/api/v1", [](Route::group api) {
    api.Get("", [](std::map<std::string, std::any>) {
        rest::json response;

        response["message"] = "Hello, world!";

        return reply(response);
    });
});
