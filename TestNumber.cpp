
#include "TinyJson.h"
#include "TestUtil.h"

using namespace std;
using namespace TinyJson;

void TestNumber()
{
    const auto Test = [] (const char *const pData, const double expected)
    {
        Reader r(pData);

        auto value = r.Read();
        Check(value->IsNumber());

        const double actual = Convert<double>(value);
        CheckEqual(actual, expected);
    };

    Test(" 123.456 ", 123.456);
    Test(" -10.5 ", -10.5);
    Test(" 10e2 ", 1000);
    Test(" -123e-3 ", -0.123);
}
