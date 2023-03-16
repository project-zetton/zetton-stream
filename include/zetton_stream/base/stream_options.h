#pragma once

#include <cstdint>

#include "zetton_stream/base/stream_uri.h"

namespace zetton {
namespace stream {

enum class StreamDeviceType {
  DEVICE_DEFAULT = 0,
  DEVICE_V4L2,
  DEVICE_CSI,
  DEVICE_IP,
  DEVICE_FILE,
  DEVICE_DISPLAY,
  DEVICE_MAX_NUM
};

const char* StreamDeviceTypeToStr(StreamDeviceType type);
StreamDeviceType StreamDeviceTypeFromStr(const char* str);

enum class StreamIoType { IO_INPUT = 0, IO_OUTPUT, IO_MAX_NUM };

const char* StreamIoTypeToStr(StreamIoType type);
StreamIoType StreamIoTypeFromStr(const char* str);

enum class StreamIoMethod {
  IO_METHOD_UNKNOWN = 0,
  IO_METHOD_READ,
  IO_METHOD_MMAP,
  IO_METHOD_USERPTR,
  IO_METHOD_MAX_NUM
};

const char* StreamIoMethodToStr(StreamIoMethod type);
StreamIoMethod StreamIoMethodFromStr(const char* str);

enum class StreamFlipMethod {
  FLIP_NONE = 0,
  FLIP_COUNTERCLOCKWISE,
  FLIP_ROTATE_180,
  FLIP_CLOCKWISE,
  FLIP_HORIZONTAL,
  FLIP_UPPER_RIGHT_DIAGONAL,
  FLIP_VERTICAL,
  FLIP_UPPER_LEFT_DIAGONAL,
  FLIP_MAX_NUM
};

const char* StreamFlipMethodToStr(StreamFlipMethod flip);
StreamFlipMethod StreamFlipMethodFromStr(const char* str);

enum class StreamCodec {
  CODEC_UNKNOWN = 0,
  CODEC_RAW,
  CODEC_H264,
  CODEC_H265,
  CODEC_VP8,
  CODEC_VP9,
  CODEC_MPEG2,
  CODEC_MPEG4,
  CODEC_MJPEG,
  CODEC_MAX_NUM
};

const char* StreamCodecToStr(StreamCodec codec);
StreamCodec StreamCodecFromStr(const char* str);

enum class StreamPixelFormat {
  PIXEL_FORMAT_UNKNOWN = 0,
  PIXEL_FORMAT_RGB,
  PIXEL_FORMAT_RGB16,
  PIXEL_FORMAT_RGBA,
  PIXEL_FORMAT_RGBA16,
  PIXEL_FORMAT_BGR,
  PIXEL_FORMAT_BGR16,
  PIXEL_FORMAT_BGRA,
  PIXEL_FORMAT_BGRA16,
  PIXEL_FORMAT_GRAY8,
  PIXEL_FORMAT_GRAY16_LE,
  PIXEL_FORMAT_YUYV,
  PIXEL_FORMAT_UYVY,
  PIXEL_FORMAT_MJPEG,
  PIXEL_FORMAT_YUVMONO10,
  PIXEL_FORMAT_NV12,
  PIXEL_FORMAT_MAX_NUM
};

const char* StreamPixelFormatToStr(StreamPixelFormat pixel_format);
StreamPixelFormat StreamPixelFormatFromStr(const char* str);

enum class StreamPlatformType {
  PLATFORM_CPU = 0,
  PLATFORM_GPU,
  PLATFORM_JETSON,
  PLATFORM_ROCKCHIP,
  PLATFORM_MAX_NUM
};

const char* StreamPlatformTypeToStr(StreamPlatformType platform);
StreamPlatformType StreamPlatformTypeFromStr(const char* str);

struct CameraSourceOptions {
  int brightness = -1;
  int contrast = -1;
  int saturation = -1;
  int sharpness = -1;
  int gain = -1;
  int white_balance = -1;
  int exposure = -1;
  int focus = -1;
  bool auto_white_balance = false;
  bool auto_exposure = false;
  bool auto_focus = false;
};

struct StreamOptions {
 public:
  StreamUri resource;
  uint32_t width;
  uint32_t height;
  uint32_t channels;
  float frame_rate;
  uint32_t bit_rate;
  uint32_t num_buffers;
  bool zero_copy;
  int loop;
  bool async;

  StreamDeviceType device_type;
  StreamIoType io_type;
  StreamIoMethod io_method;
  StreamFlipMethod flip_method;
  StreamCodec codec;
  StreamPixelFormat pixel_format;
  StreamPixelFormat output_format;
  StreamPlatformType platform;

  CameraSourceOptions camera;

 public:
  StreamOptions();
};

}  // namespace stream
}  // namespace zetton
