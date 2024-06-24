#include "../../services/serialization/Json.hpp"

int main(int argc, const char* argv[]) {
    json::parser parser;

    std::string str = R"({ "bool": true, "float": 3.215000, "int": 80, "null": null, "string": "string" })";

    auto object = parser.parse(str);

    if(object["int"]                != 80      ) return 1;
    if(object["float"]              != 3.215f  ) return 1;
    if(object["bool"]               != true    ) return 1;
    if(object["null"]               != nullptr ) return 1;
    if(object["string"].unescaped() != "string") return 1;

    return 0;
}
