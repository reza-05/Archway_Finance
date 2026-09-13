/**
 * ARCHWAY FINANCE - Pure C Personal Finance Ledger
 * 
 * MODULE 1 EXTENSION: LEDGER & BALANCE ENGINE
 * File: src/core/ledger_engine.c
 * Standard: C99 / C11
 * 
 * Side Wallet List default 3 accounts: Cash, Bank (AB BANK), bKash MFS.
 * All accounts start at 0.00 BDT balance on new device initialization.
 */

#include "../../include/core/ledger_engine.h"
#include "../../include/core/core_engine.h"

void ledger_recalculate_running_balances(LedgerState *state) {
    if (!state || state->transaction_count == 0) return;

    core_sort_transactions_by_date(state);

    double total_net = core_get_total_balance(state);
    double net_change = core_get_total_income(state) - core_get_total_expense(state);
    double running_bal = total_net - net_change;

    for (int i = 0; i < state->transaction_count; i++) {
        Transaction *tx = &state->transactions[i];
        if (tx->type == TRANSACTION_INCOME) {
            running_bal += tx->amount;
        } else if (tx->type == TRANSACTION_EXPENSE) {
            running_bal -= tx->amount;
        }
        tx->running_balance = running_bal;
    }
}

DailySummary ledger_get_daily_summary(const LedgerState *state, const char *date_prefix) {
    DailySummary summary;
    memset(&summary, 0, sizeof(DailySummary));

    if (date_prefix && date_prefix[0] != '\0') {
        strncpy(summary.date, date_prefix, 10);
    } else {
        strcpy(summary.date, "Today");
    }

    summary.daily_income = 0.0;
    summary.daily_expense = 0.0;

    for (int i = 0; i < state->transaction_count; i++) {
        const Transaction *tx = &state->transactions[i];
        if (date_prefix == NULL || date_prefix[0] == '\0' || strncmp(tx->datetime, date_prefix, 10) == 0) {
            if (tx->type == TRANSACTION_INCOME) {
                summary.daily_income += tx->amount;
            } else if (tx->type == TRANSACTION_EXPENSE) {
                summary.daily_expense += tx->amount;
            }
        }
    }

    summary.closing_balance = core_get_total_balance(state);
    return summary;
}

int core_set_account_balance(LedgerState *state, int account_id, double new_balance) {
    Account *acc = core_find_account(state, account_id);
    if (!acc) return 0;

    acc->current_balance = (new_balance < 0.0) ? 0.0 : new_balance;
    ledger_recalculate_running_balances(state);
    return 1;
}

void ledger_init_zero_state(LedgerState *state) {
    core_init_ledger(state);

    // Default 3 side wallets: Cash, Bank (AB BANK), bKash MFS starting at 0.00 BDT
    core_add_account(state, "Cash", ACCOUNT_CASH, 0.00);
    core_add_account(state, "Bank (AB BANK)", ACCOUNT_BANK, 0.00);
    core_add_account(state, "bKash MFS", ACCOUNT_MFS, 0.00);
}
