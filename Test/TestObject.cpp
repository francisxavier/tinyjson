
#include "TinyJson.h"
#include "TestUtil.h"

using namespace std;
using namespace TinyJson;

void TestObject()
{
    // Simple object
    {
        auto value = Read(R"( {"hello" : 1, "world" : 2} )");
        Check(value->IsObject());

        const auto actual = Convert<map<string, int>>(value);
        CheckEqual(actual, map<string, int>
        {
            { "hello", 1 },
            { "world", 2 }
        });
    }

    // Nested object
    {
        auto value = Read(R"( {"hello" : {"world" : 10}} )");
        Check(value->IsObject());

        const auto actual = Convert<map<string, map<string, int>>>(value);
        CheckEqual(actual, map<string, map<string, int>>{ {"hello", { {"world", 10} }} });
    }

    // Object of various values
    {
        auto value = Read(R"( {"a":10, "b":"hello", "c":true, "d":null } )");
        Check(value->IsObject());

        const auto &obj = value->AsObject();

        CheckEqual(obj.at("a")->AsNumber(), 10.0);
        CheckEqual(obj.at("b")->AsString(), string("hello"));
        CheckEqual(obj.at("c")->AsBoolean(), true);
        Check(obj.at("d")->IsNull());
    }
}
