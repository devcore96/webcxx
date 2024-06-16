#include "../services/Env.hpp"
#include "../services/Router.hpp"

int main(int argc, const char* argv[])
{
    env = std::make_shared<env_data>();

    init_router();
    init_routes();

    router();
}
