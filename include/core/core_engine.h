/**
 * ARCHWAY FINANCE - Pure C Personal Finance Ledger
 * 
 * File: include/core/core_engine.h
 * Description: Core ledger engine headers with Overdraft & Loan Safeguard logic.
 */

#ifndef CORE_ENGINE_H
#define CORE_ENGINE_H

#include "../models.h"

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


#endif // CORE_ENGINE_H
