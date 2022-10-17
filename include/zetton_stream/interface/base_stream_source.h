#pragma once

#include "zetton_stream/base/frame.h"
#include "zetton_stream/base/stream_options.h"
#include "zetton_stream/interface/base_stream_processor.h"

namespace zetton {
namespace stream {

class BaseStreamSource : public BaseStreamProcessor {
 public:
  virtual bool Capture(const CameraImagePtr& raw_image) = 0;
};

}  // namespace stream
}  // namespace zetton
