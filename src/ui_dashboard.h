#pragma once
#include <string>
#include <vector>

struct Account {
    int id;
    std::string name;
    std::string type;         // "Cash", "Savings account", "Checking account", "Credit Card", etc.
    double initial_balance;
    double current_balance;   // Dynamically calculated: initial_balance + sum(inflows) - sum(outflows)
};

struct Transaction {
    int id;
    std::string date;         // YYYY-MM-DD
    std::string time;         // HH:MM (e.g. 08:12 PM)
    std::string type;         // "Income", "Expense", "Transfer"
    std::string category;     // Food, Salary, Gift, etc.
    int account_from_id;      // -1 if None (e.g. for Income)
    int account_to_id;        // -1 if None (e.g. for Expense)
    double amount;
    std::string notes;
};

struct AppState {
    std::vector<Account> accounts;
    std::vector<Transaction> transactions;
    double total_balance;
    double total_income;
    double total_expense;

    // Filters and UI states
    int selected_account_id;  // Filter for transactions (-1 for all)
    char search_query[128];
    char filter_category[64];

    // Modals control state
    bool show_add_account_modal;
    bool show_edit_account_modal;
    int editing_account_id;

    bool show_add_transaction_modal;
    bool show_edit_transaction_modal;
    int editing_transaction_id;

    // Temporal inputs for modals
    char input_acc_name[128];
    char input_acc_type[128];
    double input_acc_initial_balance;

    char input_tx_date[32];
    char input_tx_time[32];
    char input_tx_type[32]; // "Income", "Expense", "Transfer"
    char input_tx_category[128];
    int input_tx_acc_from_id;
    int input_tx_acc_to_id;
    double input_tx_amount;
    char input_tx_notes[256];

    AppState();

    void RecalculateBalances();
    
    // CRUD Accounts
    void AddAccount(const std::string& name, const std::string& type, double initial_balance);
    void EditAccount(int id, const std::string& name, const std::string& type, double initial_balance);
    void DeleteAccount(int id);
    Account* GetAccountById(int id);

    // CRUD Transactions
    void AddTransaction(const std::string& date, const std::string& time, const std::string& type, 
                        const std::string& category, int account_from_id, int account_to_id, 
                        double amount, const std::string& notes);
    void EditTransaction(int id, const std::string& date, const std::string& time, const std::string& type, 
                         const std::string& category, int account_from_id, int account_to_id, 
                         double amount, const std::string& notes);
    void DeleteTransaction(int id);
    Transaction* GetTransactionById(int id);

    // Helpers
    int GetNextAccountId();
    int GetNextTransactionId();

    // Data persistence
    bool LoadData();
    bool SaveData();
};

// Main rendering function to be called in your ImGui frame loop
void RenderDashboardWindow(AppState& state);