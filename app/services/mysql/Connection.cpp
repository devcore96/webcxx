#include "Connection.hpp"

namespace mysql {
    std::string  connection::server   = "localhost";
    std::string  connection::password = "password";
    unsigned int connection::port     = 33060;
    std::string  connection::user     = "root";
    std::string  connection::db_name  = "test";

    connection::connection() :
        session(mysqlx::abi2::SessionSettings { server, port, user, password }),
        db(session,
           db_name) { }

    connection& connection::get_instance() {
        static auto instance = connection();

        return instance;
    }

    transaction connection::begin() {
        return { *this };
    }

    void connection::set_server(std::string server) {
        connection::server = server;
    }

    void connection::set_password(std::string password) {
        connection::password = password;
    }
    
    std::string connection::get_db_name() {
        return db_name;
    }
}
