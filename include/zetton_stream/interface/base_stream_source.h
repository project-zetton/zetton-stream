#pragma once

#include "zetton_stream/base/stream_options.h"
#include "zetton_stream/interface/base_stream_processor.h"

namespace zetton {
namespace stream {

enum StreamSourceType {
  SOURCE_UNKNOWN = 0,
  SOURCE_CAMERA,
  SOURCE_VIDEO,
  SOURCE_IMAGE,
  SOURCE_MAX_NUM,
};

const char* StreamProtocolTypeToStr(StreamProtocolType type);
StreamProtocolType StreamProtocolTypeFromStr(const char* str);

class BaseStreamSource : public BaseStreamProcessor {
 public:
  virtual inline StreamSourceType GetType() const {
    return StreamSourceType::SOURCE_UNKNOWN;
  }
};

}  // namespace stream
}  // namespace zetton
