#include "zetton_stream/source/v4l2_stream_source.h"

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "zetton_common/log/log.h"
#include "zetton_common/time/time.h"
#include "zetton_common/util/perf.h"
#include "zetton_stream/base/stream_options.h"
#include "zetton_stream/interface/base_stream_processor.h"
#include "zetton_stream/util/pixel_format.h"
#include "zetton_stream/util/v4l2.h"

// #define __STDC_CONSTANT_MACROS

namespace zetton {
namespace stream {

V4l2StreamSource::V4l2StreamSource()
    : fd_(-1),
      buffers_(nullptr),
      n_buffers_(0),
      is_capturing_(false),
      image_seq_(0),
      device_wait_sec_(2) {}

V4l2StreamSource::~V4l2StreamSource() { shutdown(); }

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
  } else if (options_.pixel_format == StreamPixelFormat::PIXEL_FORMAT_GRAY8) {
    pixel_format_ = V4L2_PIX_FMT_GREY;
    monochrome_ = true;
  } else {
    AERROR_F("Unsupported pixel format: {}",
             StreamPixelFormatToStr(options_.pixel_format));
    return false;
  }

  // Warning when diff with last > 1.5* interval
  frame_warning_interval_ = static_cast<float>(1.5 / options_.frame_rate);
  // now max fps 30, we use an appox time 0.9 to drop image.
  frame_drop_interval_ = static_cast<float>(0.9 / options_.frame_rate);

  return true;
}

bool V4l2StreamSource::Capture(const CameraImagePtr& raw_image) {
  raw_image->is_new = 0;
  // free memory in this struct desturctor
  memset(raw_image->image, 0, raw_image->image_size * sizeof(char));

  fd_set fds;
  struct timeval tv;
  int r = 0;

  FD_ZERO(&fds);
  FD_SET(fd_, &fds);

  /* Timeout. */
  tv.tv_sec = 2;
  tv.tv_usec = 0;

  r = select(fd_ + 1, &fds, nullptr, nullptr, &tv);

  if (-1 == r) {
    if (EINTR == errno) {
      return false;
    }

    // errno_exit("select");
    reconnect();
  }

  if (0 == r) {
    AERROR_F("select timeout: code {} msg {}", errno, strerror(errno));
    reconnect();
  }

  int get_new_image = read_frame(raw_image);

  if (!get_new_image) {
    return false;
  }

  raw_image->is_new = 1;
  return true;
}

bool V4l2StreamSource::open_device() {
  struct stat st;

  if (-1 == stat(options_.resource.location.c_str(), &st)) {
    AERROR_F("Cannot identify '{}': {}, {}", options_.resource.location, errno,
             strerror(errno));
    return false;
  }

  if (!S_ISCHR(st.st_mode)) {
    AERROR_F("{} is no device", options_.resource.location);
    return false;
  }

  fd_ = open(options_.resource.location.c_str(),
             O_RDWR /* required */ | O_NONBLOCK, 0);

  if (-1 == fd_) {
    AERROR_F("Cannot open '{}': {}, {}", options_.resource.location, errno,
             strerror(errno));
    return false;
  }

  AINFO_F("open device {} success", options_.resource.location);

  return true;
}

