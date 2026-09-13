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

static int form_tx_id = -1;
static int form_tx_type = 0; // 0=Expense, 1=Income, 2=Transfer
static char form_tx_category[30] = "";
static char form_tx_amount_str[30] = "0.00";
static char form_tx_datetime[25] = "";
static char form_tx_notes[100] = "";
static int form_tx_from_idx = 0;
static int form_tx_to_idx = 1;


// Overdraft prompt variables
static double overdraft_deficit = 0.0;
static int overdraft_cover_wallet_idx = 0;
static bool show_overdraft_modal = false;

// Formal Loan Repayment Prompt Modal variables
static bool show_loan_repay_modal = false;
static double repay_prompt_deposit_amount = 0.0;
static int repay_prompt_wallet_id = -1;
static char repay_prompt_wallet_name[50] = "";

// Specific Pay Now Loan Modal Variables
static int pay_now_target_loan_id = -1;
static int pay_now_wallet_idx = 0;

static int form_acc_id = -1;
static char form_acc_name[50] = "";
static int form_acc_type = 1;
static char form_acc_balance_str[30] = "0.00";

static void BuildWalletNamesBuffer() {
    strcpy(wallet_names_buf, "All Wallets\0");
    int buf_pos = strlen("All Wallets") + 1;
    for (int i = 0; i < g_state.account_count; i++) {
        int len = strlen(g_state.accounts[i].name);
        strcpy(wallet_names_buf + buf_pos, g_state.accounts[i].name);
        buf_pos += len + 1;
    }
    wallet_names_buf[buf_pos] = '\0';
}

static void GetCurrentFormattedDateTime(char* out_buf, size_t buf_size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (!t) {
        snprintf(out_buf, buf_size, "2026-08-25 12:00 PM");
        return;
    }
    int hour12 = t->tm_hour % 12;
    if (hour12 == 0) hour12 = 12;
    const char* ampm = (t->tm_hour >= 12) ? "PM" : "AM";
    snprintf(out_buf, buf_size, "%04d-%02d-%02d %02d:%02d %s",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             hour12, t->tm_min, ampm);
}

static void RefreshFilter() {
    ledger_recalculate_running_balances(&g_state);

    g_category_count = core_get_unique_categories(&g_state, g_categories, 30);
    BuildWalletNamesBuffer();

    if (wallet_filter_idx == 0) {
        g_filter.wallet_id_filter = -1;
    } else if (wallet_filter_idx - 1 < g_state.account_count) {
        g_filter.wallet_id_filter = g_state.accounts[wallet_filter_idx - 1].id;
    }

    if (category_filter_idx == 0) {
        g_filter.category_filter[0] = '\0';
    } else if (category_filter_idx - 1 < g_category_count) {
        strncpy(g_filter.category_filter, g_categories[category_filter_idx - 1], MAX_CAT_LEN - 1);
    }

    strncpy(g_filter.search_text, search_buf, MAX_NOTE_LEN - 1);
    g_filtered_count = filter_execute(&g_state, &g_filter, g_filtered_transactions, MAX_TRANSACTIONS);
}

static void ApplyImGuiTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding = 8.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 5.0f;
    style.ItemSpacing = ImVec2(10, 8);
    style.WindowPadding = ImVec2(16, 16);

    colors[ImGuiCol_WindowBg]           = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_ChildBg]            = ImVec4(0.09f, 0.10f, 0.13f, 1.00f);
    colors[ImGuiCol_PopupBg]            = ImVec4(0.14f, 0.15f, 0.19f, 1.00f);
    colors[ImGuiCol_Border]             = ImVec4(0.20f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBg]            = ImVec4(0.07f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.15f, 0.16f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBg]            = ImVec4(0.07f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);

    // Purple Accent Colors (#6c5ce7)
    colors[ImGuiCol_Button]             = ImVec4(0.42f, 0.36f, 0.91f, 1.00f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.35f, 0.29f, 0.88f, 1.00f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.28f, 0.22f, 0.80f, 1.00f);

    colors[ImGuiCol_Header]             = ImVec4(0.18f, 0.20f, 0.24f, 1.00f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.42f, 0.36f, 0.91f, 0.50f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.42f, 0.36f, 0.91f, 1.00f);

    colors[ImGuiCol_Text]               = ImVec4(0.92f, 0.93f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled]       = ImVec4(0.59f, 0.61f, 0.65f, 1.00f);
}

// Power-Scaled Relative Angle Mapping: Strictly Monotonic (1.4% > 0.5% visually) and keeps all small slices visible
static void CalculatePieAngles(const CategoryBreakdown* breakdown, int count, double total, float* out_angles) {
    if (count == 0 || total <= 0.0) return;

    double weight_sum = 0.0;
    double weights[20] = {0.0};

    for (int i = 0; i < count; i++) {
        if (breakdown[i].total_spent > 0.0) {
            double fraction = breakdown[i].total_spent / total;
            weights[i] = pow(fraction, 0.55);
            weight_sum += weights[i];
        }
    }

    if (weight_sum <= 0.0) return;

    for (int i = 0; i < count; i++) {
        if (breakdown[i].total_spent <= 0.0) {
            out_angles[i] = 0.0f;
        } else {
            out_angles[i] = (float)((weights[i] / weight_sum) * (3.14159265f * 2.0f));
        }
    }
}

