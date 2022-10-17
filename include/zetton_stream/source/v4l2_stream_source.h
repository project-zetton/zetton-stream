#pragma once

#include "zetton_stream/base/frame.h"
#include "zetton_stream/base/stream_options.h"
#include "zetton_stream/util/mjpeg_decoder.h"
#include "zetton_stream/util/v4l/cv4l-helpers.h"
#include "zetton_stream/util/v4l2.h"

namespace zetton {
namespace stream {

class V4l2StreamSource {
 public:
  V4l2StreamSource();
  ~V4l2StreamSource();

 public:
  bool Init(const StreamOptions& options);
  bool WaitForDevice();
  bool Capture(const CameraImagePtr& raw_image);

 public:
  bool IsCaptuering();

 private:
  void Shutdown();

  bool OpenDevice();
  bool CloseDevice();
  bool InitDevice();
  bool UninitDevice();
  bool StartCapturing();
  bool StopCapturing();

  bool ReadFrame(CameraImagePtr raw_image);
  bool ProcessImage(std::array<void*, VIDEO_MAX_PLANES> mplane_data,
                    std::array<unsigned int, VIDEO_MAX_PLANES> mplane_size,
                    CameraImagePtr dest);

 private:
  StreamOptions options_;
  MjpegDecoder mjpeg_decoder_;

  cv4l_fd fd_;
  cv4l_queue* buffers_;
  unsigned int n_buffers_;
  unsigned int pixel_format_;

  bool is_capturing_;
  bool monochrome_;
};

}  // namespace stream
}  // namespace zetton
