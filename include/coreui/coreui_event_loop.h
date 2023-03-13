#ifndef VH_COREUI_EVENT_LOOP_H_
#define VH_COREUI_EVENT_LOOP_H_

#include <functional>

namespace esc::coreui {
///
using Event = std::function<void()>;

///
class EventLoop {
 public:
  ///
  void PostEvent(Event event);
  ///
  void PostLateEvent(Event event);
  ///
  void ExecuteEvents();

 private:
  ///
  std::vector<Event> events_{};
  ///
  std::vector<Event> late_events_{};
};
}  // namespace esc::coreui

#endif  // VH_COREUI_EVENT_LOOP_H_
