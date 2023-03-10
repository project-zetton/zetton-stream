#include "zetton_stream/base/stream_options.h"

#include <cstring>

namespace zetton {
namespace stream {

const char* StreamIoTypeToStr(StreamIoType type) {
  switch (type) {
    case StreamIoType::IO_INPUT:
      return "input";
    case StreamIoType::IO_OUTPUT:
      return "output";
    default:
      return "input";
  }
}

StreamIoType StreamIoTypeFromStr(const char* str) {
  if (!str) return StreamIoType::IO_INPUT;
  for (int n = 0; n < static_cast<int>(StreamIoType::IO_MAX_NUM); ++n) {
    const auto value = (StreamIoType)n;
    if (strcasecmp(str, StreamIoTypeToStr(value)) == 0) return value;
  }
  return StreamIoType::IO_INPUT;
}

const char* StreamIoMethodToStr(StreamIoMethod type) {
  switch (type) {
    case StreamIoMethod::IO_METHOD_UNKNOWN:
      return "unknown";
    case StreamIoMethod::IO_METHOD_READ:
      return "read";
    case StreamIoMethod::IO_METHOD_MMAP:
      return "mmap";
    case StreamIoMethod::IO_METHOD_USERPTR:
      return "userptr";
    default:
      return "unknown";
  }
}

StreamIoMethod StreamIoMethodFromStr(const char* str) {
  if (!str) return StreamIoMethod::IO_METHOD_UNKNOWN;
  for (int n = 0; n < static_cast<int>(StreamIoMethod::IO_METHOD_MAX_NUM);
       ++n) {
    const auto value = (StreamIoMethod)n;
    if (strcasecmp(str, StreamIoMethodToStr(value)) == 0) return value;
  }
}

const char* StreamDeviceTypeToStr(StreamDeviceType type) {
  switch (type) {
    case StreamDeviceType::DEVICE_DEFAULT:
      return "default";
    case StreamDeviceType::DEVICE_V4L2:
      return "v4l2";
    case StreamDeviceType::DEVICE_CSI:
      return "csi";
    case StreamDeviceType::DEVICE_IP:
      return "ip";
    case StreamDeviceType::DEVICE_FILE:
      return "file";
    case StreamDeviceType::DEVICE_DISPLAY:
      return "display";
    default:
      return "default";
  }
}

StreamDeviceType StreamDeviceTypeFromStr(const char* str) {
  if (!str) return StreamDeviceType::DEVICE_DEFAULT;
  for (int n = 0; n < static_cast<int>(StreamDeviceType::DEVICE_MAX_NUM); ++n) {
    const auto value = (StreamDeviceType)n;
    if (strcasecmp(str, StreamDeviceTypeToStr(value)) == 0) return value;
  }
  return StreamDeviceType::DEVICE_DEFAULT;
}

const char* StreamFlipMethodToStr(StreamFlipMethod flip) {
  switch (flip) {
    case StreamFlipMethod::FLIP_NONE:
      return "none";
    case StreamFlipMethod::FLIP_COUNTERCLOCKWISE:
      return "counterclockwise";
    case StreamFlipMethod::FLIP_ROTATE_180:
      return "rotate-180";
    case StreamFlipMethod::FLIP_CLOCKWISE:
      return "clockwise";
    case StreamFlipMethod::FLIP_HORIZONTAL:
      return "horizontal";
    case StreamFlipMethod::FLIP_UPPER_RIGHT_DIAGONAL:
      return "upper-right-diagonal";
    case StreamFlipMethod::FLIP_VERTICAL:
      return "vertical";
    case StreamFlipMethod::FLIP_UPPER_LEFT_DIAGONAL:
      return "upper-left-diagonal";
    default:
      return "none";
  }
}

StreamFlipMethod StreamFlipMethodFromStr(const char* str) {
  if (!str) return StreamFlipMethod::FLIP_NONE;
  for (int n = 0; n < static_cast<int>(StreamFlipMethod::FLIP_MAX_NUM); ++n) {
    const auto value = (StreamFlipMethod)n;
    if (strcasecmp(str, StreamFlipMethodToStr(value)) == 0) return value;
  }
  return StreamFlipMethod::FLIP_NONE;
}

const char* StreamCodecToStr(StreamCodec codec) {
  switch (codec) {
    case StreamCodec::CODEC_UNKNOWN:
      return "unknown";
    case StreamCodec::CODEC_RAW:
      return "raw";
    case StreamCodec::CODEC_H264:
      return "h264";
    case StreamCodec::CODEC_H265:
      return "h265";
    case StreamCodec::CODEC_VP8:
      return "vp8";
    case StreamCodec::CODEC_VP9:
      return "vp9";
    case StreamCodec::CODEC_MPEG2:
      return "mpeg2";
    case StreamCodec::CODEC_MPEG4:
      return "mpeg4";
    case StreamCodec::CODEC_MJPEG:
      return "mjpeg";
    default:
      return "unknown";
  }
}

StreamCodec StreamCodecFromStr(const char* str) {
  if (!str) return StreamCodec::CODEC_UNKNOWN;
  for (int n = 0; n < static_cast<int>(StreamCodec::CODEC_MAX_NUM); ++n) {
    const auto value = (StreamCodec)n;
    if (strcasecmp(str, StreamCodecToStr(value)) == 0) return value;
  }
  return StreamCodec::CODEC_UNKNOWN;
}

const char* StreamPlatformTypeToStr(StreamPlatformType platform) {
  switch (platform) {
    case StreamPlatformType::PLATFORM_CPU:
      return "cput";
    case StreamPlatformType::PLATFORM_GPU:
      return "gpu";
    case StreamPlatformType::PLATFORM_JETSON:
      return "jetson";
    default:
      return "cpu";
  }
}

StreamPixelFormat StreamPixelFormatFromStr(const char* str) {
  if (!str) return StreamPixelFormat::PIXEL_FORMAT_UNKNOWN;
  for (int n = 0; n < static_cast<int>(StreamPixelFormat::PIXEL_FORMAT_MAX_NUM);
       ++n) {
    const auto value = (StreamPixelFormat)n;
    if (strcasecmp(str, StreamPixelFormatToStr(value)) == 0) return value;
  }
  return StreamPixelFormat::PIXEL_FORMAT_UNKNOWN;
}

const char* StreamPixelFormatToStr(StreamPixelFormat pixel_format) {
  switch (pixel_format) {
    case StreamPixelFormat::PIXEL_FORMAT_RGB:
      return "RGB";
    case StreamPixelFormat::PIXEL_FORMAT_RGB16:
      return "RGB16";
    case StreamPixelFormat::PIXEL_FORMAT_RGBA:
      return "RGBA";
    case StreamPixelFormat::PIXEL_FORMAT_RGBA16:
      return "RGBA16";
    case StreamPixelFormat::PIXEL_FORMAT_BGR:
      return "BGR";
    case StreamPixelFormat::PIXEL_FORMAT_BGR16:
      return "BGR16";
    case StreamPixelFormat::PIXEL_FORMAT_BGRA:
      return "BGRA";
    case StreamPixelFormat::PIXEL_FORMAT_BGRA16:
      return "BGRA16";
    case StreamPixelFormat::PIXEL_FORMAT_GRAY8:
      return "GRAY8";
    case StreamPixelFormat::PIXEL_FORMAT_GRAY16_LE:
      return "GRAY16_LE";
    case StreamPixelFormat::PIXEL_FORMAT_YUYV:
      return "YUYV";
    case StreamPixelFormat::PIXEL_FORMAT_UYVY:
      return "UYVY";
    case StreamPixelFormat::PIXEL_FORMAT_MJPEG:
      return "MJPEG";
    case StreamPixelFormat::PIXEL_FORMAT_YUVMONO10:
      return "YUVMONO10";
    default:
      return "BGR";
  }
}

StreamPlatformType StreamPlatformTypeFromStr(const char* str) {
  if (!str) return StreamPlatformType::PLATFORM_CPU;
  for (int n = 0; n < static_cast<int>(StreamPlatformType::PLATFORM_MAX_NUM);
       ++n) {
    const auto value = (StreamPlatformType)n;
    if (strcasecmp(str, StreamPlatformTypeToStr(value)) == 0) return value;
  }
  return StreamPlatformType::PLATFORM_CPU;
}

StreamOptions::StreamOptions() {
  width = 0;
  height = 0;
  channels = 3;
  frame_rate = 0;
  bit_rate = 0;
  num_buffers = 4;
  loop = 0;
  zero_copy = true;
  async = false;
  io_type = StreamIoType::IO_INPUT;
  io_method = StreamIoMethod::IO_METHOD_MMAP;
  device_type = StreamDeviceType::DEVICE_DEFAULT;
  flip_method = StreamFlipMethod::FLIP_NONE;
  codec = StreamCodec::CODEC_UNKNOWN;
  pixel_format = StreamPixelFormat::PIXEL_FORMAT_BGR;
  output_format = pixel_format;
}

}  // namespace stream
}  // namespace zetton