bool V4l2StreamSource::init_device() {
  struct v4l2_capability cap;
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;
  struct v4l2_format fmt;
  unsigned int min = 0;

  if (-1 == xioctl(fd_, VIDIOC_QUERYCAP, &cap)) {
    if (EINVAL == errno) {
      AERROR_F("{} is no V4L2 device", options_.resource.location);
      return false;
    }
    AERROR << "VIDIOC_QUERYCAP";
    return false;
  }

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    AERROR << options_.resource.location << " is no video capture device";
    return false;
  }

  switch (options_.io_method) {
    case StreamIoMethod::IO_METHOD_READ:
      if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
        AERROR << options_.resource.location << " does not support read i/o";
        return false;
      }

      break;

    case StreamIoMethod::IO_METHOD_MMAP:
    case StreamIoMethod::IO_METHOD_USERPTR:
      if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        AERROR << options_.resource.location
               << " does not support streaming i/o";
        return false;
      }

      break;
    case StreamIoMethod::IO_METHOD_UNKNOWN:
      AERROR << options_.resource.location << " does not support unknown i/o";
      return false;
      break;
  }

  /* Select video input, video standard and tune here. */
  CLEAR(cropcap);

  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (0 == xioctl(fd_, VIDIOC_CROPCAP, &cropcap)) {
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; /* reset to default */

    if (-1 == xioctl(fd_, VIDIOC_S_CROP, &crop)) {
      switch (errno) {
        case EINVAL:
          /* Cropping not supported. */
          break;
        default:
          /* Errors ignored. */
          break;
      }
    }
  } else {
    /* Errors ignored. */
  }

  CLEAR(fmt);

  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = options_.width;
  fmt.fmt.pix.height = options_.height;
  fmt.fmt.pix.pixelformat = pixel_format_;
  fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

  if (-1 == xioctl(fd_, VIDIOC_S_FMT, &fmt)) {
    AERROR << "VIDIOC_S_FMT: " << errno << " " << strerror(errno);
    return false;
  }

  /* Note VIDIOC_S_FMT may change width and height. */

  /* Buggy driver paranoia. */
  min = fmt.fmt.pix.width * 2;

  if (fmt.fmt.pix.bytesperline < min) {
    fmt.fmt.pix.bytesperline = min;
  }

  min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;

  if (fmt.fmt.pix.sizeimage < min) {
    fmt.fmt.pix.sizeimage = min;
  }

  options_.width = fmt.fmt.pix.width;
  options_.height = fmt.fmt.pix.height;

  struct v4l2_streamparm stream_params;
  memset(&stream_params, 0, sizeof(stream_params));
  stream_params.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (xioctl(fd_, VIDIOC_G_PARM, &stream_params) < 0) {
    // errno_exit("Couldn't query v4l fps!");
    AERROR << "Couldn't query v4l fps!";
    reconnect();
    return false;
  }

  ADEBUG_F("Capability flag: 0x{:x}", stream_params.parm.capture.capability);

  stream_params.parm.capture.timeperframe.numerator = 1;
  stream_params.parm.capture.timeperframe.denominator = options_.frame_rate;

  if (xioctl(fd_, VIDIOC_S_PARM, &stream_params) < 0) {
    AWARN_F("Couldn't set camera framerate");
  } else {
    AINFO << "Set framerate to be " << options_.frame_rate;
  }

  switch (options_.io_method) {
    case StreamIoMethod::IO_METHOD_READ:
      init_read(fmt.fmt.pix.sizeimage);
      break;

    case StreamIoMethod::IO_METHOD_MMAP:
      init_mmap();
      break;

    case StreamIoMethod::IO_METHOD_USERPTR:
      init_userp(fmt.fmt.pix.sizeimage);
      break;

    case StreamIoMethod::IO_METHOD_UNKNOWN:
      AERROR << " does not support unknown i/o";
      break;
  }

  return true;
}

bool V4l2StreamSource::init_read(unsigned int buffer_size) {
  buffers_ = reinterpret_cast<CameraBuffer*>(calloc(1, sizeof(*buffers_)));

  if (!buffers_) {
    AERROR << "Out of memory";
    // exit(EXIT_FAILURE);
    // reconnect();
    return false;
  }

  buffers_[0].length = buffer_size;
  buffers_[0].start = malloc(buffer_size);

  if (!buffers_[0].start) {
    AERROR << "Out of memory";
    return false;
  }

  return true;
}

