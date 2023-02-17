﻿#ifndef VH_APP_NODE_DRAWER_H_
#define VH_APP_NODE_DRAWER_H_

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_node_editor.h>

#include <optional>

#include "app_textures.h"
#include "cpp_scope_function.h"

namespace ne = ax::NodeEditor;

namespace esc {
class NodeDrawer {
 public:
  explicit NodeDrawer(ne::NodeId node_id);

  NodeDrawer(const NodeDrawer&) = delete;
  NodeDrawer(NodeDrawer&&) noexcept = default;

  auto operator=(const NodeDrawer&) noexcept -> NodeDrawer& = delete;
  auto operator=(NodeDrawer&&) noexcept -> NodeDrawer& = default;

  ~NodeDrawer();

  auto AddHeader(const draw::Texture& header_texture,
                 const ImColor& header_color) -> cpp::ScopeFunction;

  auto AddPin(ne::PinId id, ne::PinKind kind) -> cpp::ScopeFunction;

 private:
  enum class Stage {
    kInvalid,
    kBegin,
    kHeader,
    kContent,
    kInput,
    kOutput,
    kEnd
  };

  struct Header {
    draw::Texture texture{};
    ImColor color{};
    ImRect rect{};
  };

  void ProcessOldStage(Stage old_stage);
  void ProcessNewStage(Stage old_stage);
  void SetStage(Stage stage);

  void DrawHeader();

  ne::NodeId node_id_{};
  Stage stage_{};
  std::optional<Header> header_{};
};
}  // namespace esc

#endif  // VH_APP_NODE_DRAWER_H_