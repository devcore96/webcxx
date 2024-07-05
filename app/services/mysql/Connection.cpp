#include "Connection.hpp"

namespace mysql {
    std::string  connection::server   = "127.0.0.1";
    unsigned int connection::port     = 33060;
    std::string  connection::user     = "root";
    std::string  connection::password = "password";
    std::string  connection::db_name  = "test";

    connection::connection() :
        session(mysqlx::abi2::SessionSettings { server, port, user, password }),
        db(session,
           db_name) { }

    connection& connection::get_instance() {
        static auto instance = connection();

        return instance;
    }
}
