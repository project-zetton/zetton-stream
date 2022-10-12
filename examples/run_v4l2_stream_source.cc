#include <unistd.h>

#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>

#include "zetton_common/util/log.h"
#include "zetton_common/util/perf.h"
#include "zetton_stream/source/v4l2_stream_source.h"

int main(int argc, char** argv) {
  // prepare stream url
  zetton::stream::StreamOptions options;
  options.resource = zetton::stream::StreamUri("v4l2:///dev/video0");
  options.pixel_format = zetton::stream::StreamPixelFormat::PIXEL_FORMAT_MJPEG;
  options.output_format = zetton::stream::StreamPixelFormat::PIXEL_FORMAT_RGB;
  options.io_method = zetton::stream::StreamIoMethod::IO_METHOD_MMAP;
  options.width = 1280;
  options.height = 960;
  options.frame_rate = 30;

  // init streamer
  auto source = std::make_shared<zetton::stream::V4l2StreamSource>();
  source->Init(options);

  // init output image
  auto raw_image = std::make_shared<zetton::stream::CameraImage>();
  raw_image->width = options.width;
  raw_image->height = options.height;
  if (options.output_format ==
      zetton::stream::StreamPixelFormat::PIXEL_FORMAT_YUYV) {
    raw_image->image_size = raw_image->width * raw_image->height * 2;
    raw_image->bytes_per_pixel = 2;
  } else if (options.output_format ==
             zetton::stream::StreamPixelFormat::PIXEL_FORMAT_RGB) {
    raw_image->image_size = raw_image->width * raw_image->height * 3;
    raw_image->bytes_per_pixel = 3;
  }
  raw_image->is_new = 0;
  raw_image->image =
      reinterpret_cast<char*>(calloc(raw_image->image_size, sizeof(char)));
  memset(raw_image->image, 0, raw_image->image_size * sizeof(char));
  if (raw_image->image == nullptr) {
    AERROR << "system calloc memory error, size:" << raw_image->image_size;
    return 1;
  }

  // capture and save image
  while (true) {
    // wait for device
    if (!source->wait_for_device()) {
      AERROR_F("wait for device error");
      usleep(100000);
      continue;
    }
    // poll image from camera
    if (!source->poll(raw_image)) {
      AERROR << "camera device poll failed";
      usleep(100000);
      continue;
    }
    // write to file
    cv::Mat image(raw_image->height, raw_image->width, CV_8UC3,
                  raw_image->image);
    cv::imwrite("test.jpg", image);
    break;
  }

  // benchmark
  zetton::common::FpsCalculator fps;
  auto counter = 0;
  while (counter < 100) {
    // start timer
    fps.Start();
    // wait for device
    if (!source->wait_for_device()) {
      usleep(100000);
      AERROR_F("wait for device error");
      continue;
    }
    // poll image from camera
    if (!source->poll(raw_image)) {
      AERROR << "camera device poll failed";
      continue;
    }
    // stop timer
    fps.End();
    counter += 1;
  }

  // print profiling info
  fps.PrintInfo("V4l2StreamSource");

  return 0;
}
