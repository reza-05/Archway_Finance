/**
 * ARCHWAY FINANCE - Pure C Personal Finance Ledger
 * 
 * MODULE 1 EXTENSION: PURE C SEARCH & FILTER ENGINE
 * File: include/core/search_filter.h
 * 
 * Responsibilities:
 * - Dynamic O(N) searching and filtering transactions based on UI selections:
 *   1. Wallet Filter (All Wallets vs Specific Wallet)
 *   2. Category Filter (All Categories vs Specific Category)
 *   3. Search Text Query (Sub-string search in notes and categories)
 */

#ifndef SEARCH_FILTER_H
#define SEARCH_FILTER_H

#include "../models.h"

// Initialize Filter Criteria with defaults (All Wallets, All Categories, Empty Search)
void filter_init_default(TransactionFilter *filter);

// Filter matching algorithm (Pure C case-insensitive string matching)
int filter_match_transaction(const Transaction *tx, const TransactionFilter *filter);

// Execute Filter on LedgerState and return filtered pointers array
int filter_execute(const LedgerState *state, const TransactionFilter *filter, const Transaction **result_array, int max_results);

// Case-insensitive substring search helper in pure C
int c_strcasestr(const char *haystack, const char *needle);

#endif // SEARCH_FILTER_H
