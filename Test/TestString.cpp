
#include "TinyJson.h"
#include "TestUtil.h"

using namespace std;
using namespace TinyJson;

void TestString()
{
    const auto Test = [] (const string &data, const string &expected)
    {
        auto value = Read(data.begin(), data.end());
        Check(value->IsString());

        const auto actual = Convert<string>(value);
        CheckEqual(actual, expected);
    };

    Test(R"( "hello world!" )", "hello world!");
    Test(R"( "\"quoted text\"" )", "\"quoted text\"");
    Test(R"( "first line.\nsecond line." )", "first line.\nsecond line.");
}
