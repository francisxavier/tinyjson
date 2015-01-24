
#include "TinyJson.h"
#include "TestUtil.h"

using namespace std;
using namespace TinyJson;

void TestObject()
{
    // Simple object
    {
        Reader r(" { \"hello\" : 1, \"world\" : 2 } ");

        auto value = r.Read();
        Check(value->IsObject());

        const map<string, int> actual = Convert<map<string, int>>(value);
        CheckEqual(actual, map<string, int>
        {
            { "hello", 1 },
            { "world", 2 }
        });
    }

    // Nested object
    {
        Reader r(" { \"hello\" : { \"world\": 10 } } ");

        auto value = r.Read();
        Check(value->IsObject());

        const map<string, map<string, int>> actual = Convert<map<string, map<string, int>>>(value);
        CheckEqual(actual, map<string, map<string, int>>{ {"hello", { {"world", 10} }} });
    }

    // Object of various values
    {
        Reader r(" {\"a\":10, \"b\":\"hello\", \"c\":true, \"d\":null } ");

        auto value = r.Read();
        Check(value->IsObject());

        const auto &obj = value->AsObject();

        CheckEqual(obj.at("a")->AsNumber(), 10.0);
        CheckEqual(obj.at("b")->AsString(), string("hello"));
        CheckEqual(obj.at("c")->AsBoolean(), true);
        Check(obj.at("d")->IsNull());
    }
}
