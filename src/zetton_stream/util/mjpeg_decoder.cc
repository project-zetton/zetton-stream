#include "zetton_stream/util/mjpeg_decoder.h"

#include "zetton_common/log/log.h"

namespace zetton {
namespace stream {

MjpegDecoder::MjpegDecoder()
    : avframe_camera_(nullptr),
      avframe_rgb_(nullptr),
      avcodec_(nullptr),
      avoptions_(nullptr),
      avcodec_context_(nullptr),
      avframe_camera_size_(0),
      avframe_rgb_size_(0),
      video_sws_(nullptr) {}

MjpegDecoder::~MjpegDecoder() {
  if (avcodec_context_) {
    avcodec_close(avcodec_context_);
    av_free(avcodec_context_);
    avcodec_context_ = nullptr;
  }
  if (avframe_camera_) av_free(avframe_camera_);
  avframe_camera_ = nullptr;
  if (avframe_rgb_) av_free(avframe_rgb_);
  avframe_rgb_ = nullptr;
}

bool MjpegDecoder::Init(int image_width, int image_height) {
  avcodec_register_all();

  avcodec_ = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
  if (!avcodec_) {
    AERROR_F("Could not find MJPEG decoder");
    return false;
  }

  avcodec_context_ = avcodec_alloc_context3(avcodec_);

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 0, 0)
  avframe_camera_ = av_frame_alloc();
  avframe_rgb_ = av_frame_alloc();
  avpicture_alloc(reinterpret_cast<AVPicture*>(avframe_rgb_), AV_PIX_FMT_RGB24,
                  image_width, image_height);
#else
  avframe_camera_ = avcodec_alloc_frame();
  avframe_rgb_ = avcodec_alloc_frame();
  avpicture_alloc(reinterpret_cast<AVPicture*>(avframe_rgb_), PIX_FMT_RGB24,
                  image_width, image_height);
#endif

  avcodec_context_->codec_id = AV_CODEC_ID_MJPEG;
  avcodec_context_->width = image_width;
  avcodec_context_->height = image_height;

#if LIBAVCODEC_VERSION_MAJOR > 52
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 0, 0)
  avcodec_context_->pix_fmt = AV_PIX_FMT_YUV422P;
#else
  avcodec_context_->pix_fmt = PIX_FMT_YUV422P;
#endif
  avcodec_context_->codec_type = AVMEDIA_TYPE_VIDEO;
#endif

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 0, 0)
  avframe_camera_size_ =
      avpicture_get_size(AV_PIX_FMT_YUV422P, image_width, image_height);
  avframe_rgb_size_ =
      avpicture_get_size(AV_PIX_FMT_RGB24, image_width, image_height);
#else
  avframe_camera_size_ =
      avpicture_get_size(PIX_FMT_YUV422P, image_width, image_height);
  avframe_rgb_size_ =
      avpicture_get_size(PIX_FMT_RGB24, image_width, image_height);
#endif

  /* open it */
  if (avcodec_open2(avcodec_context_, avcodec_, &avoptions_) < 0) {
    AERROR_F("Could not open MJPEG Decoder");
    return false;
  }
  return true;
}

bool MjpegDecoder::ToRGB(char* mjpeg_buffer, int len, char* rgb_buffer,
                         int NumPixels) {
  int got_picture = 0;

  memset(rgb_buffer, 0, avframe_rgb_size_);

#if LIBAVCODEC_VERSION_MAJOR > 52
  int decoded_len;
  AVPacket avpkt;
  av_init_packet(&avpkt);

  avpkt.size = len;
  avpkt.data = (unsigned char*)mjpeg_buffer;
  decoded_len = avcodec_decode_video2(avcodec_context_, avframe_camera_,
                                      &got_picture, &avpkt);

  if (decoded_len < 0) {
    AERROR_F("Error while decoding frame.");
    return false;
  }
#else
  avcodec_decode_video(avcodec_context_, avframe_camera_, &got_picture,
                       reinterpret_cast<uint8_t*>(mjpeg_buffer), len);
#endif

  if (!got_picture) {
    AERROR_F("Camera: expected picture but didn't get it...");
    return false;
  }

  int xsize = avcodec_context_->width;
  int ysize = avcodec_context_->height;
  int pic_size = avpicture_get_size(avcodec_context_->pix_fmt, xsize, ysize);
  if (pic_size != avframe_camera_size_) {
    AERROR_F("outbuf size mismatch. pic_size: {} bufsize: {}", pic_size,
             avframe_camera_size_);
    return false;
  }

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 0, 0)
  video_sws_ =
      sws_getContext(xsize, ysize, avcodec_context_->pix_fmt, xsize, ysize,
                     AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);
#else
  video_sws_ =
      sws_getContext(xsize, ysize, avcodec_context_->pix_fmt, xsize, ysize,
                     PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);
#endif

  sws_scale(video_sws_, avframe_camera_->data, avframe_camera_->linesize, 0,
            ysize, avframe_rgb_->data, avframe_rgb_->linesize);
  sws_freeContext(video_sws_);

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 0, 0)
  int size = avpicture_layout(
      reinterpret_cast<AVPicture*>(avframe_rgb_), AV_PIX_FMT_RGB24, xsize,
      ysize, reinterpret_cast<uint8_t*>(rgb_buffer), avframe_rgb_size_);
#else
  int size = avpicture_layout(
      reinterpret_cast<AVPicture*>(avframe_rgb_), PIX_FMT_RGB24, xsize, ysize,
      reinterpret_cast<uint8_t*>(rgb_buffer), avframe_rgb_size_);
#endif
  if (size != avframe_rgb_size_) {
    AERROR_F("webcam: avpicture_layout error: {}", size);
    return false;
  }

  return true;
}

}  // namespace stream
}  // namespace zetton
