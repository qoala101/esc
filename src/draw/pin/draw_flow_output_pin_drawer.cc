#include "draw_flow_output_pin_drawer.h"

namespace esc::draw {
FlowOutputPinDrawer::FlowOutputPinDrawer(float value) : value_{value} {}

auto FlowOutputPinDrawer::GetLabel() const -> std::string { return {}; }

auto FlowOutputPinDrawer::GetKind() const -> ne::PinKind {
  return ne::PinKind::Output;
}

auto FlowOutputPinDrawer::GetFloat() -> float* { return &value_; }

auto FlowOutputPinDrawer::IsEditable() const -> bool { return false; }
}  // namespace esc::draw