#ifndef VH_ESC_STATE_H_
#define VH_ESC_STATE_H_

#include <imgui_node_editor.h>
#include <sys/types.h>

#include <cstdint>
#include <functional>

#include "core_app.h"
#include "core_flow_calculator.h"
#include "core_id_generator.h"
#include "imgui.h"

namespace esc {
// ---
class State {
 public:
  // ---
  State();

  // ---
  static void OpenDiagramFromFile(State &state, const std::string &file_path);
  // ---
  static void SaveDiagramToFile(const State &state,
                                const std::string &file_path);
  // ---
  static void ResetDiagram(State &state);
  // ---
  static void EraseLink(State &state, ne::LinkId link_id);
  // ---
  static void EraseNodeAndConnectedLinks(State &state, ne::NodeId node_id);

  static void ReplaceWithPlaceholder(State &state, ne::NodeId node_id);
  static void ReplaceWithFreePins(State &state, ne::NodeId node_id);
  static void MakeGroupFromSelectedNodes(State &state, std::string group_name);

  // ---
  void OnFrame();
  // ---
  void PostEvent(std::function<void(State &state)> event);

  //  private:
  core::App app_{};
  core::IdGenerator id_generator_{};
  core::FlowCalculator flow_calculator_{};

  struct Rebind {
    ne::PinId fixed_pin{};
    ne::PinKind fixed_pin_kind{};
    ne::LinkId rebinding_link_id{};
    std::optional<ImVec2> fixed_pin_pos{};
  };

  struct NewLink {
    ne::PinId pin_dragged_from{};
    std::optional<ne::PinId> pin_hovered_over{};
    std::optional<Rebind> rebind{};
  };

  struct {
    std::optional<ne::NodeId> popup_node_{};

    struct {
      bool color_flow{};
      float min{-27};
      float low{-22};
      float high{-18};
      float max{6};
    } link_colors{};

    std::optional<NewLink> new_link{};

    std::unordered_map<uintptr_t, ImVec2> pin_poses_{};
  } drawing_{};

  auto GetColorForFlowValue(float value) const -> ImColor;

  auto CanConnectFromPinToPin(ne::PinId start_pin, ne::PinId end_pin) -> bool;

 private:
  // ---
  void ExecuteEvents();

  std::vector<std::function<void(State &state)>> events_{};
};
}  // namespace esc

#endif  // VH_ESC_STATE_H_
