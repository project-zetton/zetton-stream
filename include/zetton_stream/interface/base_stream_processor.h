
#pragma once

#include "zetton_stream/base/stream_options.h"

namespace zetton {
namespace stream {

class BaseStreamProcessor {
 public:
  BaseStreamProcessor() = default;
  virtual ~BaseStreamProcessor() = default;

 public:
  virtual bool Init(const StreamOptions& options) {
    options_ = options;
    return true;
  };

  virtual bool Open() = 0;
  virtual void Close() = 0;

 public:
  inline bool IsStreaming() const { return is_streaming_; }
  inline uint32_t GetWidth() const { return options_.width; }
  inline uint32_t GetHeight() const { return options_.height; }
  inline uint32_t GetFrameRate() const { return options_.frame_rate; }
  inline const StreamUri& GetResource() const { return options_.resource; }
  inline const StreamOptions& GetOptions() const { return options_; }

 protected:
  bool is_streaming_;
  StreamOptions options_;
};

}  // namespace stream
}  // namespace zetton
