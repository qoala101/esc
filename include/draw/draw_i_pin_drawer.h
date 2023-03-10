#ifndef VH_DRAW_I_PIN_DRAWER_H_
#define VH_DRAW_I_PIN_DRAWER_H_

#include <imgui_node_editor.h>

#include <string>

#include "cpp_interface.h"

namespace ne = ax::NodeEditor;

namespace esc::draw {
// ---
class IPinDrawer : public cpp::Interface {
 public:
  // ---
  virtual auto GetLabel [[nodiscard]] () const -> std::string;
  // ---
  virtual auto GetKind [[nodiscard]] () const -> ne::PinKind = 0;
  // ---
  virtual auto GetFloat [[nodiscard]] () -> float*;
  // ---
  virtual auto IsEditable [[nodiscard]] () const -> bool;
};
}  // namespace esc::draw

#endif  // VH_DRAW_I_PIN_DRAWER_H_
