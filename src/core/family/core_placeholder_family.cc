#include "core_placeholder_family.h"

#include <ranges>

#include <memory>
#include <string>
#include <vector>

#include "core_i_family.h"
#include "core_i_node.h"
#include "core_id_generator.h"
#include "core_link.h"
#include "cpp_assert.h"
#include "crude_json.h"
#include "draw_flow_input_pin_drawer.h"
#include "draw_flow_output_pin_drawer.h"
#include "draw_i_family_drawer.h"
#include "draw_i_node_drawer.h"
#include "draw_i_pin_drawer.h"
#include "esc_state.h"
#include "imgui_node_editor.h"
#include "json_i_family_writer.h"
#include "json_i_node_parser.h"
#include "json_i_node_writer.h"

namespace esc::core {
namespace {
class Node;

constexpr auto kTypeName = "PlaceholderNode";

auto CreateNodeWriter(std::shared_ptr<Node> node)
    -> std::unique_ptr<json::INodeWriter>;
auto CreateNodeDrawer(std::shared_ptr<Node> node, const State& state)
    -> std::unique_ptr<draw::INodeDrawer>;
auto CreateFamilyWriter(std::shared_ptr<PlaceholderFamily> family)
    -> std::unique_ptr<json::IFamilyWriter>;
auto CreateFamilyDrawer(std::shared_ptr<PlaceholderFamily> family)
    -> std::unique_ptr<draw::IFamilyDrawer>;

// NOLINTNEXTLINE(*-multiple-inheritance)
class Node : public INode, public std::enable_shared_from_this<Node> {
 public:
  Node(ne::NodeId id, std::vector<ne::PinId> pin_ids, bool has_input_pin)
      : INode{id, std::move(pin_ids)}, has_input_pin_{has_input_pin} {}

  auto CreateWriter() -> std::unique_ptr<json::INodeWriter> override {
    return CreateNodeWriter(shared_from_this());
  }

  auto CreateDrawer(const State& state)
      -> std::unique_ptr<draw::INodeDrawer> override {
    return CreateNodeDrawer(shared_from_this(), state);
  }

  auto GetInitialFlow [[nodiscard]] () const -> Flow override {
    const auto& pin_ids = GetPinIds();

    auto flow_values = Flow{};

    if (has_input_pin_) {
      flow_values.input_pin_flow = std::pair{pin_ids[0].Get(), float{}};

      for (auto i = 0; i < static_cast<int>(pin_ids.size() - 1); ++i) {
        flow_values.output_pin_flows.emplace(pin_ids[i + 1], float{});
      }
    } else {
      for (auto pin_id : pin_ids) {
        flow_values.output_pin_flows.emplace(pin_id, float{});
      }
    }

    return flow_values;
  }

  bool has_input_pin_{};
};

class NodeParser : public json::INodeParser {
 private:
  auto ParseFromJson(ne::NodeId parsed_node_id,
                     std::vector<ne::PinId> parsed_pin_ids,
                     const crude_json::value& json) const
      -> std::shared_ptr<INode> override {
    return std::make_shared<Node>(
        parsed_node_id, std::move(parsed_pin_ids),
        json["has_input_pin"].get<crude_json::boolean>());
  }
};

class NodeWriter : public json::INodeWriter {
 public:
  explicit NodeWriter(std::shared_ptr<Node> node) : node_{std::move(node)} {}

 private:
  auto GetTypeName() const -> std::string override { return kTypeName; }

  auto WriteToJson() const -> crude_json::value override {
    auto json = crude_json::value{};
    json["has_input_pin"] =
        static_cast<crude_json::boolean>(node_->has_input_pin_);
    return json;
  }

  std::shared_ptr<Node> node_{};
};

auto CreateNodeWriter(std::shared_ptr<Node> node)
    -> std::unique_ptr<json::INodeWriter> {
  return std::make_unique<NodeWriter>(std::move(node));
}

class NodeDrawer : public draw::INodeDrawer {
 public:
  explicit NodeDrawer(std::shared_ptr<Node> node, const State& state)
      : node_{std::move(node)},
        flow_pin_values_{state.flow_calculator_.GetCalculatedFlow(*node_)} {}

  auto GetLabel() const -> std::string override {
    return std::make_shared<PlaceholderFamily>()->CreateDrawer()->GetLabel();
  }

