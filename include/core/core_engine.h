/**
 * ARCHWAY FINANCE - Pure C Personal Finance Ledger
 * 
 * File: include/core/core_engine.h
 * Description: Core ledger engine headers with Overdraft & Loan Safeguard logic.
 */

#ifndef CORE_ENGINE_H
#define CORE_ENGINE_H

#include "../models.h"
#include "search_filter.h"

/**
 * Overdraft Handling Modes for Ledger Bookkeeping
 */
typedef enum {
    OVERDRAFT_REJECT = 0,          // Reject transaction if funds are insufficient
    OVERDRAFT_COVER_TRANSFER = 1,  // Transfer deficit from another source wallet
    OVERDRAFT_COVER_LOAN = 2,      // Record a Loan/Credit entry to cover deficit
    OVERDRAFT_COVER_SKIP = 3       // Cover & Skip: Makeup deficit & set paying wallet balance to 0.00
} OverdraftMode;

// Core Ledger Operations
void core_init_ledger(LedgerState *state);

// Account CRUD Operations
int core_add_account(LedgerState *state, const char *name, AccountType type, double initial_balance);
int core_update_account(LedgerState *state, int account_id, const char *new_name, AccountType new_type, double new_balance);
int core_delete_account(LedgerState *state, int account_id);
Account* core_find_account(LedgerState *state, int account_id);
double core_get_total_balance(const LedgerState *state);
double core_get_total_loan_balance(const LedgerState *state);
double core_get_total_loan_taken(const LedgerState *state);
double core_get_total_loan_repaid(const LedgerState *state);
double core_get_outstanding_loan_balance(const LedgerState *state);
int core_is_loan_paid(const Transaction *tx);
int core_pay_specific_loan(LedgerState *state, int loan_tx_id, int wallet_id, const char *datetime);

// Transaction CRUD Operations with Overdraft Protection
int core_add_transaction(LedgerState *state, int wallet_from_id, int wallet_to_id, TransactionType type, 
                         const char *category, double amount, const char *datetime, const char *notes);

int core_add_transaction_with_overdraft(LedgerState *state, int wallet_from_id, int wallet_to_id, TransactionType type,
                                        const char *category, double amount, const char *datetime, const char *notes,
                                        OverdraftMode mode, int cover_source_wallet_id);

int core_repay_loan(LedgerState *state, int wallet_id, double amount, const char *datetime, const char *notes);

int core_update_transaction(LedgerState *state, int tx_id, int wallet_from_id, int wallet_to_id, TransactionType type,
                            const char *category, double amount, const char *datetime, const char *notes);
int core_delete_transaction(LedgerState *state, int tx_id);

// Dynamic Unique Category Extractor
int core_get_unique_categories(const LedgerState *state, char categories[][MAX_CAT_LEN], int max_cats);

// Analytics & Statistical Insights
double core_get_total_income(const LedgerState *state);
double core_get_total_expense(const LedgerState *state);
double core_get_monthly_income(const LedgerState *state, int year, int month);
double core_get_monthly_expense(const LedgerState *state, int year, int month);

// Category Expenditure & Income Breakdown
int core_get_category_breakdown(const LedgerState *state, CategoryBreakdown *output, int max_categories);
int core_get_type_category_breakdown(const LedgerState *state, TransactionType tx_type, CategoryBreakdown *output, int max_categories);

// Recursion: Savings Goal Projection
int core_predict_months_to_goal_recursive(double current_saved, double target, double monthly_savings_rate, int month_counter);

// Sorting Algorithms
void core_sort_transactions_by_date(LedgerState *state);
void core_sort_transactions_by_amount(LedgerState *state);

#endif // CORE_ENGINE_H
