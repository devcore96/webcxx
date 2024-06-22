#include "../app/services/router/Router.hpp"
#include "../app/services/env/Env.hpp"

auto web = Route::Group("", [](Route::group web) {
    web.Get("",
        #include <pages/index.cpphtml>
    );
});
