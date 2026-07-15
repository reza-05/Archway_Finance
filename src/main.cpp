#include <iostream>
#include <sqlite3.h>
using namespace std;

int main()
{
    cout << "Welcome to Archway - Personal Finance Manager!" << endl;
    cout << "SQLite3 Version: " << sqlite3_libversion() << endl;
    return 0;
}
