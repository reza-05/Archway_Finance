#include "ui_dashboard.h"
#include "imgui.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <cstring>

// Common categories list
static const char* CATEGORIES[] = {
    "Salary",
    "Gifts",
    "Interest/Dividends",
    "Food & Drinks",
    "Groceries",
    "Utilities",
    "Rent & Housing",
    "Health & Beauty",
    "Stationery & Tools",
    "Transportation",
    "Entertainment",
    "Transfer",
    "Other"
};
static const int CATEGORIES_COUNT = sizeof(CATEGORIES) / sizeof(CATEGORIES[0]);

// Helper to get current date as YYYY-MM-DD
static std::string GetCurrentDateString() {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", now);
    return std::string(buf);
}

// Helper to get current time as HH:MM AM/PM
static std::string GetCurrentTimeString() {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%I:%M %p", now);
    return std::string(buf);
}

// CSV Parsing Helper
static std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> result;
    std::string current;
    bool in_quotes = false;
    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (c == ',' && !in_quotes) {
            result.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    result.push_back(current);
    return result;
}

// CSV Sanitation Helper
static std::string sanitize_csv(std::string str) {
    std::replace(str.begin(), str.end(), ',', ';');
    std::replace(str.begin(), str.end(), '\n', ' ');
    std::replace(str.begin(), str.end(), '\r', ' ');
    str.erase(std::remove(str.begin(), str.end(), '"'), str.end());
    return str;
}

// UI Styling Config
static void ApplyPremiumStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.TabRounding = 6.0f;
    
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    
    style.WindowPadding = ImVec2(15.0f, 15.0f);
    style.FramePadding = ImVec2(10.0f, 8.0f);
    style.ItemSpacing = ImVec2(12.0f, 10.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
    
    ImVec4* colors = style.Colors;
    
    colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.55f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.09f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.12f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.12f, 0.14f, 0.17f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.16f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.22f, 0.25f, 0.31f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.28f, 0.31f, 0.38f, 1.00f);
    
    colors[ImGuiCol_TitleBg]                = ImVec4(0.09f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.12f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.09f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.12f, 0.14f, 0.17f, 1.00f);
    
    colors[ImGuiCol_Button]                 = ImVec4(0.41f, 0.34f, 0.74f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.48f, 0.40f, 0.85f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.35f, 0.29f, 0.63f, 1.00f);
    
    colors[ImGuiCol_Tab]                    = ImVec4(0.16f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.48f, 0.40f, 0.85f, 0.80f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.41f, 0.34f, 0.74f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.12f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    
    colors[ImGuiCol_Header]                 = ImVec4(0.22f, 0.25f, 0.31f, 1.00f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.28f, 0.31f, 0.38f, 1.00f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.35f, 0.38f, 0.45f, 1.00f);
    
    colors[ImGuiCol_Separator]              = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.41f, 0.34f, 0.74f, 1.00f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.41f, 0.34f, 0.74f, 1.00f);
    
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.12f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.24f, 0.27f, 0.32f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]    = ImVec4(0.30f, 0.34f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]     = ImVec4(0.41f, 0.34f, 0.74f, 1.00f);
}

// AppState Implementations
AppState::AppState() {
    selected_account_id = -1;
    search_query[0] = '\0';
    filter_category[0] = '\0';

    show_add_account_modal = false;
    show_edit_account_modal = false;
    editing_account_id = -1;

    show_add_transaction_modal = false;
    show_edit_transaction_modal = false;
    editing_transaction_id = -1;

    input_acc_name[0] = '\0';
    input_acc_type[0] = '\0';
    input_acc_initial_balance = 0.0;

    input_tx_date[0] = '\0';
    input_tx_time[0] = '\0';
    input_tx_type[0] = '\0';
    input_tx_category[0] = '\0';
    input_tx_acc_from_id = -1;
    input_tx_acc_to_id = -1;
    input_tx_amount = 0.0;
    input_tx_notes[0] = '\0';

    if (!LoadData()) {
        // Initialize with default mock data if load fails or files are empty
        accounts = {
            {1, "Cash", "Cash", 1515.00, 1515.00},
            {2, "AB Student", "Savings account", 22320.40, 22320.40},
            {3, "bKash", "Checking account", 279.70, 279.70}
        };

        transactions = {
            {1, "2026-07-10", "07:01 PM", "Expense", "Stationery & Tools", 1, -1, 100.00, "Pencil bag"},
            {2, "2026-07-10", "07:18 PM", "Expense", "Groceries", 1, -1, 30.00, "3x Surf Excel Minipack"},
            {3, "2026-07-10", "07:45 PM", "Expense", "Health & Beauty", 1, -1, 20.00, "2x Pocket Tissue"},
            {4, "2026-07-10", "11:00 PM", "Income", "Gifts", -1, 1, 10.00, "Birthday Token"},
            {5, "2026-07-12", "08:12 PM", "Expense", "Food & Drinks", 1, -1, 50.00, "Coin chocolates x10"}
        };
        RecalculateBalances();
        SaveData();
    }
}

