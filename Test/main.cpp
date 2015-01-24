
#include <iostream>

using namespace std;

void TestBoolean();
void TestNumber();
void TestString();
void TestArray();
void TestObject();

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
