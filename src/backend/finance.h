#ifndef FINANCE_H
#define FINANCE_H

#ifdef __cplusplus
extern "C"
{
#endif

    // category breakdown
    typedef struct
    {
        char category_name[100];
        double amount;
        double percentage;
    } CategoryBreakdown;
    // total balance calculation
    double calculate_total_balance();

    // monthly income
    double calculate_monthly_income(const char *month_yyyy_mm);

    // monthly expense
    double calculate_monthly_expense(const char *month_yyyy_mm);

    // cash flow
    double calculate_net_cash_flow(const char *month_yyyy_mm);

    // expense on different category
    int get_category_spending_breakdown(const char *month_yyyy_mm, CategoryBreakdown *out_breakdown, int max_categories);
#ifdef __cplusplus
}
#endif

#endif // FINANCE_H