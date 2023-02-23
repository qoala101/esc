/**
 * @author Volodymyr Hromakov (4y5t6r@gmail.com)
 */

#ifndef VH_DRAW_NODES_H_
#define VH_DRAW_NODES_H_

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "app_state.h"
#include "core_i_node.h"
#include "core_settings.h"
#include "coreui_i_pin_drawer.h"
#include "draw_new_link.h"
#include "draw_settings_view.h"
#include "draw_texture.h"
#include "flow_node_flow.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_node_editor.h"

namespace esc::draw {
// ---
class Nodes {
 public:
  // ---
  explicit Nodes(const Texture& node_header_texture);

  // ---
  void Draw(const AppState& app_state);
  // ---
  auto GetDrawnPinIconRect [[nodiscard]] (ne::PinId pin_id) const
      -> const ImRect&;

 private:
  // ---
  void DrawNode(core::INode& node, const flow::NodeFlow& node_flow,
                const core::Diagram& diagram, const NewLink& new_link,
                const SettingsView& settings_view,
                const core::Settings& settings);
  // ---
  void DrawPinIconArea(const coreui::IPinDrawer& pin_drawer,
                       const flow::NodeFlow& node_flow,
                       const core::Diagram& diagram, const NewLink& new_link,
                       const SettingsView& settings_view,
                       const core::Settings& settings);

  // ---
  Texture node_header_texture_{};
  // ---
  std::unordered_map<uintptr_t, ImRect> drawn_pin_icon_rects_{};
};
}  // namespace esc::draw

#endif  // VH_DRAW_NODES_H_
