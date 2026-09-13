/**
 * ARCHWAY FINANCE - Pure C Personal Finance Ledger
 * 
 * MODULE 1: CORE BACKEND & ANALYTICS ENGINE (Managed by Member 1)
 * File: src/core/core_engine.c
 * 
 * OVERDRAFT SAFEGUARD & LOAN ENGINE:
 * - A wallet balance can NEVER drop below 0.00 BDT.
 * - If expense exceeds balance, offers auto-transfer or loan entry.
 */

#include "../../include/core/core_engine.h"
// pending ledger_engine.h
// pending storage.h

void core_init_ledger(LedgerState *state) {
    if (!state) return;
    memset(state, 0, sizeof(LedgerState));
}

// ============================================================================
// ACCOUNT OPERATIONS
// ============================================================================

int core_add_account(LedgerState *state, const char *name, AccountType type, double initial_balance) {
    if (!state || state->account_count >= MAX_ACCOUNTS) return -1;
    if (!name || strlen(name) == 0) return -1;

    // Generate unique account ID
    int max_id = 0;
    for (int i = 0; i < state->account_count; i++) {
        if (state->accounts[i].id > max_id) {
            max_id = state->accounts[i].id;
        }
    }

    Account *acc = &state->accounts[state->account_count];
    acc->id = max_id + 1;
    strncpy(acc->name, name, MAX_NAME_LEN - 1);
    acc->name[MAX_NAME_LEN - 1] = '\0';
    acc->type = type;
    acc->current_balance = (initial_balance < 0.0) ? 0.0 : initial_balance;

    state->account_count++;
    return acc->id;
}

int core_update_account(LedgerState *state, int account_id, const char *new_name, AccountType new_type, double new_balance) {
    Account *acc = core_find_account(state, account_id);
    if (!acc) return 0;

    if (new_name && strlen(new_name) > 0) {
        strncpy(acc->name, new_name, MAX_NAME_LEN - 1);
        acc->name[MAX_NAME_LEN - 1] = '\0';
    }
    acc->type = new_type;
    acc->current_balance = (new_balance < 0.0) ? 0.0 : new_balance;

    return 1;
}

int core_delete_account(LedgerState *state, int account_id) {
    if (!state) return 0;

    int index = -1;
    for (int i = 0; i < state->account_count; i++) {
        if (state->accounts[i].id == account_id) {
            index = i;
            break;
        }
    }
    if (index == -1) return 0;

    for (int i = index; i < state->account_count - 1; i++) {
        state->accounts[i] = state->accounts[i + 1];
    }
    state->account_count--;
    return 1;
}

Account* core_find_account(LedgerState *state, int account_id) {
    if (!state || account_id <= 0) return NULL;
    for (int i = 0; i < state->account_count; i++) {
        if (state->accounts[i].id == account_id) {
            return &state->accounts[i];
        }
    }
    return NULL;
}

double core_get_total_balance(const LedgerState *state) {
    if (!state) return 0.0;
    double total = 0.0;
    for (int i = 0; i < state->account_count; i++) {
        total += state->accounts[i].current_balance;
    }
    return total;
}

int core_is_loan_paid(const Transaction *tx) {
    if (!tx) return 0;
    if (strstr(tx->notes, "[PAID]") != NULL || strstr(tx->notes, "[REPAID]") != NULL) {
        return 1;
    }
    return 0;
}

double core_get_total_loan_taken(const LedgerState *state) {
    if (!state) return 0.0;
    double loan_taken = 0.0;
    for (int i = 0; i < state->transaction_count; i++) {
        const Transaction *tx = &state->transactions[i];
        if (strcmp(tx->category, "Loan / Credit") == 0 || strcmp(tx->category, "Loan / Credit Entry") == 0 || strstr(tx->notes, "[Loan") != NULL) {
            if (tx->type == TRANSACTION_INCOME) {
                loan_taken += tx->amount;
            }
        }
    }
    return loan_taken;
}

