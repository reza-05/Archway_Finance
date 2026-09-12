/**
 * ARCHWAY FINANCE - Pure C Personal Finance Ledger
 * 
 * File: include/models.h
 * Description: Data models, structures, and enumerations for the entire application.
 * Standard: C99 / C11
 */

#ifndef MODELS_H
#define MODELS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_NAME_LEN 50
#define MAX_CAT_LEN 30
#define MAX_NOTE_LEN 100
#define MAX_DATE_LEN 25  // "YYYY-MM-DD HH:MM AM/PM"
#define MAX_ACCOUNTS 15
#define MAX_GOALS 10
#define MAX_TRANSACTIONS 500

/**
 * Account Types supported in Add Wallet form:
 * Cash, MFS (bKash/Nagad/Rocket), Card, Savings, Bank
 */
typedef enum {
    ACCOUNT_CASH = 0,
    ACCOUNT_MFS = 1,
    ACCOUNT_CARD = 2,
    ACCOUNT_SAVINGS = 3,
    ACCOUNT_BANK = 4
} AccountType;

/**
 * Transaction Types (Standard English Terms: Expense, Income, Transfer)
 */
typedef enum {
    TRANSACTION_EXPENSE = 0, // Expense (-BDT)
    TRANSACTION_INCOME = 1,  // Income (+BDT)
    TRANSACTION_TRANSFER = 2 // Transfer between accounts
} TransactionType;

/**
 * Account Structure (Starts at 0.00 BDT)
 */
typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    AccountType type;
    double current_balance;
} Account;

/**
 * Transaction Structure with Running Balance Tracking
 */
typedef struct {
    int id;
    int wallet_from_id;         // Account ID money came from (-1 if N/A or Income)
    int wallet_to_id;           // Account ID money went to (-1 if N/A or Expense)
    TransactionType type;
    char category[MAX_CAT_LEN];  // e.g. "Food & Drinks", "Gifts", "Health & Beauty"
    double amount;
    double running_balance;     // Running Balance after this transaction
    char datetime[MAX_DATE_LEN]; // "YYYY-MM-DD HH:MM AM/PM"
    char notes[MAX_NOTE_LEN];
} Transaction;

/**
 * Daily Summary Structure
 */
typedef struct {
    char date[11];               // "YYYY-MM-DD"
    double daily_income;         // Today's Total Income
    double daily_expense;        // Today's Total Expense
    double closing_balance;      // Today's Closing Balance
} DailySummary;

/**
 * Search & Filter Criteria Struct
 */
typedef struct {
    int wallet_id_filter;       // -1 for All Wallets, else specific Account ID
    char category_filter[MAX_CAT_LEN]; // Empty string for All Categories, else category name
    char search_text[MAX_NOTE_LEN];    // Text search query matching notes or category
} TransactionFilter;

/**
 * Saving Goal Structure
 */
typedef struct {
    int id;
    char title[MAX_NAME_LEN];
    double target_amount;
    double current_amount;
    char target_date[15];
} SavingGoal;

/**
 * Category Breakdown Result Structure
 */
typedef struct {
    char category[MAX_CAT_LEN];
    double total_spent;
    double percentage;
} CategoryBreakdown;

/**
 * Main Ledger State containing all active in-memory data
 */
typedef struct {
    Account accounts[MAX_ACCOUNTS];
    int account_count;

    Transaction transactions[MAX_TRANSACTIONS];
    int transaction_count;

    SavingGoal goals[MAX_GOALS];
    int goal_count;
} LedgerState;

// Helper to convert AccountType enum to readable string
static inline const char* get_account_type_str(AccountType type) {
    switch (type) {
        case ACCOUNT_CASH:    return "Cash";
        case ACCOUNT_MFS:     return "MFS";
        case ACCOUNT_CARD:    return "Card";
        case ACCOUNT_SAVINGS: return "Savings";
        case ACCOUNT_BANK:    return "Bank";
        default:              return "Wallet";
    }
}

#endif // MODELS_H
