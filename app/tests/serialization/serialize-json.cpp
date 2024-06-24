#include "../../services/serialization/Json.hpp"

int main(int argc, const char* argv[]) {
    json::json object;

    object["int"]    = 80;
    object["float"]  = 3.215f;
    object["bool"]   = true;
    object["null"]   = nullptr;
    object["string"] = "string";

    std::string str = object;
    std::string expected = R"({ "bool": true, "float": 3.215000, "int": 80, "null": null, "string": "string" })";

    if (str != expected) return 1;

    return 0;
}
