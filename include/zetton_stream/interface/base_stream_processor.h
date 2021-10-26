
#pragma once

#include "zetton_stream/stream/stream_options.h"

namespace zetton {
namespace stream {

class BaseStreamProcessor {
 public:
  BaseStreamProcessor() = default;
  explicit BaseStreamProcessor(const StreamOptions& options)
      : is_streaming_(false), options_(options){};
  virtual ~BaseStreamProcessor() = default;

  inline bool IsStreaming() const { return is_streaming_; }
  inline uint32_t GetWidth() const { return options_.width; }
  inline uint32_t GetHeight() const { return options_.height; }
  inline uint32_t GetFrameRate() const { return options_.frame_rate; }
  inline const StreamUri& GetResource() const { return options_.resource; }
  inline const StreamOptions& GetOptions() const { return options_; }

  virtual bool Open();
  virtual void Close();

 protected:
  bool is_streaming_;
  StreamOptions options_;
};

}  // namespace stream
}  // namespace zetton