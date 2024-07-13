#include "../../services/serialization/Json.hpp"
#include "../../services/serialization/Model.hpp"

class test_model : public base_model {
public:
    property<int>             int_val { this, "int",    10      };
    property<double>        float_val { this, "float",  3.1415  };
    property<bool>           bool_val { this, "bool",   false   };
    property<std::nullptr_t> null_val { this, "null",   nullptr };
    property<std::string>  string_val { this, "string", "test"  };
};

int main(int argc, const char* argv[]) {
    test_model test;
    json::serializer serializer;

    auto object = serializer.serialize(test);

    if(object["int"   ]             != 10     ) return 1;
    if(object["float" ]             != 3.1415 ) return 1;
    if(object["bool"  ]             != false  ) return 1;
    if(object["null"  ]             != nullptr) return 1;
    if(object["string"].unescaped() != "test" ) return 1;

    return 0;
}