void AppState::RecalculateBalances() {
    for (auto& acc : accounts) {
        acc.current_balance = acc.initial_balance;
    }
    
    total_balance = 0.0;
    total_income = 0.0;
    total_expense = 0.0;
    
    for (const auto& tx : transactions) {
        if (tx.type == "Income") {
            Account* acc = GetAccountById(tx.account_to_id);
            if (acc) {
                acc->current_balance += tx.amount;
            }
            total_income += tx.amount;
        } else if (tx.type == "Expense") {
            Account* acc = GetAccountById(tx.account_from_id);
            if (acc) {
                acc->current_balance -= tx.amount;
            }
            total_expense += tx.amount;
        } else if (tx.type == "Transfer") {
            Account* from_acc = GetAccountById(tx.account_from_id);
            Account* to_acc = GetAccountById(tx.account_to_id);
            if (from_acc) {
                from_acc->current_balance -= tx.amount;
            }
            if (to_acc) {
                to_acc->current_balance += tx.amount;
            }
        }
    }
    
    for (const auto& acc : accounts) {
        total_balance += acc.current_balance;
    }
}

void AppState::AddAccount(const std::string& name, const std::string& type, double initial_balance) {
    Account acc;
    acc.id = GetNextAccountId();
    acc.name = name;
    acc.type = type;
    acc.initial_balance = initial_balance;
    acc.current_balance = initial_balance;
    accounts.push_back(acc);
    RecalculateBalances();
    SaveData();
}

void AppState::EditAccount(int id, const std::string& name, const std::string& type, double initial_balance) {
    Account* acc = GetAccountById(id);
    if (acc) {
        acc->name = name;
        acc->type = type;
        acc->initial_balance = initial_balance;
        RecalculateBalances();
        SaveData();
    }
}

void AppState::DeleteAccount(int id) {
    accounts.erase(std::remove_if(accounts.begin(), accounts.end(), [id](const Account& a) {
        return a.id == id;
    }), accounts.end());
    
    // Delete associated transactions
    transactions.erase(std::remove_if(transactions.begin(), transactions.end(), [id](const Transaction& tx) {
        return tx.account_from_id == id || tx.account_to_id == id;
    }), transactions.end());
    
    RecalculateBalances();
    SaveData();
}

Account* AppState::GetAccountById(int id) {
    for (auto& acc : accounts) {
        if (acc.id == id) return &acc;
    }
    return nullptr;
}

void AppState::AddTransaction(const std::string& date, const std::string& time, const std::string& type, 
                             const std::string& category, int account_from_id, int account_to_id, 
                             double amount, const std::string& notes) {
    Transaction tx;
    tx.id = GetNextTransactionId();
    tx.date = date;
    tx.time = time;
    tx.type = type;
    tx.category = category;
    tx.account_from_id = account_from_id;
    tx.account_to_id = account_to_id;
    tx.amount = amount;
    tx.notes = notes;
    transactions.push_back(tx);
    RecalculateBalances();
    SaveData();
}

void AppState::EditTransaction(int id, const std::string& date, const std::string& time, const std::string& type, 
                              const std::string& category, int account_from_id, int account_to_id, 
                              double amount, const std::string& notes) {
    Transaction* tx = GetTransactionById(id);
    if (tx) {
        tx->date = date;
        tx->time = time;
        tx->type = type;
        tx->category = category;
        tx->account_from_id = account_from_id;
        tx->account_to_id = account_to_id;
        tx->amount = amount;
        tx->notes = notes;
        RecalculateBalances();
        SaveData();
    }
}

void AppState::DeleteTransaction(int id) {
    transactions.erase(std::remove_if(transactions.begin(), transactions.end(), [id](const Transaction& t) {
        return t.id == id;
    }), transactions.end());
    RecalculateBalances();
    SaveData();
}

Transaction* AppState::GetTransactionById(int id) {
    for (auto& tx : transactions) {
        if (tx.id == id) return &tx;
    }
    return nullptr;
}

int AppState::GetNextAccountId() {
    int max_id = 0;
    for (const auto& acc : accounts) {
        if (acc.id > max_id) max_id = acc.id;
    }
    return max_id + 1;
}

int AppState::GetNextTransactionId() {
    int max_id = 0;
    for (const auto& tx : transactions) {
        if (tx.id > max_id) max_id = tx.id;
    }
    return max_id + 1;
}

