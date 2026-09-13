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

double core_get_total_loan_repaid(const LedgerState *state) {
    if (!state) return 0.0;
    double loan_repay = 0.0;
    for (int i = 0; i < state->transaction_count; i++) {
        const Transaction *tx = &state->transactions[i];
        if (strcmp(tx->category, "Loan Repayment") == 0 || strcmp(tx->category, "Loan Repay") == 0) {
            loan_repay += tx->amount;
        }
    }
    return loan_repay;
}

double core_get_outstanding_loan_balance(const LedgerState *state) {
    if (!state) return 0.0;
    double outstanding = 0.0;
    for (int i = 0; i < state->transaction_count; i++) {
        const Transaction *tx = &state->transactions[i];
        if (strcmp(tx->category, "Loan / Credit") == 0 || strcmp(tx->category, "Loan / Credit Entry") == 0 || strstr(tx->notes, "[Loan") != NULL) {
            if (tx->type == TRANSACTION_INCOME && !core_is_loan_paid(tx)) {
                outstanding += tx->amount;
            }
        }
    }
    return outstanding;
}

double core_get_total_loan_balance(const LedgerState *state) {
    return core_get_outstanding_loan_balance(state);
}

int core_pay_specific_loan(LedgerState *state, int loan_tx_id, int wallet_id, const char *datetime) {
    if (!state) return -1;
    Transaction *target_tx = NULL;
    for (int i = 0; i < state->transaction_count; i++) {
        if (state->transactions[i].id == loan_tx_id) {
            target_tx = &state->transactions[i];
            break;
        }
    }

    if (!target_tx || core_is_loan_paid(target_tx)) return 0;

    Account *paying_acc = core_find_account(state, wallet_id);
    if (!paying_acc || paying_acc->current_balance < target_tx->amount) {
        return -2; // Insufficient balance in paying wallet
    }

    // Prepend [PAID] tag to transaction notes
    char updated_notes[MAX_NOTE_LEN];
    snprintf(updated_notes, sizeof(updated_notes), "[PAID] %s", target_tx->notes);
    strncpy(target_tx->notes, updated_notes, MAX_NOTE_LEN - 1);
    target_tx->notes[MAX_NOTE_LEN - 1] = '\0';

    // Record Loan Repayment Expense
    char repay_note[100];
    snprintf(repay_note, sizeof(repay_note), "[Repaid Loan #%d of BDT %.2f]", target_tx->id, target_tx->amount);
    core_add_transaction(state, wallet_id, -1, TRANSACTION_EXPENSE, "Loan Repayment", target_tx->amount, datetime, repay_note);

    return 1;
}

// ============================================================================
// TRANSACTION OPERATIONS & OVERDRAFT SAFEGUARD
// ============================================================================

int core_add_transaction(LedgerState *state, int wallet_from_id, int wallet_to_id, TransactionType type, 
                         const char *category, double amount, const char *datetime, const char *notes) {
    return core_add_transaction_with_overdraft(state, wallet_from_id, wallet_to_id, type, category, amount, datetime, notes, OVERDRAFT_REJECT, -1);
}

/**
 * Add Transaction with Overdraft Protection (NO NEGATIVE BALANCES)
 */
