#pragma once

#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <chrono>
#include <filesystem>
#include <format>
#include <unordered_set>
#include <mutex>

// Forward declarations of manipulators
template <typename CharT>
class forward_manip;

template <typename CharT>
class unforward_manip;

// The basic_logbuf class
template <typename CharT, typename Traits = std::char_traits<CharT>>
class basic_logbuf : public std::basic_streambuf<CharT, Traits> {
public:
    basic_logbuf();
    ~basic_logbuf();

    void enable_forwarding(std::basic_ostream<CharT, Traits>* os);
    void disable_forwarding(std::basic_ostream<CharT, Traits>* os);

protected:
    typename Traits::int_type overflow(typename Traits::int_type ch) override;
    std::streamsize xsputn(const CharT* s, std::streamsize n) override;

private:
    std::basic_ofstream<CharT> file_stream_;
    std::unordered_set<std::basic_ostream<CharT, Traits>*> forwarding_streams_;
    std::mutex mutex_;
};

// The basic_logstream class
template <typename CharT, typename Traits = std::char_traits<CharT>>
class basic_logstream : public std::basic_ostream<CharT, Traits> {
public:
    basic_logstream();
    ~basic_logstream();
};

// Type aliases for convenience
using logstream = basic_logstream<char>;
using wlogstream = basic_logstream<wchar_t>;

// Declare the global variables
extern logstream log;
extern wlogstream wlog;

// Definition of forward_manip
template <typename CharT>
class forward_manip {
public:
    explicit forward_manip(std::basic_ostream<CharT>& os) : os_(&os) {}
    std::basic_ostream<CharT>* os_;
};

template <typename CharT>
forward_manip<CharT> forward(std::basic_ostream<CharT>& os) {
    return forward_manip<CharT>(os);
}

// Definition of unforward_manip
template <typename CharT>
class unforward_manip {
public:
    explicit unforward_manip(std::basic_ostream<CharT>& os) : os_(&os) {}
    std::basic_ostream<CharT>* os_;
};

template <typename CharT>
unforward_manip<CharT> unforward(std::basic_ostream<CharT>& os) {
    return unforward_manip<CharT>(os);
}

// Overload operator<< for forward_manip
template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& out, forward_manip<CharT> manip) {
    auto* buf = out.rdbuf();
    if (auto* logbuf = dynamic_cast<basic_logbuf<CharT, Traits>*>(buf)) {
        logbuf->enable_forwarding(manip.os_);
    }
    return out;
}

// Overload operator<< for unforward_manip
template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& out, unforward_manip<CharT> manip) {
    auto* buf = out.rdbuf();
    if (auto* logbuf = dynamic_cast<basic_logbuf<CharT, Traits>*>(buf)) {
        logbuf->disable_forwarding(manip.os_);
    }
    return out;
}

// Implementation of basic_logbuf

