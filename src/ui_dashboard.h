#pragma once
#include <string>
#include <vector>

struct Account {
    int id;
    std::string name;
    std::string type;
    double balance;
};

struct AppState {
    std::vector<Account> accounts;
    double total_balance;
    
    // Mock data to test the UI layout
    AppState() {
        accounts = {
            {1, "Cash", "Cash", 1515.00},
            {2, "AB Student", "Savings account", 22320.40},
            {3, "bKash", "Checking account", 279.70}
        };
        CalculateTotal();
    }

    void CalculateTotal() {
        total_balance = 0.0;
        for (const auto& acc : accounts) {
            total_balance += acc.balance;
        }
    }
};

void RenderDashboardWindow(AppState& state);