int core_add_transaction_with_overdraft(LedgerState *state, int wallet_from_id, int wallet_to_id, TransactionType type,
                                        const char *category, double amount, const char *datetime, const char *notes,
                                        OverdraftMode mode, int cover_source_wallet_id) {
    if (!state || state->transaction_count >= MAX_TRANSACTIONS) return -1;
    if (amount <= 0.0) return -1;

    Account *from = core_find_account(state, wallet_from_id);
    Account *to = core_find_account(state, wallet_to_id);

    // NO NEGATIVE BALANCE SAFEGUARD
    if ((type == TRANSACTION_EXPENSE || type == TRANSACTION_TRANSFER) && from) {
        if (from->current_balance < amount) {
            double deficit = amount - from->current_balance;

            if (mode == OVERDRAFT_REJECT) {
                return -2; // Insufficient funds error code
            } else if (mode == OVERDRAFT_COVER_TRANSFER) {
                Account *cover_src = core_find_account(state, cover_source_wallet_id);
                if (!cover_src || cover_src->current_balance < deficit) {
                    return -2; // Cover wallet also lacks funds!
                }
                // Auto Transfer Deficit from cover wallet to paying wallet
                cover_src->current_balance -= deficit;
                from->current_balance += deficit;

                // Record Auto Transfer Entry
                char auto_note[100];
                snprintf(auto_note, sizeof(auto_note), "[Auto Cover Deficit for %s]", category);
                core_add_transaction(state, cover_source_wallet_id, wallet_from_id, TRANSACTION_TRANSFER, "Auto Transfer", deficit, datetime, auto_note);
            } else if (mode == OVERDRAFT_COVER_LOAN) {
                // Record Loan / Credit Entry - Set paying wallet balance to amount so after deducting expense, balance is EXACTLY 0.00 BDT
                from->current_balance = amount;
                char loan_note[100];
                snprintf(loan_note, sizeof(loan_note), "[Loan/Credit to cover %s deficit]", category);
                
                // Add loan transaction record WITHOUT mutating wallet balance again!
                Transaction *loan_tx = &state->transactions[state->transaction_count++];
                loan_tx->id = state->transaction_count;
                loan_tx->wallet_from_id = -1;
                loan_tx->wallet_to_id = wallet_from_id;
                loan_tx->type = TRANSACTION_INCOME; // Tracked as Loan Entry
                strncpy(loan_tx->category, "Loan / Credit", MAX_CAT_LEN - 1);
                loan_tx->category[MAX_CAT_LEN - 1] = '\0';
                loan_tx->amount = deficit;
                strncpy(loan_tx->datetime, datetime, MAX_DATE_LEN - 1);
                loan_tx->datetime[MAX_DATE_LEN - 1] = '\0';
                strncpy(loan_tx->notes, loan_note, MAX_NOTE_LEN - 1);
                loan_tx->notes[MAX_NOTE_LEN - 1] = '\0';
            } else if (mode == OVERDRAFT_COVER_SKIP) {
                // Cover & Skip: Set paying wallet balance so after deducting expense, balance is EXACTLY 0.00 BDT
                from->current_balance = amount;
                char skip_note[100];
                snprintf(skip_note, sizeof(skip_note), "[Cover & Skip Adjustment for %s]", category);
                
                Transaction *skip_tx = &state->transactions[state->transaction_count++];
                skip_tx->id = state->transaction_count;
                skip_tx->wallet_from_id = -1;
                skip_tx->wallet_to_id = wallet_from_id;
                skip_tx->type = TRANSACTION_INCOME;
                strncpy(skip_tx->category, "Cover & Skip", MAX_CAT_LEN - 1);
                skip_tx->category[MAX_CAT_LEN - 1] = '\0';
                skip_tx->amount = deficit;
                strncpy(skip_tx->datetime, datetime, MAX_DATE_LEN - 1);
                skip_tx->datetime[MAX_DATE_LEN - 1] = '\0';
                strncpy(skip_tx->notes, skip_note, MAX_NOTE_LEN - 1);
                skip_tx->notes[MAX_NOTE_LEN - 1] = '\0';
            }
        }
    }

    Transaction *tx = &state->transactions[state->transaction_count];
    tx->id = state->transaction_count + 1;
    tx->wallet_from_id = wallet_from_id;
    tx->wallet_to_id = wallet_to_id;
    tx->type = type;
    strncpy(tx->category, category, MAX_CAT_LEN - 1);
    tx->category[MAX_CAT_LEN - 1] = '\0';
    tx->amount = amount;
    strncpy(tx->datetime, datetime, MAX_DATE_LEN - 1);
    tx->datetime[MAX_DATE_LEN - 1] = '\0';
    if (notes) {
        strncpy(tx->notes, notes, MAX_NOTE_LEN - 1);
        tx->notes[MAX_NOTE_LEN - 1] = '\0';
    } else {
        tx->notes[0] = '\0';
    }

    if (type == TRANSACTION_INCOME) {
        if (to) to->current_balance += amount;
    } else if (type == TRANSACTION_EXPENSE) {
        if (from) from->current_balance -= amount;
    } else if (type == TRANSACTION_TRANSFER) {
        if (from) from->current_balance -= amount;
        if (to) to->current_balance += amount;
    }

    // Double check safeguard: Ensure no balance is negative
    for (int i = 0; i < state->account_count; i++) {
        if (state->accounts[i].current_balance < 0.0) {
            state->accounts[i].current_balance = 0.0;
        }
    }

    state->transaction_count++;
    ledger_recalculate_running_balances(state);
    return tx->id;
}

