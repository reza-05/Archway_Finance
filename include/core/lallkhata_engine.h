/**
 * ARCHWAY FINANCE - Pure C Personal Finance Ledger
 * 
 * MODULE 1 EXTENSION: LEDGER & BALANCE ENGINE
 * File: include/core/lallkhata_engine.h
 * Standard: C99 / C11
 */

#ifndef LALLKHATA_ENGINE_H
#define LALLKHATA_ENGINE_H

#include "../models.h"

// Recalculate running balance across transactions chronologically
void ledger_recalculate_running_balances(LedgerState *state);

// Calculate daily summary (Today's Income, Today's Expense, Closing Balance)
DailySummary ledger_get_daily_summary(const LedgerState *state, const char *date_prefix);

// Direct Balance Editor (Mutates Account Balance directly in C)
int core_set_account_balance(LedgerState *state, int account_id, double new_balance);

// Initialize fresh zero-balance ledger state (0.00 BDT start)
void ledger_init_zero_state(LedgerState *state);

#endif // LALLKHATA_ENGINE_H
