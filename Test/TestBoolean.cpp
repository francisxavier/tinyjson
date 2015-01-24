
#include "TinyJson.h"
#include "TestUtil.h"

using namespace std;
using namespace TinyJson;

void TestBoolean()
{
    const auto Test = [] (const char *const pData, const bool expected)
    {
        Reader r(pData);

        auto value = r.Read();
        Check(value->IsBoolean());

        const auto actual = Convert<bool>(value);
        CheckEqual(actual, expected);
    };

    Test(" true ", true);
    Test(" false ", false);
}
