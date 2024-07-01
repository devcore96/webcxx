#pragma once

#include <mysql-cppconn-8/mysqlx/xdevapi.h>

namespace mysql {
    class connection {
    private:
        connection();

        static std::string url;
        static std::string db_name;

    public:
        static connection& get_instance();
        static void set_url(std::string url);
        static void set_db_name(std::string db_name);

        mysqlx::Session session;
        mysqlx::Schema  db;
    };
}
