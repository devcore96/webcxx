#include "Connection.hpp"

namespace mysql {
    std::string connection::url;
    std::string connection::db_name;

    connection::connection() :
        session(mysqlx::abi2::SessionSettings { url }),
        db(session,
           db_name) { }

    connection& connection::get_instance() {
        static auto instance = connection();

        return instance;
    }

    void connection::set_url(std::string url) {
        connection::url = url;
    }

    void connection::set_db_name(std::string db_name) {
        connection::db_name = db_name;
    }
}
