#include <algorithm>
#include "./include/eventbus.h"

using namespace EVManager;
using namespace std;
using namespace Logger;
void printCStyleString(const char * s)
{
   Log("Function printCStyleString - %s", s);
}

void printStdString(std::string s)
{
   Log("Function printStdString - %s", s.c_str());
}

void reverseString(std::string & s )
{
    reverse(s.begin(), s.end());
}

// Invoke as per this exercise is always bounded by single function signature.
// ( Ideally we need an ability to send varying function paramters to single event
//   and that require hetrogenous container. )
// Since we want demo a cancel the add also takes in eventbus object which is 
// is used when cancel is invoked. 
int add(int x, int y, EventBus<string> & )
{
    int z = x + y;
    Log("Add %d + %d == %d", x, y, z );
    return x + y;
}

int cancelAdd(int x, int y, EventBus<string> & eb)
{
   eb.Cancel("opAdd");
   return 0;
}

void testSwap(int & x, int & y)
{
    int z = x;
    x = y;
    y = z;
}

int main(int argc, char const *argv[])
{
    Log(" ==== Start =====" );
 
    EventBus<string> eb;

    eb.Add("opAdd", add);
    eb.Add("opAdd", cancelAdd);
    eb.Add("opAdd", add);

    eb.Add("opPrint", printCStyleString);
    eb.Add("opPrintStdString", printStdString);

    eb.Add("opSwap", testSwap);
 
    eb.Invoke<int>("opAdd", 3, 4, eb);
    eb.Invoke<void>("opPrint", (const char *)"hello from cstyle string");

    string s("hello from std string");
    // Pass it as rvalue, so that it doesn't end up deducing as lvalue & due
    // to universal reference deduction.
    eb.Invoke<void>("opPrintStdString", std::string(s));

    int x = 1;
    int y = 2;

    int xbefore = x;
    int ybefore = y;

    eb.Invoke<void>("opSwap", x, y);
    Log("Before swap x - %d y - %d. After swap x - %d y - %d", xbefore, ybefore, x, y);
    
    eb.Add("opReverse", reverseString);
    s="god";
    string reversed(s);
    eb.Invoke<void>("opReverse", reversed);
    Log("Original string %s. Reversed string %s", s.c_str(), reversed.c_str());

    // lvalue passed as copy 
    x = 10; 
    y = 20;
    eb.Invoke<int>("opAdd", (int)x, (int)y, eb);

    Log(" ==== End =====" );

    return 0;
}
