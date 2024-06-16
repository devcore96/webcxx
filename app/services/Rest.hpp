#pragma once

#include <curl/curl.h>
#include "Json.hpp"
#include <string>
#include <vector>
#include <map>
#include <istream>
#include <ostream>
#include <concepts>
#include <variant>

namespace rest {
    using json = ::json::json_value;

    class url {
    public:
        using value_type     = char;
        using string_type    = std::basic_string<value_type>;
        using const_iterator = string_type::const_iterator;
        using iterator       = const_iterator;

    private:
        string_type data;

    public:
        url()                     noexcept : data()             { }
        url(const   url&  u)               : data(u.data)       { }
        url(        url&& u)      noexcept : data(std::move(u.data)) { u.data = ""; }
        url(string_type&& source) noexcept : data(source)       { }

        template<class Source>
        url(const Source& source)          : data(source)       { }

        template<class InputIt>
        url(InputIt first, InputIt last)   : data(first, last)  { }

        url& operator =(const url&  u)                          { data = u.data;                           return *this; }
        url& operator =(      url&& u) noexcept                 { data = std::move(u.data); u.data = "";   return *this; }
        url& operator =(string_type&& source)                   { data = std::move(source);                return *this; }
        
        template<class Source>
        url& operator =(Source& source)                         { data = url(source).native();             return *this; }

        url& operator/=(const url& u)                           { data += "/" + u.native();                return *this; }

        template<class Source>
        url& operator/=(const Source& source)                   { data += "/" + url(source).native();      return *this; }

        template<class Source>
        url&     append(const Source& source)                   { data += "/" + url(source).native();      return *this; }

        template<class InputIt>
        url&     append(InputIt first, InputIt last)            { data += "/" + url(first, last).native(); return *this; }

        url& operator+=(const url& u)                           { data += u.native();                      return *this; }
        url& operator+=(const string_type& str)                 { data += url(str).native();               return *this; }
        url& operator+=(std::basic_string_view<value_type> str) { data += url(str).native();               return *this; }
        url& operator+=(const value_type* ptr)                  { data += url(ptr).native();               return *this; }

        url& operator+=(      value_type  x)                    { return *this += std::basic_string_view(&x, 1); }

        template<class CharT>
        url& operator+=(CharT  x)                               { return *this += std::basic_string_view(&x, 1); }

        template<class Source>
        url& operator+=(const Source& source)                   { data += url(source).native();            return *this; }

        template<class Source>
        url&     concat(const Source& source)                   { data += url(source).native();            return *this; }

        template<class InputIt>
        url&     concat(InputIt first, InputIt last)            { data += url(first, last).native();       return *this; }

        void clear()                         noexcept           { data.clear(); }
        void swap(url& other)                noexcept           { std::swap(data, other.data); }

        const     value_type*  c_str() const noexcept           { return data.c_str(); }
        const    string_type& native() const noexcept           { return data; }
        operator string_type()         const                    { return data; }
                 string_type  string() const                    { return data; }

        string_type protocol()         const                    { return has_protocol() ? data.substr(0, data.find("://")) : ""; }
        string_type   domain()         const                    {
            if (!has_domain()) return "";
            size_t offset = protocol().length();
            return data.substr(offset, data.find('/', offset) - offset);
        }

        [[nodiscard]] bool empty()     const noexcept           { return data.empty(); }
        
        bool has_protocol()            const                    { return data.find("://") != data.npos; }
        bool   has_domain()            const                    {
            size_t offset = protocol().length();
            return data.find('/', offset) != data.npos;
        }

        iterator begin()               const                    { return data.begin(); }
        iterator   end()               const                    { return data.end(); }

        friend bool operator==(url& lhs, url& rhs) noexcept;
        friend bool operator!=(url& lhs, url& rhs) noexcept;
        friend  url operator /(url& lhs, url& rhs) noexcept;

        template<class Source>
        friend url operator /(url  lhs, const Source source) noexcept;

        template<class Source>
        friend url operator /(url& lhs, const Source source) noexcept;
        
        template<class CharT, class Traits>
        friend std::basic_ostream<CharT, Traits>&
            operator<<(std::basic_ostream<CharT, Traits>& os, const url& u);

        template<class CharT, class Traits>
        friend std::basic_istream<CharT, Traits>&
            operator>>(std::basic_istream<CharT, Traits>& is, const url& u);
    };

    extern void swap      (url& lhs, url& rhs) noexcept;
    extern bool operator==(url& lhs, url& rhs) noexcept;
    extern bool operator!=(url& lhs, url& rhs) noexcept;
    extern  url operator /(url& lhs, url& rhs) noexcept;

    template<class Source>
            url operator /(url  lhs,
                           const Source source) noexcept { return url(lhs) /= source; }

    template<class Source>
            url operator /(url& lhs,
                           const Source source) noexcept { return url(lhs) /= source; }

    template<class CharT, class Traits>
    std::basic_ostream<CharT, Traits>&
        operator<<(std::basic_ostream<CharT, Traits>& os, const url& u) { return os << u.native(); }

    template<class CharT, class Traits>
    std::basic_istream<CharT, Traits>&
        operator>>(std::basic_istream<CharT, Traits>& is, const url& u) { return is >> u.native(); }



    class rest_error : public std::runtime_error {
    private:
        url u;

    public:
        rest_error(const std::string& what_arg) : std::runtime_error(what_arg) { }

        rest_error(const std::string& what_arg,
                   const         url& u)        : std::runtime_error(what_arg), u(u) { }

        const url& getUrl() const noexcept { return u; }
    };

    struct response {
        int  status_code;
        std::string data;
    };

    using header = std::string;

    class curl_initializer {
    public:
         curl_initializer() { curl_global_init(CURL_GLOBAL_DEFAULT); }
        ~curl_initializer() { curl_global_cleanup(); }
    };
    
    extern curl_initializer _initializer;

    response get(url& u, std::vector<header> headers = { });
    response post(url& u, std::string data, std::vector<header> headers = { });
    response post(url& u, json data, std::vector<header> headers = { });
    response post(url& u, const char* data, std::vector<header> headers = { });
    response do_delete(url& u, std::vector<header> headers = { });
}
