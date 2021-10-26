#include "zetton_stream/interface/base_stream_output.h"

#include "zetton_common/util/log.h"

namespace zetton {
namespace stream {

bool BaseStreamOutput::Render(void *image, uint32_t width, uint32_t height) {
  const uint32_t num_outputs = outputs_.size();
  bool result = true;

  for (uint32_t n = 0; n < num_outputs; n++) {
    if (!outputs_[n]->Render(image, width, height)) {
      result = false;
    }
  }

  return result;
}

void BaseStreamOutput::SetStatus(const char *str){};

} // namespace stream
} // namespace zetton