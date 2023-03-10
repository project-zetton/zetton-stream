#include "zetton_stream/util/ocv.h"

#include "zetton_common/util/log.h"

namespace zetton {
namespace stream {

bool CheckFrame(const cv::Mat& frame, const StreamOptions& options) {
  // check empty
  if (frame.empty()) {
    AERROR_F("frame is empty");
    return false;
  }

  // check width and height
  if (frame.cols != options.width || frame.rows != options.height) {
    AERROR_F("frame size is not matched with options: {}x{} vs {}x{}",
             frame.cols, frame.rows, options.width, options.height);
    return false;
  }

  // check channels
  if (options.channels == 3 && frame.type() != CV_8UC3) {
    AERROR_F("frame type is not CV_8UC3");
    return false;
  } else if (options.channels == 1 && frame.type() != CV_8UC1) {
    AERROR_F("frame type is not CV_8UC1");
    return false;
  } else if (options.channels == 4 && frame.type() != CV_8UC4) {
    AERROR_F("frame type is not CV_8UC4");
    return false;
  } else if (options.channels != 1 && options.channels != 3 &&
             options.channels != 4) {
    AERROR_F("frame channels is not supported: {}", options.channels);
    return false;
  }

  return true;
}

}  // namespace stream
}  // namespace zetton
