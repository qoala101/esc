#ifndef VH_CORE_ID_GENERATOR_H_
#define VH_CORE_ID_GENERATOR_H_

#include <imgui_node_editor.h>

#include <vector>

namespace esc::core {
// ---
class IdGenerator {
 public:
  // ---
  explicit IdGenerator(uintptr_t next_value = 1);

  // ---
  template <typename T>
  auto GetNext() {
    return T{next_value_++};
  }

  // ---
  template <typename T>
  auto GetNextN(int n) {
    auto ids = std::vector<T>{};
    ids.reserve(n);

    for (auto i = 0; i < n; ++i) {
      ids.emplace_back(GetNext<T>());
    }

    return ids;
  }

 private:
  uintptr_t next_value_{};
};
}  // namespace esc::core

#endif  // VH_CORE_ID_GENERATOR_H_
