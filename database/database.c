#include <direct.h>


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
