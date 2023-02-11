#ifndef VH_ESC_STATE_H_
#define VH_ESC_STATE_H_

#include "core_app.h"
#include "core_id_generator.h"

namespace esc {
// ---
class State {
 public:
  // ---
  State();

  // ---
  void OnFrame() const;
  // ---
  void OpenDiagramFromFile(const std::string &file_path);
  // ---
  void SaveDiagramToFile(const std::string &file_path) const;
  // ---
  void ResetDiagram();

  //  private:
  core::App app_{};
  std::optional<core::IdGenerator> id_generator_{};
};
}  // namespace esc

#endif  // VH_ESC_STATE_H_
