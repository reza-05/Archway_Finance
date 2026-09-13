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
