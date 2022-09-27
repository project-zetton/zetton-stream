#pragma once

#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

#include <atomic>
#include <thread>

#include "opencv2/opencv.hpp"
#include "zetton_common/thread/ring_buffer.h"
#include "zetton_stream/interface/base_stream_output.h"

namespace zetton {
namespace stream {

class GstRtspStreamOutput : public BaseStreamOutput {
 public:
  ~GstRtspStreamOutput();

  static bool IsSupportedExtension(const char *ext);
  static const char *SupportedExtensions[];

  bool Init(const StreamOptions &options);
  bool Open() override;
  void Close() override;

  bool Render(const cv::Mat &frame);
  bool Render(void *image, uint32_t width, uint32_t height) override;

  void SetStatus(const char *str) override;

 public:
  /// \brief Get pointer to RTSP server instance
  /// \return pointer RTSP server instance
  GstRTSPServer *GetRtspServer() const { return rtsp_server_; }

  /// \brief Callback function when a client is connected
  /// \param url requested mountpoint
  void url_connected(const std::string &url);

  /// \brief Callback function when a client is disconnected
  /// \param url requested mountpoint
  void url_disconnected(const std::string &url);

 protected:
  bool BuildPipelineString();
  std::string pipeline_string_;

 private:
  /// \brief Initialize GStreamer and streaming thread
  void start_video_mainloop();

  /// \brief Initialize of GStreamer RTSP server
  /// \return pointer to RTSP server instance
  GstRTSPServer *create_rtsp_server();

  /// \brief Setup arguments of RTSP server
  /// \param url mountpoint
  /// \param sPipeline GStreamer pipeline
  /// \param appsrc GStreamer appsrc
  void setup_rtsp_server(const char *url, const char *sPipeline,
                         GstElement **appsrc);

  /// \brief Get capability from given image
  /// \param frame frame in cv::Mat format
  /// \return GStreamer capability
  GstCaps *gst_caps_new_from_image(const cv::Mat &frame);

 private:
  /// \brief pinter to GStreamer mainloop
  GMainLoop *loop_ = nullptr;
  /// \brief thread of GStreamer mainloop
  std::thread loop_thread_;
  /// \brief pointer to RTSP server instance
  GstRTSPServer *rtsp_server_;

  /// \brief current mountpoint of output video stream
  std::string mountpoint_ = "/test";

  /// \brief Mapping between mountpoint and streaming flag
  std::map<std::string, bool> subs_;
  /// \brief Mapping between mountpoint and GStreamer appsrc
  std::map<std::string, GstAppSrc *> appsrc_;
  /// \brief Mapping between mountpoint and #clients
  std::map<std::string, int> num_of_clients_;
};

}  // namespace stream
}  // namespace zetton