int core_update_transaction(LedgerState *state, int tx_id, int wallet_from_id, int wallet_to_id, TransactionType type,
                            const char *category, double amount, const char *datetime, const char *notes) {
    int idx = -1;
    for (int i = 0; i < state->transaction_count; i++) {
        if (state->transactions[i].id == tx_id) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return 0;

    Transaction *old_tx = &state->transactions[idx];
    if (old_tx->type == TRANSACTION_INCOME) {
        Account *to = core_find_account(state, old_tx->wallet_to_id);
        if (to) to->current_balance -= old_tx->amount;
    } else if (old_tx->type == TRANSACTION_EXPENSE) {
        Account *from = core_find_account(state, old_tx->wallet_from_id);
        if (from) from->current_balance += old_tx->amount;
    } else if (old_tx->type == TRANSACTION_TRANSFER) {
        Account *from = core_find_account(state, old_tx->wallet_from_id);
        Account *to = core_find_account(state, old_tx->wallet_to_id);
        if (from) from->current_balance += old_tx->amount;
        if (to) to->current_balance -= old_tx->amount;
    }

    old_tx->wallet_from_id = wallet_from_id;
    old_tx->wallet_to_id = wallet_to_id;
    old_tx->type = type;
    strncpy(old_tx->category, category, MAX_CAT_LEN - 1);
    old_tx->category[MAX_CAT_LEN - 1] = '\0';
    old_tx->amount = amount;
    strncpy(old_tx->datetime, datetime, MAX_DATE_LEN - 1);
    old_tx->datetime[MAX_DATE_LEN - 1] = '\0';
    if (notes) {
        strncpy(old_tx->notes, notes, MAX_NOTE_LEN - 1);
        old_tx->notes[MAX_NOTE_LEN - 1] = '\0';
    }

    if (type == TRANSACTION_INCOME) {
        Account *to = core_find_account(state, wallet_to_id);
        if (to) to->current_balance += amount;
    } else if (type == TRANSACTION_EXPENSE) {
        Account *from = core_find_account(state, wallet_from_id);
        if (from) from->current_balance -= amount;
    } else if (type == TRANSACTION_TRANSFER) {
        Account *from = core_find_account(state, wallet_from_id);
        Account *to = core_find_account(state, wallet_to_id);
        if (from) from->current_balance -= amount;
        if (to) to->current_balance += amount;
    }

    // Double check safeguard: Ensure no balance is negative
    for (int i = 0; i < state->account_count; i++) {
        if (state->accounts[i].current_balance < 0.0) {
            state->accounts[i].current_balance = 0.0;
        }
    }

    ledger_recalculate_running_balances(state);
    return 1;
}

int core_delete_transaction(LedgerState *state, int tx_id) {
    int idx = -1;
    for (int i = 0; i < state->transaction_count; i++) {
        if (state->transactions[i].id == tx_id) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return 0;

    Transaction *tx = &state->transactions[idx];

    // Revert Wallet Balances
    if (tx->type == TRANSACTION_INCOME) {
        Account *to = core_find_account(state, tx->wallet_to_id);
        if (to) to->current_balance -= tx->amount;
    } else if (tx->type == TRANSACTION_EXPENSE) {
        Account *from = core_find_account(state, tx->wallet_from_id);
        if (from) from->current_balance += tx->amount;
    } else if (tx->type == TRANSACTION_TRANSFER) {
        Account *from = core_find_account(state, tx->wallet_from_id);
        Account *to = core_find_account(state, tx->wallet_to_id);
        if (from) from->current_balance += tx->amount;
        if (to) to->current_balance -= tx->amount;
    }

    // IF DELETING A LOAN REPAYMENT: Revert target loan status from PAID back to UNPAID
    if (strcmp(tx->category, "Loan Repayment") == 0 || strcmp(tx->category, "Loan Repay") == 0 || strstr(tx->notes, "[Repaid Loan #") != NULL) {
        int target_loan_id = -1;
        const char *loan_id_ptr = strstr(tx->notes, "[Repaid Loan #");
        if (loan_id_ptr) {
            target_loan_id = atoi(loan_id_ptr + strlen("[Repaid Loan #"));
        }

        for (int i = 0; i < state->transaction_count; i++) {
            if (i == idx) continue;
            Transaction *other_tx = &state->transactions[i];

            bool matches_loan = (target_loan_id > 0) ? (other_tx->id == target_loan_id) :
                                ((strcmp(other_tx->category, "Loan / Credit") == 0) && core_is_loan_paid(other_tx));

            if (matches_loan) {
                // Strip "[PAID] " or "[REPAID] " from note
                char *paid_ptr = strstr(other_tx->notes, "[PAID] ");
                if (paid_ptr) {
                    memmove(paid_ptr, paid_ptr + strlen("[PAID] "), strlen(paid_ptr + strlen("[PAID] ")) + 1);
                } else {
                    char *repaid_ptr = strstr(other_tx->notes, "[REPAID] ");
                    if (repaid_ptr) {
                        memmove(repaid_ptr, repaid_ptr + strlen("[REPAID] "), strlen(repaid_ptr + strlen("[REPAID] ")) + 1);
                    }
                }
                if (target_loan_id > 0) break; // Reverted specific target loan
            }
        }
    }

    for (int i = idx; i < state->transaction_count - 1; i++) {
        state->transactions[i] = state->transactions[i + 1];
    }
    state->transaction_count--;

    // Double check safeguard: Ensure no balance is negative
    for (int i = 0; i < state->account_count; i++) {
        if (state->accounts[i].current_balance < 0.0) {
            state->accounts[i].current_balance = 0.0;
        }
    }

    ledger_recalculate_running_balances(state);
    return 1;
}

int core_get_unique_categories(const LedgerState *state, char categories[][MAX_CAT_LEN], int max_cats) {
    int count = 0;
    for (int i = 0; i < state->transaction_count; i++) {
        const char *cat = state->transactions[i].category;
        if (cat[0] == '\0') continue;

        int exists = 0;
        for (int j = 0; j < count; j++) {
            if (strcmp(categories[j], cat) == 0) {
                exists = 1;
                break;
            }
        }
        if (!exists && count < max_cats) {
            strncpy(categories[count], cat, MAX_CAT_LEN - 1);
            categories[count][MAX_CAT_LEN - 1] = '\0';
            count++;
        }
    }
    return count;
}

double core_get_total_income(const LedgerState *state) {
    double total = 0.0;
    for (int i = 0; i < state->transaction_count; i++) {
        const Transaction *tx = &state->transactions[i];
        if (tx->type == TRANSACTION_INCOME) {
            if (strcmp(tx->category, "Loan / Credit") != 0 &&
                strcmp(tx->category, "Loan / Credit Entry") != 0 &&
                strcmp(tx->category, "Cover & Skip") != 0 &&
                strstr(tx->notes, "[Loan") == NULL) {
                total += tx->amount;
            }
        }
    }
    return total;
}

