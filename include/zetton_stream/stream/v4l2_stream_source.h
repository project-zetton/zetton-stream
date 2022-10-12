#pragma once

#include <asm/types.h> /* for videodev2.h */
#include <malloc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "zetton_stream/stream/stream_options.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/mem.h>
#include <libswscale/swscale.h>
#include <linux/videodev2.h>
}

#include <libavcodec/version.h>
#if LIBAVCODEC_VERSION_MAJOR < 55
#define AV_CODEC_ID_MJPEG CODEC_ID_MJPEG
#endif

#include <memory>
#include <sstream>
#include <string>

#include "zetton_stream/base/frame.h"
#include "zetton_stream/interface/base_stream_source.h"

namespace zetton {
namespace stream {

class V4l2StreamSource {
 public:
  V4l2StreamSource();
  ~V4l2StreamSource();

 public:
  bool Init(const StreamOptions& options);
  bool Open();
  void Close();

 public:
  // user use this function to get camera frame data
  virtual bool poll(const CameraImagePtr& raw_image);

  bool is_capturing();
  bool wait_for_device();

 private:
  bool init_device();
  bool uninit_device();

  void set_device_config();
  // enables/disable auto focus
  void set_auto_focus(int value);
  // set video device parameters
  void set_v4l_parameter(const std::string& param, int value);
  void set_v4l_parameter(const std::string& param, const std::string& value);

  int init_mjpeg_decoder(int image_width, int image_height);
  void mjpeg2rgb(char* mjepg_buffer, int len, char* rgb_buffer, int pixels);

  bool init_read(unsigned int buffer_size);
  bool init_mmap();
  bool init_userp(unsigned int buffer_size);
  bool close_device();
  bool open_device();
  bool read_frame(CameraImagePtr raw_image);
  bool process_image(void* src, int len, CameraImagePtr dest);
  bool start_capturing();
  bool stop_capturing();
  void reconnect();
  void reset_device();
  void shutdown();

 private:
  StreamOptions options_;

  unsigned int pixel_format_;
  bool monochrome_;
  int fd_;
  CameraBuffer* buffers_;
  unsigned int n_buffers_;

  bool is_capturing_;
  uint64_t image_seq_;

  AVFrame* avframe_camera_;
  AVFrame* avframe_rgb_;
  AVCodec* avcodec_;
  AVDictionary* avoptions_;
  AVCodecContext* avcodec_context_;
  int avframe_camera_size_;
  int avframe_rgb_size_;
  struct SwsContext* video_sws_;

  float frame_warning_interval_ = 0.0;
  float device_wait_sec_ = 0.0;
  uint64_t last_nsec_ = 0;
  float frame_drop_interval_ = 0.0;
};

}  // namespace stream
}  // namespace zetton
