#include "../../services/serialization/Json.hpp"
#include "../../services/serialization/Model.hpp"

class test_model : public base_model {
public:
    property<int>             int_val { this, "int"    };
    property<float>         float_val { this, "float"  };
    property<bool>           bool_val { this, "bool"   };
    property<std::nullptr_t> null_val { this, "null"   };
    property<std::string>  string_val { this, "string" };
};

int main(int argc, const char* argv[]) {
    test_model test;
    json::serializer serializer;
    json::json object;

    object["int"   ] = 10;
    object["float" ] = 3.1415;
    object["bool"  ] = false;
    object["null"  ] = nullptr;
    object["string"] = "test";

    serializer.deserialize(test, object);

    if(test.   int_val != 10     ) return 1;
    if(test. float_val != 3.1415f) return 1;
    if(test.  bool_val != false  ) return 1;
    if(test.  null_val != nullptr) return 1;
    if(test.string_val != "test" ) return 1;

    return 0;
}
