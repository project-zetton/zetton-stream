#include "zetton_stream/stream/cv_gst_stream_source.h"

#include <memory>
#include <string>

#include "zetton_common/util/filesystem.h"
#include "zetton_common/util/log.h"
#include "zetton_stream/stream/stream_uri.h"
#include "zetton_stream/stream/stream_util.h"

namespace zetton {
namespace stream {

bool CvGstStreamSource::Init(const StreamOptions& options) {
  options_ = options;
  // check that the file exists
  if (options_.resource.protocol == StreamProtocolType::PROTOCOL_FILE) {
    if (!common::FileExists(options_.resource.location)) {
      ROS_ERROR("File not found: '%s'\n", options_.resource.location.c_str());
      return false;
    }
  }
  ROS_INFO("Creating decoder for %s", options_.resource.location.c_str());

  // flag if the user wants a specific resolution and framerate
  if (options_.width != 0 || options_.height != 0) {
    use_custom_size_ = true;
  }
  if (options_.frame_rate != 0) {
    use_custom_rate_ = true;
  }

  // build pipeline string
  if (!BuildPipelineString()) {
    ROS_ERROR("Failed to build pipeline string");
    return false;
  }

  // init by pipeline string
  return Init(pipeline_string_);
}

bool CvGstStreamSource::Init(const std::string& pipeline) {
  pipeline_string_ = pipeline;

  // create pipeline
  cap_ =
      std::make_shared<cv::VideoCapture>(pipeline_string_, cv::CAP_GSTREAMER);

  // init buffer
  buffer_.reset(new common::CircularBuffer<cv::Mat>(10));

  // open stream
  Open();

  return true;
}

void CvGstStreamSource::CaptureFrames() {
  while (!stop_flag_) {
    cv::Mat frame;
    auto ret = cap_->read(frame);
    if (ret && !frame.empty()) {
      buffer_->put(frame);
      if (callback_registered_) {
        try {
          callback_(frame);
        } catch (std::exception& e) {
          ROS_ERROR_STREAM(e.what());
        }
      }
    }
  }
};

bool CvGstStreamSource::Open() {
  is_streaming_ = true;
  stop_flag_ = false;
  thread_capturing_ = std::make_shared<std::thread>([&]() { CaptureFrames(); });
  return true;
}

void CvGstStreamSource::Close() {
  is_streaming_ = false;
  stop_flag_ = true;
  thread_capturing_->join();
}

bool CvGstStreamSource ::Capture(cv::Mat& frame) {
  if (!buffer_->empty()) {
    frame = buffer_->get();
    return !frame.empty();
  } else {
    return false;
  }
}

void CvGstStreamSource::RegisterCallback(
    std::function<void(const cv::Mat& frame)> callback) {
  callback_ = callback;
  callback_registered_ = true;
}

const char* CvGstStreamSource::SupportedExtensions[] = {
    "mkv", "mp4", "qt", "flv", "avi", "h264", "h265", nullptr};

bool CvGstStreamSource::IsSupportedExtension(const char* ext) {
  return IsGstSupportedExtension(ext, SupportedExtensions);
}

bool CvGstStreamSource::BuildPipelineString() {
  const StreamUri& uri = GetResource();
  return BuildGstPipelineString(uri, options_, pipeline_string_,
                                use_custom_size_, use_custom_rate_);
}

}  // namespace stream
}  // namespace zetton