  auto GetColor() const -> ImColor override {
    return std::make_shared<PlaceholderFamily>()->CreateDrawer()->GetColor();
  }

  auto CreatePinDrawer(ne::PinId pin_id) const
      -> std::unique_ptr<draw::IPinDrawer> override {
    const auto pin_index = node_->GetPinIndex(pin_id);

    if (node_->has_input_pin_ && (pin_index == 0)) {
      return std::make_unique<draw::FlowInputPinDrawer>(flow_pin_values_);
    }

    return std::make_unique<draw::FlowOutputPinDrawer>(flow_pin_values_,
                                                       pin_id);
  }

 private:
  std::shared_ptr<Node> node_{};
  Flow flow_pin_values_{};
};

auto CreateNodeDrawer(std::shared_ptr<Node> node, const State& state)
    -> std::unique_ptr<draw::INodeDrawer> {
  return std::make_unique<NodeDrawer>(std::move(node), state);
}
}  // namespace

PlaceholderFamily::PlaceholderFamily(std::vector<std::shared_ptr<INode>> nodes)
    : IFamily{std::move(nodes)} {}

auto PlaceholderFamily::CreateNode(IdGenerator& id_generator)
    -> std::shared_ptr<INode> {
  cpp::Expects(false);
  return {};
}

auto PlaceholderFamily::CreateNodeParser()
    -> std::unique_ptr<json::INodeParser> {
  return std::make_unique<NodeParser>();
}

auto PlaceholderFamily::CreateWriter() -> std::unique_ptr<json::IFamilyWriter> {
  return CreateFamilyWriter(shared_from_this());
}

auto PlaceholderFamily::CreateDrawer() -> std::unique_ptr<draw::IFamilyDrawer> {
  return CreateFamilyDrawer(shared_from_this());
}

auto PlaceholderFamily::EmplaceNodeFromFlow(IdGenerator& id_generator,
                                            const Flow& connected_flow)
    -> INode& {
  // auto pin_ids = std::vector<ne::PinId>{};
  const auto has_input_pin = connected_flow.input_pin_flow.has_value();
  auto num_pins = static_cast<int>(connected_flow.output_pin_flows.size());

  if (has_input_pin) {
    num_pins += 1;
  }

  return EmplaceNode(std::make_shared<Node>(
      id_generator.GetNext<ne::NodeId>(),
      id_generator.GetNextN<ne::PinId>(num_pins), has_input_pin));
}

namespace {
class FamilyParser : public json::IFamilyParser {
 public:
  auto GetTypeName() const -> std::string override { return kTypeName; }

  auto ParseFromJson(std::vector<std::shared_ptr<INode>> parsed_nodes,
                     const crude_json::value& json) const
      -> std::shared_ptr<IFamily> override {
    return std::make_shared<PlaceholderFamily>(std::move(parsed_nodes));
  }
};

class FamilyWriter : public json::IFamilyWriter {
 public:
  explicit FamilyWriter(std::shared_ptr<PlaceholderFamily> family)
      : family_{std::move(family)} {}

  auto GetTypeName() const -> std::string override { return kTypeName; }

  auto WriteToJson() const -> crude_json::value override { return {}; }

 private:
  std::shared_ptr<PlaceholderFamily> family_{};
};

auto CreateFamilyWriter(std::shared_ptr<PlaceholderFamily> family)
    -> std::unique_ptr<json::IFamilyWriter> {
  return std::make_unique<FamilyWriter>(std::move(family));
}

class FamilyDrawer : public draw::IFamilyDrawer {
 public:
  explicit FamilyDrawer(std::shared_ptr<PlaceholderFamily> family)
      : family_{std::move(family)} {}

  auto GetLabel() const -> std::string override { return "Placeholder"; }

  auto GetColor() const -> ImColor override { return {255, 255, 255}; }

  auto IsUserAccessible [[nodiscard]] () -> bool { return false; }

 private:
  std::shared_ptr<PlaceholderFamily> family_{};
};

auto CreateFamilyDrawer(std::shared_ptr<PlaceholderFamily> family)
    -> std::unique_ptr<draw::IFamilyDrawer> {
  return std::make_unique<FamilyDrawer>(std::move(family));
}
}  // namespace

auto PlaceholderFamily::CreateParser() -> std::unique_ptr<json::IFamilyParser> {
  return std::make_unique<FamilyParser>();
}
}  // namespace esc::core