#pragma once

#include "Model.hpp"
#include "Executable.hpp"

#include <memory>
#include <vector>
#include <string>
#include <sstream>

namespace db {
    class joined_table;

    class base_table : public ISearchable {
    protected:
        friend class joined_table;
        friend class IExecutable;

        std::string name;

        virtual std::vector<std::shared_ptr<model>> get(std::vector<where_query_t>    wheres,
                                                        std::vector<order_by_query_t> order_bys,
                                                        size_t                        limit) const = 0;

        virtual void remove(std::vector<where_query_t>    wheres,
                            std::vector<order_by_query_t> order_bys,
                            size_t                        limit) const = 0;

        bool joined = false;

    public:
        base_table() : ISearchable(*this, -1, { }, { }) { }
        base_table(const base_table& ) = default;
        base_table(      base_table&&) = default;

        std::vector<std::shared_ptr<model>> all() const;

        virtual std::shared_ptr<joined_table> join(const base_table& that, std::string this_key, std::string that_key) const = 0;
    };

    class table : public base_table {
    public:
        template<std::derived_from<model> Model>
        table(Model m) { name = m.table_name(); }
        
        table(const table& ) = default;
        table(      table&&) = default;

        table& operator=(const table& ) = default;
        table& operator=(      table&&) = default;

        virtual void insert(std::vector<std::shared_ptr<db::model>> models) const = 0;
        virtual void remove(std::vector<std::shared_ptr<db::model>> models) const = 0;
        virtual void create() const = 0;
        virtual void destroy() const = 0;
        virtual void clear() const = 0;
    };

    struct join_t {
        const base_table* this_table;
        const base_table* that_table;

        std::string this_key;
        std::string that_key;
    };

    class joined_table : public base_table {
    private:
        void add_joins(const base_table* t) {
            if (t->joined) {
                const joined_table* joined = (const joined_table*)t;

                joins.insert(joins.end(),
                      joined->joins.begin(),
                      joined->joins.end());
            }
        }

    protected:
        std::vector<join_t> joins;
        
        joined_table(const base_table* this_table,
                     const base_table* that_table,
                           std::string this_key,
                           std::string that_key) {
            joins.push_back({
                .this_table = this_table,
                .that_table = that_table,
                .this_key   = this_key,
                .that_key   = that_key
            });
            
            add_joins(this_table);
            add_joins(that_table);
        }
    };
}
