#pragma once

namespace db {
    class transaction {
    public:
        virtual void commit  () = 0;
        virtual void rollback() = 0;
    };
}
