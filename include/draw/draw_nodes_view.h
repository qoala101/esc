#ifndef VH_DRAW_NODES_VIEW_H_
#define VH_DRAW_NODES_VIEW_H_

#include "draw_i_drawable.h"

namespace esc::draw {
class NodesView : public IDrawable {
 public:
  void Draw() override;
};
}  // namespace esc::draw

#endif  // VH_DRAW_NODES_VIEW_H_