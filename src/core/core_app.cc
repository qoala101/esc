#include "core_app.h"

#include "cpp_assert.h"

namespace esc::core {
auto App::GetDiagram() const -> const Diagram & {
  // NOLINTNEXTLINE(*-const-cast)
  return const_cast<App *>(this)->GetDiagram();
}

auto App::GetDiagram() -> Diagram & {
  return diagram_;
}

void App::SetDiagram(Diagram diagram) {
  diagram_ = std::move(diagram);
}
}  // namespace esc::core