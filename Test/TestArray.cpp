
#include "TinyJson.h"
#include "TestUtil.h"

using namespace std;
using namespace TinyJson;

void TestArray()
{
    // Simple array
    {
        Reader r(" [10, 20, 30] ");

        auto value = r.Read();
        Check(value->IsArray());

        const auto actual = Convert<vector<int>>(value);
        CheckEqual(actual, vector<int>{10, 20, 30});
    }

    // Array of arrays
    {
        Reader r(" [[1, 2], [3, 4]] ");

        auto value = r.Read();
        Check(value->IsArray());

        const auto actual = Convert<vector<vector<int>>>(value);
        CheckEqual(actual, vector<vector<int>>
        {
            { 1, 2 },
            { 3, 4 }
        });
    }

    // Array of various values
    {
        Reader r(R"( [10, "hello", true, null] )");

        auto value = r.Read();
        Check(value->IsArray());

        const auto &arr = value->AsArray();

        CheckEqual(arr[0]->AsNumber(), 10.0);
        CheckEqual(arr[1]->AsString(), string("hello"));
        CheckEqual(arr[2]->AsBoolean(), true);
        Check(arr[3]->IsNull());
    }

    // Pair of values
    {
        Reader r(R"( ["hello", 10] )");

        auto value = r.Read();
        Check(value->IsArray());

        const auto actual = Convert<pair<string, int>>(value);
        CheckEqual(actual, make_pair(string("hello"), 10));
    }
}
