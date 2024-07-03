#if defined(__cpp_lib_format) && __cpp_lib_format <= __cplusplus
# include <format>
using std::format;
using std::vformat;
#else
// Basic alternative to <format> on systems using old compilers. Uses a stringstream to format parameters instead.
// This does not support most of the features suported by <format> but it will work in a pinch as an alternative.
#include <string>
#include <sstream>

template<class Arg>
std::string format(std::string str, Arg&& arg) {
    std::ostringstream stream;

    stream << str.substr(0, str.find("{}"));
    stream << arg;
    stream << str.substr(str.find("{}") + 2);

    return stream.str();
}

template<class Arg, class... Args>
std::string format(std::string str, Arg&& arg, Args&&... args) {
    return format(format(str, arg), args...);
}
#endif
