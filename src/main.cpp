#include <iostream>
#include <sqlite3.h>

int main() {
    std::cout << "Welcome to Archway - Personal Finance Manager!" << std::endl;
    std::cout << "SQLite3 Version: " << sqlite3_libversion() << std::endl;
    return 0;
}
