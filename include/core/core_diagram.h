#ifndef VH_CORE_DIAGRAM_H_
#define VH_CORE_DIAGRAM_H_

#include <memory>
#include <vector>

#include "core_i_node.h"
#include "core_link.h"
#include "cpp_safe_pointer.h"
#include "imgui_node_editor.h"

namespace esc::core {
///
class Diagram : public cpp::EnableSafePointer<Diagram> {
 public:
  ///
  explicit Diagram(std::vector<std::unique_ptr<INode>> nodes = {},
                   std::vector<Link> links = {});

  ///
  static auto FindPinNode(const Diagram &diagram, ne::PinId pin_id)
      -> const INode &;
  ///
  static auto FindPinLink(const Diagram &diagram, ne::PinId pin_id)
      -> std::optional<const Link *>;

  ///
  auto GetNodes() const -> const std::vector<std::unique_ptr<INode>> &;
  ///
  auto EmplaceNode(std::unique_ptr<INode> node) -> INode &;
  ///
  void EraseNode(ne::NodeId node_id);

  ///
  auto GetLinks() const -> const std::vector<Link> &;
  ///
  auto EmplaceLink(const Link &link) -> Link &;
  ///
  void EraseLink(ne::LinkId link_id);

 private:
  ///
  std::vector<std::unique_ptr<INode>> nodes_{};
  ///
  std::vector<Link> links_{};
};
}  // namespace esc::core

#endif  // VH_CORE_DIAGRAM_H_
