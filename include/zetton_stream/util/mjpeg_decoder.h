#pragma once

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

namespace zetton {
namespace stream {

class MjpegDecoder {
 public:
  MjpegDecoder();
  ~MjpegDecoder();

 public:
  bool Init(int image_width, int image_height);
  bool ToRGB(char* mjpeg_buffer, int len, char* rgb_buffer, int NumPixels);

 private:
  AVFrame* avframe_camera_;
  AVFrame* avframe_rgb_;
  AVCodec* avcodec_;
  AVDictionary* avoptions_;
  AVCodecContext* avcodec_context_;
  int avframe_camera_size_;
  int avframe_rgb_size_;
  struct SwsContext* video_sws_;
};
}  // namespace stream
}  // namespace zetton
