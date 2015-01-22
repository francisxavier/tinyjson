
#include "TinyJson.h"
#include <iostream>

using namespace std;
using namespace TinyJson;

template <class T>
static void CheckEqual(const T &value1, const T &value2)
{
    if( value1 != value2 )
        throw std::runtime_error("CheckEqual failed");
}

template <class T>
static void CheckNotEqual(const T &value1, const T &value2)
{
    if( value1 == value2 )
        throw std::runtime_error("CheckNotEqual failed");
}

static void Check(const bool flag)
{
    if( !flag )
        throw std::runtime_error("Check failed");
}

template <class T>
static void CheckNotNull(const T *const ptr)
{
    CheckNotEqual(ptr, static_cast<const T *>(nullptr));
}

static void TestBoolean()
{
    const auto Test = [] (const char *const pData, const bool expected)
    {
        Reader r(pData);

        auto value = r.Read();
        Check(value->IsBoolean());

        const bool actual = Convert<bool>(value);
        CheckEqual(actual, expected);
    };

    Test(" true ", true);
    Test(" false ", false);
}

static void TestNumber()
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

static void TestString()
{
    const auto Test = [] (const char *const pData, const string &expected)
    {
        Reader r(pData);
        
        auto value = r.Read();
        Check(value->IsString());

        const string actual = Convert<string>(value);
        CheckEqual(actual, expected);
    };

    Test(" \"hello world!\" ", "hello world!");
    Test(" \"\\\"quoted text\\\"\" ", "\"quoted text\"");
    Test(" \"first line.\\nsecond line.\" ", "first line.\nsecond line.");
}

static void TestArray()
{
    // Simple array
    {
        Reader r("[10, 20, 30]");

        auto value = r.Read();
        Check(value->IsArray());

        const vector<int> actual = Convert<vector<int>>(value);
        CheckEqual(actual, vector<int>{10, 20, 30});
    }

    // Array of arrays
    {
        Reader r("[[1, 2], [3, 4]]");

        auto value = r.Read();
        Check(value->IsArray());

        const vector<vector<int>> actual = Convert<vector<vector<int>>>(value);
        CheckEqual(actual, vector<vector<int>>
        {
            { 1, 2 },
            { 3, 4 }
        });
    }

    // Array of various values
    {
        Reader r("[10, \"hello\", true, null]");

        auto value = r.Read();
        Check(value->IsArray());

        const auto &arr = value->AsArray();

        CheckEqual(arr[0]->AsNumber(), 10.0);
        CheckEqual(arr[1]->AsString(), string("hello"));
        CheckEqual(arr[2]->AsBoolean(), true);
        Check(arr[3]->IsNull());
    }
}

static void TestObject()
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

int main()
{
    try
    {
        TestBoolean();
        TestNumber();
        TestString();
        TestArray();
        TestObject();
    }
    catch(const exception &e)
    {
        cout << "Test failed" << endl;
        cout << e.what() << endl;
    }

    return 0;
}
