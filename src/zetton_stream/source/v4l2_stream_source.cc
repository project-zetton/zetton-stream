#include "zetton_stream/source/v4l2_stream_source.h"

#include <linux/videodev2.h>
#include <sys/stat.h>

#include "zetton_stream/util/pixel_format.h"
#include "zetton_stream/util/v4l/v4l2-info.h"

namespace zetton {
namespace stream {

V4l2StreamSource::V4l2StreamSource()
    : fd_(), buffers_(nullptr), n_buffers_(4), is_capturing_(false) {}

V4l2StreamSource::~V4l2StreamSource() { Shutdown(); }

bool V4l2StreamSource::Init(const StreamOptions& options) {
  options_ = options;
  monochrome_ = false;

  if (options_.pixel_format == StreamPixelFormat::PIXEL_FORMAT_YUYV) {
    pixel_format_ = V4L2_PIX_FMT_YUYV;
  } else if (options_.pixel_format == StreamPixelFormat::PIXEL_FORMAT_UYVY) {
    pixel_format_ = V4L2_PIX_FMT_UYVY;
  } else if (options_.pixel_format == StreamPixelFormat::PIXEL_FORMAT_MJPEG) {
    pixel_format_ = V4L2_PIX_FMT_MJPEG;
    mjpeg_decoder_.Init(options_.width, options_.height);
  } else if (options_.pixel_format ==
             StreamPixelFormat::PIXEL_FORMAT_YUVMONO10) {
    // actually format V4L2_PIX_FMT_Y16 (10-bit mono expresed as 16-bit pixels),
    // but we need to use the advertised type (yuyv)
    pixel_format_ = V4L2_PIX_FMT_YUYV;
    monochrome_ = true;
  } else if (options_.pixel_format == StreamPixelFormat::PIXEL_FORMAT_RGB) {
    pixel_format_ = V4L2_PIX_FMT_RGB24;
  } else if (options_.pixel_format == StreamPixelFormat::PIXEL_FORMAT_BGR) {
    pixel_format_ = V4L2_PIX_FMT_BGR24;
  } else if (options_.pixel_format == StreamPixelFormat::PIXEL_FORMAT_NV12) {
    pixel_format_ = V4L2_PIX_FMT_NV12;
  } else if (options_.pixel_format == StreamPixelFormat::PIXEL_FORMAT_GRAY8) {
    pixel_format_ = V4L2_PIX_FMT_GREY;
    monochrome_ = true;
  } else {
    AERROR_F("Unsupported pixel format: {}",
             StreamPixelFormatToStr(options_.pixel_format));
    return false;
  }

  return true;
}

bool V4l2StreamSource::WaitForDevice() {
  if (is_capturing_) {
    ADEBUG << "is capturing";
    ADEBUG_F("device {} is already capturing", options_.resource.location);
    return true;
  }
  if (!OpenDevice()) {
    AERROR_F("failed to open device {}", options_.resource.location);
    return false;
  }
  if (!InitDevice()) {
    AERROR_F("failed to init device {}", options_.resource.location);
    CloseDevice();
    return false;
  }
  if (!StartCapturing()) {
    AERROR_F("failed to start capturing device {}", options_.resource.location);
    UninitDevice();
    CloseDevice();
    return false;
  }
  return true;
}

bool V4l2StreamSource::Capture(const CameraImagePtr& raw_image) {
  // 0. prepare
  // 0.1. reset is_new flag
  raw_image->is_new = 0;
  // 0.2. free memory in this struct desturctor
  memset(raw_image->image, 0, raw_image->image_size * sizeof(char));

  // 1. select device
  fd_set fds;
  struct timeval tv;
  int r = 0;

  FD_ZERO(&fds);
  FD_SET(fd_.g_fd(), &fds);

  /* Timeout. */
  tv.tv_sec = 2;
  tv.tv_usec = 0;

  r = select(fd_.g_fd() + 1, &fds, nullptr, nullptr, &tv);
  if (-1 == r) {
    AERROR_F("failed to select device {}: code {} string [{}]",
             options_.resource.location, errno, strerror(errno));
    if (EINTR == errno) {
      return false;
    }
    Shutdown();
  }
  if (0 == r) {
    AERROR_F("timeout to select device {}: code {} string [{}]",
             options_.resource.location, errno, strerror(errno));
    Shutdown();
  }

  // 2. read frame
  int get_new_image = ReadFrame(raw_image);
  if (!get_new_image) {
    AERROR_F("failed to read frame from device {}", options_.resource.location);
    return false;
  }
  raw_image->is_new = 1;

  return true;
}

bool V4l2StreamSource::IsCaptuering() { return is_capturing_; }

void V4l2StreamSource::Shutdown() {
  StopCapturing();
  UninitDevice();
  CloseDevice();
}

bool V4l2StreamSource::OpenDevice() {
  // 0. check if device exists
  struct stat st;

  if (-1 == stat(options_.resource.location.c_str(), &st)) {
    AERROR_F("cannot identify '{}': {}, {}", options_.resource.location, errno,
             strerror(errno));
    return false;
  }

  if (!S_ISCHR(st.st_mode)) {
    AERROR_F("{} is no device", options_.resource.location);
    return false;
  }

  // 1. open device as a file descriptor
  fd_.open(options_.resource.location.c_str(), true);
  if (fd_.g_fd() < 0) {
    AERROR_F("cannot open device {}: code {}, string [{}]",
             options_.resource.location, errno, strerror(errno));
    return false;
  }

  AINFO_F("device {} opened", options_.resource.location);

  return true;
}

bool V4l2StreamSource::CloseDevice() {
  if (fd_.close() != 0) {
    AERROR_F("failed to close device {}", options_.resource.location);
    return false;
  }
  return true;
}

bool V4l2StreamSource::InitDevice() {
  // 1. check device capabilities
  // 1.1. check if device supports v4l2
  if (!fd_.is_v4l2()) {
    AERROR_F("{} is not a V4L2 device: code {}, string [{}]",
             options_.resource.location, errno, strerror(errno));
    return false;
  }
  // 1.2. check if device supports video capture
  if (!fd_.has_vid_cap()) {
    AERROR_F("{} is not a video capture device: code {}, string [{}]",
             options_.resource.location, errno, strerror(errno));
    return false;
  }
  // 1.3. check if device supports desired io method
  switch (options_.io_method) {
    // 1.3.1. check if device supports READ io method
    case StreamIoMethod::IO_METHOD_READ:
      if (!fd_.has_rw()) {
        AERROR_F("{} does not support read i/o method: code {}, string [{}]",
                 options_.resource.location, errno, strerror(errno));
        return false;
      }
      break;
      // 1.3.2. check if device supports STREAMING io method
    case StreamIoMethod::IO_METHOD_MMAP:
    case StreamIoMethod::IO_METHOD_USERPTR:
      if (!fd_.has_streaming()) {
        AERROR_F("{} does not support streaming i/o: code {}, string [{}]",
                 options_.resource.location, errno, strerror(errno));
        return false;
      }
      break;
      // 1.3.3. check if device supports UNKNOWN io method
    case StreamIoMethod::IO_METHOD_UNKNOWN:
    default:
      AERROR_F("unknown i/o method");
      return false;
      break;
  }

  // 2. set device format
  // 2.1. set image size and pixel format
  cv4l_fmt fmt;
  fd_.g_fmt(fmt);
  fmt.s_width(options_.width);
  fmt.s_height(options_.height);
  fmt.s_pixelformat(pixel_format_);
  if (fd_.s_fmt(fmt) != 0) {
    AWARN_F("cannot set format for device {}: code {}, string [{}]",
            options_.resource.location, errno, strerror(errno));
    // return false;
  }
  // note that VIDIOC_S_FMT may change width and height
  // workaround for buggy driver paranoia.
  unsigned int min = fmt.g_width() * 2;
  if (fmt.g_bytesperline() < min) {
    fmt.s_bytesperline(min);
  }
  min = fmt.g_bytesperline() * fmt.g_height();
  if (fmt.g_sizeimage() < min) {
    fmt.s_sizeimage(min);
  }
  options_.width = fmt.g_width();
  options_.height = fmt.g_height();
  AINFO_F("image size set to {}x{} and pixel format set to {} for device {}",
          options_.width, options_.height, pixfmt2s(pixel_format_),
          options_.resource.location);
  // 2.2. set frame rate
  v4l2_fract frame_rate;
  if (fd_.get_interval(frame_rate) != 0) {
    AWARN_F("cannot get frame rate for device {}: code {}, string [{}]",
            options_.resource.location, errno, strerror(errno));
    Shutdown();
    // return false;
  }
  frame_rate.numerator = 1;
  frame_rate.denominator = options_.frame_rate;
  if (fd_.set_interval(frame_rate) != 0) {
    AWARN_F("cannot set frame rate for device {}: code {}, string [{}]",
            options_.resource.location, errno, strerror(errno));
  } else {
    AINFO_F("frame rate set to {} for device {}", options_.frame_rate,
            options_.resource.location);
  }

  // 3. init buffers
  switch (options_.io_method) {
    case StreamIoMethod::IO_METHOD_MMAP:
      buffers_ = new cv4l_queue(fd_.g_type(), V4L2_MEMORY_MMAP);
      if (buffers_->reqbufs(&fd_, n_buffers_, 0) != 0) {
        AERROR_F("cannot request buffers for device {}: code {}, string [{}]",
                 options_.resource.location, errno, strerror(errno));
        Shutdown();
        return false;
      }
      if (buffers_->obtain_bufs(&fd_) != 0) {
        AERROR_F("cannot obtain buffers for device {}: code {}, string [{}]",
                 options_.resource.location, errno, strerror(errno));
        return false;
      }
      break;

    case StreamIoMethod::IO_METHOD_READ:
    case StreamIoMethod::IO_METHOD_USERPTR:
      AERROR_F("unimplemented i/o method: {}",
               StreamIoMethodToStr(options_.io_method));
      return false;
      break;

    case StreamIoMethod::IO_METHOD_UNKNOWN:
    default:
      AERROR_F("unsupported i/o method: {}",
               StreamIoMethodToStr(options_.io_method));
      return false;
      break;
  }

  return true;
}

bool V4l2StreamSource::UninitDevice() {
  switch (options_.io_method) {
    case StreamIoMethod::IO_METHOD_MMAP:
      if (buffers_->munmap_bufs(&fd_) != 0) {
        AERROR_F("cannot unmap buffers for device {}: code {}, string [{}]",
                 options_.resource.location, errno, strerror(errno));
        return false;
      }
      break;

    case StreamIoMethod::IO_METHOD_READ:
    case StreamIoMethod::IO_METHOD_USERPTR:
      AERROR_F("unimplemented i/o method: {}",
               StreamIoMethodToStr(options_.io_method));
      return false;
      break;

    case StreamIoMethod::IO_METHOD_UNKNOWN:
    default:
      AERROR_F("unsupported i/o method: {}",
               StreamIoMethodToStr(options_.io_method));
      return false;
      break;
  }
  return true;
}

bool V4l2StreamSource::StartCapturing() {
  // 0. check if capturing is already started
  if (is_capturing_) {
    return true;
  }

  // 1. init buffers
  if (buffers_->queue_all(&fd_) != 0) {
    AERROR_F("cannot queue buffers for device {}: code {}, string [{}]",
             options_.resource.location, errno, strerror(errno));
    return false;
  }

  // 2. start streaming
  if (fd_.streamon() != 0) {
    AERROR_F("cannot start streaming for device {}: code {}, string [{}]",
             options_.resource.location, errno, strerror(errno));
    return false;
  }

  is_capturing_ = true;
  return true;
}

bool V4l2StreamSource::StopCapturing() {
  // 0. check if capturing is already stopped
  if (!is_capturing_) {
    return true;
  }

  is_capturing_ = false;
  switch (options_.io_method) {
    case StreamIoMethod::IO_METHOD_READ:
      // Nothing to do
      break;

    case StreamIoMethod::IO_METHOD_MMAP:
    case StreamIoMethod::IO_METHOD_USERPTR:
      if (fd_.streamoff() != 0) {
        AERROR_F("cannot stop streaming for device {}: code {}, string [{}]",
                 options_.resource.location, errno, strerror(errno));
        return false;
      }
      break;

    default:
    case StreamIoMethod::IO_METHOD_UNKNOWN:
      AERROR_F("Unsupport i/o method: {}",
               StreamIoMethodToStr(options_.io_method));
      return false;
      break;
  }

  return true;
}

bool V4l2StreamSource::ReadFrame(CameraImagePtr raw_image) {
  cv4l_buffer buf(fd_.g_type());
  std::array<void*, VIDEO_MAX_PLANES> mplane_data;
  std::array<unsigned int, VIDEO_MAX_PLANES> mplane_size;

  switch (options_.io_method) {
    case StreamIoMethod::IO_METHOD_MMAP:
      // deque buffer
      if (fd_.dqbuf(buf) != 0) {
        AERROR_F("cannot dequeue buffer for device {}: code {}, string [{}]",
                 options_.resource.location, errno, strerror(errno));
        switch (errno) {
          case EAGAIN:
            return false;
          case EIO:
            /* Could ignore EIO, see spec. */
            /* fall through */
          default:
            Shutdown();
            return false;
        }
      }
      if (buf.g_index() >= n_buffers_) {
        AERROR_F("invalid buffer index: {} for device {}", buf.g_index(),
                 options_.resource.location);
        return false;
      }
      // get timestamp
      raw_image->tv_sec = static_cast<int>(buf.g_timestamp().tv_sec);
      raw_image->tv_usec = static_cast<int>(buf.g_timestamp().tv_usec);
      // get data
      for (unsigned int i = 0; i < buf.g_num_planes(); ++i) {
        mplane_data[i] = buffers_->g_dataptr(buf.g_index(), i);
        mplane_size[i] = buf.g_bytesused(i);
      }
      // process data
      ProcessImage(mplane_data, mplane_size, raw_image);
      // enqueue buffer
      if (fd_.qbuf(buf) != 0) {
        AERROR_F("cannot enqueue buffer for device {}: code {}, string [{}]",
                 options_.resource.location, errno, strerror(errno));
        return false;
      }
      break;

    case StreamIoMethod::IO_METHOD_READ:
    case StreamIoMethod::IO_METHOD_USERPTR:
      AERROR_F("unimplemented i/o method: {}",
               StreamIoMethodToStr(options_.io_method));
      return false;
      break;

    case StreamIoMethod::IO_METHOD_UNKNOWN:
    default:
      AERROR_F("unsupported i/o method: {}",
               StreamIoMethodToStr(options_.io_method));
      return false;
      break;
  }

  return true;
}

bool V4l2StreamSource::ProcessImage(
    std::array<void*, VIDEO_MAX_PLANES> mplane_data,
    std::array<unsigned int, VIDEO_MAX_PLANES> mplane_size,
    CameraImagePtr dest) {
  // 0. check validiy of image pointers
  if (mplane_data[0] == nullptr || dest == nullptr) {
    AERROR_F("process image error. src or dest is null");
    return false;
  }

  // 1. do conversion
  if (buffers_->g_num_planes() == 1) {
    auto src = mplane_data[0];
    auto len = mplane_size[0];
    if (options_.output_format == StreamPixelFormat::PIXEL_FORMAT_RGB) {
      // 1.1. convert to RGB
      if (pixel_format_ == V4L2_PIX_FMT_YUYV) {
        if (monochrome_) {
          // 1.1.1. convert Y16 to RGB
          // actually format V4L2_PIX_FMT_Y16, but xioctl gets
          // unhappy if you don't use the advertised type (yuyv)
          mono102mono8((char*)src, dest->image, dest->width * dest->height);
        } else {
          // 1.1.2. convert YUYV to RGB
#if 0
        yuyv2rgb((char*)src, dest->image, dest->width * dest->height);
#else
#ifdef WITH_AVX
          yuyv2rgb_avx((unsigned char*)src, (unsigned char*)dest->image,
                       dest->width * dest->height);
#else
          convert_yuv_to_rgb_buffer((unsigned char*)src,
                                    (unsigned char*)dest->image, dest->width,
                                    dest->height);
#endif
#endif
        }
      } else if (pixel_format_ == V4L2_PIX_FMT_UYVY) {
// 1.1.3. convert UYUV to RGB
#if 0
      uyvy2rgb((char*)src, dest->image, dest->width * dest->height);
#else
        uyvy2yuyv((char*)src, len);
#ifdef WITH_AVX
        yuyv2rgb_avx((unsigned char*)src, (unsigned char*)dest->image,
                     dest->width * dest->height);
#else
        convert_yuv_to_rgb_buffer((unsigned char*)src,
                                  (unsigned char*)dest->image, dest->width,
                                  dest->height);
#endif
#endif
      } else if (pixel_format_ == V4L2_PIX_FMT_MJPEG) {
        // 1.1.4. convert MJPEG to RGB
        mjpeg_decoder_.ToRGB((char*)src, len, dest->image,
                             dest->width * dest->height);
      } else if (pixel_format_ == V4L2_PIX_FMT_RGB24) {
        // 1.1.5. convert RGB to RGB
        rgb242rgb((char*)src, dest->image, dest->width * dest->height);
      } else if (pixel_format_ == V4L2_PIX_FMT_BGR24) {
        // 1.1.6. convert BGR to RGB
        bgr242rgb((char*)src, dest->image, dest->width * dest->height);
      } else if (pixel_format_ == V4L2_PIX_FMT_NV12) {
        // 1.1.7. convert NV12 to RGB
#if 0
        yuyv2rgb((char*)src, dest->image, dest->width * dest->height);
#else
#ifdef WITH_AVX
        yuyv2rgb_avx((unsigned char*)src, (unsigned char*)dest->image,
                     dest->width * dest->height);
#else
        convert_yuv_to_rgb_buffer((unsigned char*)src,
                                  (unsigned char*)dest->image, dest->width,
                                  dest->height);
#endif
#endif
      } else if (pixel_format_ == V4L2_PIX_FMT_GREY) {
        // 1.1.8. convert GRAY to RGB
        memcpy(dest->image, (char*)src, dest->width * dest->height);
      } else {
        AERROR_F("Unsupported pixel format: {}", pixfmt2s(pixel_format_));
        return false;
      }
    } else if (options_.output_format == StreamPixelFormat::PIXEL_FORMAT_YUYV) {
      // 1.2. convert to YUYV
      if (pixel_format_ == V4L2_PIX_FMT_YUYV && !monochrome_) {
        // 1.2.1. convert YUYV to YUYV
        memcpy(dest->image, src, dest->width * dest->height * 2);
      } else if (pixel_format_ == V4L2_PIX_FMT_UYVY) {
        // 1.2.2. convert UYUV to YUYV
        uyvy2yuyv((char*)src, len);
        memcpy(dest->image, src, dest->width * dest->height * 2);
      } else {
        AERROR_F("Unsupported pixel format: {}", pixfmt2s(pixel_format_));
        return false;
      }
    } else {
      AERROR << "unsupported output format:"
             << StreamPixelFormatToStr(options_.output_format);
      return false;
    }
  } else {
    AERROR_F("unimplemented proceessing function for plane number: {} ",
             buffers_->g_num_planes());
    return false;
  }

  return true;
}

}  // namespace stream
}  // namespace zetton
