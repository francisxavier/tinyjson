
#include "TinyJson.h"
#include "TestUtil.h"

using namespace std;
using namespace TinyJson;

void TestNumber()
{
    const auto Test = [] (const char *const pData, const double expected)
    {
        auto stream = MakeStream(pData);

        auto value = Read(stream);
        Check(value->IsNumber());

        const auto actual = Convert<double>(value);
        CheckEqual(actual, expected);
    };

    Test(" 123.456 ", 123.456);
    Test(" -10.5 ", -10.5);
    Test(" 10e2 ", 1000);
    Test(" -123e-3 ", -0.123);
}
