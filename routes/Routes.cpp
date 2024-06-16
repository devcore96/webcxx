#include <random>
#include <concepts>
#include <functional>
#include <fstream>
#include <filesystem>

#include "../app/services/Router.hpp"
#include "../app/services/Env.hpp"

#include <chrono>
#include <format>

void init_routes() {
    route("/", 
        #include <pages/index.cpphtml>
    );
}
