#ifndef VH_COREUI_LINK_CREATION_H_
#define VH_COREUI_LINK_CREATION_H_

#include <imgui.h>
#include <imgui_node_editor.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "core_i_node.h"
#include "core_link.h"
#include "cpp_callbacks.h"

namespace esc::coreui {
///
struct MousePos {};

///
struct NewNodePos {};

///
using PosVariant = std::variant<MousePos, NewNodePos, ne::PinId>;

///
struct ManualLink {
  ///
  PosVariant start_pos{};
  ///
  PosVariant end_pos{};
  ///
  ImColor color{};
  ///
  float thickness{};
};

///
class LinkCreation {
 public:
  ///
  struct Callbacks {
    ///
    cpp::Query<const core::INode&, ne::PinId> find_pin_node{};
    ///
    cpp::Query<std::optional<const core::Link*>, ne::PinId> find_pin_link{};
    ///
    cpp::Action<void(ne::PinId, ne::PinId)> create_link{};
    ///
    cpp::Action<void(ne::LinkId)> delete_link{};
  };

  ///
  explicit LinkCreation(Callbacks callbacks);

  ///
  void SetPins(const std::optional<ne::PinId>& dragged_from_pin,
               const std::optional<ne::PinId>& hovering_over_pin = {});
  ///
  auto CanConnectToPin(ne::PinId pin_id) const -> bool;
  ///
  auto CanConnectToNode(const core::INode& node) const -> bool;
  ///
  auto GetCanCreateLinkReason() const -> std::pair<bool, std::string>;
  ///
  auto IsRepiningLink(ne::LinkId link_id) const -> bool;
  ///
  void AcceptNewLink();
  ///
  void StartCreatingNode();
  ///
  auto IsCreatingNode() const -> bool;
  ///
  void AcceptNewNode(core::INode& node);
  ///
  void DiscardNewNode();
  ///
  auto GetManualLink() const -> std::optional<const ManualLink*>;

 private:
  ///
  struct HoveringData {
    ///
    ne::PinId hovering_over_pin{};
    ///
    ne::PinKind hovering_over_pin_kind{};
    ///
    bool can_connect_to_hovered_pin{};
    ///
    std::string reason{};
  };

  ///
  struct RepinningData {
    ///
    ne::LinkId link_to_repin{};
    ///
    ne::PinId fixed_pin{};
    ///
    ne::PinKind fixed_pin_kind{};
    ///
    ne::NodeId fixed_pin_node{};
  };

  ///
  struct CreatingData {
    ///
    ne::PinId dragged_from_pin{};
    ///
    ne::NodeId dragged_from_node{};
    ///
    ne::PinKind dragged_from_pin_kind{};
    ///
    bool creating_node{};
    ///
    std::optional<ManualLink> manual_link{};
    ///
    std::optional<HoveringData> hovering_data{};
    ///
    std::optional<RepinningData> repinning_data{};
  };

  ///
  auto GetCurrentLinkSourcePin() const;
  ///
  auto GetRepinningLinkColor() const;
  ///
  auto CreateManualLinkTo(const PosVariant& target_pos,
                          const ImColor& color) const;
  ///
  auto GetCanConnectToPinReason(ne::PinId pin_id) const
      -> std::pair<bool, std::string>;

  ///
  Callbacks callbacks_{};
  ///
  std::optional<CreatingData> creating_data_{};
};
}  // namespace esc::coreui

#endif  // VH_COREUI_LINK_CREATION_H_
