#include <unistd.h>

#include "zetton_common/util/log.h"
#include "zetton_stream/stream/cv_gst_stream_source.h"
#include "zetton_stream/stream/stream_options.h"
#include "zetton_stream/stream/stream_uri.h"

int main(int argc, char** argv) {
  // prepare stream url
  std::string url =
      "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov";
  zetton::stream::StreamOptions options;
  options.resource = url;
  options.platform = zetton::stream::StreamPlatformType::PLATFORM_CPU;
  options.codec = zetton::stream::StreamCodec::CODEC_H264;
  options.async = false;

  // init streamer
  std::shared_ptr<zetton::stream::CvGstStreamSource> source;
  source = std::make_shared<zetton::stream::CvGstStreamSource>();
  source->Init(options);

  // start capturing
  while (true) {
    cv::Mat frame;
    if (source->Capture(frame)) {
      AWARN_F("Recv frame: {}x{}", frame.cols, frame.rows);
    }
    usleep(100000);
  }

  return 0;
}