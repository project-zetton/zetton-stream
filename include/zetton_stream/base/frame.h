#pragma once

#include <malloc.h>

#include <memory>

namespace zetton {
namespace stream {

// camera raw image struct
struct CameraImage {
  int width;
  int height;
  int bytes_per_pixel;
  int image_size;
  int is_new;
  int tv_sec;
  int tv_usec;
  char* image;

  ~CameraImage() {
    if (image != nullptr) {
      free(reinterpret_cast<void*>(image));
      image = nullptr;
    }
  }
};

using CameraImagePtr = std::shared_ptr<CameraImage>;

struct CameraBuffer {
  void* start;
  size_t length;
};

}  // namespace stream
}  // namespace zetton
