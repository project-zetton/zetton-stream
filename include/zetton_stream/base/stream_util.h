#pragma once

#include <cstring>
#include <sstream>
#include <string>

#include "zetton_common/util/log.h"
#include "zetton_stream/base/stream_options.h"

namespace zetton {
namespace stream {

inline bool BuildGstPipelineString(const StreamUri& uri, StreamOptions& options,
                                   std::string& pipeline_string,
                                   const bool& use_custom_size = false,
                                   const bool& use_custom_rate = false) {
  std::ostringstream ss;

  // determine the source protocol
  if (uri.protocol == StreamProtocolType::PROTOCOL_FILE) {
    // build pipeline for file source
    options.device_type = StreamDeviceType::DEVICE_FILE;
    // source
    ss << "filesrc location=" << options.resource.location << " ! ";
    if (uri.extension == "mkv") {
      ss << "matroskademux ! ";
    } else if (uri.extension == "mp4" || uri.extension == "qt") {
      ss << "qtdemux ! ";
    } else if (uri.extension == "flv") {
      ss << "flvdemux ! ";
    } else if (uri.extension == "avi") {
      ss << "avidemux ! ";
    } else if (uri.extension != "h264" && uri.extension != "h265") {
      AERROR_F("Unsupported video file extension ({})", uri.extension);
      return false;
    }
    // parser
    ss << "queue ! ";
    if (options.codec == StreamCodec::CODEC_H264) {
      ss << "h264parse ! ";
    } else if (options.codec == StreamCodec::CODEC_H265) {
      ss << "h265parse ! ";
    } else if (options.codec == StreamCodec::CODEC_MPEG2) {
      ss << "mpegvideoparse ! ";
    } else if (options.codec == StreamCodec::CODEC_MPEG4) {
      ss << "mpeg4videoparse ! ";
    }
  } else if (uri.protocol == StreamProtocolType::PROTOCOL_RTP) {
    // build pipeline for RTP source
    options.device_type = StreamDeviceType::DEVICE_IP;
    // check port number
    if (uri.port <= 0) {
      AERROR_F("Invalid RTP port ({})", uri.port);
      return false;
    }
    // source
    ss << "udpsrc port=" << uri.port;
    ss << " multicast-group=" << uri.location << " auto-multicast=true";
    ss << " caps=\""
       << "application/"
          "x-rtp,media=(string)video,clock-rate=(int)90000,encoding-name=("
          "string)";
    // demux
    if (options.codec == StreamCodec::CODEC_H264) {
      ss << "H264\" ! rtpjitterbuffer ! rtph264depay ! h264parse ! ";
    } else if (options.codec == StreamCodec::CODEC_H265) {
      ss << "H265\" ! rtpjitterbuffer ! rtph265depay ! h265parse ! ";
    } else if (options.codec == StreamCodec::CODEC_VP8) {
      ss << "VP8\" ! rtpjitterbuffer ! rtpvp8depay ! ";
    } else if (options.codec == StreamCodec::CODEC_VP9) {
      ss << "VP9\" ! rtpjitterbuffer ! rtpvp9depay ! ";
    } else if (options.codec == StreamCodec::CODEC_MPEG2) {
      ss << "MP2T\" ! rtpjitterbuffer ! rtpmp2tdepay ! ";
    } else if (options.codec == StreamCodec::CODEC_MPEG4) {
      ss << "MP4V-ES\" ! rtpjitterbuffer ! rtpmp4vdepay ! ";
    } else if (options.codec == StreamCodec::CODEC_MJPEG) {
      ss << "JPEG\" ! rtpjitterbuffer ! rtpjpegdepay ! ";
    }
  } else if (uri.protocol == StreamProtocolType::PROTOCOL_RTSP) {
    // build pipeline for RTSP source
    options.device_type = StreamDeviceType::DEVICE_IP;
    // source
    ss << "rtspsrc location=" << uri.string;
    ss << " ! queue ! ";
    // demux
    if (options.codec == StreamCodec::CODEC_H264) {
      ss << "rtph264depay ! h264parse ! ";
    } else if (options.codec == StreamCodec::CODEC_H265) {
      ss << "rtph265depay ! h265parse ! ";
    } else if (options.codec == StreamCodec::CODEC_VP8) {
      ss << "rtpvp8depay ! ";
    } else if (options.codec == StreamCodec::CODEC_VP9) {
      ss << "rtpvp9depay ! ";
    } else if (options.codec == StreamCodec::CODEC_MPEG2) {
      ss << "rtpmp2tdepay ! ";
    } else if (options.codec == StreamCodec::CODEC_MPEG4) {
      ss << "rtpmp4vdepay ! ";
    } else if (options.codec == StreamCodec::CODEC_MJPEG) {
      ss << "rtpjpegdepay ! ";
    }
  } else {
    AERROR_F("Unsupported protocol ({})",
             StreamProtocolTypeToStr(uri.protocol));
    return false;
  }

  // determine the decoder
  if (options.platform == StreamPlatformType::PLATFORM_CPU) {
    // use libav for decoding on CPU
    if (options.codec == StreamCodec::CODEC_H264) {
      ss << "avdec_h264 ! ";
    } else if (options.codec == StreamCodec::CODEC_H265) {
      ss << "avdec_h265 ! ";
    } else if (options.codec == StreamCodec::CODEC_VP8) {
      ss << "avdev_vp8 ! ";
    } else if (options.codec == StreamCodec::CODEC_VP9) {
      ss << "avdec_vp9 ! ";
    } else if (options.codec == StreamCodec::CODEC_MPEG2) {
      ss << "avdec_mpeg2video ! ";
    } else if (options.codec == StreamCodec::CODEC_MPEG4) {
      ss << "avdec_mpeg4 ! ";
    } else if (options.codec == StreamCodec::CODEC_MJPEG) {
      ss << "avdec_mjpeg ! ";
    } else {
      AERROR_F("Unsupported codec ({})", StreamCodecToStr(options.codec));
      return false;
    }
  } else if (options.platform == StreamPlatformType::PLATFORM_GPU) {
    // use Video Codec SDK for GPU-accelerated decoding
    if (options.codec == StreamCodec::CODEC_H264 ||
        options.codec == StreamCodec::CODEC_H265 ||
        options.codec == StreamCodec::CODEC_MPEG2 ||
        options.codec == StreamCodec::CODEC_MPEG4 ||
        options.codec == StreamCodec::CODEC_MJPEG) {
      // FIXME gldownload doesn't work on headless mode
      ss << "nvdec ! gldownload ! ";
    } else {
      AERROR_F("Unsupported codec ({})", StreamCodecToStr(options.codec));
      return false;
    }
  } else if (options.platform == StreamPlatformType::PLATFORM_JETSON) {
    // use omx for hardware-accelerated decoding on Jetson devices
    if (options.codec == StreamCodec::CODEC_H264) {
      ss << "omxh264dec ! ";
    } else if (options.codec == StreamCodec::CODEC_H265) {
      ss << "omxh265dec ! ";
    } else if (options.codec == StreamCodec::CODEC_VP8) {
      ss << "omxvp8dec ! ";
    } else if (options.codec == StreamCodec::CODEC_VP9) {
      ss << "omxvp9dec ! ";
    } else if (options.codec == StreamCodec::CODEC_MPEG2) {
      ss << "omxmpeg2videodec ! ";
    } else if (options.codec == StreamCodec::CODEC_MPEG4) {
      ss << "omxmpeg4videodec ! ";
    } else if (options.codec == StreamCodec::CODEC_MJPEG) {
      ss << "nvjpegdec ! ";
    } else {
      AERROR_F("Unsupported codec ({})", StreamCodecToStr(options.codec));
      return false;
    }
  }

  // resize or flip if requested
  if (use_custom_size || options.flip_method != StreamFlipMethod::FLIP_NONE) {
    if (options.platform == StreamPlatformType::PLATFORM_CPU ||
        options.platform == StreamPlatformType::PLATFORM_GPU) {
      ss << "videoconvert";
      // flip on CPU
      if (options.flip_method != StreamFlipMethod::FLIP_NONE) {
        ss << " ! videoflip flip-method=(string)"
           << StreamFlipMethodToStr(options.flip_method);
      }
      // rescale
      if (options.width != 0 && options.height != 0) {
        ss << " ! videoscale ! video/x-raw";
        ss << ", width=(int)" << options.width << ", height=(int)"
           << options.height << ", format=(string)BGRx";
      }
      ss << " ! ";
    } else if (options.platform == StreamPlatformType::PLATFORM_JETSON) {
      // TODO check the validity on GPU
      ss << "nvvidconv";
      // flip on GPU
      if (options.flip_method != StreamFlipMethod::FLIP_NONE) {
        ss << " flip-method=" << (int)options.flip_method;
      }
      // rescale
      ss << " ! video/x-raw";
      if (options.width != 0 && options.height != 0) {
        ss << ", width=(int)" << options.width << ", height=(int)"
           << options.height << ", format=(string)BGRx";
      }
      ss << " ! ";
    }
  } else {
    ss << "videoconvert ! video/x-raw ! ";
  }

  // rate-limit if requested
  if (use_custom_rate)
    ss << "videorate drop-only=true max-rate=" << (int)options.frame_rate
       << " ! ";

  // add the app sink
  ss << "appsink";  // wait-on-eos=false;

  if (options.async) {
    ss << " sync=false";
  }

  pipeline_string = ss.str();
  AINFO_F("Built pipeline string: {}", pipeline_string);
  return true;
}

inline bool IsGstSupportedExtension(const char* ext, const char** supported) {
  if (!ext) return false;

  uint32_t ext_count = 0;
  while (true) {
    if (!supported[ext_count]) break;
    if (strcasecmp(supported[ext_count], ext) == 0) return true;
    ext_count++;
  }

  return false;
}

}  // namespace stream
}  // namespace zetton
