/**
 * ARCHWAY FINANCE - Pure C Personal Finance Ledger
 * 
 * MODULE 1 EXTENSION: PURE C SEARCH & FILTER ENGINE
 * File: src/core/search_filter.c
 * 
 * Detailed explanation for SP Lab Evaluation:
 * - Implements custom case-insensitive substring search (`c_strcasestr`) using standard C string functions (`tolower`, `strlen`).
 * - Filters transactions dynamically based on Wallet ID, Category, and Notes text query without relying on any external C++ search library!
 */

#include "../../include/core/search_filter.h"
#include <ctype.h>

/**
 * Case-insensitive string search helper in Pure C
 */
int c_strcasestr(const char *haystack, const char *needle) {
    if (!haystack || !needle) return 0;
    if (needle[0] == '\0') return 1; // Empty search string matches everything

    size_t h_len = strlen(haystack);
    size_t n_len = strlen(needle);

    if (n_len > h_len) return 0;

    for (size_t i = 0; i <= h_len - n_len; i++) {
        size_t j = 0;
        while (j < n_len && tolower((unsigned char)haystack[i + j]) == tolower((unsigned char)needle[j])) {
            j++;
        }
        if (j == n_len) return 1; // Match found!
    }

    return 0; // No match
}

/**
 * Reset filter to default state (matches everything)
 */
void filter_init_default(TransactionFilter *filter) {
    if (!filter) return;
    filter->wallet_id_filter = -1; // -1 means All Wallets
    filter->category_filter[0] = '\0'; // Empty means All Categories
    filter->search_text[0] = '\0'; // Empty means no search filter
}

/**
 * Match an individual transaction against the filter criteria
 */
int filter_match_transaction(const Transaction *tx, const TransactionFilter *filter) {
    if (!tx || !filter) return 0;

    // 1. Wallet Filter Match
    if (filter->wallet_id_filter != -1) {
        if (tx->wallet_from_id != filter->wallet_id_filter && tx->wallet_to_id != filter->wallet_id_filter) {
            return 0;
        }
    }

    // 2. Category Filter Match
    if (filter->category_filter[0] != '\0' && strcmp(filter->category_filter, "All Categories") != 0) {
        if (strcmp(tx->category, filter->category_filter) != 0) {
            return 0;
        }
    }

    // 3. Search Text Query Match (in notes or category)
    if (filter->search_text[0] != '\0') {
        int note_match = c_strcasestr(tx->notes, filter->search_text);
        int cat_match = c_strcasestr(tx->category, filter->search_text);
        if (!note_match && !cat_match) {
            return 0;
        }
    }

    return 1; // Passed all criteria
}

/**
 * Execute filter on all transactions in state and populate output pointers array
 */
int filter_execute(const LedgerState *state, const TransactionFilter *filter, const Transaction **result_array, int max_results) {
    if (!state || !filter || !result_array) return 0;

    int match_count = 0;
    for (int i = 0; i < state->transaction_count; i++) {
        if (filter_match_transaction(&state->transactions[i], filter)) {
            if (match_count < max_results) {
                result_array[match_count++] = &state->transactions[i];
            }
        }
    }

    return match_count;
}
