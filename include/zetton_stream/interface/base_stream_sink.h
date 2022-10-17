#pragma once

#include <vector>

#include "zetton_stream/base/stream_options.h"
#include "zetton_stream/interface/base_stream_processor.h"

namespace zetton {
namespace stream {

class BaseStreamSink : public BaseStreamProcessor {
 public:
  template <typename T>
  bool Render(T* image, uint32_t width, uint32_t height) {
    return Render((void**)image, width, height);
  }

  virtual bool Render(void* image, uint32_t width, uint32_t height);

 public:
  inline void AddOutput(BaseStreamSink* output) {
    if (output != nullptr) outputs_.push_back(output);
  }

  inline uint32_t GetNumOutputs(BaseStreamSink* output) const {
    return outputs_.size();
  }

  inline BaseStreamSink* GetOutput(uint32_t index) const {
    return outputs_[index];
  }

  virtual void SetStatus(const char* str);

 protected:
  std::vector<BaseStreamSink*> outputs_;
};

}  // namespace stream
}  // namespace zetton
