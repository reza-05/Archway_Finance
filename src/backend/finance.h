#ifndef FINANCE_H
#define FINANCE_H

#ifdef __cplusplus
extern "C" {
#endif

//category breakdown
typedef struct {
    char category_name[100];
    double amount;
    double percentage;
} CategoryBreakdown;

#ifdef __cplusplus
}
#endif

#endif // FINANCE_H