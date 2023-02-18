/**
 * @author Volodymyr Hromakov (4y5t6r@gmail.com)
 */

#ifndef VH_CORE_I_NODE_H_
#define VH_CORE_I_NODE_H_

#include <imgui_node_editor.h>

#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "cpp_interface.h"
#include "flow_node_flow.h"
#include "imgui.h"

namespace ne = ax::NodeEditor;

namespace esc {
class StateNoQueue;

namespace json {
class INodeWriter;
}  // namespace json

namespace coreui {
class INodeDrawer;
}  // namespace coreui

namespace core {
class INode : public cpp::Interface {
 public:
  // ---
  virtual auto CreateWriter [[nodiscard]] ()
  -> std::unique_ptr<json::INodeWriter> = 0;

  virtual auto CreateDrawer [[nodiscard]] (const StateNoQueue &state)
  -> std::unique_ptr<coreui::INodeDrawer> = 0;

  // ---
  auto GetId [[nodiscard]] () const -> ne::NodeId;
  // ---
  auto GetInputPinId [[nodiscard]] () const -> const std::optional<ne::PinId> &;
  // ---
  auto GetOutputPinIds [[nodiscard]] () const -> const std::vector<ne::PinId> &;
  // ---
  auto GetPosition [[nodiscard]] () const -> ImVec2;
  // ---
  void SetPosition(const ImVec2 &position);
  // ---
  auto GetInitialFlow [[nodiscard]] () const -> flow::NodeFlow;

 protected:
  // ---
  explicit INode(ne::NodeId id, std::vector<ne::PinId> output_pin_ids = {});
  // ---
  INode(ne::NodeId id, ne::PinId input_pin_id,
        std::vector<ne::PinId> output_pin_ids = {});

 private:
  // ---
  virtual void SetInitialFlowValues(flow::NodeFlow &node_flow) const;

  // ---
  ne::NodeId id_{};
  // ---
  std::optional<ne::PinId> input_pin_id_{};
  // ---
  std::vector<ne::PinId> output_pin_ids_{};
};

// ---
auto GetAllPinIds [[nodiscard]] (const INode &node) -> std::vector<ne::PinId>;
}  // namespace core
}  // namespace esc

#endif  // VH_CORE_I_NODE_H_
