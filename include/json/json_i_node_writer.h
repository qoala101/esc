#ifndef VH_JSON_I_NODE_WRITER_H_
#define VH_JSON_I_NODE_WRITER_H_

#include <memory>

#include "core_i_node.h"
#include "cpp_interface.h"
#include "crude_json.h"

namespace esc::json {
///
class INodeWriter : public cpp::Interface {
 public:
  ///
  auto WriteToJson(const core::INode &node) const -> crude_json::value;

 private:
  ///
  virtual auto WriteToJson() const -> crude_json::value;
};
}  // namespace esc::json

#endif  // VH_JSON_I_NODE_WRITER_H_
