#pragma once

#include "Model.hpp"
#include "../tools/Container.hpp"

#include <vector>
#include <memory>
#include <string>
#include <sstream>

namespace db {
    class base_table;
    
    struct order_by_query_t {
        std::string key;
        bool asc;
    };

    struct where_query_t {
        std::string key;
        std::string value;
        std::string query_operator;
    };

    class ILimitable;
    class IOrderable;
    class ISearchable;

    class IExecutable {
    protected:
        friend class ILimitable;
        friend class IOrderable;
        friend class ISearchable;

        const base_table&                   t;
              size_t                        limit = (size_t)-1;
              std::vector<order_by_query_t> order_bys;
              std::vector<where_query_t>    wheres;

        IExecutable(const base_table&                   t,
                          size_t                        limit,
                          std::vector<order_by_query_t> order_bys,
                          std::vector<where_query_t>    wheres) :
            t(t),
            limit(limit),
            order_bys(order_bys),
            wheres(wheres) { }

    public:
        std::vector<std::shared_ptr<model>>    get() const;
        void remove() const;
    };
    
    class ILimitable : public IExecutable {
    protected:
        friend class IOrderable;
        friend class ISearchable;

        ILimitable(const base_table&                   t,
                         size_t                        limit,
                         std::vector<order_by_query_t> order_bys,
                         std::vector<where_query_t>    wheres) :
            IExecutable(t, limit, order_bys, wheres) { }

    public:
        IExecutable limit(size_t limit) {
            return IExecutable {
                t,
                limit,
                order_bys,
                wheres
            };
        }
    };

    class IOrderable : public ILimitable {
    protected:
        friend class ISearchable;

        IOrderable(const base_table&                   t,
                         size_t                        limit,
                         std::vector<order_by_query_t> order_bys,
                         std::vector<where_query_t>    wheres) :
            ILimitable(t, limit, order_bys, wheres) { }

    public:
        IOrderable order_by(std::string key, bool asc = true) {
            order_by_query_t query {
                .key = key,
                .asc = asc
            };

            order_bys.push_back(query);

            return IOrderable {
                t,
                (size_t)-1,
                order_bys,
                wheres
            };
        }
    };

    class ISearchable : public IOrderable {
    protected:
        friend class ISearchable;

        ISearchable(const base_table&                   t,
                          size_t                        limit,
                          std::vector<order_by_query_t> order_bys,
                          std::vector<where_query_t>    wheres) :
            IOrderable(t, limit, order_bys, wheres) { }

    public:
        template<typename T>
        ISearchable where(std::string key, T value, std::string query_operator) {
            std::ostringstream stream;

            if constexpr (std::same_as<T, const char*> || std::same_as<T, char*> || std::same_as<T, std::string>) {
                stream << "\"";
            }

            stream << value;

            if constexpr (std::same_as<T, const char*> || std::same_as<T, char*> || std::same_as<T, std::string>) {
                stream << "\"";
            }

            wheres.push_back(where_query_t {
                .key = key,
                .value = stream.str(),
                .query_operator = query_operator
            });

            return ISearchable {
                t,
                (size_t)-1,
                { },
                wheres
            };
        }

        template<template<typename> class IterableType, typename ValueType>
            requires iterable<IterableType<ValueType>>
        ISearchable whereIn(std::string key, IterableType<ValueType> values) {
            std::ostringstream stream;
            
            stream << "[";

            for (auto& value : values) {

                if constexpr (std::same_as<ValueType, const char*> || std::same_as<ValueType, char*> || std::same_as<ValueType, std::string>) {
                    stream << "\"";
                }

                stream << value;

                if constexpr (std::same_as<ValueType, const char*> || std::same_as<ValueType, char*> || std::same_as<ValueType, std::string>) {
                    stream << "\"";
                }

            }

            stream << "]";

            wheres.push_back(where_query_t {
                .key = key,
                .value = stream.str(),
                .query_operator = "IN"
            });

            return ISearchable {
                t,
                (size_t)-1,
                { },
                wheres
            };
        }
    };
}

#include "Table.hpp"
