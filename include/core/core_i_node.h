#ifndef VH_CORE_I_NODE_H_
#define VH_CORE_I_NODE_H_

#include <imgui_node_editor.h>

#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "core_flow.h"
#include "cpp_interface.h"
#include "imgui.h"

namespace ne = ax::NodeEditor;

namespace esc {
class State;

namespace json {
class INodeWriter;
}  // namespace json

namespace draw {
class INodeDrawer;
}  // namespace draw

namespace core {

class INode : public cpp::Interface {
 public:
  // ---
  virtual auto GetInitialFlow [[nodiscard]] () const -> Flow = 0;
  // ---
  virtual auto CreateWriter [[nodiscard]] ()
  -> std::unique_ptr<json::INodeWriter> = 0;
  // ---
  virtual auto CreateDrawer [[nodiscard]] (const State &state)
  -> std::unique_ptr<draw::INodeDrawer> = 0;

  // ---
  auto GetId [[nodiscard]] () const -> ne::NodeId;
  // ---
  auto GetPinIds [[nodiscard]] () const -> const std::vector<ne::PinId> &;
  // ---
  auto GetPinIndex [[nodiscard]] (ne::PinId pin_id) const -> int;
  // ---
  auto GetPosition [[nodiscard]] () const -> ImVec2;
  // ---
  void SetPosition(const ImVec2 &position);

 protected:
  // ---
  INode(ne::NodeId id, std::vector<ne::PinId> pin_ids);

 private:
  ne::NodeId id_{};
  std::vector<ne::PinId> pin_ids_{};
};
}  // namespace core
}  // namespace esc

#endif  // VH_CORE_I_NODE_H_
