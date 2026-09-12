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