bool V4l2StreamSource::init_mmap() {
  struct v4l2_requestbuffers req;
  CLEAR(req);

  req.count = 4;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if (-1 == xioctl(fd_, VIDIOC_REQBUFS, &req)) {
    if (EINVAL == errno) {
      AERROR << options_.resource.location
             << " does not support memory mapping";
      return false;
    }
    AERROR_F("VIDIOC_REQBUFS");
    reconnect();
    return false;
  }

  if (req.count < 2) {
    AERROR_F("Insufficient buffer memory on {}", options_.resource.location);
    exit(EXIT_FAILURE);
  }

  buffers_ =
      reinterpret_cast<CameraBuffer*>(calloc(req.count, sizeof(*buffers_)));

  if (!buffers_) {
    AERROR << "Out of memory";
    return false;
  }

  for (n_buffers_ = 0; n_buffers_ < req.count; ++n_buffers_) {
    struct v4l2_buffer buf;
    CLEAR(buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = n_buffers_;

    if (-1 == xioctl(fd_, VIDIOC_QUERYBUF, &buf)) {
      AERROR << "VIDIOC_QUERYBUF";
      return false;
    }

    buffers_[n_buffers_].length = buf.length;
    buffers_[n_buffers_].start =
        mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_,
             buf.m.offset);

    if (MAP_FAILED == buffers_[n_buffers_].start) {
      AERROR << "mmap";
      return false;
    }
  }

  return true;
}

bool V4l2StreamSource::init_userp(unsigned int buffer_size) {
  struct v4l2_requestbuffers req;
  unsigned int page_size = 0;

  page_size = getpagesize();
  buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

  CLEAR(req);

  req.count = 4;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_USERPTR;

  if (-1 == xioctl(fd_, VIDIOC_REQBUFS, &req)) {
    if (EINVAL == errno) {
      AERROR << options_.resource.location
             << " does not support "
                "user pointer i/o";
      return false;
    }
    AERROR << "VIDIOC_REQBUFS";
    return false;
  }

  buffers_ = reinterpret_cast<CameraBuffer*>(calloc(4, sizeof(*buffers_)));

  if (!buffers_) {
    AERROR << "Out of memory";
    return false;
  }

  for (n_buffers_ = 0; n_buffers_ < 4; ++n_buffers_) {
    buffers_[n_buffers_].length = buffer_size;
    buffers_[n_buffers_].start =
        memalign(/* boundary */ page_size, buffer_size);

    if (!buffers_[n_buffers_].start) {
      AERROR << "Out of memory";
      return false;
    }
  }

  return true;
}

bool V4l2StreamSource::start_capturing() {
  if (is_capturing_) {
    return true;
  }

  unsigned int i = 0;
  enum v4l2_buf_type type;

  switch (options_.io_method) {
    case StreamIoMethod::IO_METHOD_READ:
      /* Nothing to do. */
      break;

    case StreamIoMethod::IO_METHOD_MMAP:
      for (i = 0; i < n_buffers_; ++i) {
        struct v4l2_buffer buf;
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (-1 == xioctl(fd_, VIDIOC_QBUF, &buf)) {
          AERROR << "VIDIOC_QBUF";
          return false;
        }
      }

      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

      if (-1 == xioctl(fd_, VIDIOC_STREAMON, &type)) {
        AERROR << "VIDIOC_STREAMON";
        return false;
      }

      break;

    case StreamIoMethod::IO_METHOD_USERPTR:
      for (i = 0; i < n_buffers_; ++i) {
        struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;
        buf.index = i;
        buf.m.userptr = reinterpret_cast<uint64_t>(buffers_[i].start);
        buf.length = static_cast<unsigned int>(buffers_[i].length);

        if (-1 == xioctl(fd_, VIDIOC_QBUF, &buf)) {
          AERROR << "VIDIOC_QBUF";
          return false;
        }
      }

      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

      if (-1 == xioctl(fd_, VIDIOC_STREAMON, &type)) {
        AERROR << "VIDIOC_STREAMON";
        return false;
      }

      break;

    case StreamIoMethod::IO_METHOD_UNKNOWN:
      AERROR << "unknown IO";
      return false;
      break;
  }

  is_capturing_ = true;
  return true;
}

