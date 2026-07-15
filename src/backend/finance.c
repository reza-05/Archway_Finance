#include "finance.h"
#include <string.h>

// total balance till now
double calculate_total_balance(const Database *db)
{
    if (db == NULL)
        return 0.0;

    double total = 0.0;
    for (int i = 0; i < db->count; i++)
    {
        if (db->list[i].is_income == 1)
        {
            total += db->list[i].amount; // add income
        }
        else
        {
            total -= db->list[i].amount; // minus expense
        }
    }
    return total;
}
// total income of a specific month
double calculate_monthly_income(const Database *db, const char *month_yyyy_mm)
{
    if (db == NULL || month_yyyy_mm == NULL)
        return 0.0;

    double total = 0.0;
    for (int i = 0; i < db->count; i++)
    {
        // add income if date matches
        if (db->list[i].is_income == 1 && strncmp(db->list[i].date, month_yyyy_mm, 7) == 0)
        {
            total += db->list[i].amount;
        }
    }
    return total;
}
