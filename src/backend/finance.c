#include "finance.h"
#include <string.h>

//total balance till now
double calculate_total_balance(const Database *db) {
    if (db == NULL) return 0.0;
    
    double total = 0.0;
    for (int i = 0; i < db->count; i++) {
        if (db->list[i].is_income == 1) {
            total += db->list[i].amount; // add income
        } else {
            total -= db->list[i].amount; // minus expense
        }
    }
    return total;
}
