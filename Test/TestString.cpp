
#include "TinyJson.h"
#include "TestUtil.h"

using namespace std;
using namespace TinyJson;

void TestString()
{
    const auto Test = [] (const char *const pData, const string &expected)
    {
        Reader r(pData);
        
        auto value = r.Read();
        Check(value->IsString());

        const string actual = Convert<string>(value);
        CheckEqual(actual, expected);
    };

    Test(R"( "hello world!" )", "hello world!");
    Test(R"( "\"quoted text\"" )", "\"quoted text\"");
    Test(R"( "first line.\nsecond line." )", "first line.\nsecond line.");
}
