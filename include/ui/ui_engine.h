/**
 * ARCHWAY FINANCE - Pure C Personal Finance Ledger
 * 
 * MODULE 3: FRONTEND & USER INTERFACE (Managed by Member 3)
 * File: include/ui/ui_engine.h
 * 
 * Responsibilities:
 * - Render Rich Interactive Dashboard & Visualizations in Pure C
 * - Render Balance Trend Graphs (ANSI visual plot)
 * - Render Category Expenditure Bar Charts
 * - Form Input Handling & Menu Navigation
 * - Formatted Table Display for Accounts, Transactions, and Goals
 */

#ifndef UI_ENGINE_H
#define UI_ENGINE_H

#include "../models.h"

// Terminal Formatting ANSI Color Macros
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BG_DARK "\033[40m"

// UI Engine API
void ui_clear_screen(void);
void ui_draw_header(const char *title);
void ui_render_dashboard(const LedgerState *state);

// Visual Graphs & Plotting (Pure C Math + ANSI Graphics)
void ui_render_balance_trend_chart(const LedgerState *state);
void ui_render_category_bar_chart(const LedgerState *state);
void ui_render_savings_goals(const LedgerState *state);

// Form Inputs & Menus
int ui_show_main_menu(void);
void ui_prompt_add_account(LedgerState *state);
void ui_prompt_add_transaction(LedgerState *state);
void ui_prompt_add_saving_goal(LedgerState *state);
void ui_render_transaction_history(const LedgerState *state);

#endif // UI_ENGINE_H
