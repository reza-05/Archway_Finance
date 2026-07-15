#ifndef FINANCE_H
#define FINANCE_H

// database.h linking
#include "../../database/database.h"

// total balance calculation

double calculate_total_balance(const Database *db);

// total income

double calculate_monthly_income(const Database *db, const char *month_yyyy_mm);

// monthly expense

double calculate_monthly_expense(const Database *db, const char *month_yyyy_mm);

// cash flow

double calculate_net_cash_flow(const Database *db, const char *month_yyyy_mm);

#endif // FINANCE_H