void V4l2StreamSource::set_device_config() {
  if (options_.camera.brightness >= 0) {
    set_v4l_parameter("brightness", options_.camera.brightness);
  }

  if (options_.camera.contrast >= 0) {
    set_v4l_parameter("contrast", options_.camera.contrast);
  }

  if (options_.camera.saturation >= 0) {
    set_v4l_parameter("saturation", options_.camera.saturation);
  }

  if (options_.camera.sharpness >= 0) {
    set_v4l_parameter("sharpness", options_.camera.sharpness);
  }

  if (options_.camera.gain >= 0) {
    set_v4l_parameter("gain", options_.camera.gain);
  }

  // check auto white balance
  if (options_.camera.auto_white_balance) {
    set_v4l_parameter("white_balance_temperature_auto", 1);
  } else {
    set_v4l_parameter("white_balance_temperature_auto", 0);
    set_v4l_parameter("white_balance_temperature",
                      options_.camera.white_balance);
  }

  // check auto exposure
  if (!options_.camera.auto_exposure) {
    // turn down exposure control (from max of 3)
    set_v4l_parameter("auto_exposure", 1);
    // change the exposure level
    set_v4l_parameter("exposure_absolute", options_.camera.exposure);
  }

  // check auto focus
  if (options_.camera.auto_focus) {
    set_auto_focus(1);
    set_v4l_parameter("focus_auto", 1);
  } else {
    set_v4l_parameter("focus_auto", 0);
    if (options_.camera.focus >= 0) {
      set_v4l_parameter("focus_absolute", options_.camera.focus);
    }
  }
}

bool V4l2StreamSource::uninit_device() {
  unsigned int i = 0;

  switch (options_.io_method) {
    case StreamIoMethod::IO_METHOD_READ:
      free(buffers_[0].start);
      break;

    case StreamIoMethod::IO_METHOD_MMAP:
      for (i = 0; i < n_buffers_; ++i) {
        if (-1 == munmap(buffers_[i].start, buffers_[i].length)) {
          AERROR << "munmap";
          return false;
        }
      }

      break;

    case StreamIoMethod::IO_METHOD_USERPTR:
      for (i = 0; i < n_buffers_; ++i) {
        free(buffers_[i].start);
      }

      break;

    case StreamIoMethod::IO_METHOD_UNKNOWN:
      AERROR << "unknown IO";
      break;
  }

  return true;
}

bool V4l2StreamSource::close_device() {
  if (-1 == close(fd_)) {
    AERROR << "close";
    return false;
  }

  fd_ = -1;
  return true;
}

bool V4l2StreamSource::stop_capturing() {
  if (!is_capturing_) {
    return true;
  }

  is_capturing_ = false;
  enum v4l2_buf_type type;

  switch (options_.io_method) {
    case StreamIoMethod::IO_METHOD_READ:
      /* Nothing to do. */
      break;

    case StreamIoMethod::IO_METHOD_MMAP:
    case StreamIoMethod::IO_METHOD_USERPTR:
      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

      if (-1 == xioctl(fd_, VIDIOC_STREAMOFF, &type)) {
        AERROR << "VIDIOC_STREAMOFF";
        return false;
      }

      break;
    case StreamIoMethod::IO_METHOD_UNKNOWN:
      AERROR << "unknown IO";
      return false;
      break;
  }

  return true;
}

