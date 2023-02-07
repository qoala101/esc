#ifndef VH_ESC_APP_H_
#define VH_ESC_APP_H_

#include <application.h>
#include <imgui_node_editor.h>

#include <memory>

#include "esc_auto_incrementable.h"
#include "esc_nodes_and_links.h"
#include "esc_textures_handle.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "esc_editor_context_handle.h"
#include "esc_types.h"

namespace ne = ax::NodeEditor;

// NOLINTNEXTLINE(*-multiple-inheritance)
class App : public Application, public std::enable_shared_from_this<App> {
 public:
  App(const char* name, int argc, char** argv);

  App(const App&) = delete;
  App(App&&) noexcept = delete;

  auto operator=(const App&) noexcept -> App& = delete;
  auto operator=(App&&) noexcept -> App& = delete;

  virtual ~App() = default;

 private:
  auto GetNextLinkId() -> ne::LinkId;

  void OnStart() override;
  void OnStop() override;
  void OnFrame(float deltaTime) override;

  void DrawLeftPane(float paneWidth);
  void DrawFrame();

  //

  auto CreateEditorConfig() -> ne::Config;

  void AddInitialNodes();
  void AddInitialLinks();

  std::optional<esc::EditorContextHandle> editor_context_{};
  std::optional<esc::TexturesHandle> textures_{};

  std::shared_ptr<esc::AutoIncrementable> auto_incrementable_object_id_{};

  esc::NodesAndLinks nodes_and_links_;

  bool show_ordinals_{};
};

#endif  // VH_ESC_APP_H_
