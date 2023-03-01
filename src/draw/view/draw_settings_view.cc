#include "draw_settings_view.h"

#include <imgui.h>

#include "core_project.h"
#include "core_settings.h"
#include "coreui_project.h"
#include "cpp_scope.h"

namespace esc::draw {
auto SettingsView::GetLabel() const -> std::string { return "Settings"; }

void SettingsView::Draw(core::Settings& settings) {
  if (!IsVisible()) {
    return;
  }

  if (ImGui::Begin("Settings", &GetVisible())) {
    const auto low_high_mid =
        settings.low_flow + (settings.high - settings.low_flow) / 2;

    ImGui::Checkbox("Color Flow", &settings.color_flow);

    ImGui::ColorButton("##1", ImColor{0.F, 0.F, 1.F});
    ImGui::SameLine();
    ImGui::DragFloat("Very Low##link_color", &settings.min_flow, 0.01F,
                     -FLT_MAX, settings.low_flow, "%.3f");

    ImGui::ColorButton("##2", ImColor{0.F, 1.F, 1.F});
    ImGui::SameLine();
    ImGui::SliderFloat("Low##link_color", &settings.low_flow, settings.min_flow,
                       settings.high);

    ImGui::ColorButton("##5", ImColor{0.F, 1.F, 0.F});
    ImGui::SameLine();
    ImGui::Text("%.3f Good", low_high_mid);

    ImGui::ColorButton("##3", ImColor{1.F, 1.F, 0.F});
    ImGui::SameLine();
    ImGui::SliderFloat("High##link_color", &settings.high, settings.low_flow,
                       settings.max_flow);

    ImGui::ColorButton("##4", ImColor{1.F, 0.F, 0.F});
    ImGui::SameLine();
    ImGui::DragFloat("Very High##link_color", &settings.max_flow, 0.01F,
                     settings.high, +FLT_MAX, "%.3f");
  }

  ImGui::End();
}
}  // namespace esc::draw