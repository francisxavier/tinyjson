
#include "TinyJson.h"
#include "TestUtil.h"

using namespace std;
using namespace TinyJson;

void TestBoolean()
{
    const auto Test = [] (const char *const pData, const bool expected)
    {
        auto value = Read(pData);
        Check(value->IsBoolean());

        const auto actual = Convert<bool>(value);
        CheckEqual(actual, expected);
    };

    Test(" true ", true);
    Test(" false ", false);
}
