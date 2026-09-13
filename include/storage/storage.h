/**
 * ARCHWAY FINANCE - Pure C Personal Finance Ledger
 * 
 * MODULE 2: DATABASE & STORAGE SYSTEM (Managed by Member 2)
 * File: include/storage/storage.h
 * 
 * Responsibilities:
 * - Persistent Data Storage using Pure C File I/O (`stdio.h`)
 * - Binary Serialization (`.dat` files) using `fwrite` and `fread`
 * - CSV Data Export and Import (`fprintf`, `fscanf`, `strtok`)
 * - Seed Data Generator for immediate evaluation / demonstration
 * - File Verification & System Backup
 */

#ifndef STORAGE_H
#define STORAGE_H

#include "../models.h"

// Directory & File Path Constants
#define DATA_DIR "data"
#define ACCOUNTS_FILE "data/accounts.dat"
#define TRANSACTIONS_FILE "data/transactions.dat"
#define GOALS_FILE "data/goals.dat"
#define CSV_EXPORT_FILE "data/transactions_export.csv"

// Data Storage API
int storage_init_environment(void);
int storage_save_ledger(const LedgerState *state);
int storage_load_ledger(LedgerState *state);

// CSV Import & Export API
int storage_export_transactions_csv(const LedgerState *state, const char *filepath);

// Seed Data Initialization
void storage_seed_initial_data(LedgerState *state);

#endif // STORAGE_H