// Pie Chart Renderer with Power-Scaled Angles and In-Slice Percentage Text
static void RenderSolidPieChart(ImDrawList* draw_list, ImVec2 center, float radius, const CategoryBreakdown* breakdown, const ImU32* colors, int count, double total) {
    if (count == 0 || total <= 0.0) {
        draw_list->AddCircleFilled(center, radius, IM_COL32(45, 50, 60, 255));
        draw_list->AddCircle(center, radius, IM_COL32(20, 22, 28, 255), 64, 3.0f);
        return;
    }

    float sweep_angles[20] = {0};
    CalculatePieAngles(breakdown, count, total, sweep_angles);

    ImVec2 mouse_pos = ImGui::GetMousePos();
    float dx = mouse_pos.x - center.x;
    float dy = mouse_pos.y - center.y;
    float mouse_dist = sqrtf(dx * dx + dy * dy);

    float mouse_angle = atan2f(dy, dx);
    if (mouse_angle < -3.14159265f * 0.5f) {
        mouse_angle += 3.14159265f * 2.0f;
    }

    float current_angle = -3.14159265f * 0.5f;

    for (int i = 0; i < count; i++) {
        if (breakdown[i].total_spent <= 0.0) continue;

        float sweep_angle = sweep_angles[i];
        float next_angle = current_angle + sweep_angle;

        bool is_hovered = (mouse_dist <= radius) && (mouse_angle >= current_angle && mouse_angle < next_angle);

        // 1. Draw Solid Colored Pie Slice
        ImU32 slice_col = colors[i % 9];
        if (is_hovered) {
            ImVec4 vec_col = ImColor(slice_col).Value;
            float r = (vec_col.x * 1.25f > 1.0f) ? 1.0f : vec_col.x * 1.25f;
            float g = (vec_col.y * 1.25f > 1.0f) ? 1.0f : vec_col.y * 1.25f;
            float b = (vec_col.z * 1.25f > 1.0f) ? 1.0f : vec_col.z * 1.25f;
            slice_col = ImColor(r, g, b, 1.0f);
        }

        draw_list->PathClear();
        draw_list->PathLineTo(center);
        draw_list->PathArcTo(center, radius, current_angle, next_angle, 32);
        draw_list->PathFillConvex(slice_col);

        // 2. Draw Dark Border Lines Between Slices
        draw_list->PathClear();
        draw_list->PathLineTo(center);
        draw_list->PathArcTo(center, radius, current_angle, next_angle, 32);
        draw_list->PathStroke(is_hovered ? IM_COL32(255, 255, 255, 255) : IM_COL32(20, 22, 28, 255), 0, is_hovered ? 3.5f : 2.5f);

        // 3. Draw Clean In-Slice Percentage Text
        float mid_angle = current_angle + sweep_angle * 0.5f;
        float text_r = radius * 0.65f;
        ImVec2 text_center = ImVec2(center.x + cosf(mid_angle) * text_r, center.y + sinf(mid_angle) * text_r);

        char label_str[20];
        snprintf(label_str, sizeof(label_str), "%.1f%%", breakdown[i].percentage);
        ImVec2 text_sz = ImGui::CalcTextSize(label_str);
        ImVec2 text_pos = ImVec2(text_center.x - text_sz.x * 0.5f, text_center.y - text_sz.y * 0.5f);

        // Black shadow text background for crisp visibility
        draw_list->AddText(ImVec2(text_pos.x + 1, text_pos.y + 1), IM_COL32(0, 0, 0, 240), label_str);
        draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), label_str);

        // 4. Hover Tooltip showing Category Name + BDT Amount + Percentage
        if (is_hovered) {
            ImGui::BeginTooltip();
            ImGui::TextColored(ImVec4(0.42f, 0.36f, 0.91f, 1.00f), "%s", breakdown[i].category);
            ImGui::Separator();
            ImGui::Text("Amount: BDT %.2f", breakdown[i].total_spent);
            ImGui::Text("Share:  %.1f%%", breakdown[i].percentage);
            ImGui::EndTooltip();
        }

        current_angle = next_angle;
    }

    // Outer Circle Border Outline
    draw_list->AddCircle(center, radius, IM_COL32(20, 22, 28, 255), 64, 3.0f);
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 760, "Financial Manager - Accounts & Ledger", NULL, NULL);
    if (!window) return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ApplyImGuiTheme();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    storage_init_environment();
    if (!storage_load_ledger(&g_state) || g_state.account_count == 0) {
        storage_seed_initial_data(&g_state);
    }

    for (int i = 0; i < g_state.account_count; i++) {
        if (g_state.accounts[i].current_balance < 0.0) {
            g_state.accounts[i].current_balance = 0.0;
        }
    }

    filter_init_default(&g_filter);
    RefreshFilter();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)display_w, (float)display_h));
        ImGui::Begin("MainDockWindow", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        float sidebar_width = (display_w < 850) ? 240.0f : ((display_w > 1400) ? 310.0f : (display_w * 0.24f));
        if (sidebar_width < 230.0f) sidebar_width = 230.0f;

        // LEFT SIDEBAR (STATIONARY)
        ImGui::BeginChild("Sidebar", ImVec2(sidebar_width, 0), true);

        ImGui::TextDisabled("ARCHWAY LEDGER");
        ImGui::Spacing();

        // Total Balance Card
        ImGui::BeginChild("BalanceCard", ImVec2(0, 115), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::TextDisabled("TOTAL BALANCE");

        double total_balance = core_get_total_balance(&g_state);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.72f, 0.58f, 1.0f));
        ImGui::SetWindowFontScale(1.3f);
        ImGui::Text("BDT %.2f", total_balance);
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();

        ImGui::Separator();

        double total_inc = core_get_total_income(&g_state);
        double total_exp = core_get_total_expense(&g_state);

        float half_w = (ImGui::GetContentRegionAvail().x - 10) * 0.5f;
        ImGui::BeginGroup();
        ImGui::TextDisabled("Total Income");
        ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.58f, 1.0f), "+%.2f", total_inc);
        ImGui::EndGroup();

        ImGui::SameLine(half_w + 10);
        ImGui::BeginGroup();
        ImGui::TextDisabled("Total Expenses");
        ImGui::TextColored(ImVec4(1.0f, 0.46f, 0.46f, 1.0f), "-%.2f", total_exp);
        ImGui::EndGroup();

        ImGui::EndChild();

        ImGui::Spacing();

        ImGui::TextDisabled("MY WALLETS / ACCOUNTS");

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.42f, 0.36f, 0.91f, 1.00f));
        if (ImGui::Button("+ Add Wallet", ImVec2(-1, 32))) {
            form_acc_id = -1;
            strcpy(form_acc_name, "Nagad MFS");
            form_acc_type = 1;
            strcpy(form_acc_balance_str, "0.00");
            trigger_open_add_acc = true;
        }
        ImGui::PopStyleColor();

        ImGui::Spacing();

        // Wallets Cards List
        ImGui::BeginChild("WalletsList", ImVec2(0, 0), false);

        for (int i = 0; i < g_state.account_count; i++) {
            Account* acc = &g_state.accounts[i];
            char card_id[32]; snprintf(card_id, sizeof(card_id), "WCard_%d", acc->id);

            ImGui::BeginChild(card_id, ImVec2(0, 80), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            ImGui::Text("%s", acc->name);
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 55);
            ImGui::TextDisabled("%s", get_account_type_str(acc->type));

            ImGui::Text("BDT %.2f", acc->current_balance);

            char edit_acc_btn_id[32]; snprintf(edit_acc_btn_id, sizeof(edit_acc_btn_id), "Edit##acc_%d", acc->id);
            if (ImGui::Button(edit_acc_btn_id, ImVec2(45, 20))) {
                form_acc_id = acc->id;
                strncpy(form_acc_name, acc->name, sizeof(form_acc_name) - 1);
                form_acc_type = (int)acc->type;
                snprintf(form_acc_balance_str, sizeof(form_acc_balance_str), "%.2f", acc->current_balance);
                trigger_open_edit_acc = true;
            }

            char delete_btn_id[32]; snprintf(delete_btn_id, sizeof(delete_btn_id), "Del##acc_%d", acc->id);
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
            if (ImGui::Button(delete_btn_id, ImVec2(45, 20))) {
                core_delete_account(&g_state, acc->id);
                storage_save_ledger(&g_state);
                RefreshFilter();
            }
            ImGui::PopStyleColor();

            ImGui::EndChild();
        }
        ImGui::EndChild();

        ImGui::EndChild(); // Sidebar

        // =========================================================================
        // RIGHT MAIN CONTENT PANEL (WITH NAVIGATION TAB BAR)
        // =========================================================================
        ImGui::SameLine();
        ImGui::BeginChild("MainContent", ImVec2(0, 0), true);

        // TOP NAVIGATION TABS (Ledger vs Statistics)
        if (ImGui::BeginTabBar("MainNavBar", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("  Ledger & Transactions  ")) {
                active_nav_tab = 0;
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("  Statistics & Analytics  ")) {
                active_nav_tab = 1;
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::Spacing();

        if (active_nav_tab == 0) {
            // =====================================================================
            // VIEW 1: LEDGER & TRANSACTIONS VIEW
            // =====================================================================

            // 1. TOP ACTION BUTTONS BAR + INLINE LOAN MANAGER BUTTON (ALWAYS 100% VISIBLE)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.25f, 0.25f, 1.00f));
            if (ImGui::Button("+ Add Expense", ImVec2(120, 32))) {
                form_tx_id = -1;
                form_tx_type = 0; // Expense
                selected_exp_cat_idx = 0;
                strcpy(form_tx_category, student_expense_cats[0]);
                strcpy(form_tx_amount_str, "0.00");
                GetCurrentFormattedDateTime(form_tx_datetime, sizeof(form_tx_datetime));
                strcpy(form_tx_notes, "");
                form_tx_from_idx = 0;
                form_tx_to_idx = -1;
                trigger_open_add_exp = true;
            }
            ImGui::PopStyleColor();

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.65f, 0.45f, 1.00f));
            if (ImGui::Button("+ Add Income", ImVec2(120, 32))) {
                form_tx_id = -1;
                form_tx_type = 1; // Income
                selected_inc_cat_idx = 0;
                strcpy(form_tx_category, student_income_cats[0]);
                strcpy(form_tx_amount_str, "0.00");
                GetCurrentFormattedDateTime(form_tx_datetime, sizeof(form_tx_datetime));
                strcpy(form_tx_notes, "");
                form_tx_from_idx = -1;
                form_tx_to_idx = 0;
                trigger_open_add_inc = true;
            }
            ImGui::PopStyleColor();

            ImGui::SameLine();
            if (ImGui::Button("+ Transfer", ImVec2(95, 32))) {
                form_tx_id = -1;
                form_tx_type = 2; // Transfer
                strcpy(form_tx_category, "Transfer");
                strcpy(form_tx_amount_str, "0.00");
                GetCurrentFormattedDateTime(form_tx_datetime, sizeof(form_tx_datetime));
                strcpy(form_tx_notes, "");
                form_tx_from_idx = 0;
                form_tx_to_idx = (g_state.account_count > 1) ? 1 : 0;
                trigger_open_add_transfer = true;
            }

            // TOP ACTION BAR INTERACTIVE LOAN MANAGER BUTTON (INLINE PLACED)
            double outstanding_loan = core_get_outstanding_loan_balance(&g_state);
            double total_loan_taken = core_get_total_loan_taken(&g_state);

            char loan_btn_label[64];
            if (outstanding_loan > 0.0) {
                snprintf(loan_btn_label, sizeof(loan_btn_label), "[!] LOAN: BDT %.2f UNPAID", outstanding_loan);
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.2f, 0.2f, 1.00f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.3f, 0.3f, 1.00f));
                if (ImGui::Button(loan_btn_label, ImVec2(215, 32))) {
                    trigger_open_loan_manager = true;
                }
                ImGui::PopStyleColor(2);
            } else {
                if (total_loan_taken > 0.0) {
                    snprintf(loan_btn_label, sizeof(loan_btn_label), "LOANS: BDT %.2f (ALL PAID)", total_loan_taken);
                } else {
                    snprintf(loan_btn_label, sizeof(loan_btn_label), "LOANS: BDT 0.00 (NO LOAN)");
                }
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.55f, 0.35f, 1.00f));
                if (ImGui::Button(loan_btn_label, ImVec2(215, 32))) {
                    trigger_open_loan_manager = true;
                }
                ImGui::PopStyleColor();
            }

            ImGui::Separator();

            // 2. DAILY SUMMARY BANNER
            DailySummary summary = ledger_get_daily_summary(&g_state, "");

            float main_avail_w = ImGui::GetContentRegionAvail().x;
            if (main_avail_w > 650) {
                ImGui::TextDisabled("DAILY SUMMARY:"); ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.58f, 1.0f), "Today's Income: +BDT %.2f", summary.daily_income); ImGui::SameLine();
                ImGui::TextDisabled("|"); ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.46f, 0.46f, 1.0f), "Today's Expenses: -BDT %.2f", summary.daily_expense); ImGui::SameLine();
                ImGui::TextDisabled("|"); ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Closing Balance: BDT %.2f", summary.closing_balance);
            } else {
                ImGui::TextDisabled("DAILY SUMMARY:");
                ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.58f, 1.0f), "Income: +BDT %.2f", summary.daily_income); ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.46f, 0.46f, 1.0f), "Expenses: -BDT %.2f", summary.daily_expense); ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Closing: BDT %.2f", summary.closing_balance);
            }

            ImGui::Separator();

            // 3. RESPONSIVE FILTER TOOLBAR WITH LOAN MANAGER ACCESS
            ImGui::TextDisabled("FILTERS:"); ImGui::SameLine();

            float filter_item_w = ((main_avail_w - 300.0f) * 0.28f > 110.0f) ? ((main_avail_w - 300.0f) * 0.28f) : 110.0f;

            ImGui::SetNextItemWidth(filter_item_w);
            if (ImGui::Combo("##WalletFilter", &wallet_filter_idx, wallet_names_buf)) {
                RefreshFilter();
            }

            ImGui::SameLine();

            char cat_names_buf[512] = "All Categories\0";
            int cat_pos = strlen("All Categories") + 1;
            for (int i = 0; i < g_category_count; i++) {
                int len = strlen(g_categories[i]);
                strcpy(cat_names_buf + cat_pos, g_categories[i]);
                cat_pos += len + 1;
            }
            cat_names_buf[cat_pos] = '\0';

            ImGui::SetNextItemWidth(filter_item_w);
            if (ImGui::Combo("##CategoryFilter", &category_filter_idx, cat_names_buf)) {
                RefreshFilter();
            }

            ImGui::SameLine();

            ImGui::SetNextItemWidth(filter_item_w);
            if (ImGui::InputText("##SearchNotes", search_buf, sizeof(search_buf))) {
                RefreshFilter();
            }
            if (search_buf[0] == '\0') {
                ImGui::SameLine(); ImGui::TextDisabled("Search...");
            }

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.42f, 0.36f, 0.91f, 1.00f));
            if (ImGui::Button("Manage Loans", ImVec2(105, 24))) {
                trigger_open_loan_manager = true;
            }
            ImGui::PopStyleColor();

            ImGui::Separator();

            // 4. TRANSACTION HISTORY TABLE V6 (SCROLLABLE & FIXED FIT SO EDIT/DEL BUTTONS & AMOUNT ARE ALWAYS ACCESSIBLE)
            if (ImGui::BeginTable("LedgerTransactionsTableV6", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("Date/Time", ImGuiTableColumnFlags_WidthFixed, 115.0f);
                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 55.0f);
                ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed, 95.0f);
                ImGui::TableSetupColumn("Wallet From", ImGuiTableColumnFlags_WidthFixed, 75.0f);
                ImGui::TableSetupColumn("Wallet To", ImGuiTableColumnFlags_WidthFixed, 75.0f);
                ImGui::TableSetupColumn("Notes", ImGuiTableColumnFlags_WidthFixed, 110.0f);
                ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 85.0f);
                ImGui::TableHeadersRow();

                for (int i = 0; i < g_filtered_count; i++) {
                    const Transaction* tx = g_filtered_transactions[i];
                    Account* from_acc = core_find_account(&g_state, tx->wallet_from_id);
                    Account* to_acc = core_find_account(&g_state, tx->wallet_to_id);

                    bool is_loan_category = (strcmp(tx->category, "Loan / Credit") == 0 ||
                                             strcmp(tx->category, "Loan / Credit Entry") == 0 ||
                                             strstr(tx->notes, "[Loan") != NULL);
                    bool is_paid = core_is_loan_paid(tx);

                    ImGui::TableNextRow();
                    // Col 0: Date/Time
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%s", tx->datetime);

                    // Col 1: Type
                    ImGui::TableSetColumnIndex(1);
                    if (tx->type == TRANSACTION_INCOME) ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.58f, 1.0f), "Income");
                    else if (tx->type == TRANSACTION_EXPENSE) ImGui::TextColored(ImVec4(1.0f, 0.46f, 0.46f, 1.0f), "Expense");
                    else ImGui::TextColored(ImVec4(0.2f, 0.6f, 0.9f, 1.0f), "Transfer");

                    // Col 2: Category
                    ImGui::TableSetColumnIndex(2); ImGui::Text("%s", tx->category);

                    // Col 3: Wallet From
                    ImGui::TableSetColumnIndex(3); ImGui::Text("%s", from_acc ? from_acc->name : "-");

                    // Col 4: Wallet To
                    ImGui::TableSetColumnIndex(4); ImGui::Text("%s", to_acc ? to_acc->name : "-");

                    // Col 5: Notes
                    ImGui::TableSetColumnIndex(5); ImGui::Text("%s", tx->notes);

                    // Col 6: Amount (RED WHEN UNPAID, GREEN WHEN PAID - AMOUNT PRESERVED & NOT SET TO ZERO!)
                    ImGui::TableSetColumnIndex(6);
                    if (is_loan_category) {
                        if (is_paid) {
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(20, 140, 60, 220));
                            ImGui::TextColored(ImVec4(0.9f, 1.0f, 0.9f, 1.0f), "+BDT %.2f", tx->amount);
                        } else {
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(180, 25, 25, 230));
                            ImGui::TextColored(ImVec4(1.0f, 0.95f, 0.95f, 1.0f), "+BDT %.2f", tx->amount);
                        }
                    } else {
                        if (tx->type == TRANSACTION_EXPENSE) ImGui::TextColored(ImVec4(1.0f, 0.46f, 0.46f, 1.0f), "-BDT %.2f", tx->amount);
                        else ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.58f, 1.0f), "+BDT %.2f", tx->amount);
                    }

                    // Col 7: Actions (Edit & Del Buttons)
                    ImGui::TableSetColumnIndex(7);

                    char edit_tx_btn_id[32]; snprintf(edit_tx_btn_id, sizeof(edit_tx_btn_id), "Edit##tx_%d", tx->id);
                    if (ImGui::Button(edit_tx_btn_id, ImVec2(32, 20))) {
                        form_tx_id = tx->id;
                        form_tx_type = (int)tx->type;
                        strncpy(form_tx_category, tx->category, sizeof(form_tx_category) - 1);
                        snprintf(form_tx_amount_str, sizeof(form_tx_amount_str), "%.2f", tx->amount);
                        strncpy(form_tx_datetime, tx->datetime, sizeof(form_tx_datetime) - 1);
                        strncpy(form_tx_notes, tx->notes, sizeof(form_tx_notes) - 1);

                        form_tx_from_idx = 0; form_tx_to_idx = 0;
                        for (int a = 0; a < g_state.account_count; a++) {
                            if (g_state.accounts[a].id == tx->wallet_from_id) form_tx_from_idx = a;
                            if (g_state.accounts[a].id == tx->wallet_to_id) form_tx_to_idx = a;
                        }

                        trigger_open_edit_tx = true;
                    }

                    ImGui::SameLine();
                    char del_tx_id[32]; snprintf(del_tx_id, sizeof(del_tx_id), "Del##tx_%d", tx->id);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
                    if (ImGui::Button(del_tx_id, ImVec2(32, 20))) {
                        core_delete_transaction(&g_state, tx->id);
                        storage_save_ledger(&g_state);
                        RefreshFilter();
                    }
                    ImGui::PopStyleColor();
                }

                ImGui::EndTable();
            }
    } else {
            // =====================================================================
            // VIEW 2: 100% VERTICALLY SCROLLABLE STATISTICS & ANALYTICS PAGE
            // =====================================================================
            ImGui::BeginChild("StatsScrollView", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

            ImGui::TextColored(ImVec4(0.42f, 0.36f, 0.91f, 1.00f), "FINANCIAL INSIGHTS & PIE CHART ANALYTICS");
            ImGui::Separator();
            ImGui::Spacing();

            // 1. RESPONSIVE AUTO-WRAPPING KPI OVERVIEW CARDS GRID
            float avail_w = ImGui::GetContentRegionAvail().x;
            int cards_per_row = (avail_w >= 1050.0f) ? 5 : ((avail_w >= 620.0f) ? 3 : 2);
            float card_w = (avail_w - (cards_per_row - 1) * 10.0f) / (float)cards_per_row;

            // Card 1: Total Net Worth
            ImGui::BeginChild("KpiCard1", ImVec2(card_w, 65), true);
            ImGui::TextDisabled("TOTAL NET WORTH");
            ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.58f, 1.0f), "BDT %.2f", total_balance);
            ImGui::EndChild();

            // Card 2: Total Income
            if (1 % cards_per_row != 0) ImGui::SameLine();
            ImGui::BeginChild("KpiCard2", ImVec2(card_w, 65), true);
            ImGui::TextDisabled("LIFETIME INCOME");
            ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.58f, 1.0f), "+BDT %.2f", total_inc);
            ImGui::EndChild();

            // Card 3: Total Expense
            if (2 % cards_per_row != 0) ImGui::SameLine();
            ImGui::BeginChild("KpiCard3", ImVec2(card_w, 65), true);
            ImGui::TextDisabled("LIFETIME EXPENSES");
            ImGui::TextColored(ImVec4(1.0f, 0.46f, 0.46f, 1.0f), "-BDT %.2f", total_exp);
            ImGui::EndChild();

   // Card 4: Net Cashflow
            if (3 % cards_per_row != 0) ImGui::SameLine();
            double net_cashflow = total_inc - total_exp;
            ImGui::BeginChild("KpiCard4", ImVec2(card_w, 65), true);
            ImGui::TextDisabled("NET CASHFLOW");
            if (net_cashflow >= 0) {
                ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.58f, 1.0f), "+BDT %.2f", net_cashflow);
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.46f, 0.46f, 1.0f), "-BDT %.2f", -net_cashflow);
            }
            ImGui::EndChild();

            // Card 5: Outstanding Loan Highlight
            if (4 % cards_per_row != 0) ImGui::SameLine();
            double stat_loan_balance = core_get_outstanding_loan_balance(&g_state);
            ImGui::BeginChild("KpiCard5", ImVec2(card_w, 65), true);
            ImGui::TextDisabled("LOAN LIABILITIES");
            if (stat_loan_balance > 0.0) {
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "-BDT %.2f", stat_loan_balance);
            } else {
                ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.58f, 1.0f), "BDT 0.00 (PAID)");
            }
            ImGui::EndChild();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

              // 2. 100% RESPONSIVE PIE CHARTS WITH AUTO-FIT LEGENDS
            bool side_by_side = (avail_w >= 1050.0f);
            float chart_panel_w = side_by_side ? ((avail_w - 15.0f) * 0.5f) : (avail_w - 5.0f);
            float panel_h = 360.0f;

            // --- EXPENSE SOLID PIE CHART PANEL ---
            ImGui::BeginChild("ExpensePiePanel", ImVec2(chart_panel_w, panel_h), true);
            ImGui::TextColored(ImVec4(1.0f, 0.46f, 0.46f, 1.0f), "EXPENSE CATEGORY PIE CHART");
            ImGui::Separator();
            ImGui::Spacing();

            CategoryBreakdown exp_breakdown[20];
            int exp_cat_count = core_get_type_category_breakdown(&g_state, TRANSACTION_EXPENSE, exp_breakdown, 20);

            // Pie chart positioning
            float pie_cx = (chart_panel_w > 450.0f) ? 140.0f : (chart_panel_w * 0.5f);
            ImVec2 pie_center = ImVec2(ImGui::GetCursorScreenPos().x + pie_cx, ImGui::GetCursorScreenPos().y + 145);
            RenderSolidPieChart(ImGui::GetWindowDrawList(), pie_center, 115.0f, exp_breakdown, pie_colors_expense, exp_cat_count, total_exp);

            if (chart_panel_w > 450.0f) {
                ImGui::SetCursorPosX(270.0f);
                ImGui::BeginChild("ExpLegend", ImVec2(chart_panel_w - 280.0f, 290), false);
            } else {
                ImGui::SetCursorPosY(285.0f);
                ImGui::BeginChild("ExpLegend", ImVec2(0, 180), false);
            }

            if (exp_cat_count == 0 || total_exp <= 0.0) {
                ImGui::TextDisabled("No expense records logged yet.");
            } else {
                ImGui::TextDisabled("Expense Category Legend");
                ImGui::Spacing();
                for (int e = 0; e < exp_cat_count; e++) {
                    ImU32 col = pie_colors_expense[e % 9];
                    ImVec4 vec_col = ImColor(col).Value;
                    ImGui::ColorButton("##exp_col", vec_col, ImGuiColorEditFlags_NoTooltip, ImVec2(12, 12));
                    ImGui::SameLine();
                    ImGui::Text("%s:", exp_breakdown[e].category);
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(1.0f, 0.46f, 0.46f, 1.0f), "%.1f%% (BDT %.0f)", exp_breakdown[e].percentage, exp_breakdown[e].total_spent);
                }
            }
            ImGui::EndChild();
            ImGui::EndChild(); // ExpensePiePanel

             if (side_by_side) ImGui::SameLine();

            // --- INCOME SOLID PIE CHART PANEL ---
            ImGui::BeginChild("IncomePiePanel", ImVec2(chart_panel_w, panel_h), true);
            ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.58f, 1.0f), "INCOME CATEGORY PIE CHART");
            ImGui::Separator();
            ImGui::Spacing();

            CategoryBreakdown inc_breakdown[20];
            int inc_cat_count = core_get_type_category_breakdown(&g_state, TRANSACTION_INCOME, inc_breakdown, 20);

            // Pie chart positioning
            ImVec2 inc_pie_center = ImVec2(ImGui::GetCursorScreenPos().x + pie_cx, ImGui::GetCursorScreenPos().y + 145);
            RenderSolidPieChart(ImGui::GetWindowDrawList(), inc_pie_center, 115.0f, inc_breakdown, pie_colors_income, inc_cat_count, total_inc);

            if (chart_panel_w > 450.0f) {
                ImGui::SetCursorPosX(270.0f);
                ImGui::BeginChild("IncLegend", ImVec2(chart_panel_w - 280.0f, 290), false);
            } else {
                ImGui::SetCursorPosY(285.0f);
                ImGui::BeginChild("IncLegend", ImVec2(0, 180), false);
            }

            if (inc_cat_count == 0 || total_inc <= 0.0) {
                ImGui::TextDisabled("No income records logged yet.");
            } else {
                ImGui::TextDisabled("Income Category Legend");
                ImGui::Spacing();
                for (int c = 0; c < inc_cat_count; c++) {
                    ImU32 col = pie_colors_income[c % 6];
                    ImVec4 vec_col = ImColor(col).Value;
                    ImGui::ColorButton("##inc_col", vec_col, ImGuiColorEditFlags_NoTooltip, ImVec2(12, 12));
                    ImGui::SameLine();
                    ImGui::Text("%s:", inc_breakdown[c].category);
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.58f, 1.0f), "%.1f%% (BDT %.0f)", inc_breakdown[c].percentage, inc_breakdown[c].total_spent);
                }
            }
            ImGui::EndChild();
            ImGui::EndChild(); // IncomePiePanel

              ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // 3. RESPONSIVE WALLET ASSET ALLOCATION DISTRIBUTION REPORT
            float alloc_panel_h = (float)(60 + g_state.account_count * 38);
            if (alloc_panel_h < 180.0f) alloc_panel_h = 180.0f;

            ImGui::BeginChild("WalletAllocationPanel", ImVec2(0, alloc_panel_h), true);
            ImGui::TextColored(ImVec4(0.42f, 0.36f, 0.91f, 1.00f), "WALLET ASSET ALLOCATION DISTRIBUTION");
            ImGui::Separator();
            ImGui::Spacing();

            if (total_balance <= 0.0) {
                ImGui::TextDisabled("Total balance is 0.00 BDT. Add income or funds to view distribution.");
            } else {
                for (int a = 0; a < g_state.account_count; a++) {
                    Account* acc = &g_state.accounts[a];
                    float frac = (float)(acc->current_balance / total_balance);
                    if (frac < 0.0f) frac = 0.0f;

                    float pbar_w = ImGui::GetContentRegionAvail().x - 280.0f;
                    if (pbar_w < 120.0f) pbar_w = 120.0f;

                    ImGui::Text("%-16s (BDT %.2f)", acc->name, acc->current_balance);
                    ImGui::SameLine(220);
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.42f, 0.36f, 0.91f, 1.00f));
                    ImGui::ProgressBar(frac, ImVec2(pbar_w, 20), "");
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                    ImGui::Text("%.1f%%", frac * 100.0f);
                    ImGui::Spacing();
                }
            }
            ImGui::EndChild();

            ImGui::EndChild(); // StatsScrollView
        }

        ImGui::EndChild(); // MainContent

        // =========================================================================
        // GLOBAL ONE-SHOT POPUP DISPATCHERS (TRIGGERED AT ROOT SCOPE FOR 100% RELIABILITY)
        // =========================================================================
        if (trigger_open_add_acc) { ImGui::OpenPopup("Add New Wallet Account"); trigger_open_add_acc = false; }
        if (trigger_open_edit_acc) { ImGui::OpenPopup("Edit Wallet Account"); trigger_open_edit_acc = false; }
        if (trigger_open_add_exp) { ImGui::OpenPopup("Add Expense Record"); trigger_open_add_exp = false; }
        if (trigger_open_add_inc) { ImGui::OpenPopup("Add Income Record"); trigger_open_add_inc = false; }
        if (trigger_open_add_transfer) { ImGui::OpenPopup("Transfer Funds Between Wallets"); trigger_open_add_transfer = false; }
        if (trigger_open_edit_tx) { ImGui::OpenPopup("Edit Record Details"); trigger_open_edit_tx = false; }
        if (trigger_open_loan_manager) { ImGui::OpenPopup("Loan & Credit Ledger Manager"); trigger_open_loan_manager = false; }
        if (trigger_open_pay_loan) { ImGui::OpenPopup("Pay Now - Wallet Selection"); trigger_open_pay_loan = false; }

         // =========================================================================
        // MODAL 1: ADD / EDIT WALLET ACCOUNT
        // =========================================================================
        ImGui::SetNextWindowSize(ImVec2(520, 340), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("Add New Wallet Account", NULL, ImGuiWindowFlags_None) ||
            ImGui::BeginPopupModal("Edit Wallet Account", NULL, ImGuiWindowFlags_None)) {
            
            if (ImGui::IsMouseClicked(0) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) {
                ImGui::CloseCurrentPopup();
            }
            
            bool is_edit = (form_acc_id > 0);
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.42f, 0.36f, 0.91f, 1.00f), is_edit ? "MODIFY WALLET ACCOUNT DETAILS" : "CREATE NEW WALLET ACCOUNT");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Account Name:");
            ImGui::SetNextItemWidth(480);
            ImGui::InputText("##FormAccName", form_acc_name, sizeof(form_acc_name));

            ImGui::Spacing();
            ImGui::Text("Account Type:");
            ImGui::SetNextItemWidth(480);
            ImGui::Combo("##FormAccType", &form_acc_type, "Cash\0MFS (bKash/Nagad/Rocket)\0Card (Debit/Credit)\0Savings Account\0Bank Account\0");

            ImGui::Spacing();
            ImGui::Text("Initial Balance (BDT - Numeric Only):");
            ImGui::SetNextItemWidth(480);
            ImGui::InputText("##FormAccBal", form_acc_balance_str, sizeof(form_acc_balance_str), ImGuiInputTextFlags_CharsDecimal);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Save Wallet", ImVec2(160, 38))) {
                double init_bal = atof(form_acc_balance_str);
                int acc_id_result = -1;
                if (is_edit) {
                    core_update_account(&g_state, form_acc_id, form_acc_name, (AccountType)form_acc_type, init_bal);
                    acc_id_result = form_acc_id;
                } else {
                    acc_id_result = core_add_account(&g_state, form_acc_name, (AccountType)form_acc_type, init_bal);
                }
                storage_save_ledger(&g_state);
                RefreshFilter();

                // Check for formal loan repayment prompt on deposit
                double cur_loan = core_get_outstanding_loan_balance(&g_state);
                if (cur_loan > 0.0 && init_bal > 0.0) {
                    repay_prompt_deposit_amount = init_bal;
                    repay_prompt_wallet_id = acc_id_result;
                    strncpy(repay_prompt_wallet_name, form_acc_name, sizeof(repay_prompt_wallet_name) - 1);
                    show_loan_repay_modal = true;
                }

                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(140, 38))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

         // =========================================================================
        // MODAL 2: TRANSACTION FORM WITH NUMERIC-ONLY AMOUNT INPUT
        // =========================================================================
        ImGui::SetNextWindowSize(ImVec2(560, 520), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("Add Expense Record", NULL, ImGuiWindowFlags_None) ||
            ImGui::BeginPopupModal("Add Income Record", NULL, ImGuiWindowFlags_None) ||
            ImGui::BeginPopupModal("Transfer Funds Between Wallets", NULL, ImGuiWindowFlags_None) ||
            ImGui::BeginPopupModal("Edit Record Details", NULL, ImGuiWindowFlags_None)) {
            
            if (ImGui::IsMouseClicked(0) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) {
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::Spacing();
            
            const char* header_str = (form_tx_type == 0) ? "RECORD EXPENSE (-BDT)" : 
                                    ((form_tx_type == 1) ? "RECORD INCOME (+BDT)" : "TRANSFER BETWEEN ACTIVE WALLETS");
            ImVec4 header_color = (form_tx_type == 0) ? ImVec4(1.0f, 0.46f, 0.46f, 1.0f) : 
                                 ((form_tx_type == 1) ? ImVec4(0.0f, 0.72f, 0.58f, 1.0f) : ImVec4(0.42f, 0.36f, 0.91f, 1.00f));
            
            ImGui::TextColored(header_color, "%s", header_str);
            ImGui::Separator();
            ImGui::Spacing();

            // 1. DYNAMIC WALLET SELECTION
            if (form_tx_type == 0) {
                ImGui::Text("Pay From Wallet:");
                ImGui::SetNextItemWidth(520);
                ImGui::Combo("##FormTxFrom", &form_tx_from_idx, wallet_names_buf + strlen("All Wallets") + 1);
            } else if (form_tx_type == 1) {
                ImGui::Text("Deposit To Wallet:");
                ImGui::SetNextItemWidth(520);
                ImGui::Combo("##FormTxTo", &form_tx_to_idx, wallet_names_buf + strlen("All Wallets") + 1);
            } else {
                float half_field_w = 250.0f;
                ImGui::BeginGroup();
                ImGui::Text("Transfer From (Active Wallet):");
                ImGui::SetNextItemWidth(half_field_w);
                ImGui::Combo("##FormTxFrom", &form_tx_from_idx, wallet_names_buf + strlen("All Wallets") + 1);
                ImGui::EndGroup();

                ImGui::SameLine(280);
                ImGui::BeginGroup();
                ImGui::Text("Transfer To (Active Wallet):");
                ImGui::SetNextItemWidth(half_field_w);
                ImGui::Combo("##FormTxTo", &form_tx_to_idx, wallet_names_buf + strlen("All Wallets") + 1);
                ImGui::EndGroup();
            }

           ImGui::Spacing();

            // 2. SELECTABLE CATEGORY & NUMERIC-ONLY AMOUNT FIELD
            float half_field_w = 250.0f;
            ImGui::BeginGroup();
            ImGui::Text("Category:");
            ImGui::SetNextItemWidth(half_field_w);

            if (form_tx_type == 0) {
                char cat_combo_str[512] = "";
                int cat_pos = 0;
                int num_exp_cats = sizeof(student_expense_cats) / sizeof(student_expense_cats[0]);
                for (int c = 0; c < num_exp_cats; c++) {
                    int len = strlen(student_expense_cats[c]);
                    strcpy(cat_combo_str + cat_pos, student_expense_cats[c]);
                    cat_pos += len + 1;
                }
                cat_combo_str[cat_pos] = '\0';

                if (ImGui::Combo("##FormTxCatCombo", &selected_exp_cat_idx, cat_combo_str)) {
                    strncpy(form_tx_category, student_expense_cats[selected_exp_cat_idx], sizeof(form_tx_category) - 1);
                }
            } else if (form_tx_type == 1) {
                char cat_combo_str[512] = "";
                int cat_pos = 0;
                int num_inc_cats = sizeof(student_income_cats) / sizeof(student_income_cats[0]);
                for (int c = 0; c < num_inc_cats; c++) {
                    int len = strlen(student_income_cats[c]);
                    strcpy(cat_combo_str + cat_pos, student_income_cats[c]);
                    cat_pos += len + 1;
                }
                cat_combo_str[cat_pos] = '\0';

                if (ImGui::Combo("##FormTxCatCombo", &selected_inc_cat_idx, cat_combo_str)) {
                    strncpy(form_tx_category, student_income_cats[selected_inc_cat_idx], sizeof(form_tx_category) - 1);
                }
            } else {
                ImGui::InputText("##FormTxCatText", form_tx_category, sizeof(form_tx_category));
            }
            ImGui::EndGroup();

            ImGui::SameLine(280);
            ImGui::BeginGroup();
            ImGui::Text("Amount (BDT - Numeric Only):");
            ImGui::SetNextItemWidth(half_field_w);
            ImGui::InputText("##FormTxAmount", form_tx_amount_str, sizeof(form_tx_amount_str), ImGuiInputTextFlags_CharsDecimal);
            ImGui::EndGroup();

            ImGui::Spacing();

            // 3. REAL-TIME DATE & TIME
            ImGui::Text("Date & Time:");
            ImGui::SetNextItemWidth(520);
            ImGui::InputText("##FormTxDateTime", form_tx_datetime, sizeof(form_tx_datetime));

            ImGui::Spacing();

            // 4. CLEAN NOTES / DESCRIPTION MULTILINE FIELD
            ImGui::Text("Notes / Description:");
            ImGui::InputTextMultiline("##FormTxNotes", form_tx_notes, sizeof(form_tx_notes), ImVec2(520, 65));

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            
 
    return 0;
}
