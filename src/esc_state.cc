#include "esc_state.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "core_app.h"
#include "core_diagram.h"
#include "core_flow.h"
#include "core_i_node.h"
#include "core_id_generator.h"
#include "core_link.h"
#include "core_placeholder_family.h"
#include "crude_json.h"
#include "draw_i_node_drawer.h"
#include "imgui.h"
#include "imgui_node_editor.h"
#include "impl_attenuator_node.h"
#include "impl_client_node.h"
#include "impl_coupler_node.h"
#include "impl_input_node.h"
#include "impl_splitter_node.h"
#include "json_diagram_serializer.h"

namespace esc {
namespace {
auto CreateFamilies() {
  auto families = std::vector<std::shared_ptr<core::IFamily>>{
      impl::InputNode::CreateFamily(), impl::ClientNode::CreateFamily()};

  for (auto percentage_index = 0; percentage_index < 10; ++percentage_index) {
    families.emplace_back(impl::CouplerNode::CreateFamily(percentage_index));
  }

  for (auto num_outputs : {2, 4, 8, 16}) {
    families.emplace_back(impl::SplitterNode::CreateFamily(num_outputs));
  }

  families.emplace_back(impl::AttenuatorNode::CreateFamily());

  return families;
}

auto CreateFamilyParsers() {
  auto family_parsers = std::vector<std::unique_ptr<json::IFamilyParser>>{};
  family_parsers.emplace_back(impl::InputNode::CreateFamilyParser());
  family_parsers.emplace_back(impl::ClientNode::CreateFamilyParser());
  family_parsers.emplace_back(impl::CouplerNode::CreateFamilyParser());
  family_parsers.emplace_back(impl::SplitterNode::CreateFamilyParser());
  family_parsers.emplace_back(impl::AttenuatorNode::CreateFamilyParser());
  family_parsers.emplace_back(core::PlaceholderFamily::CreateParser());
  return family_parsers;
}

auto FindMaxId(const core::Diagram &diagram) {
  auto max_id = uintptr_t{1};

  for (const auto &family : diagram.GetFamilies()) {
    for (const auto &node : family->GetNodes()) {
      max_id = std::max(node->GetId().Get(), max_id);

      for (const auto pin_id : node->GetPinIds()) {
        max_id = std::max(pin_id.Get(), max_id);
      }
    }
  }

  for (const auto &link : diagram.GetLinks()) {
    max_id = std::max(link.id.Get(), max_id);
  }

  return max_id;
}
}  // namespace

State::State() { ResetDiagram(*this); }

void State::OpenDiagramFromFile(State &state, const std::string &file_path) {
  ResetDiagram(state);

  const auto json = crude_json::value::load(file_path).first;

  auto diagram =
      json::DiagramSerializer::ParseFromJson(json, CreateFamilyParsers());
  auto max_id = FindMaxId(diagram);

  state.app_.SetDiagram(std::move(diagram));
  state.id_generator_ = core::IdGenerator{max_id + 1};
}

void State::SaveDiagramToFile(const State &state,
                              const std::string &file_path) {
  const auto &diagram = state.app_.GetDiagram();
  const auto json = json::DiagramSerializer::WriteToJson(diagram);
  json.save(file_path);
}

void State::ResetDiagram(State &state) {
  const auto &diagram = state.app_.GetDiagram();
  const auto &links = diagram.GetLinks();

  for (const auto &link : links) {
    ne::DeleteLink(link.id);
  }

  const auto &families = diagram.GetFamilies();

  for (const auto &family : families) {
    for (const auto &node : family->GetNodes()) {
      ne::DeleteNode(node->GetId());
    }
  }

  state.drawing_.new_link.reset();

  state.id_generator_ = core::IdGenerator{};
  state.app_.SetDiagram(core::Diagram{CreateFamilies()});
}

void State::EraseLink(State &state, ne::LinkId link_id) {
  ne::DeleteLink(link_id);
  state.app_.GetDiagram().EraseLink(link_id);
}

void State::EraseNodeAndConnectedLinks(State &state, ne::NodeId node_id) {
  auto &diagram = state.app_.GetDiagram();
  const auto &links = diagram.GetLinks();
  const auto &node = diagram.FindNode(node_id);
  const auto &node_pins = node.GetPinIds();

  const auto links_to_erase = [&links, &node_pins]() {
    auto links_to_erase = std::vector<ne::LinkId>{};

    for (const auto &link : links) {
      for (const auto node_pin : node_pins) {
        if ((link.start_pin_id == node_pin) || (link.end_pin_id == node_pin)) {
          links_to_erase.emplace_back(link.id);
        }
      }
    }
    return links_to_erase;
  }();

  for (const auto link : links_to_erase) {
    ne::DeleteLink(link);
  }

  for (const auto link : links_to_erase) {
    diagram.EraseLink(link);
  }

  ne::DeleteNode(node_id);
  state.app_.GetDiagram().EraseNode(node_id);
}

void State::ReplaceWithPlaceholder(State &state, ne::NodeId node_id) {
  auto &diagram = state.app_.GetDiagram();
  const auto &node = diagram.FindNode(node_id);
  const auto node_flow = node.GetInitialFlow();
  const auto node_position = node.GetPosition();

  EraseNodeAndConnectedLinks(state, node_id);

  auto &placeholder_family = diagram.GetPlaceholderFamily();
  auto &placeholder =
      placeholder_family.EmplaceNodeFromFlow(state.id_generator_, node_flow);

  placeholder.SetPosition(node_position);
}

void State::ReplaceWithFreePins(State &state, ne::NodeId node_id) {
  auto &diagram = state.app_.GetDiagram();
  const auto &node = diagram.FindNode(node_id);
  const auto node_flow = node.GetInitialFlow();
  const auto node_position = node.GetPosition();

  const auto &links = diagram.GetLinks();

  auto connected_node_input_pin = std::optional<ne::PinId>{};
  auto connected_node_output_pins = std::vector<ne::PinId>{};

  if (node_flow.input_pin_flow.has_value()) {
    if (diagram.FindLinkFromPin(node_flow.input_pin_flow->first) != nullptr) {
      connected_node_input_pin = node_flow.input_pin_flow->first;
    }
  }

  for (const auto &[pin, value] : node_flow.output_pin_flows) {
    if (diagram.FindLinkFromPin(pin) != nullptr) {
      connected_node_output_pins.emplace_back(pin);
    }
  }

  ne::DeleteNode(node_id);
  state.app_.GetDiagram().EraseNode(node_id);

  auto &free_pin_family = diagram.GetFreePinFamily();

  if (connected_node_input_pin.has_value()) {
    auto &free_pin = free_pin_family.EmplaceNodeFromFlow(
        state.id_generator_, *connected_node_input_pin, true);

    free_pin.SetPosition(
        state.drawing_.pin_poses_.at(connected_node_input_pin->Get()));
  }

  for (const auto pin : connected_node_output_pins) {
    auto &free_pin =
        free_pin_family.EmplaceNodeFromFlow(state.id_generator_, pin, false);
    free_pin.SetPosition(state.drawing_.pin_poses_.at(pin.Get()));
  }
}

void State::MakeGroupFromSelectedNodes(State &state, std::string group_name) {
  auto selectedNodes = state.app_.GetDiagram().GetSelectedNodeIds();

  if (state.drawing_.popup_node_.has_value()) {
    const auto popup_node = *state.drawing_.popup_node_;

    if (std::ranges::none_of(selectedNodes, [popup_node](const auto node) {
          return node == popup_node;
        })) {
      selectedNodes.emplace_back(popup_node);
    }

    state.drawing_.popup_node_.reset();
  }

  auto &group = state.app_.GetDiagram().EmplaceGroup(selectedNodes);
  group.name_ = std::move(group_name);
}

auto State::GetColorForFlowValue(float value) const -> ImColor {
  if (!drawing_.link_colors.color_flow) {
    return ImColor{255, 255, 255};
  }

  const auto blue = ImColor{0, 0, 255};
  const auto blue_green = ImColor{0, 255, 255};
  const auto green = ImColor{0, 255, 0};
  const auto green_red = ImColor{255, 255, 0};
  const auto red = ImColor{255, 0, 0};

  if (value < drawing_.link_colors.min) {
    return blue;
  }

  if (value >= drawing_.link_colors.max) {
    return red;
  }

  const auto range = (drawing_.link_colors.max - drawing_.link_colors.min);
  const auto value_percentage = (value - drawing_.link_colors.min) / range;
  const auto low_percentage =
      (drawing_.link_colors.low - drawing_.link_colors.min) / range;
  const auto high_percentage =
      (drawing_.link_colors.high - drawing_.link_colors.min) / range;

  auto percentage = 0.0F;
  auto start_color = ImColor{};
  auto end_color = ImColor{};

  if (value_percentage < low_percentage) {
    percentage = value_percentage / low_percentage;
    start_color = blue;
    end_color = blue_green;
  } else if (value_percentage >= high_percentage) {
    percentage =
        (value_percentage - high_percentage) / (1.0F - high_percentage);
    start_color = green_red;
    end_color = red;
  } else {
    const auto low_high_range = (high_percentage - low_percentage);
    percentage = (value_percentage - low_percentage) / low_high_range;

    if (percentage < 0.5F) {
      percentage = percentage * 2;
      start_color = blue_green;
      end_color = green;
    } else {
      percentage = (percentage - 0.5F) * 2;
      start_color = green;
      end_color = green_red;
    }
  }

  return ImColor{start_color.Value.x +
                     percentage * (end_color.Value.x - start_color.Value.x),
                 start_color.Value.y +
                     percentage * (end_color.Value.y - start_color.Value.y),
                 start_color.Value.z +
                     percentage * (end_color.Value.z - start_color.Value.z)};
}

auto State::CanConnectFromPinToPin(ne::PinId start_pin, ne::PinId end_pin)
    -> bool {
  if (start_pin == end_pin) {
    return false;
  }

  const auto rebind =
      drawing_.new_link.has_value() && drawing_.new_link->rebind.has_value();
  const auto &families = app_.GetDiagram().GetFamilies();
  const auto &links = app_.GetDiagram().GetLinks();

  auto *start_node = static_cast<core::INode *>(nullptr);
  auto *end_node = static_cast<core::INode *>(nullptr);

  for (const auto &family : families) {
    for (const auto &node : family->GetNodes()) {
      for (const auto pin : node->GetPinIds()) {
        if (pin == start_pin) {
          start_node = &*node;
        } else if (pin == end_pin) {
          end_node = &*node;
        }
      }
    }
  }

  if (start_node == end_node) {
    std::cout << "r0\n";
    return false;
  }

  const auto start_drawer =
      start_node->CreateDrawer(*this)->CreatePinDrawer(start_pin);
  const auto end_drawer =
      end_node->CreateDrawer(*this)->CreatePinDrawer(end_pin);

  if (rebind) {
    if (start_drawer->GetKind() != end_drawer->GetKind()) {
      std::cout << "r1\n";
      return false;
    }
  } else {
    if (start_drawer->GetKind() == end_drawer->GetKind()) {
      std::cout << "r2\n";
      return false;
    }
  }

  auto *start_link = static_cast<const core::Link *>(nullptr);
  auto *end_link = static_cast<const core::Link *>(nullptr);

  for (const auto &link : links) {
    if (link.start_pin_id == start_pin || link.end_pin_id == start_pin) {
      start_link = &link;
    } else if (link.start_pin_id == end_pin || link.end_pin_id == end_pin) {
      end_link = &link;
    }
  }

  if (rebind) {
    std::cout << "rebind1\n";
    if (start_link != nullptr && end_link != nullptr) {
      std::cout << "rebind2\n";
      if (start_link != end_link) {
        std::cout << "rebind3\n";
        return false;
      }

      // return f
    }
  } else {
    if (start_link != nullptr || end_link != nullptr) {
      std::cout << "r3\n";
      return false;
    }
  }

  return true;
}

void State::OnFrame() {
  ExecuteEvents();
  flow_calculator_.OnFrame(*this);
}

void State::PostEvent(std::function<void(State &state)> event) {
  events_.emplace_back(std::move(event));
}

void State::ExecuteEvents() {
  for (const auto &event : events_) {
    event(*this);
  }

  events_.clear();
}
}  // namespace esc