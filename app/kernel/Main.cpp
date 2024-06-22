#include "../services/env/Env.hpp"
#include "../services/router/Router.hpp"

int main(int argc, const char* argv[])
{
    env = std::make_shared<env_data>();

    init_router();

    router();
}
