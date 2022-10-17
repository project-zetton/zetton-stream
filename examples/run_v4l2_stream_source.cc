#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <unistd.h>

#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "zetton_common/util/log.h"
#include "zetton_common/util/perf.h"
#include "zetton_stream/source/legacy_v4l2_stream_source.h"
#include "zetton_stream/source/v4l2_stream_source.h"

ABSL_FLAG(std::string, device, "/dev/video0", "path to video device");
ABSL_FLAG(int, width, 320, "image width to capture");
ABSL_FLAG(int, height, 240, "image height to capture");
ABSL_FLAG(int, frame_rate, 30, "frame rate to capture");
ABSL_FLAG(std::string, pixel_format, "YUYV", "pixel format to capture");
ABSL_FLAG(std::string, output_file, "test.png", "path to captured image file");

int main(int argc, char** argv) {
  // parse args
  absl::ParseCommandLine(argc, argv);
  auto device = absl::GetFlag(FLAGS_device);
  auto width = absl::GetFlag(FLAGS_width);
  auto height = absl::GetFlag(FLAGS_height);
  auto frame_rate = absl::GetFlag(FLAGS_frame_rate);
  auto pixel_format = absl::GetFlag(FLAGS_pixel_format);
  auto output_file = absl::GetFlag(FLAGS_output_file);

  // prepare stream url
  zetton::stream::StreamOptions options;
  options.resource =
      zetton::stream::StreamUri(fmt::format("v4l2://{}", device));
  options.output_format = zetton::stream::StreamPixelFormat::PIXEL_FORMAT_RGB;
  options.io_method = zetton::stream::StreamIoMethod::IO_METHOD_MMAP;
  if (pixel_format == "YUYV") {
    options.pixel_format = zetton::stream::StreamPixelFormat::PIXEL_FORMAT_YUYV;
  } else if (pixel_format == "MJPEG") {
    options.pixel_format =
        zetton::stream::StreamPixelFormat::PIXEL_FORMAT_MJPEG;
  } else {
    AERROR_F("Unsupported pixel format: {}", pixel_format);
    return -1;
  }
  options.width = width;
  options.height = height;
  options.frame_rate = frame_rate;
  // disable auto exposure
  options.camera.auto_exposure = 1;

// init streamer
#if 1
  auto source = std::make_shared<zetton::stream::V4l2StreamSource>();
#else
  auto source = std::make_shared<
      zetton::stream::LegacyV4l2StreamSourceV4l2StreamSource>();
#endif
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
    if (!source->WaitForDevice()) {
      AERROR_F("wait for device error");
      usleep(100000);
      continue;
    }
    // poll image from camera
    if (!source->Capture(raw_image)) {
      AERROR << "camera device poll failed";
      usleep(100000);
      continue;
    }
    // write to file
    cv::Mat image(raw_image->height, raw_image->width, CV_8UC3,
                  raw_image->image);
    cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
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
    if (!source->WaitForDevice()) {
      usleep(100000);
      AERROR_F("wait for device error");
      continue;
    }
    // poll image from camera
    if (!source->Capture(raw_image)) {
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
