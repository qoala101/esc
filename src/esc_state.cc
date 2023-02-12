#include "esc_state.h"

#include <memory>
#include <utility>

#include "core_app.h"
#include "core_diagram.h"
#include "core_id_generator.h"
#include "crude_json.h"
#include "impl_attenuator_node.h"
#include "impl_client_node.h"
#include "impl_coupler_node.h"
#include "impl_hub_node.h"
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
  families.emplace_back(impl::HubNode::CreateFamily());

  return families;
}

auto CreateFamilyParsers() {
  auto family_parsers = std::vector<std::unique_ptr<json::IFamilyParser>>{};
  family_parsers.emplace_back(impl::InputNode::CreateFamilyParser());
  family_parsers.emplace_back(impl::ClientNode::CreateFamilyParser());
  family_parsers.emplace_back(impl::CouplerNode::CreateFamilyParser());
  family_parsers.emplace_back(impl::SplitterNode::CreateFamilyParser());
  family_parsers.emplace_back(impl::AttenuatorNode::CreateFamilyParser());
  family_parsers.emplace_back(impl::HubNode::CreateFamilyParser());
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
  const auto json = crude_json::value::load(file_path).first;

  auto diagram =
      json::DiagramSerializer::ParseFromJson(json, CreateFamilyParsers());
  auto max_id = FindMaxId(diagram);

  state.app_.SetDiagram(std::move(diagram));
  state.id_generator_ = core::IdGenerator{max_id};
}

void State::SaveDiagramToFile(const State &state,
                              const std::string &file_path) {
  const auto &diagram = state.app_.GetDiagram();
  const auto json = json::DiagramSerializer::WriteToJson(diagram);
  json.save(file_path);
}

void State::ResetDiagram(State &state) {
  state.app_.SetDiagram(core::Diagram{CreateFamilies()});
  state.id_generator_ = core::IdGenerator{};
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