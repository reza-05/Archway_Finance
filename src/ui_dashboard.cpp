// ui_dashboard.cpp
#include "ui_dashboard.h"
#include "imgui.h"
#include <string>

void RenderDashboardWindow(AppState& state) {
    // Full-screen or large window for the main app
    ImGui::SetNextWindowSize(ImVec2(1000, 600), ImGuiCond_FirstUseEver);
    ImGui::Begin("Financial Dashboard", nullptr, ImGuiWindowFlags_NoCollapse);

    // Nav / Header Area
    ImGui::Text("Total Balance: BDT %.2f", state.total_balance);
    ImGui::Separator();
    ImGui::Spacing();

    // Accounts Row 
    ImGui::TextDisabled("ACCOUNTS");
    ImGui::Spacing();

    // Table to layout the accounts horizontally
    if (ImGui::BeginTable("AccountsTable", state.accounts.size(), ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextRow();
        
        for (const auto& acc : state.accounts) {
            ImGui::TableSetColumnIndex(&acc - &state.accounts[0]);
            
            // Create a visually distinct panel for each account
            ImGui::PushID(acc.id);
            ImGui::BeginChild("AccountCard", ImVec2(0, 80), true);
            
            ImGui::TextUnformatted(acc.name.c_str());
            ImGui::TextDisabled("%s", acc.type.c_str());
            ImGui::Spacing();
            ImGui::Text("BDT %.2f", acc.balance);
            
            ImGui::EndChild();
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::End();
}