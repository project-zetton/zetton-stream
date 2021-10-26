#include "zetton_stream/stream/stream_options.h"

#include <cstring>

namespace zetton {
namespace stream {

const char* StreamIoTypeToStr(StreamIoType type) {
  switch (type) {
    case IO_INPUT:
      return "input";
    case IO_OUTPUT:
      return "output";
    default:
      return "input";
  }
}

StreamIoType StreamIoTypeFromStr(const char* str) {
  if (!str) return IO_INPUT;
  for (int n = 0; n < IO_MAX_NUM; n++) {
    const StreamIoType value = (StreamIoType)n;
    if (strcasecmp(str, StreamIoTypeToStr(value)) == 0) return value;
  }
  return IO_INPUT;
}

const char* StreamDeviceTypeToStr(StreamDeviceType type) {
  switch (type) {
    case DEVICE_DEFAULT:
      return "default";
    case DEVICE_V4L2:
      return "v4l2";
    case DEVICE_CSI:
      return "csi";
    case DEVICE_IP:
      return "ip";
    case DEVICE_FILE:
      return "file";
    case DEVICE_DISPLAY:
      return "display";
    default:
      return "default";
  }
}

StreamDeviceType StreamDeviceTypeFromStr(const char* str) {
  if (!str) return DEVICE_DEFAULT;
  for (int n = 0; n < DEVICE_MAX_NUM; n++) {
    const StreamDeviceType value = (StreamDeviceType)n;
    if (strcasecmp(str, StreamDeviceTypeToStr(value)) == 0) return value;
  }
  return DEVICE_DEFAULT;
}

const char* StreamFlipMethodToStr(StreamFlipMethod flip) {
  switch (flip) {
    case FLIP_NONE:
      return "none";
    case FLIP_COUNTERCLOCKWISE:
      return "counterclockwise";
    case FLIP_ROTATE_180:
      return "rotate-180";
    case FLIP_CLOCKWISE:
      return "clockwise";
    case FLIP_HORIZONTAL:
      return "horizontal";
    case FLIP_UPPER_RIGHT_DIAGONAL:
      return "upper-right-diagonal";
    case FLIP_VERTICAL:
      return "vertical";
    case FLIP_UPPER_LEFT_DIAGONAL:
      return "upper-left-diagonal";
    default:
      return "none";
  }
}

StreamFlipMethod StreamFlipMethodFromStr(const char* str) {
  if (!str) return FLIP_NONE;
  for (int n = 0; n < FLIP_MAX_NUM; n++) {
    const StreamFlipMethod value = (StreamFlipMethod)n;
    if (strcasecmp(str, StreamFlipMethodToStr(value)) == 0) return value;
  }
  return FLIP_NONE;
}

const char* StreamCodecToStr(StreamCodec codec) {
  switch (codec) {
    case CODEC_UNKNOWN:
      return "unknown";
    case CODEC_RAW:
      return "raw";
    case CODEC_H264:
      return "h264";
    case CODEC_H265:
      return "h265";
    case CODEC_VP8:
      return "vp8";
    case CODEC_VP9:
      return "vp9";
    case CODEC_MPEG2:
      return "mpeg2";
    case CODEC_MPEG4:
      return "mpeg4";
    case CODEC_MJPEG:
      return "mjpeg";
    default:
      return "unknown";
  }
}

StreamCodec StreamCodecFromStr(const char* str) {
  if (!str) return CODEC_UNKNOWN;
  for (int n = 0; n < CODEC_MAX_NUM; n++) {
    const StreamCodec value = (StreamCodec)n;
    if (strcasecmp(str, StreamCodecToStr(value)) == 0) return value;
  }
  return CODEC_UNKNOWN;
}

const char* StreamPlatformTypeToStr(StreamPlatformType platform) {
  switch (platform) {
    case PLATFORM_CPU:
      return "cput";
    case PLATFORM_GPU:
      return "gpu";
    case PLATFORM_JETSON:
      return "jetson";
    default:
      return "cpu";
  }
}

StreamPixelFormat StreamPixelFormatFromStr(const char* str) {
  if (!str) return PIXEL_FORMAT_UNKNOWN;
  for (int n = 0; n < PIXEL_FORMAT_MAX_NUM; n++) {
    const StreamPixelFormat value = (StreamPixelFormat)n;
    if (strcasecmp(str, StreamPixelFormatToStr(value)) == 0) return value;
  }
  return PIXEL_FORMAT_UNKNOWN;
}

const char* StreamPixelFormatToStr(StreamPixelFormat pixel_format) {
  switch (pixel_format) {
    case PIXEL_FORMAT_RGB:
      return "RGB";
    case PIXEL_FORMAT_RGB16:
      return "RGB16";
    case PIXEL_FORMAT_RGBA:
      return "RGBA";
    case PIXEL_FORMAT_RGBA16:
      return "RGBA16";
    case PIXEL_FORMAT_BGR:
      return "BGR";
    case PIXEL_FORMAT_BGR16:
      return "BGR16";
    case PIXEL_FORMAT_BGRA:
      return "BGRA";
    case PIXEL_FORMAT_BGRA16:
      return "BGRA16";
    case PIXEL_FORMAT_GRAY8:
      return "GRAY8";
    case PIXEL_FORMAT_GRAY16_LE:
      return "GRAY16_LE";
    default:
      return "BGR";
  }
}

StreamPlatformType StreamPlatformTypeFromStr(const char* str) {
  if (!str) return StreamPlatformType::PLATFORM_CPU;
  for (int n = 0; n < PLATFORM_MAX_NUM; n++) {
    const StreamPlatformType value = (StreamPlatformType)n;
    if (strcasecmp(str, StreamPlatformTypeToStr(value)) == 0) return value;
  }
  return StreamPlatformType::PLATFORM_CPU;
}

StreamOptions::StreamOptions() {
  width = 0;
  height = 0;
  frame_rate = 0;
  bit_rate = 0;
  num_buffers = 4;
  loop = 0;
  zero_copy = true;
  async = false;
  io_type = StreamIoType::IO_INPUT;
  device_type = StreamDeviceType::DEVICE_DEFAULT;
  flip_method = StreamFlipMethod::FLIP_NONE;
  codec = StreamCodec::CODEC_UNKNOWN;
  pixel_format = StreamPixelFormat::PIXEL_FORMAT_BGR;
}

}  // namespace stream
}  // namespace zetton