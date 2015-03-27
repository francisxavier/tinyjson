
#include "TinyJson.h"
#include "TestUtil.h"

using namespace std;
using namespace TinyJson;

void TestArray()
{
    // Simple array
    {
        auto stream = MakeStream(" [10, 20, 30] ");

        auto value = Read(stream);
        Check(value->IsArray());

        const auto actual = Convert<vector<int>>(value);
        CheckEqual(actual, vector<int>{10, 20, 30});
    }

    // Array of arrays
    {
        auto stream = MakeStream(" [[1, 2], [3, 4]] ");

        auto value = Read(stream);
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
        auto stream = MakeStream(R"( [10, "hello", true, null] )");

        auto value = Read(stream);
        Check(value->IsArray());

        const auto &arr = value->AsArray();

        CheckEqual(arr[0]->AsNumber(), 10.0);
        CheckEqual(arr[1]->AsString(), string("hello"));
        CheckEqual(arr[2]->AsBoolean(), true);
        Check(arr[3]->IsNull());
    }

    // Pair of values
    {
        auto stream = MakeStream(R"( ["hello", 10] )");

        auto value = Read(stream);
        Check(value->IsArray());

        const auto actual = Convert<pair<string, int>>(value);
        CheckEqual(actual, make_pair(string("hello"), 10));
    }
}