bool V4l2StreamSource::read_frame(CameraImagePtr raw_image) {
  struct v4l2_buffer buf;
  unsigned int i = 0;
  int len = 0;

  switch (options_.io_method) {
    case StreamIoMethod::IO_METHOD_READ:
      len = static_cast<int>(read(fd_, buffers_[0].start, buffers_[0].length));

      if (len == -1) {
        switch (errno) {
          case EAGAIN:
            AINFO << "EAGAIN";
            return false;
          case EIO:
            /* Could ignore EIO, see spec. */
            /* fall through */
          default:
            AERROR_F("read");
            return false;
        }
      }

      process_image(buffers_[0].start, len, raw_image);

      break;

    case StreamIoMethod::IO_METHOD_MMAP:
      CLEAR(buf);

      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;

      if (-1 == xioctl(fd_, VIDIOC_DQBUF, &buf)) {
        switch (errno) {
          case EAGAIN:
            return false;
          case EIO:
            /* Could ignore EIO, see spec. */
            /* fall through */
          default:
            AERROR << "VIDIOC_DQBUF";
            reconnect();
            return false;
        }
      }

      assert(buf.index < n_buffers_);
      len = buf.bytesused;
      raw_image->tv_sec = static_cast<int>(buf.timestamp.tv_sec);
      raw_image->tv_usec = static_cast<int>(buf.timestamp.tv_usec);

#if 0
      // check the timestamp from buffer
      {
        common::Time image_time(raw_image->tv_sec, 1000 * raw_image->tv_usec);
        uint64_t camera_timestamp = image_time.ToNanosecond();
        if (last_nsec_ == 0) {
          last_nsec_ = camera_timestamp;
        } else {
          double diff =
              static_cast<double>(camera_timestamp - last_nsec_) / 1e9;
          // drop image by frame_rate
          if (diff < frame_drop_interval_) {
            AINFO << "drop image:" << camera_timestamp;
            if (-1 == xioctl(fd_, VIDIOC_QBUF, &buf)) {
              AERROR << "VIDIOC_QBUF ERROR";
            }
            return false;
          }
          if (frame_warning_interval_ < diff) {
            AWARN << "stamp jump.last stamp:" << last_nsec_
                  << " current stamp:" << camera_timestamp;
          }
          last_nsec_ = camera_timestamp;
        }

        double now_s = static_cast<double>(common::Time::Now().ToSecond());
        double image_s = static_cast<double>(camera_timestamp) / 1e9;
        double diff = now_s - image_s;
        if (diff > 0.5 || diff < 0) {
          AWARN_F(
              "camera time diff exception, diff: {:.6f}, now: {:.6f}, image: "
              "{:.6f}; dev: {}",
              diff, now_s, image_s, options_.resource.location);
        }
      }
#endif
      if (len < raw_image->width * raw_image->height &&
          pixel_format_ != V4L2_PIX_FMT_MJPEG) {
        AERROR << "Wrong Buffer Len: " << len
               << ", dev: " << options_.resource.location;
      } else {
        process_image(buffers_[buf.index].start, len, raw_image);
      }

      if (-1 == xioctl(fd_, VIDIOC_QBUF, &buf)) {
        AERROR << "VIDIOC_QBUF";
        return false;
      }

      break;

    case StreamIoMethod::IO_METHOD_USERPTR:
      CLEAR(buf);

      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_USERPTR;

      if (-1 == xioctl(fd_, VIDIOC_DQBUF, &buf)) {
        switch (errno) {
          case EAGAIN:
            return false;
          case EIO:
            /* Could ignore EIO, see spec. */
            /* fall through */
          default:
            AERROR << "VIDIOC_DQBUF";
            return false;
        }
      }

      for (i = 0; i < n_buffers_; ++i) {
        if (buf.m.userptr == reinterpret_cast<uint64_t>(buffers_[i].start) &&
            buf.length == buffers_[i].length) {
          break;
        }
      }

      assert(i < n_buffers_);
      len = buf.bytesused;
      process_image(reinterpret_cast<void*>(buf.m.userptr), len, raw_image);

      if (-1 == xioctl(fd_, VIDIOC_QBUF, &buf)) {
        AERROR << "VIDIOC_QBUF";
        return false;
      }

      break;

    case StreamIoMethod::IO_METHOD_UNKNOWN:
      AERROR << "unknown IO";
      return false;
      break;
  }

  return true;
}

