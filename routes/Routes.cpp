#include <random>
#include <concepts>
#include <functional>
#include <fstream>
#include <filesystem>

#include "../app/services/Router.hpp"
#include "../app/services/Env.hpp"

#include <chrono>

#ifdef __cpp_lib_format
# include <format>
#endif

void init_routes() {
    route("/", 
        #include <pages/index.cpphtml>
    );
}
