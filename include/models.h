/**
 * ARCHWAY FINANCE - Pure C Personal Finance Ledger
 * 
 * File: include/models.h
 * Description: Data models, structures, and enumerations for the entire application.
 * Standard: C99 / C11
 */

#ifndef MODELS_H
#define MODELS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_NAME_LEN 50
#define MAX_CAT_LEN 30
#define MAX_NOTE_LEN 100
#define MAX_DATE_LEN 25  // "YYYY-MM-DD HH:MM AM/PM"
#define MAX_ACCOUNTS 15
#define MAX_GOALS 10
#define MAX_TRANSACTIONS 500

/**
 * Account Types supported in Add Wallet form:
 * Cash, MFS (bKash/Nagad/Rocket), Card, Savings, Bank
 */
typedef enum {
    ACCOUNT_CASH = 0,
    ACCOUNT_MFS = 1,
    ACCOUNT_CARD = 2,
    ACCOUNT_SAVINGS = 3,
    ACCOUNT_BANK = 4
} AccountType;

/**
 * Transaction Types (Standard English Terms: Expense, Income, Transfer)
 */
typedef enum {
    TRANSACTION_EXPENSE = 0, // Expense (-BDT)
    TRANSACTION_INCOME = 1,  // Income (+BDT)
    TRANSACTION_TRANSFER = 2 // Transfer between accounts
} TransactionType;

/**
 * Account Structure (Starts at 0.00 BDT)
 */
typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    AccountType type;
    double current_balance;
} Account;


#endif // MODELS_H