bool V4l2StreamSource::process_image(void* src, int len, CameraImagePtr dest) {
  // 0. check validiy of image pointers
  if (src == nullptr || dest == nullptr) {
    AERROR << "process image error. src or dest is null";
    return false;
  }

  // 1. do conversion
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
    } else if (pixel_format_ == V4L2_PIX_FMT_GREY) {
      // 1.1.6. convert GRAY to RGB
      memcpy(dest->image, (char*)src, dest->width * dest->height);
    } else {
      AERROR << "unsupported pixel format:" << pixel_format_;
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
      AERROR << "unsupported pixel format:" << pixel_format_;
      return false;
    }
  } else {
    AERROR << "unsupported output format:"
           << StreamPixelFormatToStr(options_.output_format);
    return false;
  }

  return true;
}

bool V4l2StreamSource::IsCapturing() { return is_capturing_; }

// enables/disables auto focus
void V4l2StreamSource::set_auto_focus(int value) {
  struct v4l2_queryctrl queryctrl;
  struct v4l2_ext_control control;

  memset(&queryctrl, 0, sizeof(queryctrl));
  queryctrl.id = V4L2_CID_FOCUS_AUTO;

  if (-1 == xioctl(fd_, VIDIOC_QUERYCTRL, &queryctrl)) {
    if (errno != EINVAL) {
      perror("VIDIOC_QUERYCTRL");
      return;
    } else {
      AINFO << "V4L2_CID_FOCUS_AUTO is not supported";
      return;
    }
  } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
    AINFO << "V4L2_CID_FOCUS_AUTO is not supported";
    return;
  } else {
    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_FOCUS_AUTO;
    control.value = value;

    if (-1 == xioctl(fd_, VIDIOC_S_CTRL, &control)) {
      perror("VIDIOC_S_CTRL");
      return;
    }
  }
}

/**
 * Set video device parameter via call to v4l-utils.
 *
 * @param param The name of the parameter to set
 * @param param The value to assign
 */
void V4l2StreamSource::set_v4l_parameter(const std::string& param, int value) {
  set_v4l_parameter(param, std::to_string(value));
}
/**
 * Set video device parameter via call to v4l-utils.
 *
 * @param param The name of the parameter to set
 * @param param The value to assign
 */
void V4l2StreamSource::set_v4l_parameter(const std::string& param,
                                         const std::string& value) {
  // build the command
  std::stringstream ss;
  ss << "v4l2-ctl --device=" << options_.resource.location << " -c " << param
     << "=" << value << " 2>&1";
  std::string cmd = ss.str();

  // capture the output
  std::string output;
  char buffer[256];
  FILE* stream = popen(cmd.c_str(), "r");
  if (stream) {
    while (!feof(stream)) {
      if (fgets(buffer, 256, stream) != nullptr) {
        output.append(buffer);
      }
    }

    pclose(stream);
    // any output should be an error
    if (output.length() > 0) {
      AERROR << output.c_str();
    }
  } else {
    AERROR << "usb_cam_node could not run " << cmd.c_str();
  }
}

bool V4l2StreamSource::WaitForDevice() {
  if (is_capturing_) {
    ADEBUG << "is capturing";
    return true;
  }
  if (!open_device()) {
    AERROR_F("open device failed");
    return false;
  }
  if (!init_device()) {
    AERROR_F("init device failed");
    close_device();
    return false;
  }
  if (!start_capturing()) {
    AERROR_F("start capturing failed");
    uninit_device();
    close_device();
    return false;
  }
  return true;
}

void V4l2StreamSource::reconnect() {
  stop_capturing();
  uninit_device();
  close_device();
}

void V4l2StreamSource::shutdown() {
  stop_capturing();
  uninit_device();
  close_device();
}

}  // namespace stream
}  // namespace zetton