bool AppState::LoadData() {
    std::ifstream acc_file("accounts.csv");
    if (!acc_file.is_open()) return false;

    accounts.clear();
    std::string line;
    // skip header
    std::getline(acc_file, line);
    while (std::getline(acc_file, line)) {
        if (line.empty()) continue;
        auto tokens = split_csv_line(line);
        if (tokens.size() < 4) continue;
        
        Account acc;
        acc.id = std::stoi(tokens[0]);
        acc.name = tokens[1];
        acc.type = tokens[2];
        acc.initial_balance = std::stod(tokens[3]);
        acc.current_balance = acc.initial_balance;
        accounts.push_back(acc);
    }
    acc_file.close();

    std::ifstream tx_file("transactions.csv");
    if (!tx_file.is_open()) {
        RecalculateBalances();
        return true;
    }

    transactions.clear();
    // skip header
    std::getline(tx_file, line);
    while (std::getline(tx_file, line)) {
        if (line.empty()) continue;
        auto tokens = split_csv_line(line);
        if (tokens.size() < 9) continue;
        
        Transaction tx;
        tx.id = std::stoi(tokens[0]);
        tx.date = tokens[1];
        tx.time = tokens[2];
        tx.type = tokens[3];
        tx.category = tokens[4];
        tx.account_from_id = std::stoi(tokens[5]);
        tx.account_to_id = std::stoi(tokens[6]);
        tx.amount = std::stod(tokens[7]);
        tx.notes = tokens[8];
        transactions.push_back(tx);
    }
    tx_file.close();

    RecalculateBalances();
    return true;
}

bool AppState::SaveData() {
    std::ofstream acc_file("accounts.csv");
    if (!acc_file.is_open()) return false;
    
    acc_file << "id,name,type,initial_balance\n";
    for (const auto& acc : accounts) {
        acc_file << acc.id << "," 
                 << sanitize_csv(acc.name) << "," 
                 << sanitize_csv(acc.type) << "," 
                 << std::fixed << std::setprecision(2) << acc.initial_balance << "\n";
    }
    acc_file.close();

    std::ofstream tx_file("transactions.csv");
    if (!tx_file.is_open()) return false;

    tx_file << "id,date,time,type,category,account_from_id,account_to_id,amount,notes\n";
    for (const auto& tx : transactions) {
        tx_file << tx.id << ","
                << sanitize_csv(tx.date) << ","
                << sanitize_csv(tx.time) << ","
                << sanitize_csv(tx.type) << ","
                << sanitize_csv(tx.category) << ","
                << tx.account_from_id << ","
                << tx.account_to_id << ","
                << std::fixed << std::setprecision(2) << tx.amount << ","
                << sanitize_csv(tx.notes) << "\n";
    }
    tx_file.close();

    return true;
}

// Forward Declarations
static void RenderTransactionsTab(AppState& state);

