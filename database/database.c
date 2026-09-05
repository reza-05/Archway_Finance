#include <direct.h>

int storage_save_ledger(const LedgerState *state) {
    if (!state) return 0;
    storage_init_environment();

    FILE *fa = fopen(ACCOUNTS_FILE, "wb");
    if (fa) {
        fwrite(&state->account_count, sizeof(int), 1, fa);
        if (state->account_count > 0) {
            fwrite(state->accounts, sizeof(Account), state->account_count, fa);
        }
        fclose(fa);
    } else return 0;

    FILE *ft = fopen(TRANSACTIONS_FILE, "wb");
    if (ft) {
        fwrite(&state->transaction_count, sizeof(int), 1, ft);
        if (state->transaction_count > 0) {
            fwrite(state->transactions, sizeof(Transaction), state->transaction_count, ft);
        }
        fclose(ft);
    } else return 0;

    FILE *fg = fopen(GOALS_FILE, "wb");
    if (fg) {
        fwrite(&state->goal_count, sizeof(int), 1, fg);
        if (state->goal_count > 0) {
            fwrite(state->goals, sizeof(SavingGoal), state->goal_count, fg);
        }
        fclose(fg);
    } else return 0;

    return 1;
}


int storage_load_ledger(LedgerState *state) {
    if (!state) return 0;
    core_init_ledger(state);

    FILE *fa = fopen(ACCOUNTS_FILE, "rb");
    if (fa) {
        fread(&state->account_count, sizeof(int), 1, fa);
        if (state->account_count > MAX_ACCOUNTS) state->account_count = MAX_ACCOUNTS;
        if (state->account_count > 0) {
            fread(state->accounts, sizeof(Account), state->account_count, fa);
        }
        fclose(fa);
    } else return 0;

    FILE *ft = fopen(TRANSACTIONS_FILE, "rb");
    if (ft) {
        fread(&state->transaction_count, sizeof(int), 1, ft);
        if (state->transaction_count > MAX_TRANSACTIONS) state->transaction_count = MAX_TRANSACTIONS;
        if (state->transaction_count > 0) {
            fread(state->transactions, sizeof(Transaction), state->transaction_count, ft);
        }
        fclose(ft);
    }

    FILE *fg = fopen(GOALS_FILE, "rb");
    if (fg) {
        fread(&state->goal_count, sizeof(int), 1, fg);
        if (state->goal_count > MAX_GOALS) state->goal_count = MAX_GOALS;
        if (state->goal_count > 0) {
            fread(state->goals, sizeof(SavingGoal), state->goal_count, fg);
        }
        fclose(fg);
    }

    return 1;
}

int storage_export_transactions_csv(const LedgerState *state, const char *filepath) {
    if (!state || !filepath) return 0;

    FILE *fp = fopen(filepath, "w");
    if (!fp) return 0;

    fprintf(fp, "ID,Date/Time,Type,Category,WalletFrom,WalletTo,Notes,Amount,RunningBalance\n");

    for (int i = 0; i < state->transaction_count; i++) {
        const Transaction *tx = &state->transactions[i];
        Account *from = core_find_account((LedgerState*)state, tx->wallet_from_id);
        Account *to = core_find_account((LedgerState*)state, tx->wallet_to_id);

        const char *from_name = from ? from->name : "-";
        const char *to_name = to ? to->name : "-";
        const char *type_str = (tx->type == TRANSACTION_INCOME) ? "Income" : 
                               ((tx->type == TRANSACTION_EXPENSE) ? "Expense" : "Transfer");

        fprintf(fp, "%d,%s,%s,%s,%s,%s,\"%s\",%.2f,%.2f\n",
                tx->id, tx->datetime, type_str, tx->category,
                from_name, to_name, tx->notes, tx->amount, tx->running_balance);
    }

    fclose(fp);
    return 1;
}
