#pragma once

#include <atomic>
#include <thread>

#include "opencv2/opencv.hpp"
#include "zetton_common/thread/ring_buffer.h"
#include "zetton_stream/interface/base_stream_output.h"

namespace zetton {
namespace stream {

class CvGstStreamOutput : public BaseStreamOutput {
 public:
  ~CvGstStreamOutput() = default;

  static bool IsSupportedExtension(const char* ext);
  static const char* SupportedExtensions[];

  bool Init(const StreamOptions& options);
  bool Open() override;
  void Close() override;
  bool Render(void* image, uint32_t width, uint32_t height) override;

  void SetStatus(const char* str) override;

 protected:
  bool BuildPipelineString();
  std::string pipeline_string_;
  std::shared_ptr<cv::VideoCapture> cap_;
};

}  // namespace stream
}  // namespace zetton