template <typename CharT, typename Traits>
basic_logbuf<CharT, Traits>::basic_logbuf() {
    using namespace std::chrono;

    // Get current date and time
    auto now = system_clock::now();
    // Create a time string in a filename-safe format
    std::basic_string<CharT> time_str = std::format(
        []() constexpr {
            if constexpr (std::is_same_v<CharT, char>)
                return "{:%Y-%m-%d_%H-%M-%S}";
            else
                return L"{:%Y-%m-%d_%H-%M-%S}";
        }(),
        floor<seconds>(now)
    );

    // Retrieve the executable name
    std::basic_string<CharT> executable_name;
    try {
        std::filesystem::path exe_path = std::filesystem::read_symlink("/proc/self/exe");
        if constexpr (std::is_same_v<CharT, char>)
            executable_name = exe_path.filename().string();
        else
            executable_name = exe_path.filename().wstring();
    } catch (...) {
        executable_name = []() constexpr {
            if constexpr (std::is_same_v<CharT, char>)
                return "unknown";
            else
                return L"unknown";
        }();
    }

    // Build default and fallback file paths
    std::filesystem::path default_path = std::filesystem::path(
        []() constexpr {
            if constexpr (std::is_same_v<CharT, char>)
                return "/var/log/webcxx/";
            else
                return L"/var/log/webcxx/";
        }()
    ) / executable_name;

    std::filesystem::path fallback_path = std::filesystem::path(
        []() constexpr {
            if constexpr (std::is_same_v<CharT, char>)
                return "./log/";
            else
                return L"./log/";
        }()
    ) / executable_name;

    // Append the filename using append method
    std::basic_string<CharT> filename = time_str;
    filename.append([]() constexpr {
        if constexpr (std::is_same_v<CharT, char>)
            return ".log";
        else
            return L".log";
    }());

    default_path /= filename;
    fallback_path /= filename;

    // Try to open the default file path
    try {
        std::filesystem::create_directories(default_path.parent_path());
        file_stream_.open(default_path, std::ios_base::out | std::ios_base::app);
    } catch (...) {
        // Ignore errors here; will attempt fallback
    }

    // If opening the default path failed, try the fallback
    if (!file_stream_.is_open()) {
        try {
            std::filesystem::create_directories(fallback_path.parent_path());
            file_stream_.open(fallback_path, std::ios_base::out | std::ios_base::app);
            if (!file_stream_) {
                throw std::ios_base::failure("Failed to open log file at fallback path");
            }
        } catch (...) {
            throw std::ios_base::failure("Failed to open log file at both default and fallback paths");
        }
    }

    // Set the stream to unbuffered
    this->setp(nullptr, nullptr);
}

template <typename CharT, typename Traits>
basic_logbuf<CharT, Traits>::~basic_logbuf() {
    std::lock_guard<std::mutex> lock(mutex_);
    file_stream_.flush();
    file_stream_.close();
}

template <typename CharT, typename Traits>
void basic_logbuf<CharT, Traits>::enable_forwarding(std::basic_ostream<CharT, Traits>* os) {
    if (os) {
        std::lock_guard<std::mutex> lock(mutex_);
        forwarding_streams_.insert(os);
    }
}

template <typename CharT, typename Traits>
void basic_logbuf<CharT, Traits>::disable_forwarding(std::basic_ostream<CharT, Traits>* os) {
    if (os) {
        std::lock_guard<std::mutex> lock(mutex_);
        forwarding_streams_.erase(os);
    }
}

template <typename CharT, typename Traits>
typename Traits::int_type basic_logbuf<CharT, Traits>::overflow(typename Traits::int_type ch) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (Traits::eq_int_type(ch, Traits::eof())) {
        return Traits::not_eof(ch);
    } else {
        CharT c = Traits::to_char_type(ch);

        // Write character to file
        file_stream_.put(c);
        if (c == Traits::to_char_type('\n')) {
            file_stream_.flush();
        }

        // Write character to forwarded ostreams
        for (auto os : forwarding_streams_) {
            os->put(c);
            if (c == Traits::to_char_type('\n')) {
                os->flush();
            }
        }

        return ch;
    }
}

template <typename CharT, typename Traits>
std::streamsize basic_logbuf<CharT, Traits>::xsputn(const CharT* s, std::streamsize n) {
    std::lock_guard<std::mutex> lock(mutex_);
    // Write to file
    file_stream_.write(s, n);
    file_stream_.flush();

    // Write to forwarded ostreams
    for (auto os : forwarding_streams_) {
        os->write(s, n);
        os->flush();
    }

    return n;
}

template <typename CharT, typename Traits>
basic_logstream<CharT, Traits>::basic_logstream()
    : std::basic_ostream<CharT, Traits>(new basic_logbuf<CharT, Traits>()) {
    // Ensure flush after each insertion
    this->setf(std::ios_base::unitbuf);
}

template <typename CharT, typename Traits>
basic_logstream<CharT, Traits>::~basic_logstream() {
    // Clean up the stream buffer
    delete this->rdbuf();
}
