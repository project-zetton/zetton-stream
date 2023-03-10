#pragma once

#include <opencv2/opencv.hpp>

#include "zetton_stream/base/stream_options.h"

namespace zetton {
namespace stream {

bool CheckFrame(const cv::Mat& frame, const StreamOptions& options);

}
}  // namespace zetton
