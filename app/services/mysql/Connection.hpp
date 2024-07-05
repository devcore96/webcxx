#pragma once

#include <mysql-cppconn-8/mysqlx/xdevapi.h>

namespace mysql {
    class connection {
    private:
        connection();

        static std::string  server;
        static unsigned int port;
        static std::string  user;
        static std::string  password;
        static std::string  db_name;

    public:
        static connection& get_instance();

        mysqlx::Session session;
        mysqlx::Schema  db;
    };
}
