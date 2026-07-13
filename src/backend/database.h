#ifndef DATABASE_H
#define DATABASE_H

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_NAME_LEN 100
#define MAX_TYPE_LEN 50
#define MAX_DATE_LEN 20
#define MAX_DESC_LEN 256

    // account structure
    typedef struct
    {
        int id;
        char name[MAX_NAME_LEN];
        char type[MAX_TYPE_LEN];
        double balance;
    } Account;

    // category sturcture
    typedef struct
    {
        int id;
        char name[MAX_NAME_LEN];
        char type[MAX_TYPE_LEN];
    } Category;

    //  transaction sturcture
    typedef struct
    {
        int id;
        int account_id;
        int category_id;
        double amount;
        char date[MAX_DATE_LEN];
        char description[MAX_DESC_LEN];
        char account_name[MAX_NAME_LEN];
        char category_name[MAX_NAME_LEN];
        char category_type[MAX_TYPE_LEN];
    } Transaction;

    // savings goal sturcture
    typedef struct
    {
        int id;
        char name[MAX_NAME_LEN];
        double target_amount;
        double current_amount;
        char target_date[MAX_DATE_LEN];
    } SavingGoal;

    // database controll functions
    int init_db(const char *db_path);
    void close_db();

    // account operations
    int add_account(const char *name, const char *type, double initial_balance);
    int get_accounts(Account *out_accounts, int max_accounts);
    int update_account_balance(int account_id, double new_balance);

    // category operations
    int add_category(const char *name, const char *type);
    int get_categories(Category *out_categories, int max_categories);

    // transaction operations
    int add_transaction(int account_id, int category_id, double amount, const char *date, const char *description);
    int get_transactions(Transaction *out_transactions, int max_transactions);
    int delete_transaction(int transaction_id);
#ifdef __cplusplus
}
#endif

#endif // DATABASE_H
