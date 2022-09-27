#include "zetton_stream/interface/base_stream_processor.h"

namespace zetton {
namespace stream {

bool BaseStreamProcessor::Open() {
  is_streaming_ = true;
  return true;
}

void BaseStreamProcessor::Close() { is_streaming_ = false; }

}  // namespace stream
}  // namespace zetton
