#ifndef DATABASE_H
#define DATABASE_H
#define INT_MAX 2147483647
typedef struct {
    int id;   
    double amount;
    int is_income;       // 1 for income, 0 for expense
    char category[50];   // food,transport for piechart
    char date[20];       // YYYY-MM-DD
    char description[50];  
} Transaction;

typedef struct {
    Transaction list[INT_MAX];
    int count;              //count no of transactions
} Database;

#endif