void RenderDashboardWindow(AppState& state) {
    static bool style_applied = false;
    if (!style_applied) {
        ApplyPremiumStyle();
        style_applied = true;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);

    ImGui::Begin("Archway Finance Manager Dashboard", nullptr, 
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | 
                 ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save Data", "Ctrl+S")) {
                state.SaveData();
            }
            if (ImGui::MenuItem("Reload Data")) {
                state.LoadData();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                // Exit request is typically handled by closing the GLFW window
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    float sidebar_width = 300.0f;
    float content_width = io.DisplaySize.x - sidebar_width - ImGui::GetStyle().WindowPadding.x * 2.0f - ImGui::GetStyle().ItemSpacing.x;
    
    // --- SIDEBAR (Accounts & Summary) ---
    ImGui::BeginChild("Sidebar", ImVec2(sidebar_width, 0), true);
    
    ImGui::Text("ARCHWAY FINANCE LEDGER");
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::TextDisabled("TOTAL BALANCE");
    ImGui::TextColored(ImVec4(0.41f, 0.85f, 0.74f, 1.00f), "BDT %.2f", state.total_balance);
    ImGui::Spacing();
    
    ImGui::BeginChild("QuickStats", ImVec2(0, 70), true);
    ImGui::Columns(2, nullptr, false);
    ImGui::TextDisabled("Total Income");
    ImGui::TextColored(ImVec4(0.12f, 0.73f, 0.38f, 1.00f), "+BDT %.2f", state.total_income);
    ImGui::NextColumn();
    ImGui::TextDisabled("Total Expenses");
    ImGui::TextColored(ImVec4(0.89f, 0.22f, 0.22f, 1.00f), "-BDT %.2f", state.total_expense);
    ImGui::Columns(1);
    ImGui::EndChild();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::TextDisabled("MY WALLETS / ACCOUNTS");
    ImGui::Spacing();
    
    // Account list container
    ImGui::BeginChild("AccountList", ImVec2(0, -60), false);
    for (const auto& acc : state.accounts) {
        ImGui::PushID(acc.id);
        
        ImGui::BeginChild("AccountCard", ImVec2(0, 105), true);
        ImGui::Text("%s", acc.name.c_str());
        ImGui::SameLine(ImGui::GetWindowWidth() - 75);
        ImGui::TextDisabled("%s", acc.type.c_str());
        
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.95f, 0.96f, 0.98f, 1.00f), "BDT %.2f", acc.current_balance);
        ImGui::Spacing();
        
        if (ImGui::Button("Edit", ImVec2(50, 22))) {
            state.editing_account_id = acc.id;
            strncpy(state.input_acc_name, acc.name.c_str(), sizeof(state.input_acc_name));
            strncpy(state.input_acc_type, acc.type.c_str(), sizeof(state.input_acc_type));
            state.input_acc_initial_balance = acc.initial_balance;
            state.show_edit_account_modal = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete", ImVec2(60, 22))) {
            state.DeleteAccount(acc.id);
        }
        
        ImGui::EndChild();
        ImGui::PopID();
        ImGui::Spacing();
    }
    ImGui::EndChild();
    
    if (ImGui::Button("Add Account (+)", ImVec2(-FLT_MIN, 35))) {
        state.input_acc_name[0] = '\0';
        state.input_acc_type[0] = '\0';
        state.input_acc_initial_balance = 0.0;
        state.show_add_account_modal = true;
    }
    
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // --- MAIN RIGHT CONTENT PANEL ---
    ImGui::BeginChild("RightContent", ImVec2(content_width, 0), false);
    
    RenderTransactionsTab(state);
    
    ImGui::EndChild();

    // ========================================================
    // MODAL DIALOGS
    // ========================================================

    // 1. Add Account Modal
    if (state.show_add_account_modal) {
        ImGui::OpenPopup("Add Account Modal");
        state.show_add_account_modal = false;
    }
    if (ImGui::BeginPopupModal("Add Account Modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Create New Account");
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::InputText("Account Name", state.input_acc_name, IM_ARRAYSIZE(state.input_acc_name));
        
        const char* types[] = { "Cash", "Savings account", "Checking account", "Credit Card", "Investment" };
        static int selected_type_idx = 0;
        if (ImGui::Combo("Account Type", &selected_type_idx, types, IM_ARRAYSIZE(types))) {
            strncpy(state.input_acc_type, types[selected_type_idx], sizeof(state.input_acc_type));
        }
        if (state.input_acc_type[0] == '\0') {
            strncpy(state.input_acc_type, types[selected_type_idx], sizeof(state.input_acc_type));
        }
        
        ImGui::InputDouble("Initial Balance (BDT)", &state.input_acc_initial_balance, 100.0, 1000.0, "%.2f");
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button("Save", ImVec2(120, 0))) {
            if (strlen(state.input_acc_name) > 0) {
                state.AddAccount(state.input_acc_name, state.input_acc_type, state.input_acc_initial_balance);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }

    // 2. Edit Account Modal
    if (state.show_edit_account_modal) {
        ImGui::OpenPopup("Edit Account Modal");
        state.show_edit_account_modal = false;
    }
    if (ImGui::BeginPopupModal("Edit Account Modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Edit Account Details");
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::InputText("Account Name", state.input_acc_name, IM_ARRAYSIZE(state.input_acc_name));
        
        const char* types[] = { "Cash", "Savings account", "Checking account", "Credit Card", "Investment" };
        int type_idx = -1;
        for (int i = 0; i < 5; ++i) {
            if (strcmp(state.input_acc_type, types[i]) == 0) {
                type_idx = i;
                break;
            }
        }
        if (type_idx == -1) type_idx = 0;
        if (ImGui::Combo("Account Type", &type_idx, types, IM_ARRAYSIZE(types))) {
            strncpy(state.input_acc_type, types[type_idx], sizeof(state.input_acc_type));
        }
        
        ImGui::InputDouble("Initial Balance (BDT)", &state.input_acc_initial_balance, 100.0, 1000.0, "%.2f");
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button("Update", ImVec2(120, 0))) {
            if (strlen(state.input_acc_name) > 0) {
                state.EditAccount(state.editing_account_id, state.input_acc_name, state.input_acc_type, state.input_acc_initial_balance);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }

    // 3. Add Transaction Modal
    if (state.show_add_transaction_modal) {
        ImGui::OpenPopup("Add Transaction Modal");
        state.show_add_transaction_modal = false;
    }
    if (ImGui::BeginPopupModal("Add Transaction Modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Record Activity");
        ImGui::Separator();
        ImGui::Spacing();
        
        int type_idx = 0;
        if (strcmp(state.input_tx_type, "Income") == 0) type_idx = 0;
        else if (strcmp(state.input_tx_type, "Expense") == 0) type_idx = 1;
        else if (strcmp(state.input_tx_type, "Transfer") == 0) type_idx = 2;
        
        bool type_changed = false;
        if (ImGui::RadioButton("Income", type_idx == 0)) { state.input_tx_type[0] = '\0'; strcpy(state.input_tx_type, "Income"); type_changed = true; }
        ImGui::SameLine();
        if (ImGui::RadioButton("Expense", type_idx == 1)) { state.input_tx_type[0] = '\0'; strcpy(state.input_tx_type, "Expense"); type_changed = true; }
        ImGui::SameLine();
        if (ImGui::RadioButton("Transfer", type_idx == 2)) { state.input_tx_type[0] = '\0'; strcpy(state.input_tx_type, "Transfer"); type_changed = true; }
        
        if (type_changed) {
            if (strcmp(state.input_tx_type, "Transfer") == 0) {
                strcpy(state.input_tx_category, "Transfer");
            } else {
                strcpy(state.input_tx_category, "Food & Drinks");
            }
        }
        
        ImGui::Spacing();
        ImGui::InputText("Date (YYYY-MM-DD)", state.input_tx_date, IM_ARRAYSIZE(state.input_tx_date));
        ImGui::InputText("Time (e.g. 08:12 PM)", state.input_tx_time, IM_ARRAYSIZE(state.input_tx_time));
        
        if (strcmp(state.input_tx_type, "Transfer") != 0) {
            int cat_idx = -1;
            for (int i = 0; i < CATEGORIES_COUNT; ++i) {
                if (strcmp(state.input_tx_category, CATEGORIES[i]) == 0) {
                    cat_idx = i;
                    break;
                }
            }
            if (cat_idx == -1) cat_idx = CATEGORIES_COUNT - 1;
            
            if (ImGui::Combo("Category", &cat_idx, CATEGORIES, CATEGORIES_COUNT)) {
                strncpy(state.input_tx_category, CATEGORIES[cat_idx], sizeof(state.input_tx_category));
            }
            
            if (strcmp(state.input_tx_category, "Other") == 0) {
                static char custom_cat[128] = "";
                if (ImGui::InputText("Custom Category", custom_cat, IM_ARRAYSIZE(custom_cat))) {
                    strncpy(state.input_tx_category, custom_cat, sizeof(state.input_tx_category));
                }
            }
        }
        
        if (strcmp(state.input_tx_type, "Income") == 0) {
            state.input_tx_acc_from_id = -1;
            Account* to_acc = state.GetAccountById(state.input_tx_acc_to_id);
            if (state.input_tx_acc_to_id == -1 && !state.accounts.empty()) {
                state.input_tx_acc_to_id = state.accounts[0].id;
                to_acc = &state.accounts[0];
            }
            if (ImGui::BeginCombo("Deposit To Wallet", to_acc ? to_acc->name.c_str() : "")) {
                for (const auto& acc : state.accounts) {
                    if (ImGui::Selectable(acc.name.c_str(), state.input_tx_acc_to_id == acc.id)) {
                        state.input_tx_acc_to_id = acc.id;
                    }
                }
                ImGui::EndCombo();
            }
        } 
        else if (strcmp(state.input_tx_type, "Expense") == 0) {
            state.input_tx_acc_to_id = -1;
            Account* from_acc = state.GetAccountById(state.input_tx_acc_from_id);
            if (state.input_tx_acc_from_id == -1 && !state.accounts.empty()) {
                state.input_tx_acc_from_id = state.accounts[0].id;
                from_acc = &state.accounts[0];
            }
            if (ImGui::BeginCombo("Pay From Wallet", from_acc ? from_acc->name.c_str() : "")) {
                for (const auto& acc : state.accounts) {
                    if (ImGui::Selectable(acc.name.c_str(), state.input_tx_acc_from_id == acc.id)) {
                        state.input_tx_acc_from_id = acc.id;
                    }
                }
                ImGui::EndCombo();
            }
        } 
        else if (strcmp(state.input_tx_type, "Transfer") == 0) {
            Account* from_acc = state.GetAccountById(state.input_tx_acc_from_id);
            Account* to_acc = state.GetAccountById(state.input_tx_acc_to_id);
            if (state.input_tx_acc_from_id == -1 && !state.accounts.empty()) {
                state.input_tx_acc_from_id = state.accounts[0].id;
                from_acc = &state.accounts[0];
            }
            if (state.input_tx_acc_to_id == -1 && state.accounts.size() > 1) {
                state.input_tx_acc_to_id = state.accounts[1].id;
                to_acc = &state.accounts[1];
            } else if (state.input_tx_acc_to_id == -1 && !state.accounts.empty()) {
                state.input_tx_acc_to_id = state.accounts[0].id;
                to_acc = &state.accounts[0];
            }
            
            if (ImGui::BeginCombo("Transfer From", from_acc ? from_acc->name.c_str() : "")) {
                for (const auto& acc : state.accounts) {
                    if (ImGui::Selectable(acc.name.c_str(), state.input_tx_acc_from_id == acc.id)) {
                        state.input_tx_acc_from_id = acc.id;
                    }
                }
                ImGui::EndCombo();
            }
            if (ImGui::BeginCombo("Transfer To", to_acc ? to_acc->name.c_str() : "")) {
                for (const auto& acc : state.accounts) {
                    if (ImGui::Selectable(acc.name.c_str(), state.input_tx_acc_to_id == acc.id)) {
                        state.input_tx_acc_to_id = acc.id;
                    }
                }
                ImGui::EndCombo();
            }
        }
        
        ImGui::InputDouble("Amount (BDT)", &state.input_tx_amount, 10.0, 100.0, "%.2f");
        ImGui::InputText("Notes", state.input_tx_notes, IM_ARRAYSIZE(state.input_tx_notes));
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button("Save Record", ImVec2(120, 0))) {
            bool valid = true;
            if (state.input_tx_amount <= 0.0) valid = false;
            if (strcmp(state.input_tx_type, "Transfer") == 0 && state.input_tx_acc_from_id == state.input_tx_acc_to_id) {
                valid = false;
            }
            if (strcmp(state.input_tx_type, "Income") == 0 && state.input_tx_acc_to_id == -1) valid = false;
            if (strcmp(state.input_tx_type, "Expense") == 0 && state.input_tx_acc_from_id == -1) valid = false;
            
            if (valid) {
                state.AddTransaction(state.input_tx_date, state.input_tx_time, state.input_tx_type,
                                     state.input_tx_category, state.input_tx_acc_from_id,
                                     state.input_tx_acc_to_id, state.input_tx_amount, state.input_tx_notes);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }

    // 4. Edit Transaction Modal
    if (state.show_edit_transaction_modal) {
        ImGui::OpenPopup("Edit Transaction Modal");
        state.show_edit_transaction_modal = false;
    }
    if (ImGui::BeginPopupModal("Edit Transaction Modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Edit Record Details");
        ImGui::Separator();
        ImGui::Spacing();
        
        int type_idx = 0;
        if (strcmp(state.input_tx_type, "Income") == 0) type_idx = 0;
        else if (strcmp(state.input_tx_type, "Expense") == 0) type_idx = 1;
        else if (strcmp(state.input_tx_type, "Transfer") == 0) type_idx = 2;
        
        bool type_changed = false;
        if (ImGui::RadioButton("Income", type_idx == 0)) { state.input_tx_type[0] = '\0'; strcpy(state.input_tx_type, "Income"); type_changed = true; }
        ImGui::SameLine();
        if (ImGui::RadioButton("Expense", type_idx == 1)) { state.input_tx_type[0] = '\0'; strcpy(state.input_tx_type, "Expense"); type_changed = true; }
        ImGui::SameLine();
        if (ImGui::RadioButton("Transfer", type_idx == 2)) { state.input_tx_type[0] = '\0'; strcpy(state.input_tx_type, "Transfer"); type_changed = true; }
        
        if (type_changed) {
            if (strcmp(state.input_tx_type, "Transfer") == 0) {
                strcpy(state.input_tx_category, "Transfer");
            } else {
                strcpy(state.input_tx_category, "Food & Drinks");
            }
        }
        
        ImGui::Spacing();
        ImGui::InputText("Date (YYYY-MM-DD)", state.input_tx_date, IM_ARRAYSIZE(state.input_tx_date));
        ImGui::InputText("Time (e.g. 08:12 PM)", state.input_tx_time, IM_ARRAYSIZE(state.input_tx_time));
        
        if (strcmp(state.input_tx_type, "Transfer") != 0) {
            int cat_idx = -1;
            for (int i = 0; i < CATEGORIES_COUNT; ++i) {
                if (strcmp(state.input_tx_category, CATEGORIES[i]) == 0) {
                    cat_idx = i;
                    break;
                }
            }
            if (cat_idx == -1) cat_idx = CATEGORIES_COUNT - 1;
            
            if (ImGui::Combo("Category", &cat_idx, CATEGORIES, CATEGORIES_COUNT)) {
                strncpy(state.input_tx_category, CATEGORIES[cat_idx], sizeof(state.input_tx_category));
            }
            
            if (strcmp(state.input_tx_category, "Other") == 0) {
                static char custom_cat[128] = "";
                if (ImGui::InputText("Custom Category", custom_cat, IM_ARRAYSIZE(custom_cat))) {
                    strncpy(state.input_tx_category, custom_cat, sizeof(state.input_tx_category));
                }
            }
        }
        
        if (strcmp(state.input_tx_type, "Income") == 0) {
            state.input_tx_acc_from_id = -1;
            Account* to_acc = state.GetAccountById(state.input_tx_acc_to_id);
            if (ImGui::BeginCombo("Deposit To Wallet", to_acc ? to_acc->name.c_str() : "")) {
                for (const auto& acc : state.accounts) {
                    if (ImGui::Selectable(acc.name.c_str(), state.input_tx_acc_to_id == acc.id)) {
                        state.input_tx_acc_to_id = acc.id;
                    }
                }
                ImGui::EndCombo();
            }
        } 
        else if (strcmp(state.input_tx_type, "Expense") == 0) {
            state.input_tx_acc_to_id = -1;
            Account* from_acc = state.GetAccountById(state.input_tx_acc_from_id);
            if (ImGui::BeginCombo("Pay From Wallet", from_acc ? from_acc->name.c_str() : "")) {
                for (const auto& acc : state.accounts) {
                    if (ImGui::Selectable(acc.name.c_str(), state.input_tx_acc_from_id == acc.id)) {
                        state.input_tx_acc_from_id = acc.id;
                    }
                }
                ImGui::EndCombo();
            }
        } 
        else if (strcmp(state.input_tx_type, "Transfer") == 0) {
            Account* from_acc = state.GetAccountById(state.input_tx_acc_from_id);
            Account* to_acc = state.GetAccountById(state.input_tx_acc_to_id);
            
            if (ImGui::BeginCombo("Transfer From", from_acc ? from_acc->name.c_str() : "")) {
                for (const auto& acc : state.accounts) {
                    if (ImGui::Selectable(acc.name.c_str(), state.input_tx_acc_from_id == acc.id)) {
                        state.input_tx_acc_from_id = acc.id;
                    }
                }
                ImGui::EndCombo();
            }
            if (ImGui::BeginCombo("Transfer To", to_acc ? to_acc->name.c_str() : "")) {
                for (const auto& acc : state.accounts) {
                    if (ImGui::Selectable(acc.name.c_str(), state.input_tx_acc_to_id == acc.id)) {
                        state.input_tx_acc_to_id = acc.id;
                    }
                }
                ImGui::EndCombo();
            }
        }
        
        ImGui::InputDouble("Amount (BDT)", &state.input_tx_amount, 10.0, 100.0, "%.2f");
        ImGui::InputText("Notes", state.input_tx_notes, IM_ARRAYSIZE(state.input_tx_notes));
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button("Update Record", ImVec2(120, 0))) {
            bool valid = true;
            if (state.input_tx_amount <= 0.0) valid = false;
            if (strcmp(state.input_tx_type, "Transfer") == 0 && state.input_tx_acc_from_id == state.input_tx_acc_to_id) {
                valid = false;
            }
            if (strcmp(state.input_tx_type, "Income") == 0 && state.input_tx_acc_to_id == -1) valid = false;
            if (strcmp(state.input_tx_type, "Expense") == 0 && state.input_tx_acc_from_id == -1) valid = false;
            
            if (valid) {
                state.EditTransaction(state.editing_transaction_id, state.input_tx_date, state.input_tx_time,
                                      state.input_tx_type, state.input_tx_category, state.input_tx_acc_from_id,
                                      state.input_tx_acc_to_id, state.input_tx_amount, state.input_tx_notes);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }

    ImGui::End();
}



static void RenderTransactionsTab(AppState& state) {
    ImGui::TextDisabled("FILTERS");
    ImGui::Spacing();
    
    ImGui::PushItemWidth(140.0f);
    std::string active_acc_label = (state.selected_account_id == -1) ? "All Wallets" : "";
    if (state.selected_account_id != -1) {
        Account* active_acc = state.GetAccountById(state.selected_account_id);
        if (active_acc) active_acc_label = active_acc->name;
        else active_acc_label = "All Wallets";
    }
    if (ImGui::BeginCombo("Wallet Filter", active_acc_label.c_str())) {
        if (ImGui::Selectable("All Wallets", state.selected_account_id == -1)) {
            state.selected_account_id = -1;
        }
        for (const auto& acc : state.accounts) {
            if (ImGui::Selectable(acc.name.c_str(), state.selected_account_id == acc.id)) {
                state.selected_account_id = acc.id;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
    
    ImGui::SameLine();
    
    ImGui::PushItemWidth(150.0f);
    if (ImGui::BeginCombo("Category Filter", state.filter_category[0] == '\0' ? "All Categories" : state.filter_category)) {
        if (ImGui::Selectable("All Categories", state.filter_category[0] == '\0')) {
            state.filter_category[0] = '\0';
        }
        for (int i = 0; i < CATEGORIES_COUNT; ++i) {
            if (ImGui::Selectable(CATEGORIES[i], strcmp(state.filter_category, CATEGORIES[i]) == 0)) {
                strncpy(state.filter_category, CATEGORIES[i], sizeof(state.filter_category));
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
    
    ImGui::SameLine();
    
    ImGui::PushItemWidth(180.0f);
    ImGui::InputText("Search Notes", state.search_query, IM_ARRAYSIZE(state.search_query));
    ImGui::PopItemWidth();
    
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 130.0f);
    
    if (ImGui::Button("Add Record (+)", ImVec2(130.0f, 32.0f))) {
        std::string cur_date = GetCurrentDateString();
        std::string cur_time = GetCurrentTimeString();
        strncpy(state.input_tx_date, cur_date.c_str(), sizeof(state.input_tx_date));
        strncpy(state.input_tx_time, cur_time.c_str(), sizeof(state.input_tx_time));
        strncpy(state.input_tx_type, "Expense", sizeof(state.input_tx_type));
        strncpy(state.input_tx_category, "Food & Drinks", sizeof(state.input_tx_category));
        
        state.input_tx_acc_from_id = -1;
        state.input_tx_acc_to_id = -1;
        if (!state.accounts.empty()) {
            state.input_tx_acc_from_id = state.accounts[0].id;
            state.input_tx_acc_to_id = state.accounts[0].id;
        }
        state.input_tx_amount = 0.0;
        state.input_tx_notes[0] = '\0';
        
        state.show_add_transaction_modal = true;
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("LedgerTable", 8, flags, ImVec2(0, -FLT_MIN))) {
        ImGui::TableSetupColumn("Date/Time", ImGuiTableColumnFlags_WidthFixed, 140.0f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Wallet From", ImGuiTableColumnFlags_WidthFixed, 105.0f);
        ImGui::TableSetupColumn("Wallet To", ImGuiTableColumnFlags_WidthFixed, 105.0f);
        ImGui::TableSetupColumn("Notes", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed, 110.0f);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 110.0f);
        ImGui::TableHeadersRow();
        
        for (int i = (int)state.transactions.size() - 1; i >= 0; --i) {
            const auto& tx = state.transactions[i];
            
            if (state.selected_account_id != -1) {
                if (tx.account_from_id != state.selected_account_id && tx.account_to_id != state.selected_account_id) {
                    continue;
                }
            }
            if (state.filter_category[0] != '\0') {
                if (tx.category != state.filter_category) {
                    continue;
                }
            }
            if (state.search_query[0] != '\0') {
                std::string notes_lower = tx.notes;
                std::transform(notes_lower.begin(), notes_lower.end(), notes_lower.begin(), ::tolower);
                std::string q_lower = state.search_query;
                std::transform(q_lower.begin(), q_lower.end(), q_lower.begin(), ::tolower);
                if (notes_lower.find(q_lower) == std::string::npos) {
                    continue;
                }
            }
            
            ImGui::TableNextRow();
            
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s %s", tx.date.c_str(), tx.time.c_str());
            
            ImGui::TableSetColumnIndex(1);
            if (tx.type == "Income") {
                ImGui::TextColored(ImVec4(0.12f, 0.73f, 0.38f, 1.00f), "%s", tx.type.c_str());
            } else if (tx.type == "Expense") {
                ImGui::TextColored(ImVec4(0.89f, 0.22f, 0.22f, 1.00f), "%s", tx.type.c_str());
            } else {
                ImGui::TextColored(ImVec4(0.35f, 0.60f, 0.90f, 1.00f), "%s", tx.type.c_str());
            }
            
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", tx.category.c_str());
            
            ImGui::TableSetColumnIndex(3);
            if (tx.account_from_id != -1) {
                Account* acc = state.GetAccountById(tx.account_from_id);
                ImGui::Text("%s", acc ? acc->name.c_str() : "Deleted Account");
            } else {
                ImGui::TextDisabled("-");
            }
            
            ImGui::TableSetColumnIndex(4);
            if (tx.account_to_id != -1) {
                Account* acc = state.GetAccountById(tx.account_to_id);
                ImGui::Text("%s", acc ? acc->name.c_str() : "Deleted Account");
            } else {
                ImGui::TextDisabled("-");
            }
            
            ImGui::TableSetColumnIndex(5);
            ImGui::TextWrapped("%s", tx.notes.c_str());
            
            ImGui::TableSetColumnIndex(6);
            if (tx.type == "Income") {
                ImGui::TextColored(ImVec4(0.12f, 0.73f, 0.38f, 1.00f), "+BDT %.2f", tx.amount);
            } else if (tx.type == "Expense") {
                ImGui::TextColored(ImVec4(0.89f, 0.22f, 0.22f, 1.00f), "-BDT %.2f", tx.amount);
            } else {
                ImGui::TextColored(ImVec4(0.35f, 0.60f, 0.90f, 1.00f), "BDT %.2f", tx.amount);
            }
            
            ImGui::TableSetColumnIndex(7);
            ImGui::PushID(tx.id);
            if (ImGui::Button("Edit", ImVec2(45, 20))) {
                state.editing_transaction_id = tx.id;
                strncpy(state.input_tx_date, tx.date.c_str(), sizeof(state.input_tx_date));
                strncpy(state.input_tx_time, tx.time.c_str(), sizeof(state.input_tx_time));
                strncpy(state.input_tx_type, tx.type.c_str(), sizeof(state.input_tx_type));
                strncpy(state.input_tx_category, tx.category.c_str(), sizeof(state.input_tx_category));
                state.input_tx_acc_from_id = tx.account_from_id;
                state.input_tx_acc_to_id = tx.account_to_id;
                state.input_tx_amount = tx.amount;
                strncpy(state.input_tx_notes, tx.notes.c_str(), sizeof(state.input_tx_notes));
                
                state.show_edit_transaction_modal = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Delete", ImVec2(55, 20))) {
                state.DeleteTransaction(tx.id);
            }
            ImGui::PopID();
        }
        
        ImGui::EndTable();
    }
}