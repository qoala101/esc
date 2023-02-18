#ifndef VH_COREUI_I_FAMILY_DRAWER_H_
#define VH_COREUI_I_FAMILY_DRAWER_H_

#include <string>

#include "core_i_family.h"
#include "cpp_interface.h"
#include "imgui.h"

namespace esc::coreui {
// ---
class IFamilyDrawer : public cpp::Interface {
 public:
  // ---
  virtual auto GetLabel [[nodiscard]] () const -> std::string = 0;
  // ---
  virtual auto GetColor [[nodiscard]] () const -> ImColor = 0;
  // ---
  virtual auto GetGroupLabel [[nodiscard]] () const -> std::string;
  // ---
  virtual auto IsUserAccessible [[nodiscard]] () -> bool;
};

// ---
auto GroupByLabels(const std::vector<std::shared_ptr<core::IFamily>> &families)
    -> std::vector<
        std::pair<std::string, std::vector<std::shared_ptr<core::IFamily>>>>;
}  // namespace esc::coreui

#endif  // VH_COREUI_I_FAMILY_DRAWER_H_
