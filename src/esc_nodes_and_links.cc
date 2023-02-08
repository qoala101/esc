#include "esc_nodes_and_links.h"

#include <crude_json.h>
#include <imgui_node_editor.h>
#include <imgui_node_editor_internal.h>
#include <sys/types.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "esc_app.h"
#include "esc_cpp.h"
#include "esc_enums.h"
#include "esc_types.h"
#include "imgui.h"

namespace esc {
NodesAndLinks::NodesAndLinks(std::shared_ptr<App> app)
    : app_{(cpp::Expects(app != nullptr), std::move(app))} {
  cpp::Ensures(app_ != nullptr);
}

auto NodesAndLinks::SpawnInputNode() -> Node* {
  auto& node = nodes_.emplace_back(app_->GetNextObjectId(), "Input",
                                   ImColor{255, 127, 127});

  auto& output = node.Outputs.emplace_back(
      app_->GetNextObjectId(), "", PinType::Flow, PinKind::Output, &node);
  output.editable = true;

  return &node;
}

auto NodesAndLinks::SpawnClientNode() -> Node* {
  auto& node = nodes_.emplace_back(app_->GetNextObjectId(), "Client",
                                   ImColor{127, 255, 127});

  node.Inputs.emplace_back(app_->GetNextObjectId(), "In", PinType::Flow,
                           PinKind::Input, &node);

  return &node;
}

auto NodesAndLinks::SpawnCommentNode() -> Node* {
  auto& node = nodes_.emplace_back(app_->GetNextObjectId(), "Comment");

  node.Type = NodeType::Comment;
  node.Size = ImVec2{300, 200};

  return &node;
}

auto NodesAndLinks::SpawnCoupler1To2Node() -> Node* {
  auto& node = nodes_.emplace_back(app_->GetNextObjectId(), "Coupler 1x2",
                                   ImColor{127, 127, 127});

  node.Inputs.emplace_back(app_->GetNextObjectId(), "", PinType::Flow,
                           PinKind::Input, &node);
  auto* input = &node.Inputs.emplace_back(
      app_->GetNextObjectId(), "", PinType::Float, PinKind::Input, &node);
  // input->editable = true;
  input = &node.Inputs.emplace_back(app_->GetNextObjectId(), "", PinType::Float,
                                    PinKind::Input, &node);
  // input->editable = true;

  node.Outputs.emplace_back(app_->GetNextObjectId(), "", PinType::Empty,
                            PinKind::Output, &node);
  node.Outputs.emplace_back(app_->GetNextObjectId(), "", PinType::Flow,
                            PinKind::Output, &node);
  node.Outputs.emplace_back(app_->GetNextObjectId(), "", PinType::Flow,
                            PinKind::Output, &node);

  return &node;
}

auto NodesAndLinks::SpawnSplitter1ToNNode(int n) -> Node* {
  const auto n_string = std::to_string(n);
  const auto node_name = "Splitter 1x" + n_string;

  auto& node = nodes_.emplace_back(app_->GetNextObjectId(), node_name.c_str(),
                                   ImColor{127 / n, 127 / n, 255});

  node.Inputs.emplace_back(app_->GetNextObjectId(), "", PinType::Flow,
                           PinKind::Input, &node);

  auto& input = node.Inputs.emplace_back(app_->GetNextObjectId(), "",
                                         PinType::Float, PinKind::Input, &node);

  for (auto i = 0; i < n; ++i) {
    node.Outputs.emplace_back(app_->GetNextObjectId(), "", PinType::Flow,
                              PinKind::Output, &node);
  }

  return &node;
}

auto NodesAndLinks::GetNodes() -> std::vector<Node>& { return nodes_; }

auto NodesAndLinks::GetLinks() const -> const std::vector<Link>& {
  return links_;
}

auto NodesAndLinks::FindNode(ne::NodeId id) -> Node* {
  for (auto& node : nodes_)
    if (node.ID == id) return &node;

  return nullptr;
}

auto NodesAndLinks::FindPin(ne::PinId id) -> Pin* {
  if (!id) return nullptr;

  for (auto& node : nodes_) {
    for (auto& pin : node.Inputs)
      if (pin.ID == id) return &pin;

    for (auto& pin : node.Outputs)
      if (pin.ID == id) return &pin;
  }

  return nullptr;
}

auto NodesAndLinks::FindLink(ne::LinkId id) -> Link* {
  for (auto& link : links_)
    if (link.ID == id) return &link;

  return nullptr;
}

auto NodesAndLinks::IsPinLinked(ne::PinId id) -> bool {
  if (!id) return false;

  for (auto& link : links_)
    if (link.StartPinID == id || link.EndPinID == id) return true;

  return false;
}

void NodesAndLinks::SpawnLink(const Link& link) { links_.emplace_back(link); }

void NodesAndLinks::EraseLinkWithId(ne::LinkId linkId) {
  auto id = std::find_if(links_.begin(), links_.end(),
                         [linkId](auto& link) { return link.ID == linkId; });
  if (id != links_.end()) links_.erase(id);
}

void NodesAndLinks::EraseNodeWithId(ne::NodeId id) {
  auto node =
      std::ranges::find_if(nodes_, [id](auto& node) { return node.ID == id; });
  nodes_.erase(node);
}

auto NodesAndLinks::GetSelectedNodeIds() -> std::vector<ne::NodeId> {
  const auto num_selected_objects = ne::GetSelectedObjectCount();

  auto selected_ids = std::vector<ne::NodeId>{};
  selected_ids.resize(num_selected_objects);

  const auto num_selected_nodes = ne::GetSelectedNodes(
      selected_ids.data(), static_cast<int>(selected_ids.size()));
  selected_ids.resize(num_selected_nodes);

  return selected_ids;
}

auto NodesAndLinks::GetSelectedLinkIds() -> std::vector<ne::LinkId> {
  const auto num_selected_objects = ne::GetSelectedObjectCount();

  auto selected_ids = std::vector<ne::LinkId>{};
  selected_ids.resize(num_selected_objects);

  const auto num_selected_links = ne::GetSelectedLinks(
      selected_ids.data(), static_cast<int>(selected_ids.size()));
  selected_ids.resize(num_selected_links);

  return selected_ids;
}

auto NodesAndLinks::SpawnNodeByTypeName(const std::string& type_name) -> Node* {
  if (type_name == "Input") {
    return SpawnInputNode();
  }

  if (type_name == "Client") {
    return SpawnClientNode();
  }

  if (type_name == "Comment") {
    return SpawnCommentNode();
  }

  if (type_name == "Coupler 1x2") {
    return SpawnCoupler1To2Node();
  }

  if (type_name.starts_with("Splitter 1x")) {
    const auto index_substring = std::string{
        type_name.begin() + static_cast<int>(std::string{"Splitter 1x"}.size()),
        type_name.end()};
    const auto index = std::stoi(index_substring);
    return SpawnSplitter1ToNNode(index);
  }

  cpp::Expects(false);
}

auto NodesAndLinks::GetNodeTypeNames() -> std::vector<std::string> {
  auto names =
      std::vector<std::string>{"Input", "Client", "Comment", "Coupler 1x2"};

  for (auto i : {2, 4, 8, 16}) {
    names.emplace_back("Splitter 1x" + std::to_string(i));
  }

  return names;
}

void NodesAndLinks::SpawnLinkFromPinToNode(const Pin* pin, const Node* node) {
  const auto& node_pins =
      (pin->Kind == PinKind::Input) ? node->Outputs : node->Inputs;
  const auto matching_node_pin = std::ranges::find_if(
      node_pins,
      [pin](const auto& node_pin) { return CanCreateLink(pin, &node_pin); });

  if (matching_node_pin == node_pins.end()) {
    return;
  }

  const auto is_link_starts_on_existing_node = pin->Kind == PinKind::Output;
  const auto link = Link{
      .ID = app_->GetNextLinkId(),
      .StartPinID =
          is_link_starts_on_existing_node ? pin->ID : matching_node_pin->ID,
      .EndPinID =
          is_link_starts_on_existing_node ? matching_node_pin->ID : pin->ID,
      .Color = GetIconColor(pin->Type)};

  SpawnLink(link);
}

void NodesAndLinks::SafeToFile(const std::string& file_path) {
  auto json = crude_json::value{};

  {
    json["nodes_size"] = static_cast<crude_json::number>(nodes_.size());
    auto& nodes_json = json["nodes"];
    auto node_index = 0;

    for (const auto& node : nodes_) {
      auto& node_json = nodes_json[node_index++];

      node_json["id"] = static_cast<crude_json::number>(node.ID.Get());
      node_json["name"] = node.Name;
      node_json["coupler_percentage_index"] =
          static_cast<crude_json::number>(node.coupler_percentage_index_);

      const auto pos = ne::GetNodePosition(node.ID);

      node_json["pos_x"] = pos.x;
      node_json["pos_y"] = pos.y;

      const auto size = ne::GetNodeSize(node.ID);

      node_json["size_x"] = size.x;
      node_json["size_y"] = size.y;

      {
        auto& input_pins_json = node_json["input_pins"];
        auto input_pin_index = 0;

        for (const auto& input_pin : node.Inputs) {
          auto& input_pin_json = input_pins_json[input_pin_index++];
          input_pin_json["id"] =
              static_cast<crude_json::number>(input_pin.ID.Get());
          input_pin_json["value"] = input_pin.value;
        }
      }

      {
        auto& output_pins_json = node_json["output_pins"];
        auto output_pin_index = 0;

        for (const auto& output_pin : node.Outputs) {
          auto& output_pin_json = output_pins_json[output_pin_index++];
          output_pin_json["id"] =
              static_cast<crude_json::number>(output_pin.ID.Get());
          output_pin_json["value"] = output_pin.value;
        }
      }
    }
  }

  {
    json["links_size"] = static_cast<crude_json::number>(links_.size());
    auto& links_json = json["links"];
    auto link_index = 0;

    for (const auto& link : links_) {
      auto& link_json = links_json[link_index++];

      link_json["id"] = static_cast<crude_json::number>(link.ID.Get());
      link_json["start_pin_id"] =
          static_cast<crude_json::number>(link.StartPinID.Get());
      link_json["end_pin_id"] =
          static_cast<crude_json::number>(link.EndPinID.Get());
    }
  }

  std::cout << "save:" << json.dump() << "\n";
  json.save(file_path);
}

void NodesAndLinks::DeleteAll() {
  for (const auto& link : links_) {
    ne::DeleteLink(link.ID);
  }

  links_.clear();

  for (const auto& node : nodes_) {
    ne::DeleteNode(node.ID);
  }

  nodes_.clear();
}

void NodesAndLinks::LoadFromFile(const std::string& file_path) {
  DeleteAll();

  const auto json = crude_json::value::load(file_path).first;
  const auto nodes_size = json["nodes_size"].get<crude_json::number>();
  const auto& nodes_json = json["nodes"];

  for (auto i = 0; i < nodes_size; ++i) {
    const auto& node_json = nodes_json[i];

    auto* node =
        SpawnNodeByTypeName(node_json["name"].get<crude_json::string>());
    node->coupler_percentage_index_ = static_cast<int>(
        node_json["coupler_percentage_index"].get<crude_json::number>());

    {
      const auto& input_pins = node_json["input_pins"];

      for (auto i = 0; i < static_cast<int>(node->Inputs.size()); ++i) {
        const auto& input_pin_json = input_pins[i];
        node->Inputs[i].ID = static_cast<uint64_t>(
            input_pin_json["id"].get<crude_json::number>());
        node->Inputs[i].value = static_cast<float>(
            input_pin_json["value"].get<crude_json::number>());
      }
    }

    {
      const auto& output_pins = node_json["output_pins"];

      for (auto i = 0; i < static_cast<int>(node->Outputs.size()); ++i) {
        const auto& output_pin_json = output_pins[i];
        node->Outputs[i].ID = static_cast<uint64_t>(
            output_pin_json["id"].get<crude_json::number>());
        node->Outputs[i].value = static_cast<float>(
            output_pin_json["value"].get<crude_json::number>());
      }
    }

    ne::SetNodePosition(
        node->ID,
        {static_cast<float>(node_json["pos_x"].get<crude_json::number>()),
         static_cast<float>(node_json["pos_y"].get<crude_json::number>())});

    if (node->Name == "Comment") {
      ne::SetGroupSize(
          node->ID,
          {static_cast<float>(node_json["size_x"].get<crude_json::number>()),
           static_cast<float>(node_json["size_y"].get<crude_json::number>())});
    }
  }

  const auto links_size = json["links_size"].get<crude_json::number>();
  const auto& links_json = json["links"];

  for (auto i = 0; i < links_size; ++i) {
    const auto& link_json = links_json[i];

    SpawnLink({static_cast<uint64_t>(link_json["id"].get<crude_json::number>()),
               static_cast<uint64_t>(
                   link_json["start_pin_id"].get<crude_json::number>()),
               static_cast<uint64_t>(
                   link_json["end_pin_id"].get<crude_json::number>())});
  }
}

void NodesAndLinks::OnFrame() {
  UpdateNodePointerOnPins();
  UpdatePinValues();
}

void NodesAndLinks::UpdateNodePointerOnPins() {
  for (auto& node : nodes_) {
    for (auto& input : node.Inputs) {
      input.node = &node;
    }

    for (auto& output : node.Outputs) {
      output.node = &node;
    }
  }
}

void ClearAllValuesExceptInput(std::vector<Node>& nodes_) {
  const auto& coupler_percentage_values = GetCouplerPercentageValues();

  for (auto& node : nodes_) {
    for (const auto& pins : {&node.Inputs, &node.Outputs}) {
      for (auto& pin : *pins) {
        if (!pin.editable) {
          pin.value = 0;
        }
      }
    }

    if (node.Name.starts_with("Splitter 1x")) {
      const auto index_substring =
          std::string{node.Name.begin() +
                          static_cast<int>(std::string{"Splitter 1x"}.size()),
                      node.Name.end()};
      const auto index = std::stoi(index_substring);

      static const auto kSplitterValuesMap =
          std::map<int, float>{{2, 4.3}, {4, 7.4}, {8, 10.7}, {16, 13.9}};

      node.Inputs[1].value = kSplitterValuesMap.at(index);
    } else if (node.Name == "Coupler 1x2") {
      const auto& values =
          coupler_percentage_values[node.coupler_percentage_index_];

      node.Inputs[1].value = values.first;
      node.Inputs[2].value = values.second;
    }
  }
}

void NodesAndLinks::UpdatePinValues() {
  ClearAllValuesExceptInput(nodes_);

  auto input_nodes = std::vector<Node*>{};

  for (auto& node : nodes_) {
    if (node.Name == "Input") {
      input_nodes.emplace_back(&node);
    }
  }

  auto visited_nodes =
      std::unordered_set<Node*>{input_nodes.begin(), input_nodes.end()};

  while (!input_nodes.empty()) {
    auto next_input_nodes = std::unordered_set<Node*>{};

    for (auto* input_node : input_nodes) {
      if (!visited_nodes.contains(input_node)) {
        visited_nodes.emplace(input_node);

        // for (auto& input_pin : input_node->Inputs) {
        //   if (input_pin.editable) {
        //     for (auto& output_pin : input_node->Outputs) {
        //       output_pin.value -= input_pin.value;
        //     }
        //   }
        // }

        if (input_node->Name.starts_with("Splitter 1x")) {
          const auto splitter_value = input_node->Inputs[1].value;

          for (auto& output_pin : input_node->Outputs) {
            output_pin.value -= splitter_value;
          }
        } else if (input_node->Name == "Coupler 1x2") {
          input_node->Outputs[1].value -= input_node->Inputs[1].value;
          input_node->Outputs[2].value -= input_node->Inputs[2].value;
        }
      }

      for (auto& input_node_output_pin : input_node->Outputs) {
        for (auto& link : links_) {
          if (link.StartPinID == input_node_output_pin.ID) {
            auto* end_pin = FindPin(link.EndPinID);

            if ((end_pin == nullptr) || (end_pin->node == nullptr)) {
              continue;
            }

            end_pin->value += input_node_output_pin.value;

            for (auto& output_pin : end_pin->node->Outputs) {
              output_pin.value += input_node_output_pin.value;
            }

            next_input_nodes.emplace(end_pin->node);
          }
        }
      }
    }

    input_nodes.assign(next_input_nodes.begin(), next_input_nodes.end());
  }
}
}  // namespace esc