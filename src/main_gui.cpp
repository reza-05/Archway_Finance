/*
 * ARCHWAY FINANCE - Dear ImGui Desktop GUI Application Wrapper
 *
 * File: src/gui/main_gui.cpp
 * Standard: C++11 (Graphics Renderer Wrapper)
 *
 * INTERACTIVE LOAN MANAGER & REPAYMENT SYSTEM:
 * - Top Action Bar Inline Loan Button (+ Add Expense | + Add Income | + Transfer | LOAN: BDT XXX).
 * - Auto-Stretched Table Geometry (V5) so Amount and Actions columns are ALWAYS 100% visible on screen.
 * - Interactive Loan Manager Modal with "Pay Now" options.
 * - Unpaid loans highlighted in BOLD SOLID RED; Paid loans turn BOLD GREEN (amounts preserved).
 * - Wallet choice selection for single-click loan settlement.
 * - Minimum arc angle solid pie chart rendering (150px radius).
 */

#define GLFW_INCLUDE_NONE
#include <OpenGL/gl3.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

extern "C" {
    #include "../include/models.h"
    #include "../include/core/core_engine.h"
    #include "../include/core/lallkhata_engine.h"
    #include "../include/core/search_filter.h"
    #include "../include/storage/storage.h"
}

static LedgerState g_state;
static TransactionFilter g_filter;
static const Transaction *g_filtered_transactions[MAX_TRANSACTIONS];
static int g_filtered_count = 0;

static int active_nav_tab = 0; // 0 = Ledger & Transactions, 1 = Statistics & Analytics

static char search_buf[100] = "";
static int wallet_filter_idx = 0;   // 0 = All Wallets
static int category_filter_idx = 0; // 0 = All Categories

static char g_categories[30][MAX_CAT_LEN];
static int g_category_count = 0;
static char wallet_names_buf[512] = "All Wallets\0";

// Global One-Shot Popup Triggers
static bool trigger_open_add_acc = false;
static bool trigger_open_edit_acc = false;
static bool trigger_open_add_exp = false;
static bool trigger_open_add_inc = false;
static bool trigger_open_add_transfer = false;
static bool trigger_open_edit_tx = false;
static bool trigger_open_loan_manager = false;
static bool trigger_open_pay_loan = false;

// Pre-populated Student Life Category Presets (Ending with "Others")
static const char* student_expense_cats[] = {
    "Food & Dining",
    "Stationery & Books",
    "Tuition & Semester Fees",
    "Transportation & Fuel",
    "Rent & Utilities",
    "Mobile & Data Recharge",
    "Entertainment & Outings",
    "Shopping & Personal",
    "Health & Medicine",
    "Gifts & Treats",
    "Others"
};
static int selected_exp_cat_idx = 0;

static const char* student_income_cats[] = {
    "Pocket Money / Allowance",
    "Tuition / Coaching Fee",
    "Part-time Job / Salary",
    "Scholarship / Stipend",
    "Freelancing & Projects",
    "Gift & Bonus",
    "Others"
};
static int selected_inc_cat_idx = 0;

// High-Contrast Vibrant Color Palette for Solid Pie Chart Slices
static ImU32 pie_colors_expense[] = {
    IM_COL32(41, 128, 185, 255),  // Vibrant Blue
    IM_COL32(230, 126, 34, 255),  // Bright Orange
    IM_COL32(233, 30, 99, 255),   // Rose Pink
    IM_COL32(155, 89, 182, 255),  // Purple
    IM_COL32(46, 204, 113, 255),  // Emerald Green
    IM_COL32(241, 196, 15, 255),  // Yellow Gold
    IM_COL32(26, 188, 156, 255),  // Turquoise
    IM_COL32(231, 76, 60, 255),   // Crimson Red
    IM_COL32(149, 165, 166, 255)  // Cool Grey
};

static ImU32 pie_colors_income[] = {
    IM_COL32(46, 204, 113, 255),  // Emerald Green
    IM_COL32(41, 128, 185, 255),  // Blue
    IM_COL32(155, 89, 182, 255),  // Purple
    IM_COL32(241, 196, 15, 255),  // Gold
    IM_COL32(26, 188, 156, 255),  // Cyan
    IM_COL32(233, 30, 99, 255)    // Pink
};
